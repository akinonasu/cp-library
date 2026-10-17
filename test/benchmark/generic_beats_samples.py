import argparse
import json
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SAMPLES = [('3 5\n2 5 6\n3 1 3\n1 2 3 2\n3 1 2\n2 1 2 3\n3 1 3\n', [13, 4, 9]),
           ('6 11\n10 3 5 20 6 7\n3 1 6\n1 2 4 3\n3 1 3\n2 1 4 10\n3 3 6\n1 3 6 2\n2 1 4 5\n3 1 6\n2 1 3 100\n1 2 5 6\n3 1 4\n', [51, 12, 33, 26, 132])]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cp-beats-samples-') as folder:
        binary = Path(folder) / 'division'
        subprocess.run(['g++', '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT),
                        str(ROOT / 'test/benchmark/generic_segtree_beats_division.cpp'), '-o', str(binary)], check=True)
        for data, expected in SAMPLES:
            actual = list(map(int, subprocess.check_output([binary], input=data, text=True).split()))
            assert actual == expected
        rng = random.Random(713905)
        for trial in range(30):
            n, q = rng.randrange(1, 101), 500
            a = [rng.randrange(1, 100001) for _ in range(n)]
            lines = [f'{n} {q}', ' '.join(map(str, a))]
            expected = []
            for _ in range(q):
                l = rng.randrange(n)
                r = rng.randrange(l + 1, n + 1)
                t = rng.randrange(1, 4)
                x = rng.randrange(2, 100001)
                lines.append(f'{t} {l + 1} {r}' + (f' {x}' if t < 3 else ''))
                if t == 1:
                    a[l:r] = [value // x for value in a[l:r]]
                elif t == 2:
                    a[l:r] = [x] * (r - l)
                else:
                    expected.append(sum(a[l:r]))
            actual = list(map(int, subprocess.check_output([binary], input='\n'.join(lines) + '\n', text=True).split()))
            assert actual == expected, trial
        source = (ROOT / 'docs/generic_segtree_beats.md').read_text().split('```cpp\n', 1)[1].split('```', 1)[0]
        prefix, body = source.split('Tree seg(', 1)
        cpp = Path(folder) / 'example.cpp'
        cpp.write_text(prefix.replace('lib/', '') + '\nint main() {\nTree seg(' + body + '\nreturn sum != 9;\n}\n')
        subprocess.run(['g++', '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT), str(cpp), '-o', str(binary)], check=True)
        subprocess.run([binary], check=True)
    result = {'problem': 'https://atcoder.jp/contests/abc256/tasks/abc256_h', 'official_samples': len(SAMPLES),
              'random_cases': 30, 'queries_per_random_case': 500, 'seed': 713905, 'documentation_example': 'passed',
              'scope': 'official public samples and independent random oracle; not an AtCoder submission or hidden cases'}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(result)


if __name__ == '__main__':
    main()
