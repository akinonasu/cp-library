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
    output = root / 'test/benchmark/results/shared_optimization'
    selection_path = output / 'selection.json'
    selection = json.loads(selection_path.read_text())

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    for name, options in [('adopted_check', ['--cxx', 'g++', 'clang++']),
                          ('adopted_sanitize', ['--cxx', 'clang++', '--sanitize'])]:
        with (output / (name + '.log')).open('w') as stream:
            run([sys.executable, 'test/verify.py', '--unit-only', '--selection', str(selection_path), *options],
                stdout=stream, stderr=subprocess.STDOUT)
        print(name, 'passed', flush=True)

    result = {'result': 'passed', 'headers': selection['headers'], 'unit': selection['unit'],
              'sanitize': selection['sanitize'], 'judges': [], 'docs': []}
    with tempfile.TemporaryDirectory(prefix='shared-check-') as temporary:
        temporary = Path(temporary)
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = temporary / compiler
            for header in selection['headers']:
                code = subprocess.check_output(['oj-bundle', header], cwd=root, text=True, stderr=subprocess.DEVNULL)
                assert '#include "' not in code
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
                docs = [path for path in (root / 'docs').glob('*.md')
                        if re.search(r'documentation_of:\s*[\'"]?//' + re.escape(header) + r'[\'"\s]', path.read_text())]
                assert len(docs) == 1, (header, docs)
                text = docs[0].read_text()
                front = yaml.safe_load(text.split('---', 2)[1])
                assert front['title']
                for row in text.splitlines():
                    if row.startswith('|'):
                        assert len(row.split('|')) == 5, (docs[0], row)
                example = re.search(r'```cpp\n(.*?)\n```', text, re.S)[1].replace('"lib/', '"')
                lines = example.splitlines()
                code = '\n'.join(line for line in lines if line.startswith('#include'))
                code += '\nint main() {\n' + '\n'.join(line for line in lines if not line.startswith('#include')) + '\n}\n'
                run([*flags[:-3], '-I', str(root), '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True)
                result['docs'].append({'compiler': compiler, 'header': header, 'bundle': 'passed', 'example': 'passed'})
            for driver in ['test/run_enumerate.test.cpp', 'test/lcs.test.cpp',
                           'test/lcs_restore.test.cpp', 'test/edit_distance.test.cpp']:
                run([*flags, '-DNDEBUG', driver, '-o', str(executable)])
                url = re.search(r'#define PROBLEM "([^"]+)"', (root / driver).read_text())[1]
                cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
                inputs = sorted(cases.glob('*.in'))
                assert inputs, (driver, 'No cached official cases')
                for path in inputs:
                    with path.open('rb') as stream:
                        actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60).split()
                    assert actual == path.with_suffix('.out').read_bytes().split(), (compiler, driver, path)
                result['judges'].append({'compiler': compiler, 'driver': driver, 'cases': len(inputs)})
                print(compiler, driver, len(inputs), 'cases passed', flush=True)
    names = set(selection['headers'] + selection['unit'] + selection['sanitize'])
    names.update(['test/run_enumerate.test.cpp', 'test/lcs.test.cpp', 'test/lcs_restore.test.cpp', 'test/edit_distance.test.cpp'])
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in sorted(names)}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('All shared optimization checks passed', flush=True)


if __name__ == '__main__':
    main()
