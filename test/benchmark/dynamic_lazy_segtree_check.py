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
    output = root / 'test/benchmark/results/dynamic_lazy_segtree'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'clang++', '--sanitize',
             '--selection', str(output / 'selection.json')], stdout=stream, stderr=subprocess.STDOUT)
    print('ASan/UBSan passed', flush=True)
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    with tempfile.TemporaryDirectory(prefix='dynamic-lazy-check-') as temporary:
        temporary = Path(temporary)
        for name, source in [('range_affine_range_sum', 'test/dynamic_lazy_segtree.test.cpp'),
                             ('range_affine_range_sum_large_array', 'test/dynamic_lazy_segtree_large.test.cpp')]:
            url = 'https://judge.yosupo.jp/problem/' + name
            cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
            if not list(cases.glob('*.in')):
                with (output / (name + '_download.log')).open('w') as stream:
                    run(['oj', 'download', '--system', '--silent', '--directory', str(cases), url],
                        stdout=stream, stderr=subprocess.STDOUT)
            inputs = sorted(cases.glob('*.in'))
            assert inputs, 'No official cases found'
            result['cases'][name] = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
            for compiler in ['g++', 'clang++']:
                executable = temporary / (name + compiler)
                run([compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root), source, '-o', str(executable)])
                for path in inputs:
                    with path.open('rb') as stream:
                        actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60)
                    assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
                result['official'].append({'compiler': compiler, 'problem': name, 'cases': len(inputs)})
                print(compiler, name, len(inputs), 'official cases passed', flush=True)
        header = 'data_structure/segment_tree/dynamic_lazy_segtree.hpp'
        text = (root / 'docs/dynamic_lazy_segtree.md').read_text()
        assert yaml.safe_load(text.split('---', 2)[1])['documentation_of'] == '//' + header
        assert '`' + header + '`' in (root / 'README.md').read_text()
        code = subprocess.check_output(['oj-bundle', header], cwd=root, text=True, stderr=subprocess.DEVNULL)
        assert '#include "' not in code
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
            examples = re.findall(r'```cpp\n(.*?)\n```', text, re.S)
            for example in examples:
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=example.replace('"lib/', '"'), text=True)
            result['docs'].append({'compiler': compiler, 'bundle': 'passed', 'examples': len(examples)})
    names = ['data_structure/segment_tree/dynamic_lazy_segtree.hpp', 'test/unit/dynamic_lazy_segtree.cpp',
             'test/unit/standalone.cpp', 'test/dynamic_lazy_segtree.test.cpp', 'test/dynamic_lazy_segtree_large.test.cpp',
             'docs/dynamic_lazy_segtree.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Official cases, bundle and examples passed', flush=True)


if __name__ == '__main__':
    main()
