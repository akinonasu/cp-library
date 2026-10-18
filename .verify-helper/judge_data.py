import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
from urllib.parse import urlparse

from ci import affected_inventory, read_json, write_json


def urls_for(root, plan):
    result = {}
    for name in plan['verification']:
        source = (root / name).read_text()
        match = re.search(r'^\s*#\s*define\s+PROBLEM\s+"([^"\n]+)"\s*$', source, re.M)
        if not match:
            from onlinejudge_verify.languages import list as languages
            url = languages.get(Path(name)).list_attributes(Path(name), basedir=root)['PROBLEM']
        else:
            url = match[1]
        result.setdefault(url, []).append(name)
    return result


def repository():
    from onlinejudge.service.library_checker import LibraryCheckerService
    return LibraryCheckerService._get_cloned_repository_path()


def source_entries(path):
    entries = {}
    for line in subprocess.check_output(['git', '-C', str(path), 'ls-tree', '-rz', 'HEAD']).split(b'\0'):
        if line:
            metadata, name = line.split(b'\t', 1)
            entries[name.decode()] = metadata.decode().split()[-1]
    return entries


def problem_fingerprint(entries, directory):
    sources = [(name, oid) for name, oid in sorted(entries.items())
               if name.startswith(directory + '/') or name.startswith('common/') or '/' not in name]
    return hashlib.sha256(json.dumps(sources).encode()).hexdigest()


def build_manifest(root, plan, repo, toolchain):
    if affected_inventory(plan):
        raise ValueError('Explicit no-cache verification cannot restore data caches')
    urls = urls_for(root, plan)
    entries = source_entries(repo) if any(urlparse(url).netloc == 'judge.yosupo.jp' for url in urls) else {}
    requirements = (root / 'test/requirements.txt').read_bytes()
    result = []
    for url, drivers in sorted(urls.items()):
        identifier = hashlib.md5(url.encode()).hexdigest()
        cases = root / '.verify-helper/cache' / identifier / 'test'
        item = {'url': url, 'drivers': drivers, 'cases': str(cases), 'paths': [str(cases)]}
        source = ''
        if urlparse(url).netloc == 'judge.yosupo.jp':
            problem = urlparse(url).path.removeprefix('/problem/')
            if not re.fullmatch(r'\w+', problem):
                raise ValueError('Invalid Library Checker problem')
            matches = [name.rsplit('/', 1)[0] for name in entries if name.endswith('/' + problem + '/info.toml')]
            if len(matches) != 1:
                raise ValueError('Missing or ambiguous Library Checker problem: ' + problem)
            directory = matches[0]
            source = problem_fingerprint(entries, directory)
            item['problem'] = str(repo / directory)
            item['paths'].append(item['problem'])
        signature = hashlib.sha256(requirements + toolchain.encode() + source.encode()).hexdigest()
        item['key'] = f'judge-data-v2-{platform.system()}-{platform.machine()}-{identifier}-{signature}'
        result.append(item)
    return {'repository': str(repo) if entries else None, 'entries': result}


def normalize_sources(repo, problem):
    paths = ['generate.py', 'common', problem.relative_to(repo).as_posix()]
    subprocess.run(['git', '-C', str(repo), 'diff', '--exit-code', '--quiet', '--', *paths], check=True)
    names = subprocess.check_output(['git', '-C', str(repo), 'ls-files', '-z', '--', *paths])
    for name in names.split(b'\0'):
        if name:
            os.utime(repo / name.decode(), ns=(0, 0))


def prepare_selected(root, plan, manifest_path, drivers=None):
    manifest = read_json(manifest_path)
    if manifest is None or {url: sorted(names) for url, names in urls_for(root, plan).items()} != {
            item['url']: sorted(item['drivers']) for item in manifest['entries']}:
        raise ValueError('Judge data manifest differs from selected tests')
    repo = Path(manifest['repository']) if manifest['repository'] else None
    for item in manifest['entries']:
        if 'problem' not in item or item.get('ready') or (drivers is not None and not set(drivers).intersection(item['drivers'])):
            continue
        problem = Path(item['problem'])
        normalize_sources(repo, problem)
        subprocess.run([sys.executable, str(repo / 'generate.py'), str(problem / 'info.toml'), '--compile-checker'], check=True)
        cases = Path(item['cases'])
        if cases.exists():
            shutil.rmtree(cases)
        cases.mkdir(parents=True)
        for source in sorted([*(problem / 'in').glob('*.in'), *(problem / 'out').glob('*.out')]):
            try:
                os.link(source, cases / source.name)
            except OSError:
                shutil.copy2(source, cases / source.name)
        if not list(cases.glob('*.in')):
            raise ValueError('No generated cases for ' + item['url'])
        item['ready'] = True
        write_json(manifest_path, manifest)
    if repo:
        from onlinejudge.service.library_checker import LibraryCheckerService
        LibraryCheckerService.is_repository_updated = True
    write_json(manifest_path, manifest)


def mark_ready(manifest_path, name):
    manifest = read_json(manifest_path)
    for item in manifest['entries']:
        if name in item['drivers']:
            item['ready'] = True
    write_json(manifest_path, manifest)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--selection', type=Path, default=Path('.verify-helper/selection.json'))
    parser.add_argument('--output', type=Path, default=Path('.verify-helper/judge-cache.json'))
    args = parser.parse_args()
    root = Path.cwd()
    plan = read_json(args.selection)
    urls = urls_for(root, plan)
    if affected_inventory(plan):
        raise ValueError('No-cache scope cannot use judge data caches')
    has_library_checker = any(urlparse(url).netloc == 'judge.yosupo.jp' for url in urls)
    repo = repository() if has_library_checker else Path('/unused')
    if has_library_checker:
        if repo.exists():
            subprocess.run(['git', '-C', str(repo), 'pull', '--ff-only'], check=True)
        else:
            repo.parent.mkdir(parents=True, exist_ok=True)
            subprocess.run(['git', 'clone', '--depth', '1', 'https://github.com/yosupo06/library-checker-problems', str(repo)], check=True)
    compiler = os.environ.get('CXX', 'g++')
    toolchain = subprocess.check_output([compiler, '--version'], text=True) + sys.version if urls else ''
    overrides = {key: os.environ[key] for key in ('CXX', 'CXXFLAGS') if key in os.environ}
    if overrides:
        toolchain += json.dumps(overrides, sort_keys=True)
    manifest = build_manifest(root, plan, repo, toolchain)
    write_json(args.output, manifest)
    if os.environ.get('GITHUB_OUTPUT'):
        with open(os.environ['GITHUB_OUTPUT'], 'a') as stream:
            stream.write('needed=' + str(bool(urls)).lower() + '\n')
    if os.environ.get('GITHUB_ENV'):
        with open(os.environ['GITHUB_ENV'], 'a') as stream:
            stream.write('VERIFY_JUDGE_CACHE=' + str(args.output.resolve()) + '\n')
    print(f'Judge data caches selected: {len(manifest["entries"])}', flush=True)


if __name__ == '__main__':
    main()
