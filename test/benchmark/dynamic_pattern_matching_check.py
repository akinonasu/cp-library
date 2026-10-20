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
    output = root / 'test/benchmark/results/dynamic_pattern_matching'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    for name, extra in [('unit', ['--cxx', 'g++', 'clang++']), ('sanitize', ['--cxx', 'clang++', '--sanitize'])]:
        with (output / (name + '.log')).open('w') as stream:
            run([sys.executable, 'test/verify.py', '--unit-only', '--selection', str(output / 'selection.json'), *extra],
                stdout=stream, stderr=subprocess.STDOUT)
        print(name, 'passed', flush=True)
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    with tempfile.TemporaryDirectory(prefix='dynamic-pattern-check-') as temporary:
        temporary = Path(temporary)
        url = 'https://judge.yosupo.jp/problem/suffixarray'
        cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
        inputs = sorted(cases.glob('*.in'))
        assert inputs, 'Run oj download --system for suffixarray first'
        result['cases']['suffixarray'] = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}
        header, document = 'string/dynamic_pattern_matching.hpp', 'docs/dynamic_pattern_matching.md'
        text = (root / document).read_text()
        assert yaml.safe_load(text.split('---', 2)[1])['documentation_of'] == '//' + header
        assert '`' + header + '`' in (root / 'README.md').read_text()
        assert '{{' not in text
        bundle = subprocess.check_output(['oj-bundle', header], cwd=root, text=True, stderr=subprocess.DEVNULL)
        driver = subprocess.check_output(['oj-bundle', 'test/dynamic_pattern_matching_array.test.cpp'], cwd=root, text=True, stderr=subprocess.DEVNULL)
        assert '#include "' not in bundle and '#include "' not in driver
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = temporary / ('suffixarray' + compiler)
            run([*flags, '-x', 'c++', '-', '-o', str(executable)], input=driver, text=True)
            for path in inputs:
                with path.open('rb') as stream:
                    actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60)
                assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            result['official'].append({'compiler': compiler, 'problem': 'suffixarray', 'cases': len(inputs)})
            print(compiler, 'suffixarray', len(inputs), 'official cases passed', flush=True)
            run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True)
            examples = re.findall(r'```cpp\n(.*?)\n```', text, re.S)
            for example in examples:
                executable = temporary / 'example'
                run([*flags, '-x', 'c++', '-', '-o', str(executable)], input=example.replace('"lib/', '"'), text=True)
                run([str(executable)])
            result['docs'].append({'compiler': compiler, 'bundle': 'passed', 'driver_bundle': 'passed', 'executed_examples': len(examples)})
    names = ['string/dynamic_pattern_matching.hpp', 'components/persistent_rope.hpp', 'components/suffix_range.hpp',
             'test/unit/dynamic_pattern_matching.cpp', 'test/unit/standalone.cpp',
             'test/dynamic_pattern_matching_array.test.cpp', 'docs/dynamic_pattern_matching.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Official cases, bundles and example passed', flush=True)


if __name__ == '__main__':
    main()
