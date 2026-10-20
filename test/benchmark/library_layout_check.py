import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
import fcntl
import hashlib
import json
from pathlib import Path
import posixpath
import re
import shutil
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / 'test/benchmark/results/library_layout'


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--jobs', type=int, default=3)
    args = parser.parse_args()
    manifest = json.loads((RESULTS / 'manifest.json').read_text())
    selection = json.loads((RESULTS / 'selection.json').read_text())
    include = re.compile(r'(^\s*#\s*include\s*")([^"\r\n]+)(")', re.M)
    for name, expected in manifest['cpp_without_include_sha256'].items():
        data = include.sub(lambda match: match[1] + '<local>' + match[3], (ROOT / name).read_text())
        for old, new in manifest['moves'].items():
            data = data.replace(old, new)
        assert hashlib.sha256(data.encode()).hexdigest() == expected, name
    headers = selection['state']['targets']['headers']
    sources = headers + selection['unit'] + selection['verification']
    hashes = {name: digest(ROOT / name) for name in sources}
    pending, visited, support = list(sources), set(sources), set()
    while pending:
        name = pending.pop()
        for match in include.finditer((ROOT / name).read_text()):
            candidates = [posixpath.normpath(posixpath.join(posixpath.dirname(name), match[2])), match[2]]
            target = next((path for path in candidates if (ROOT / path).is_file()), None)
            assert target is not None, (name, match[2])
            assert (ROOT / target).resolve().is_relative_to(ROOT)
            if target not in visited:
                visited.add(target)
                pending.append(target)
                support.add(target)
    auxiliary = sorted(support)
    auxiliary_hashes = {name: digest(ROOT / name) for name in auxiliary}
    for name in sources:
        for match in include.finditer((ROOT / name).read_text()):
            candidates = [posixpath.normpath(posixpath.join(posixpath.dirname(name), match[2])), match[2]]
            assert any((ROOT / path).is_file() for path in candidates), (name, match[2])
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror']
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), result='running', flags=flags,
                  implementation_body_identity='passed', quoted_includes_resolve='passed',
                  sources=hashes, auxiliary_sources=auxiliary_hashes,
                  compilers={name: subprocess.check_output([name, '--version'], text=True).splitlines()[0]
                                            for name in ['g++', 'clang++']}, checks={})
    record = RESULTS / 'validation.json'
    if record.exists():
        previous = json.loads(record.read_text())
        if previous.get('flags') == flags and previous.get('compilers') == report['compilers']:
            prior_sources = previous.get('sources', {}) | previous.get('auxiliary_sources', {})
            current_sources = hashes | auxiliary_hashes
            def dependencies(name):
                pending, visited = [name], set()
                while pending:
                    path = pending.pop()
                    if path in visited:
                        continue
                    visited.add(path)
                    for match in include.finditer((ROOT / path).read_text()):
                        candidates = [posixpath.normpath(posixpath.join(posixpath.dirname(path), match[2])), match[2]]
                        pending.extend(next(([name] for name in candidates if (ROOT / name).is_file()), []))
                return visited
            report['checks'] = {key: value for key, value in previous['checks'].items() if value['result'] == 'passed'
                                and key.split(':', 2)[2] in current_sources
                                and all(prior_sources.get(name) == current_sources[name]
                                        for name in dependencies(key.split(':', 2)[2]))}
    report['reused_checks'] = len(report['checks'])

    def save():
        temporary = record.with_suffix('.tmp')
        for attempt in range(5):
            try:
                temporary.write_text(json.dumps(report, indent=2) + '\n')
                temporary.replace(record)
                return
            except OSError:
                if attempt == 4:
                    raise
                time.sleep(0.05 * (attempt + 1))

    with open('/tmp/cp-wm-opt.lock', 'w') as lock, tempfile.TemporaryDirectory(prefix='cp-layout-') as directory:
        fcntl.flock(lock, fcntl.LOCK_EX)
        work = Path(directory)
        tree = work / 'source'
        for name in sources + auxiliary:
            target = tree / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, target)
        jobs = []
        for cxx in ['g++', 'clang++']:
            jobs += [(cxx, 'header', name) for name in selection['headers']]
            jobs += [(cxx, 'unit', name) for name in selection['unit']]
        jobs += [('g++', 'driver', name) for name in selection['verification']]
        jobs = [job for job in jobs if ':'.join(job) not in report['checks']]
        report['executed_checks'] = len(jobs)

        def check(job):
            cxx, kind, name = job
            key = cxx + ':' + kind + ':' + name
            if key in report['checks']:
                return key, report['checks'][key]
            unique = hashlib.sha256(key.encode()).hexdigest()[:16]
            source = tree / name
            binary = work / unique
            if kind == 'header':
                source = work / (unique + '.cpp')
                source.write_text(f'#include "{name}"\n#include "{name}"\nint main() {{}}\n')
            command = [cxx, *flags, '-I', str(tree), str(source)]
            command += ['-o', str(binary)] if kind == 'unit' else ['-fsyntax-only']
            compiled = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            row = dict(result='passed' if compiled.returncode == 0 else 'failed')
            if compiled.returncode:
                row['output'] = compiled.stdout
            elif kind == 'unit':
                try:
                    executed = subprocess.run([binary], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=180)
                    if executed.returncode:
                        row = dict(result='failed', output=executed.stdout, returncode=executed.returncode)
                except subprocess.TimeoutExpired:
                    row = dict(result='failed', output='unit timeout')
            return key, row

        save()
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            futures = [pool.submit(check, job) for job in jobs]
            for future in as_completed(futures):
                key, value = future.result()
                report['checks'][key] = value
                save()
                print(key, value['result'], flush=True)
        assert all(digest(ROOT / name) == expected for name, expected in hashes.items())
        assert all(digest(ROOT / name) == expected for name, expected in auxiliary_hashes.items())
        report['sources_unchanged'] = True
        report['completed_at'] = datetime.now(timezone.utc).isoformat()
        report['result'] = 'passed' if all(row['result'] == 'passed' for row in report['checks'].values()) else 'failed'
        save()
        assert report['result'] == 'passed'


if __name__ == '__main__':
    main()
