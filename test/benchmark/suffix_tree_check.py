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
    output = root / 'test/benchmark/results/suffix_tree'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'clang++', '--sanitize',
             '--selection', str(output / 'selection.json')], stdout=stream, stderr=subprocess.STDOUT)
    print('ASan/UBSan passed', flush=True)
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    with tempfile.TemporaryDirectory(prefix='suffix-tree-check-') as temporary:
        temporary = Path(temporary)
        for problem, source in [('suffixarray', 'test/suffix_tree_array.test.cpp'),
                                ('number_of_substrings', 'test/suffix_tree_substrings.test.cpp')]:
            url = 'https://judge.yosupo.jp/problem/' + problem
            cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
            if not list(cases.glob('*.in')):
                with (output / (problem + '_download.log')).open('w') as stream:
                    run(['oj', 'download', '--system', '--silent', '--directory', str(cases), url],
                        stdout=stream, stderr=subprocess.STDOUT)
            inputs = sorted(cases.glob('*.in'))
            assert inputs, 'No official cases found'
            result['cases'][problem] = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
            for compiler in ['g++', 'clang++']:
                executable = temporary / (problem + compiler)
                run([compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root), source, '-o', str(executable)])
                for path in inputs:
                    with path.open('rb') as stream:
                        actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60)
                    assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
                result['official'].append({'compiler': compiler, 'problem': problem, 'cases': len(inputs)})
                print(compiler, problem, len(inputs), 'official cases passed', flush=True)
        for header, document in [('string/suffix_tree.hpp', 'docs/suffix_tree.md'),
                                 ('components/suffix_array.hpp', 'docs/suffix_array.md')]:
            text = (root / document).read_text()
            assert yaml.safe_load(text.split('---', 2)[1])['documentation_of'] == '//' + header
            assert '`' + header + '`' in (root / 'README.md').read_text()
            code = subprocess.check_output(['oj-bundle', header], cwd=root, text=True, stderr=subprocess.DEVNULL)
            assert '#include "' not in code
            for compiler in ['g++', 'clang++']:
                flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
                examples = re.findall(r'```cpp\n(.*?)\n```', text, re.S)
                for example in examples:
                    executable = temporary / 'example'
                    run([*flags, '-x', 'c++', '-', '-o', str(executable)], input=example.replace('"lib/', '"'), text=True)
                    run([str(executable)])
                result['docs'].append({'compiler': compiler, 'header': header, 'bundle': 'passed', 'executed_examples': len(examples)})
    names = ['string/suffix_tree.hpp', 'components/suffix_array.hpp', 'test/unit/suffix_tree.cpp', 'test/unit/standalone.cpp',
             'test/suffix_tree_array.test.cpp', 'test/suffix_tree_substrings.test.cpp', 'docs/suffix_tree.md', 'docs/suffix_array.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Official cases, bundles and examples passed', flush=True)


if __name__ == '__main__':
    main()
