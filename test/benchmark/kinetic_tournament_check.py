import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time

import yaml


def main():
    root = Path(__file__).resolve().parents[2]
    output = root / 'test/benchmark/results/kinetic_tournament'
    selection = {'headers': ['data_structure/segment_tree/kinetic_tournament.hpp'],
                 'unit': ['test/unit/kinetic_tournament.cpp', 'test/unit/standalone.cpp'],
                 'sanitize': ['test/unit/kinetic_tournament.cpp']}
    (output / 'selection.json').write_text(json.dumps(selection, indent=2) + '\n')

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    command = [sys.executable, 'test/verify.py', '--unit-only', '--selection', str(output / 'selection.json')]
    for name, flags in [('check.log', ['--cxx', 'g++', 'clang++']), ('sanitize.log', ['--cxx', 'clang++', '--sanitize'])]:
        with (output / name).open('w') as stream:
            run([*command, *flags], stdout=stream, stderr=subprocess.STDOUT)
        print(name, 'passed', flush=True)
    url = 'https://judge.yosupo.jp/problem/line_add_get_min'
    cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
    if not list(cases.glob('*.in')):
        with (output / 'download.log').open('w') as stream:
            run(['oj', 'download', '--system', '--directory', str(cases), url], stdout=stream, stderr=subprocess.STDOUT)
    inputs = sorted(cases.glob('*.in'))
    assert inputs
    source = 'test/kinetic_tournament.test.cpp'
    header = 'data_structure/segment_tree/kinetic_tournament.hpp'
    document = (root / 'docs/kinetic_tournament.md').read_text()
    assert yaml.safe_load(document.split('---', 2)[1])['documentation_of'] == '//' + header
    assert '`' + header + '`' in (root / 'README.md').read_text()
    example = re.search(r'```cpp\n(.*?)\n```', document, re.S)[1].replace('"lib/', '"')
    checks = []
    with tempfile.TemporaryDirectory(prefix='kinetic-check-') as temporary:
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = str(Path(temporary) / 'driver')
            run([*flags, source, '-o', executable])
            durations = {}
            for path in inputs:
                start = time.perf_counter()
                with path.open('rb') as stream:
                    actual = subprocess.check_output([executable], stdin=stream, timeout=30)
                durations[path.name] = time.perf_counter() - start
                assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            for target in [source, header]:
                bundle = subprocess.check_output(['oj-bundle', target], cwd=root, text=True, stderr=subprocess.DEVNULL)
                assert '#include "' not in bundle
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True)
            run([*flags, '-x', 'c++', '-', '-o', executable], input=example, text=True)
            run([executable])
            checks.append({'compiler': compiler, 'official_cases': len(inputs), 'seconds': durations,
                           'bundles': 'passed', 'example': 'compiled and executed'})
            print(compiler, len(inputs), 'official cases, bundles and example passed', flush=True)
    names = [source, header, 'docs/kinetic_tournament.md', 'test/unit/kinetic_tournament.cpp', 'test/unit/standalone.cpp', 'components/fast_io.hpp']
    result = {'result': 'passed', 'checks': checks, 'selection': selection,
              'cases': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
              'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names},
              'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']}}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
