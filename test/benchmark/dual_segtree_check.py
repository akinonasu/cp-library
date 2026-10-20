import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

import yaml


def main():
    root = Path(__file__).resolve().parents[2]
    output = root / 'test/benchmark/results/dual_segtree'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'clang++', '--sanitize',
             '--selection', str(output / 'selection.json')], stdout=stream, stderr=subprocess.STDOUT)
    print('ASan/UBSan passed', flush=True)
    url = 'https://judge.yosupo.jp/problem/range_affine_point_get'
    cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
    if not list(cases.glob('*.in')):
        with (output / 'download.log').open('w') as stream:
            run(['oj', 'download', '--system', '--silent', '--directory', str(cases), url],
                stdout=stream, stderr=subprocess.STDOUT)
    inputs = sorted(cases.glob('*.in'))
    assert inputs, 'No official cases found'
    documentation = (root / 'docs/dual_segtree.md').read_text()
    assert yaml.safe_load(documentation.split('---', 2)[1])['documentation_of'] == '//data_structure/segment_tree/dual_segtree.hpp'
    assert '`data_structure/segment_tree/dual_segtree.hpp`' in (root / 'README.md').read_text()
    code = subprocess.check_output(['oj-bundle', 'data_structure/segment_tree/dual_segtree.hpp'],
                                   cwd=root, text=True, stderr=subprocess.DEVNULL)
    assert '#include "' not in code
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    for path in inputs:
        result['cases'][path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    with tempfile.TemporaryDirectory(prefix='dual-segtree-check-') as temporary:
        temporary = Path(temporary)
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = temporary / compiler
            run([*flags, 'test/dual_segtree.test.cpp', '-o', str(executable)])
            for path in inputs:
                with path.open('rb') as stream:
                    actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60).split()
                assert actual == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            result['official'].append({'compiler': compiler, 'cases': len(inputs)})
            run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
            examples = re.findall(r'```cpp\n(.*?)\n```', documentation, re.S)
            for example in examples:
                example = example.replace('"lib/', '"')
                declaration, body = example.split('Tree seg(', 1)
                source = declaration + '\nint main() {\nTree seg(' + body + '\n}\n'
                run([*flags, '-Wno-unused-variable', '-Wno-unused-but-set-variable', '-fsyntax-only', '-x', 'c++', '-'], input=source, text=True)
            result['docs'].append({'compiler': compiler, 'bundle': 'passed', 'examples': len(examples)})
            print(compiler, len(inputs), 'official cases, bundle and examples passed', flush=True)
    names = ['data_structure/segment_tree/dual_segtree.hpp', 'test/unit/dual_segtree.cpp', 'test/unit/standalone.cpp',
             'test/dual_segtree.test.cpp', 'docs/dual_segtree.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('All dual segment tree checks passed', flush=True)


if __name__ == '__main__':
    main()
