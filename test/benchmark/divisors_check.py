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
    output = root / 'test/benchmark/results/divisors'
    output.mkdir(parents=True, exist_ok=True)
    target = root / 'math/number_theory/factorize.hpp'

    def depends(path, seen=None):
        seen = set() if seen is None else seen
        if path == target:
            return True
        if path in seen:
            return False
        seen.add(path)
        for name in re.findall(r'^\s*#include\s*"([^"]+)"', path.read_text(), re.M):
            for candidate in [path.parent / name, root / name]:
                candidate = candidate.resolve()
                if candidate.is_file():
                    if depends(candidate, seen):
                        return True
                    break
        return False

    headers = [p.relative_to(root).as_posix() for folder in ['math', 'convolution', 'components', 'fps', 'data_structure', 'graph', 'tree', 'string']
               for p in sorted((root / folder).rglob('*.hpp')) if depends(p)]
    units = [p.relative_to(root).as_posix() for p in sorted((root / 'test/unit').glob('*.cpp')) if depends(p)]
    judges = [p.relative_to(root).as_posix() for p in sorted((root / 'test').glob('*.test.cpp')) if depends(p)]
    selection = {'headers': headers, 'unit': units, 'sanitize': ['test/unit/divisors.cpp']}
    (output / 'selection.json').write_text(json.dumps(selection, indent=2) + '\n')
    command = [sys.executable, 'test/verify.py', '--unit-only', '--selection', str(output / 'selection.json')]
    for name, flags in [('check.log', ['--cxx', 'g++', 'clang++']), ('sanitize.log', ['--cxx', 'clang++', '--sanitize'])]:
        with (output / name).open('w') as stream:
            subprocess.run([*command, *flags], cwd=root, stdout=stream, stderr=subprocess.STDOUT, check=True)
        print(name, 'passed', flush=True)
    document = (root / 'docs/factorize.md').read_text()
    assert yaml.safe_load(document.split('---', 2)[1])['documentation_of'] == '//math/number_theory/factorize.hpp'
    assert '`math/number_theory/factorize.hpp`' in (root / 'README.md').read_text()
    example = re.search(r'```cpp\n(.*?)\n```', document, re.S)[1].replace('"lib/', '"')
    example += '\nint main() { assert((ds == std::vector<std::uint64_t>{1,2,3,4,6,12})); assert(prime && reused.size() == 24); }\n'
    with tempfile.TemporaryDirectory(prefix='divisors-check-') as temporary:
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            executable = str(Path(temporary) / 'example')
            subprocess.run([*flags, '-x', 'c++', '-', '-o', executable], input=example, text=True, cwd=root, check=True)
            subprocess.run([executable], check=True)
            for source in headers + judges:
                bundle = subprocess.check_output(['oj-bundle', source], cwd=root, text=True, stderr=subprocess.DEVNULL)
                subprocess.run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True, cwd=root, check=True)
    names = ['math/number_theory/factorize.hpp', 'docs/factorize.md', 'test/unit/divisors.cpp', 'test/unit/standalone.cpp']
    result = {'result': 'passed', 'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']},
              'selection': selection, 'judge_bundles': judges, 'docs': 'compiled and executed with both compilers',
              'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names}}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Bundles and example passed', flush=True)


if __name__ == '__main__':
    main()
