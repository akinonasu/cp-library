import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

import yaml


ROOT = Path(__file__).resolve().parents[2]
PREFIX = 'docs'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def split_front(text):
    match = re.match(r'\A---\r?\n(.*?)\r?\n---(?:\r?\n|\Z)', text, re.S)
    return (yaml.safe_load(match[1]) or {}, text[match.end():]) if match else ({}, text)


def sources(selection):
    paths = {ROOT / 'README.md', ROOT / 'AGENTS.md', ROOT / 'test/verify.py',
             ROOT / 'test/requirements.txt', selection, Path(__file__).resolve()}
    for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'tree', 'math', 'string']:
        paths.update((ROOT / folder).rglob('*.hpp'))
    paths.update((ROOT / 'docs').rglob('*.md'))
    paths.update((ROOT / 'test').glob('*.test.cpp'))
    paths.update((ROOT / 'test/unit').rglob('*.cpp'))
    paths.update((ROOT / '.verify-helper').glob('*.py'))
    paths.add(ROOT / '.verify-helper/config.toml')
    paths.update(p for p in (ROOT / '.verify-helper/docs').rglob('*') if p.is_file())
    return {str(p.relative_to(ROOT)): sha(p) for p in sorted(paths) if p.is_file()}


def audit(plan):
    headers = set(plan['state']['targets']['headers'])
    generated = ROOT / '.verify-helper/markdown'
    metadata_errors, body_mismatches, link_errors, liquid = [], [], [], []
    targets, docs_hashes, bundle_hashes = {}, {}, {}
    documents = [ROOT / 'README.md', *sorted((ROOT / 'docs').rglob('*.md'))]
    for page in documents:
        relative = str(page.relative_to(ROOT))
        text = page.read_text()
        front, body = split_front(text)
        target = front.get('documentation_of')
        if target is not None:
            if not isinstance(target, str):
                metadata_errors.append(dict(path=relative, reason='invalid documentation_of'))
                continue
            target = target.removeprefix('//').removeprefix('/')
            targets.setdefault(target, []).append(relative)
            if not (ROOT / target).is_file():
                metadata_errors.append(dict(path=relative, reason='missing header', target=target))
            rendered = generated / (target + '.md')
            if not rendered.is_file() or split_front(rendered.read_text())[1] != body:
                body_mismatches.append(relative)
        for match in re.finditer(r'\{\{|\{%\s*(?!raw|endraw)', body):
            liquid.append(dict(path=relative, line=body.count('\n', 0, match.start()) + 1))
        visible = re.sub(r'```[^\n]*\n.*?```|~~~[^\n]*\n.*?~~~|`[^`\n]*`',
                         lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text, flags=re.S)
        for match in re.finditer(r'\[[^\]]*\]\(([^)]+)\)', visible):
            link = match[1].strip().strip('<>').split('#', 1)[0]
            if not link:
                continue
            if link.startswith('https://qandori.github.io/cp-library/') and link.endswith('.hpp.html'):
                local = ROOT / link.removeprefix('https://qandori.github.io/cp-library/').removesuffix('.html')
            elif re.match(r'[a-zA-Z][a-zA-Z0-9+.-]*:', link) or link.startswith('//'):
                continue
            else:
                local = ROOT / link.lstrip('/') if link.startswith('/') else page.parent / link
            if not local.exists():
                link_errors.append(dict(path=relative, target=link))
        docs_hashes[relative] = sha(page)
    for name in sorted(headers):
        if name not in targets:
            metadata_errors.append(dict(header=name, reason='missing documentation'))
        elif len(targets[name]) != 1:
            metadata_errors.append(dict(header=name, reason='duplicate documentation', pages=targets[name]))
        rendered = generated / (name + '.md')
        if rendered.is_file():
            front, body = split_front(rendered.read_text())
            if front.get('title') in (None, name):
                metadata_errors.append(dict(header=name, reason='missing generated title'))
            for match in re.finditer(r'\{\{', body):
                liquid.append(dict(path=str(rendered.relative_to(ROOT)), line=body.count('\n', 0, match.start()) + 1))
            code = front.get('data', {}).get('bundledCode', '')
            if 'Traceback (most recent call last)' in code or re.search(r'^\s*#\s*include\s*"', code, re.M):
                metadata_errors.append(dict(header=name, reason='invalid bundle'))
            bundle_hashes[name] = hashlib.sha256(code.encode()).hexdigest()
    readme = (ROOT / 'README.md').read_text()
    entries = set(re.findall(r'`((?:components|convolution|fps|data_structure|graph|tree|math|string)/[^`]+\.hpp)`', readme))
    previous_readme = subprocess.check_output(['git', 'show', 'HEAD:README.md'], cwd=ROOT, text=True)
    omissions = sorted(headers - entries)
    preexisting_omissions = all(name not in previous_readme for name in omissions)
    new_header = 'components/fps_convolution_constant.hpp'
    index_front, _ = split_front((generated / 'components/index.md').read_text())
    catalog = [page for category in index_front.get('data', {}).get('libraryCategories', [])
               if category.get('name') == 'components' for page in category.get('pages', [])]
    matching = [page for page in catalog if page.get('path') == new_header]
    new_doc = (ROOT / 'docs/fps_convolution_constant.md').read_text()
    complexity = dict(time='O(n+m)' in new_doc, scratch='O(1)' in new_doc,
                      output='O(n+m)' in new_doc,
                      manual_review='Constant inputs give triangular/plateau counts min(k+1,m,n,n+m-1-k); scanning and constructing n+m-1 live output elements take O(n+m), with a fixed eight-lane workspace.')
    catalog_ok = new_header in entries and len(matching) == 1 and matching[0].get('title') != new_header
    success = not any([metadata_errors, body_mismatches, link_errors, liquid]) and preexisting_omissions and catalog_ok and all(complexity[k] for k in ['time', 'scratch', 'output'])
    return dict(success=success, library_count=len(headers), documentation_count=len(targets),
                metadata_errors=metadata_errors, generated_body_mismatches=body_mismatches,
                unresolved_local_links=link_errors, liquid_body_occurrences=liquid,
                readme_omissions=omissions, readme_omissions_preexisting=preexisting_omissions,
                new_component_catalog=matching, new_component_catalog_ok=catalog_ok,
                new_component_complexity=complexity, source_docs_sha256=docs_hashes,
                bundled_code_sha256=bundle_hashes,
                scope='README and source docs only; benchmark reports, remote URLs, anchors and reference-style links are not audited here. All generated source-doc bodies are compared after YAML front matter. No Jekyll rendering test is claimed.')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--selection', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--python', default='/tmp/fps-venv/bin/python')
    args = parser.parse_args()
    os.chdir(ROOT)
    selection, output = args.selection.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / (PREFIX + '_result.json')).exists():
        raise ValueError('Use a new output directory for a new run')
    temporary = Path('/tmp/cp-fps-docs-bin')
    temporary.mkdir(exist_ok=True)
    clang = temporary / 'clang++'
    if not clang.exists():
        clang.symlink_to('/tmp/fps-clang/toolchain/usr/bin/clang++-19')
    environment = dict(os.environ, PATH=str(temporary) + ':' + os.environ['PATH'])
    plan = json.loads(selection.read_text())
    selected = sorted(set(plan.get('bundles', plan['headers'])))
    all_headers = set(plan['state']['targets']['headers'])
    config = yaml.safe_load((ROOT / '.verify-helper/docs/_config.yml').read_text())
    excluded = [Path(name) for name in config.get('exclude', [])]
    expected_sources = {name for name in all_headers | set(plan['state']['targets']['verification'])
                        if not any(Path(name) == path or path in Path(name).parents for path in excluded)}
    commands = [[args.python, '.verify-helper/ci.py', 'docs', '--selection', str(selection.relative_to(ROOT))],
                [args.python, '.verify-helper/check-docs.py', '--selection', str(selection.relative_to(ROOT))]]

    def save(suffix, value):
        (output / (PREFIX + suffix + '.json')).write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n')

    before = sources(selection)
    save('_sources_before', before)
    conditions = dict(started=datetime.datetime.now(datetime.timezone.utc).isoformat(), baseline='bd29f70c70e24e27770b9e9a3777b6cf85664b63',
                      head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
                      selection=str(selection.relative_to(ROOT)), selection_sha256=sha(selection),
                      script_sha256=sha(Path(__file__)), compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                      clang=subprocess.check_output(['clang++', '--version'], env=environment, text=True).splitlines()[0],
                      python=subprocess.check_output([args.python, '--version'], text=True).strip(),
                      verify_helper_version=subprocess.check_output([args.python, '-c', 'from importlib.metadata import version; print(version("online-judge-verify-helper"))'], text=True).strip(),
                      lock='flock -s /tmp/cp-fps-tuning.lock', commands=commands,
                      selected_bundles=selected, total_library_count=len(all_headers),
                      expected_generated_source_count=len(expected_sources),
                      source_scope='All production headers, source docs, README/AGENTS, judge/unit C++, verification scripts/config/templates, docs harness and selection. Mutable benchmark reports/results and generated outputs are excluded.')
    save('_commands', conditions)
    rows, audit_result, error = [], None, None
    actual_generated = metadata_count = bundle_count = None
    warnings = []
    try:
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock, (output / (PREFIX + '.log')).open('w') as log:
            fcntl.flock(lock, fcntl.LOCK_SH)
            for command in commands:
                log.write('COMMAND ' + json.dumps(command) + '\n'); log.flush()
                begin = time.monotonic()
                run = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
                rows.append(dict(command=command, seconds=time.monotonic() - begin, returncode=run.returncode))
                log.write('RESULT ' + json.dumps(rows[-1]) + '\n'); log.flush()
                print(rows[-1], flush=True)
                if run.returncode:
                    raise RuntimeError('Documentation command failed')
            diagnostics = (output / (PREFIX + '.log')).read_text()
            warnings = re.findall(r'^failed to list attributes of ([^:]+):', diagnostics, re.M)
            if any(name != 'test/dijkstra_bigint_yuki.test.cpp' for name in warnings):
                raise RuntimeError('Unexpected source metadata warning')
            if diagnostics.count('Traceback (most recent call last)') != len(warnings):
                raise RuntimeError('Unexpected generator traceback')
            if warnings and 'boost/multiprecision/cpp_int.hpp: No such file or directory' not in diagnostics:
                raise RuntimeError('Allowed metadata warning has a different cause')
            generated = re.search(r'Generated documentation for (\d+) library and judge sources', diagnostics)
            checked = re.search(r'Checked documentation for (\d+) libraries; compiled (\d+) affected bundles', diagnostics)
            actual_generated = int(generated[1]) if generated else None
            if checked:
                metadata_count, bundle_count = map(int, checked.groups())
            if (actual_generated, metadata_count, bundle_count) != (len(expected_sources), len(all_headers), len(selected)):
                raise RuntimeError('Unexpected generated/metadata/bundle count')
            audit_result = audit(plan)
            save('_audit', audit_result)
            if not audit_result['success']:
                raise RuntimeError('Documentation body/link/catalog audit failed')
    except Exception as failure:
        error = str(failure)
    after = sources(selection)
    save('_sources_after', after)
    changed = sorted(name for name in before.keys() | after.keys() if before.get(name) != after.get(name))
    result = dict(started=conditions['started'], finished=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  selection_sha256=conditions['selection_sha256'], commands=rows,
                  source_count=len(before), source_hashes_unchanged=not changed, changed_sources=changed,
                  generated_source_count=actual_generated, checked_library_metadata_count=metadata_count,
                  planned_affected_bundle_count=len(selected), compiled_affected_bundle_count=bundle_count,
                  nonfatal_metadata_warning_sources=warnings, audit_success=audit_result and audit_result['success'],
                  error=error, success=error is None and not changed and audit_result is not None)
    save('_result', result)
    print(json.dumps(result, ensure_ascii=False), flush=True)
    if not result['success']:
        sys.exit(1)


if __name__ == '__main__':
    main()
