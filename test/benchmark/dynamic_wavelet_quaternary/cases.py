#!/usr/bin/env python3
"""Deterministic ABC343 F workloads and independent top-two segment-tree answers."""
import argparse
from collections import Counter
import datetime
import hashlib
import itertools
import json
from pathlib import Path
import random
import sys

SHAPES = ('random30', 'duplicates16', 'uniform_same', 'no_op_random', 'clustered', 'short')
LIMIT = 1_000_000_000
EMPTY = (0, 0, 0, 0)


def merge(a, b):
    av, ac, ax, an = a
    bv, bc, bx, bn = b
    if av == bv:
        return (av, ac + bc, ax, an + bn) if ax == bx else ((av, ac + bc, ax, an) if ax > bx else (av, ac + bc, bx, bn))
    if av > bv:
        return (av, ac, ax, an + bc) if ax == bv else ((av, ac, ax, an) if ax > bv else (av, ac, bv, bc))
    return (bv, bc, bx, bn + ac) if bx == av else ((bv, bc, bx, bn) if bx > av else (bv, bc, av, ac))


class Oracle:
    def __init__(self, values):
        size = 1
        while size < len(values):
            size *= 2
        self.size = size
        self.tree = [EMPTY] * (2 * size)
        for i, value in enumerate(values):
            self.tree[size + i] = (value, 1, 0, 0)
        for i in range(size - 1, 0, -1):
            self.tree[i] = merge(self.tree[2 * i], self.tree[2 * i + 1])

    def set(self, p, value):
        p += self.size
        self.tree[p] = (value, 1, 0, 0)
        while p > 1:
            p //= 2
            self.tree[p] = merge(self.tree[2 * p], self.tree[2 * p + 1])

    def answer(self, l, r):
        l += self.size
        r += self.size
        left, right = EMPTY, EMPTY
        while l < r:
            if l & 1:
                left = merge(left, self.tree[l])
                l += 1
            if r & 1:
                r -= 1
                right = merge(self.tree[r], right)
            l //= 2
            r //= 2
        return merge(left, right)[3]


class HashedWriter:
    def __init__(self, path):
        self.file = path.open('wb')
        self.hash = hashlib.sha256()
        self.bytes = 0

    def write(self, value):
        data = value.encode()
        self.file.write(data)
        self.hash.update(data)
        self.bytes += len(data)

    def close(self):
        self.file.close()
        return {'sha256': self.hash.hexdigest(), 'bytes': self.bytes}


def naive(values, l, r):
    counts = Counter(values[l:r])
    keys = sorted(counts)
    return counts[keys[-2]] if len(keys) > 1 else 0


def write_case(directory, name, initial, operations, seed=None, cross_check=False):
    directory.mkdir(parents=True, exist_ok=True)
    operations = list(operations)
    values = initial.copy()
    oracle = Oracle(values)
    input_path, answer_path = directory / (name + '.in'), directory / (name + '.out')
    source, answers = HashedWriter(input_path), HashedWriter(answer_path)
    source.write(f'{len(values)} {len(operations)}\n')
    source.write(' '.join(map(str, values)) + '\n')
    stats = {'set': 0, 'set_same': 0, 'set_changed': 0, 'query': 0,
             'query_single': 0, 'query_2_to_16': 0, 'query_full': 0,
             'answer_zero': 0, 'naive_cross_checks': 0}
    for kind, a, b in operations:
        if kind == 1:
            assert 0 <= a < len(values) and 1 <= b <= LIMIT
            source.write(f'1 {a + 1} {b}\n')
            stats['set'] += 1
            stats['set_same' if values[a] == b else 'set_changed'] += 1
            values[a] = b
            oracle.set(a, b)
        else:
            assert kind == 2 and 0 <= a < b <= len(values)
            source.write(f'2 {a + 1} {b}\n')
            result = oracle.answer(a, b)
            if cross_check:
                expected = naive(values, a, b)
                assert result == expected, (name, a, b, result, expected)
                stats['naive_cross_checks'] += 1
            answers.write(str(result) + '\n')
            stats['query'] += 1
            stats['query_single'] += b - a == 1
            stats['query_2_to_16'] += 2 <= b - a <= 16
            stats['query_full'] += a == 0 and b == len(values)
            stats['answer_zero'] += result == 0
    result = {'name': name, 'n': len(initial), 'q': len(operations), 'seed': seed,
              'input': {'path': str(input_path), **source.close()},
              'expected': {'path': str(answer_path), **answers.close()}, 'stats': stats,
              'generator_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              'python': sys.version, 'created_at': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'oracle': 'independent iterative segment tree, highest two distinct values with multiplicities'}
    (directory / (name + '.json')).write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: result[k] for k in ('name', 'n', 'q', 'stats')}), flush=True)
    return result


