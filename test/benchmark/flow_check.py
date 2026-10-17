import argparse
from fractions import Fraction
import heapq
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--samples', type=Path, required=True)
    parser.add_argument('--only', choices=['max_flow_yuki', 'dijkstra_bigint_yuki'])
    args = parser.parse_args()
    rng = random.Random(3669)
    cases = []
    for trial in range(60):
        n = rng.randint(2, 25)
        pairs = {(i - 1, i) for i in range(1, n)}
        for _ in range(n * 2):
            u, v = sorted(rng.sample(range(n), 2))
            pairs.add((u, v))
        edges, graph = [], [[] for _ in range(n)]
        for u, v in sorted(pairs):
            w = Fraction(rng.randint(1, 300), rng.randint(1, 300))
            edges.append((u + 1, v + 1, w.numerator, w.denominator))
            graph[u].append((v, w)); graph[v].append((u, w))
        dist = [None] * n
        dist[0] = Fraction(0)
        heap = [(dist[0], 0)]
        while heap:
            d, u = heapq.heappop(heap)
            if dist[u] != d: continue
            for v, c in graph[u]:
                if dist[v] is None or d + c < dist[v]:
                    dist[v] = d + c
                    heapq.heappush(heap, (dist[v], v))
        data = f'{n} {len(edges)}\n' + ''.join(' '.join(map(str, e)) + '\n' for e in edges)
        expected = [x for d in dist[1:] for x in (d.numerator, d.denominator)]
        cases.append((data.encode(), expected))
    with tempfile.TemporaryDirectory(prefix='cp-flow-check-') as directory:
        for compiler in ['g++', 'clang++']:
            for name, problem in [('max_flow_yuki', '3668'), ('dijkstra_bigint_yuki', '3669')]:
                if args.only and args.only != name: continue
                binary = Path(directory) / f'{compiler}-{name}'
                subprocess.run([compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT),
                                str(ROOT / f'test/{name}.test.cpp'), '-o', str(binary)], check=True)
                samples = sorted((args.samples / problem).glob('*.in'))
                assert samples
                for sample in samples:
                    actual = subprocess.check_output([str(binary)], input=sample.read_bytes()).split()
                    assert actual == sample.with_suffix('.out').read_bytes().split(), (compiler, sample)
                if name == 'dijkstra_bigint_yuki':
                    for data, expected in cases:
                        actual = list(map(int, subprocess.check_output([str(binary)], input=data).split()))
                        assert actual == expected, compiler
                message = f'{compiler} {name}: {len(samples)} samples passed'
                if name == 'dijkstra_bigint_yuki': message += '; 60 Fraction oracle cases passed'
                print(message, flush=True)


if __name__ == '__main__': main()
