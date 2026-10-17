import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--samples', type=Path, required=True)
    args = parser.parse_args()
    pairs, expected = [], []
    for n in range(1, 17):
        sums = [0] * (n + 1)
        for x in range(1, 1 << n): sums[x.bit_count()] += x
        total = 0
        for m in range(1, n + 1):
            total += sums[m]
            pairs.append((n, m)); expected.append(total % 998244353)
    data = (str(len(pairs)) + '\n' + ''.join(f'{n} {m}\n' for n, m in pairs)).encode()
    with tempfile.TemporaryDirectory(prefix='cp-binomial-prefix-check-') as directory:
        for compiler in ['g++', 'clang++']:
            for kind in ['online', 'offline']:
                binary = Path(directory) / f'{compiler}-{kind}'
                source = ROOT / f'test/binomial_prefix_sum_{kind}.test.cpp'
                subprocess.run([compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
                actual = list(map(int, subprocess.check_output([str(binary)], input=data).split()))
                assert actual == expected, (compiler, kind)
                samples = sorted(args.samples.glob('*.in'))
                assert samples
                for sample in samples:
                    actual = subprocess.check_output([str(binary)], input=sample.read_bytes()).split()
                    assert actual == sample.with_suffix('.out').read_bytes().split(), (compiler, kind, sample)
                print(compiler, kind, ':', len(pairs), 'exhaustive cases and', len(samples), 'public samples passed', flush=True)


if __name__ == '__main__': main()