def large_case(directory, shape, n, q, seed):
    rng = random.Random(seed)
    def value():
        if shape == 'uniform_same':
            return 7
        if shape == 'duplicates16':
            return rng.randrange(1, 17)
        return rng.randrange(1, LIMIT + 1)
    initial = [value() for _ in range(n)]
    current = initial.copy()
    operations = []
    for i in range(q):
        if i & 1 == 0:
            if shape == 'clustered':
                p = n // 2 if i % 10 else max(0, min(n - 1, n // 2 + rng.randrange(-64, 65)))
            else:
                p = rng.randrange(n)
            x = current[p] if shape == 'no_op_random' else value()
            operations.append((1, p, x))
            current[p] = x
        else:
            l = rng.randrange(n)
            if shape == 'short':
                r = min(n, l + (1 if i % 4 == 1 else rng.randrange(2, 17)))
            elif i % 97 == 1:
                l, r = 0, n
            elif i % 97 == 3:
                r = l + 1
            else:
                r = rng.randrange(n)
                if l > r:
                    l, r = r, l
                r += 1
            operations.append((2, l, r))
    return write_case(directory, shape, initial, operations, seed)


def small_cases(directory):
    rows = []
    operations = []
    for value in [1, LIMIT, 7, 7, 1]:
        operations.extend([(1, 0, value), (2, 0, 1)])
    rows.append(write_case(directory, 'small_single', [1], operations, cross_check=True))
    operations = []
    for values in itertools.product((1, 2, 3), repeat=6):
        operations.extend((1, i, x) for i, x in enumerate(values))
        operations.extend((2, l, r) for l in range(6) for r in range(l + 1, 7))
    rows.append(write_case(directory, 'small_exhaustive', [1] * 6, operations, cross_check=True))
    n = 1050
    initial = [1 if i % 3 == 0 else LIMIT if i % 3 == 1 else 7 for i in range(n)]
    operations = []
    for p in [0, 1, 31, 32, 63, 64, 65, 511, 512, 1023, 1024, 1049]:
        for x in [1, 2, 7, LIMIT]:
            operations.append((1, p, x))
            for l, r in [(0, n), (p, p + 1), (max(0, p - 1), min(n, p + 2)),
                         (max(0, p - 16), min(n, p + 17)), (0, p + 1), (p, n)]:
                operations.append((2, l, r))
    operations.extend((1, p, 7) for p in range(n))
    operations.extend([(2, 0, n), (2, 63, 65), (2, 1023, 1025), (1, 0, 1), (1, n - 1, LIMIT), (2, 0, n)])
    rows.append(write_case(directory, 'small_boundaries', initial, operations, cross_check=True))
    return rows


def main():
    parser = argparse.ArgumentParser(__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    generate = sub.add_parser('generate')
    generate.add_argument('--directory', type=Path, required=True)
    generate.add_argument('--large', action='store_true')
    generate.add_argument('--small', action='store_true')
    generate.add_argument('--shape', choices=SHAPES, action='append')
    generate.add_argument('--n', type=int, default=200000)
    generate.add_argument('--q', type=int, default=200000)
    generate.add_argument('--seed', type=int, default=20261020)
    validate = sub.add_parser('validate')
    validate.add_argument('--manifest', type=Path, required=True)
    validate.add_argument('--actual', type=Path, required=True)
    args = parser.parse_args()
    if args.command == 'validate':
        manifest = json.loads(args.manifest.read_text())
        expected_path = Path(manifest['expected']['path'])
        expected_bytes = expected_path.read_bytes()
        assert hashlib.sha256(expected_bytes).hexdigest() == manifest['expected']['sha256']
        actual_bytes = args.actual.read_bytes()
        expected, actual = expected_bytes.split(), actual_bytes.split()
        if len(expected) != len(actual):
            raise SystemExit(f'answer count mismatch: {len(expected)} expected, {len(actual)} actual')
        for i, (a, b) in enumerate(zip(expected, actual)):
            if int(a) != int(b):
                raise SystemExit(f'answer {i}: expected {a.decode()}, got {b.decode()}')
        print(json.dumps({'status': 'pass', 'answers': len(expected), 'actual_sha256': hashlib.sha256(actual_bytes).hexdigest(), 'case': manifest['name']}))
        return
    assert 1 <= args.n <= 200000 and 1 <= args.q <= 200000
    rows = small_cases(args.directory) if args.small or not args.large else []
    if args.large:
        for i, shape in enumerate(SHAPES):
            if args.shape is None or shape in args.shape:
                rows.append(large_case(args.directory, shape, args.n, args.q, args.seed + i))
    (args.directory / 'manifest.json').write_text(json.dumps(rows, indent=2) + '\n')


if __name__ == '__main__':
    main()
