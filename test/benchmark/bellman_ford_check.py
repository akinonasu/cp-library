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
    output = root / 'test/benchmark/results/bellman_ford'

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'clang++', '--sanitize',
             '--selection', str(output / 'selection.json')], stdout=stream, stderr=subprocess.STDOUT)
    print('ASan/UBSan passed', flush=True)
    result = {'result': 'passed', 'official': [], 'docs': [], 'cases': {}}
    problems = [('negative', 'https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_1_B', 'test/bellman_ford.test.cpp'),
                ('path', 'https://judge.yosupo.jp/problem/shortest_path', 'test/bellman_ford_path.test.cpp')]
    with tempfile.TemporaryDirectory(prefix='negative-path-check-') as temporary:
        temporary = Path(temporary)
        for compiler in ['g++', 'clang++']:
            executable = temporary / ('long-chain-' + compiler)
            run([compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root),
                 'test/unit/shortest_path_scc.cpp', '-o', str(executable)])
            run([str(executable)], timeout=60)
        print('GCC and Clang long-chain checks passed', flush=True)
        for name, url, source in problems:
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
                        actual = subprocess.check_output([str(executable)], stdin=stream, timeout=60).split()
                    expected = path.with_suffix('.out').read_bytes().split()
                    if name == 'negative' or expected == [b'-1']:
                        assert actual == expected, (compiler, path)
                    else:
                        values = list(map(int, path.read_bytes().split()))
                        n, m, s, t = values[:4]
                        length, count = int(actual[0]), int(actual[1])
                        assert length == int(expected[0]) and len(actual) == 2 + 2 * count
                        weights = {}
                        for i in range(m):
                            v, to, w = values[4 + 3 * i:7 + 3 * i]
                            weights[v, to] = min(weights.get((v, to), w), w)
                        v, cost = s, 0
                        for i in range(count):
                            a, b = map(int, actual[2 + 2 * i:4 + 2 * i])
                            assert a == v and (a, b) in weights and 0 <= b < n
                            cost += weights[a, b]
                            v = b
                        assert v == t and cost == length
                result['official'].append({'compiler': compiler, 'problem': url, 'cases': len(inputs)})
                print(compiler, name, len(inputs), 'official cases passed', flush=True)
        for header, document in [('graph/bellman_ford.hpp', 'docs/bellman_ford.md'),
                                 ('components/shortest_path_scc.hpp', 'docs/shortest_path_scc.md')]:
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
    names = ['graph/bellman_ford.hpp', 'components/shortest_path_scc.hpp', 'test/unit/bellman_ford.cpp',
             'test/unit/shortest_path_scc.cpp', 'test/unit/standalone.cpp', 'test/bellman_ford.test.cpp',
             'test/bellman_ford_path.test.cpp', 'docs/bellman_ford.md', 'docs/shortest_path_scc.md']
    result['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Official cases, bundles and examples passed', flush=True)


if __name__ == '__main__':
    main()
