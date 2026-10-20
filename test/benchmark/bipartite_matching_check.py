import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'test/benchmark/results/bipartite_matching'


def main():
    from onlinejudge.service.library_checker import LibraryCheckerService
    problem = Path(LibraryCheckerService._get_cloned_repository_path()) / 'graph/bipartitematching'
    cases = sorted((problem / 'in').glob('*.in'))
    assert cases and (problem / 'checker').exists()
    report = {'result': 'passed', 'judge': [], 'compilers': {},
              'exhaustive_graphs': sum(1 << (l * r) for l in range(5) for r in range(5) if l * r <= 12),
              'random_multigraphs': 500, 'long_path_vertices_per_side': 200000,
              'problem_commit': subprocess.check_output(['git', '-C', str(problem), 'rev-parse', 'HEAD'], text=True).strip()}
    selection = OUT / 'selection.json'
    for compiler in ['g++', 'clang++']:
        name = 'gcc' if compiler == 'g++' else 'clang'
        with (OUT / (name + '_unit.log')).open('w') as log:
            subprocess.run(['python3', 'test/verify.py', '--unit-only', '--cxx', compiler, '--selection', str(selection)],
                           cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        print(compiler, 'unit tests passed', flush=True)
        report['compilers'][compiler] = subprocess.check_output([compiler, '--version'], text=True)
    with (OUT / 'sanitize.log').open('w') as log:
        subprocess.run(['python3', 'test/verify.py', '--unit-only', '--sanitize', '--cxx', 'clang++', '--selection', str(selection)],
                       cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
    print('ASan/UBSan passed', flush=True)
    with tempfile.TemporaryDirectory(prefix='bipartite-matching-') as directory:
        binary = str(Path(directory) / 'check')
        output = Path(directory) / 'output.txt'
        flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1', '-I', str(ROOT)]
        example = re.search(r'```cpp\n(.*?)\n```', (ROOT / 'docs/bipartite_matching.md').read_text(), re.S)[1].replace('"lib/', '"')
        for compiler in ['g++', 'clang++']:
            subprocess.run([compiler, *flags, 'test/bipartite_matching.test.cpp', '-o', binary], cwd=ROOT, check=True)
            for path in cases:
                with path.open('rb') as source, output.open('wb') as destination:
                    subprocess.run([binary], stdin=source, stdout=destination, check=True, timeout=20)
                expected = problem / 'out' / (path.stem + '.out')
                result = subprocess.run([str(problem / 'checker'), str(path), str(output), str(expected)], capture_output=True)
                assert result.returncode == 0, (compiler, path, result.stderr)
            print(compiler, len(cases), 'official cases passed', flush=True)
            for path in ['graph/matching/bipartite_matching.hpp', 'components/bipartite_matching_core.hpp', 'test/bipartite_matching.test.cpp']:
                bundle = subprocess.check_output(['oj-bundle', path], cwd=ROOT, stderr=subprocess.DEVNULL)
                assert b'#include "' not in bundle
                extra = ['-fconstexpr-depth=1024', '-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152'] if compiler == 'g++' else []
                subprocess.run([compiler, *flags, *extra, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, cwd=ROOT, check=True)
            subprocess.run([compiler, *flags, '-x', 'c++', '-', '-o', binary], input=example, text=True, cwd=ROOT, check=True)
            subprocess.run([binary], check=True)
            report['judge'].append({'compiler': compiler, 'cases': len(cases), 'checker': 'official', 'bundles': 'passed', 'example': 'passed'})
            print(compiler, 'bundles and example passed', flush=True)
    names = ['graph/matching/bipartite_matching.hpp', 'components/bipartite_matching_core.hpp',
             'test/unit/bipartite_matching.cpp', 'test/bipartite_matching.test.cpp', 'docs/bipartite_matching.md',
             'docs/bipartite_matching_core.md', 'test/benchmark/bipartite_matching_check.py']
    report['sources'] = {name: hashlib.sha256((ROOT / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (OUT / 'validation.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
