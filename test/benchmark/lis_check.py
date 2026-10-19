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
    output = root / 'test/benchmark/results/lis'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'clang++', '--sanitize',
             '--selection', str(output / 'selection.json')], stdout=stream, stderr=subprocess.STDOUT)
    print('ASan/UBSan passed', flush=True)
    url = 'https://judge.yosupo.jp/problem/longest_increasing_subsequence'
    cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
    if not list(cases.glob('*.in')):
        with (output / 'download.log').open('w') as stream:
            run(['oj', 'download', '--system', '--silent', '--directory', str(cases), url],
                stdout=stream, stderr=subprocess.STDOUT)
    inputs = sorted(cases.glob('*.in'))
    assert inputs, 'No official cases found'
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    for path in inputs:
        result['cases'][path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    with tempfile.TemporaryDirectory(prefix='lis-check-') as temporary:
        temporary = Path(temporary)
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = temporary / compiler
            run([*flags, 'test/lis.test.cpp', '-o', str(executable)])
            for path in inputs:
                data = list(map(int, path.read_bytes().split()))
                n, a = data[0], data[1:]
                assert len(a) == n
                with path.open('rb') as stream:
                    actual = list(map(int, subprocess.check_output([str(executable)], stdin=stream, timeout=60).split()))
                expected_length = int(path.with_suffix('.out').read_bytes().split()[0])
                assert actual and actual[0] == expected_length and len(actual) == expected_length + 1, (compiler, path)
                indices = actual[1:]
                assert all(0 <= p < n for p in indices), (compiler, path)
                assert all(p < q and a[p] < a[q] for p, q in zip(indices, indices[1:])), (compiler, path)
            result['official'].append({'compiler': compiler, 'cases': len(inputs), 'length_and_subsequence': 'passed'})
            for header, document in [('components/binary_search.hpp', 'docs/binary_search.md'), ('string/lis.hpp', 'docs/lis.md')]:
                text = (root / document).read_text()
                assert yaml.safe_load(text.split('---', 2)[1])['documentation_of'] == '//' + header
                assert '`' + header + '`' in (root / 'README.md').read_text()
                code = subprocess.check_output(['oj-bundle', header], cwd=root, text=True, stderr=subprocess.DEVNULL)
                assert '#include "' not in code
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
                examples = re.findall(r'```cpp\n(.*?)\n```', text, re.S)
                for example in examples:
                    lines = example.replace('"lib/', '"').splitlines()
                    declarations = '\n'.join(line for line in lines if line.startswith('#include'))
                    body = '\n'.join(line for line in lines if not line.startswith('#include'))
                    source = declarations + '\nint main() {\n' + body + '\n}\n'
                    run([*flags, '-Wno-unused-variable', '-Wno-unused-but-set-variable', '-fsyntax-only', '-x', 'c++', '-'], input=source, text=True)
                result['docs'].append({'compiler': compiler, 'header': header, 'bundle': 'passed', 'examples': len(examples)})
            print(compiler, len(inputs), 'official cases, bundles and examples passed', flush=True)
    names = ['components/binary_search.hpp', 'string/lis.hpp', 'test/unit/binary_search.cpp', 'test/unit/lis.cpp',
             'test/unit/standalone.cpp', 'test/lis.test.cpp', 'docs/binary_search.md', 'docs/lis.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('All LIS checks passed', flush=True)


if __name__ == '__main__':
    main()
