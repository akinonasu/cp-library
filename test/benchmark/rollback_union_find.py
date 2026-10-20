import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time
import urllib.parse
import urllib.request


ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/rollback-union-find'
OUTPUT = ROOT / 'test/benchmark/results/rollback_union_find'


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def references():
    def get(url):
        with urllib.request.urlopen(url, timeout=60) as response:
            return json.load(response)
    url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(
        dict(problem='persistent_unionfind', status='AC', order='+time', limit=20))
    listing = get(url)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing_url=url, listing=listing, references=[])
    ids = list(dict.fromkeys([r['id'] for r in listing['submissions'] if r['lang'].startswith('cpp')][:3] + [409831]))
    for identity in ids:
        detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(identity))
        code = detail['source'].encode()
        (CACHE / f'{identity}.cpp').write_bytes(code)
        result['references'].append(dict(**detail['overview'], url=f'https://judge.yosupo.jp/submission/{identity}',
                                         source_sha256=hashlib.sha256(code).hexdigest()))
    (OUTPUT / 'references.json').write_text(json.dumps(result, indent=2) + '\n')


def workloads():
    rng = random.Random(20261020)
    operations = []
    for _ in range(2000):
        operations.extend((0, rng.randrange(100000), rng.randrange(100000)) for _ in range(100))
        operations.extend((1, rng.randrange(100000), rng.randrange(100000)) for _ in range(20))
        operations.append((2, 0, 0))
    yield 'branches', 100000, operations
    n = 131072
    operations = []
    width = 1
    while width < n:
        operations.extend((0, v, v + width) for v in range(0, n, 2 * width))
        width *= 2
    operations.extend((1, rng.randrange(n), rng.randrange(n)) for _ in range(300000))
    operations.append((2, 0, 0))
    yield 'balanced', n, operations
    operations = [(0, 0, v) for v in range(1, 64)]
    for i in range(400000):
        operations.append((int(i % 4 == 0), rng.randrange(64), rng.randrange(64)))
    operations.append((2, 0, 0))
    yield 'redundant', 64, operations
    operations, state = [], 0
    for _ in range(400000):
        kind = rng.randrange(20)
        if kind < 12:
            operations.append((0, rng.randrange(1000), rng.randrange(1000)))
            state += 1
        elif kind < 16:
            operations.append((1, rng.randrange(1000), rng.randrange(1000)))
        elif kind < 18 and state:
            operations.append((3, 0, 0))
            state -= 1
        else:
            state = rng.randrange(state + 1)
            operations.append((2, state, 0))
    yield 'mixed', 1000, operations


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', action='store_true')
    parser.add_argument('--problems', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.chdir(ROOT)
    CACHE.mkdir(parents=True, exist_ok=True)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    if args.references:
        references()
        return
    assert args.trials > 0
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-DONLINE_JUDGE', '-I', str(ROOT)]
    binaries = {}
    for name, compiler, source in [('library', 'g++', ROOT / 'test/rollback_union_find.test.cpp'),
                                    ('clang', 'clang++', ROOT / 'test/rollback_union_find.test.cpp'),
                                    ('409831', 'g++', CACHE / '409831.cpp')]:
        binary = CACHE / name
        subprocess.run([compiler, *flags, str(source), '-o', str(binary)], check=True)
        binaries[name] = binary
    problem = args.problems / 'data_structure/persistent_unionfind'
    cases = sorted((problem / 'in').glob('*.in'))
    assert len(cases) == 14
    source_names = ['data_structure/rollback_union_find.hpp', 'test/rollback_union_find.test.cpp',
                    'test/unit/rollback_union_find.cpp', 'test/benchmark/rollback_union_find.cpp', 'test/benchmark/rollback_union_find.py']
    report = dict(compilers={c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']},
                  flags=flags, cpu=subprocess.check_output(['lscpu'], text=True), kernel=subprocess.check_output(['uname', '-a'], text=True),
                  affinity=[args.cpu], trials=args.trials, warmup=1, seed=20261020,
                  problem_commit=subprocess.check_output(['git', '-C', str(args.problems), 'rev-parse', 'HEAD'], text=True).strip(),
                  source_sha256={p: digest(ROOT / p) for p in source_names}, reference_source_sha256=digest(CACHE / '409831.cpp'),
                  judge=[], end_to_end={}, api={}, complete=False)

    def save():
        (OUTPUT / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')

    for path in cases:
        expected = (problem / 'out' / (path.stem + '.out')).read_bytes().split()
        passed = []
        for name, binary in binaries.items():
            with path.open('rb') as stream:
                actual = subprocess.check_output([str(binary)], stdin=stream, timeout=30)
            assert actual.split() == expected, (path, name)
            passed.append(name)
        report['judge'].append(dict(case=path.stem, input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), passed=passed))
    print('All 14 official cases passed for GCC, Clang and reference', flush=True)
    save()
    rng = random.Random(8472)
    for name in ['max_random_00', 'max_random_04', 'conchon_filliatre_tle_00', 'conchon_filliatre_tle_01']:
        path = problem / 'in' / (name + '.in')
        samples = {mode: [] for mode in ['library', '409831']}
        for trial in range(args.trials + 1):
            order = list(samples)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    start = time.perf_counter_ns()
                    subprocess.run([str(binaries[mode])], stdin=stream, stdout=subprocess.DEVNULL, check=True)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                if trial:
                    samples[mode].append(elapsed)
        report['end_to_end'][name] = dict(milliseconds=samples, median_ms={k: statistics.median(v) for k, v in samples.items()})
        print(name, report['end_to_end'][name]['median_ms'], flush=True)
        save()
    original = (ROOT / 'data_structure/rollback_union_find.hpp').read_text()
    full = original.replace('struct Change { int child, size; };', 'struct Change { int parent, child, parent_size, child_size; };')
    full = full.replace('history_.push_back({-1, 0});', 'history_.push_back({-1, -1, 0, 0});')
    full = full.replace('history_.push_back({y, parent_[y]});', 'history_.push_back({x, y, parent_[x], parent_[y]});')
    full = full.replace('auto [child, size] = history_.back();', 'auto [parent, child, parent_size, child_size] = history_.back();')
    full = full.replace('parent_[parent_[child]] -= size, parent_[child] = size;', 'parent_[parent] = parent_size, parent_[child] = child_size;')
    assert full != original and 'child_size' in full
    (CACHE / 'full.hpp').write_text(full)
    apis = {}
    for name, options in [('compact', []), ('full', ['-DROLLBACK_HEADER="' + str(CACHE / 'full.hpp') + '"'])]:
        binary = CACHE / ('api_' + name)
        subprocess.run(['g++', *flags, *options, 'test/benchmark/rollback_union_find.cpp', '-o', str(binary)], check=True)
        apis[name] = binary
    for name, n, operations in workloads():
        path = CACHE / (name + '.in')
        path.write_text(f'{n} {len(operations)}\n' + ''.join(f'{t} {x} {y}\n' for t, x, y in operations))
        samples = {mode: [] for mode in apis}
        expected = None
        for trial in range(args.trials + 1):
            order = list(apis)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    result = subprocess.check_output([str(apis[mode])], stdin=stream, text=True, timeout=30).split()
                if expected is None:
                    expected = result[1:]
                assert result[1:] == expected, (name, mode)
                if trial:
                    samples[mode].append(float(result[0]))
        report['api'][name] = dict(n=n, operations=len(operations), input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                                  result=expected, milliseconds=samples, median_ms={k: statistics.median(v) for k, v in samples.items()})
        print(name, report['api'][name]['median_ms'], flush=True)
        save()
    for path, expected in report['source_sha256'].items():
        assert digest(ROOT / path) == expected, path
    report['complete'] = True
    save()


if __name__ == '__main__':
    main()
