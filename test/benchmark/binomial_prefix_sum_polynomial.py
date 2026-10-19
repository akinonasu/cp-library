import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import shlex
import statistics
import subprocess
import time


ROOT = Path(__file__).resolve().parents[2]
BASE = '444ac7f'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def generate(shape, size, count, seed):
    rng = random.Random(seed)
    queries = []
    pool = [(rng.randint(1, size), 0) for _ in range(256)]
    pool = [(n, rng.randint(0, n + 1)) for n, _ in pool]
    for _ in range(count):
        if shape == 'uniform':
            n = rng.randint(0, size)
            k = rng.randint(0, n + 1)
        elif shape == 'rectangle':
            n, k = rng.randint(size // 2, size), rng.randint(0, size // 2)
        elif shape == 'independent':
            n, k = rng.randint(0, size), rng.randint(0, size + 1)
        elif shape == 'narrow_n':
            n = rng.randint(max(0, size - 2047), size)
            k = rng.randint(0, n + 1)
        elif shape == 'edges':
            n = rng.randint(0, size)
            k = rng.choice([0, 1, 2, n - 1, n, n + 1])
        elif shape == 'duplicates':
            n, k = rng.choice(pool)
        elif shape == 'small_rows':
            n = rng.randint(0, min(256, size))
            k = rng.randint(0, n + 1)
        else:
            raise ValueError(shape)
        queries.append((n, k))
    queries[-1] = (size, queries[-1][1])
    return (str(count) + '\n' + ''.join(f'{n} {k}\n' for n, k in queries)).encode()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--compiler', default='/usr/bin/g++')
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--size', type=int, default=200000)
    parser.add_argument('--queries', type=int, default=200000)
    parser.add_argument('--seed', type=int, default=20261020)
    args = parser.parse_args()
    assert args.size >= 0 and args.queries > 0 and args.trials > 0
    work, output = args.work.resolve(), args.output.resolve()
    work.mkdir(parents=True, exist_ok=False)
    output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    original = subprocess.check_output(['git', 'show', BASE + ':math/combinatorics/binomial_prefix_sum.hpp'], cwd=ROOT, text=True)
    body = original[original.index('template<class Mint = mint, class K = int>'):original.rfind('#endif')].strip()
    reference = (ROOT / 'test/benchmark/binomial_prefix_sum_comparison.hpp').read_text()
    assert body.replace('binomial_prefix_sum(', 'binomial_prefix_sum_baseline(', 1) in reference
    for name in ('components/binomial_prefix_core.hpp', 'components/mo_order.hpp', 'math/mint.hpp'):
        expected = subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=ROOT)
        assert expected == (ROOT / name).read_bytes(), name
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=haswell', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/binomial_prefix_sum_polynomial.cpp'
    binary, dependency = work / 'compare', work / 'compare.d'
    command = [args.compiler, *flags, '-I', str(ROOT), '-MD', '-MF', str(dependency), str(source), '-o', str(binary)]
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock, (output / 'compile.log').open('w') as log:
        fcntl.flock(lock, fcntl.LOCK_SH)
        subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
    dependencies = shlex.split(dependency.read_text().replace('\\\n', ' ').split(':', 1)[1])
    hashes = {str(Path(p).resolve()): sha(Path(p).resolve()) for p in dependencies}
    compiler = Path(args.compiler).resolve()
    result = {
        'status': 'running', 'baseline': subprocess.check_output(['git', 'rev-parse', BASE], cwd=ROOT, text=True).strip(),
        'platform': platform.platform(), 'cpuinfo': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
        'affinity': sorted(os.sched_getaffinity(0)), 'compiler': subprocess.check_output([args.compiler, '--version'], text=True),
        'compiler_sha256': sha(compiler), 'command': command, 'binary_sha256': sha(binary),
        'harness_sha256': sha(Path(__file__).resolve()), 'flags': flags, 'dependencies': hashes,
        'trials': args.trials, 'warmups': 1, 'seed': args.seed, 'max_n': args.size, 'queries': args.queries,
        'metric': 'API wall time includes all factorial preprocessing, query filtering, ordering, polynomial work and destruction; process time also records startup and shared input/output',
        'cases': [],
    }

    def save():
        (output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')

    def run(mode, data):
        start = time.perf_counter_ns()
        child = subprocess.run([str(binary), mode], input=data, capture_output=True, check=True)
        wall = time.perf_counter_ns() - start
        return child.stdout, int(child.stderr.strip()), wall

    modes = ['baseline', 'polynomial']
    shapes = ['uniform', 'rectangle', 'independent', 'narrow_n', 'edges', 'duplicates', 'small_rows']
    schedule_rng = random.Random(args.seed + 1)
    save()
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for index, shape in enumerate(shapes):
            seed = args.seed + 1009 * index
            data = generate(shape, args.size, args.queries, seed)
            (work / (shape + '.in')).write_bytes(data)
            row = {'shape': shape, 'seed': seed, 'input_sha256': hashlib.sha256(data).hexdigest(), 'runs': []}
            expected = None
            for mode in modes:
                actual, _, _ = run(mode, data)
                if expected is None:
                    expected = actual
                assert actual == expected, (shape, mode, 'warmup')
            row['output_sha256'] = hashlib.sha256(expected).hexdigest()
            for trial in range(args.trials):
                order = modes.copy()
                schedule_rng.shuffle(order)
                for mode in order:
                    actual, api, wall = run(mode, data)
                    assert actual == expected, (shape, trial, mode)
                    row['runs'].append({'trial': trial, 'mode': mode, 'api_ns': api, 'process_ns': wall})
            row['summary'] = {
                mode: {
                    'api_median_ms': statistics.median(r['api_ns'] for r in row['runs'] if r['mode'] == mode) / 1e6,
                    'process_median_ms': statistics.median(r['process_ns'] for r in row['runs'] if r['mode'] == mode) / 1e6,
                } for mode in modes
            }
            row['paired_speedup_median_percent'] = statistics.median(
                100 * (1 - next(r['api_ns'] for r in row['runs'] if r['trial'] == trial and r['mode'] == 'polynomial') /
                       next(r['api_ns'] for r in row['runs'] if r['trial'] == trial and r['mode'] == 'baseline'))
                for trial in range(args.trials)
            )
            result['cases'].append(row)
            save()
            print(shape, {m: round(row['summary'][m]['api_median_ms'], 3) for m in modes}, flush=True)
    assert all(sha(Path(path)) == digest for path, digest in hashes.items())
    assert sha(compiler) == result['compiler_sha256'] and sha(binary) == result['binary_sha256']
    assert sha(Path(__file__).resolve()) == result['harness_sha256']
    result.update(status='passed', dependencies_unchanged=True, compiler_unchanged=True, binary_unchanged=True)
    save()
    print('PASS identical outputs and bound source/compiler/binary/dependency hashes', flush=True)


if __name__ == '__main__':
    main()
