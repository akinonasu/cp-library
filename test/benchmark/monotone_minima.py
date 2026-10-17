import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def workloads():
    rng = random.Random(31674389)
    result = {}
    for mode in ['random', 'moving', 'ties', 'thin', 'single_column']:
        n, m = (64, 400000) if mode == 'thin' else (400000, 1) if mode == 'single_column' else (200000, 200000)
        if mode == 'random':
            slopes = sorted(rng.randint(-32, 32) for _ in range(n - 1))
            a = [0]
            for slope in slopes:
                a.append(a[-1] + slope)
            low = min(a)
            a = [x - low for x in a]
            b = [rng.randrange(1000000001) for _ in range(m)]
        elif mode == 'moving':
            a, b = [7 * i for i in range(n)], [0] * m
        elif mode == 'ties':
            a, b = [0] * n, [0] * m
        else:
            a = [abs(i - n // 2) for i in range(n)]
            b = [rng.randrange(1000000001) for _ in range(m)]
        assert all(a[i + 1] - a[i] <= a[i + 2] - a[i + 1] for i in range(n - 2))
        assert 0 <= min(a) <= max(a) <= 1000000000 and 0 <= min(b) <= max(b) <= 1000000000
        data = (f'{n} {m}\n' + ' '.join(map(str, a)) + '\n' + ' '.join(map(str, b)) + '\n').encode()
        length = n + m - 1
        rows = {0, length - 1, length // 2} | {rng.randrange(length) for _ in range(32)}
        checks = {k: min(a[k - j] + b[j] for j in range(max(0, k - n + 1), min(m - 1, k) + 1)) for k in rows}
        result[mode] = data, n, m, checks
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    if args.trials < 1:
        parser.error('trials must be positive')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/monotone-minima-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/monotone_minima.cpp', [f'-DMONOTONE_MINIMA_VARIANT={variant}'])
                for name, variant in [('library', 0), ('recursive', 1), ('smawk', 2)]}
    for identity in [409564, 375997, 202204]:
        programs[str(identity)] = (args.references.resolve() / f'{identity}.cpp', ['-mavx2'] if identity == 409564 else [])
    binaries = {}
    for name, (source, extra) in programs.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-DONLINE_JUDGE', *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    cases = workloads()
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'input_seed': 31674389, 'schedule_seed': 28063,
              'metric': 'wall time including process startup, input, computation, output and exit',
              'input_transport': 'regular file for every program',
              'sources': {name: {'sha256': digest(source), 'extra_flags': extra} for name, (source, extra) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in ['components/monotone_minima.hpp', 'components/smawk.hpp', 'components/fast_io.hpp']},
              'workloads': {}}
    rng = random.Random(28063)
    for mode, (data, n, m, checks) in cases.items():
        input_path = cache / (mode + '.in')
        input_path.write_bytes(data)
        samples, expected = {name: [] for name in binaries}, None
        for trial in range(args.trials + 1):
            names = list(binaries)
            rng.shuffle(names)
            for name in names:
                with input_path.open('rb') as input_file:
                    start = time.perf_counter_ns()
                    run = subprocess.run([binaries[name]], stdin=input_file, capture_output=True, check=True, timeout=60)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                values = list(map(int, run.stdout.split()))
                assert len(values) == n + m - 1, (mode, name, len(values))
                assert all(values[k] == v for k, v in checks.items()), (mode, name)
                checksum = hashlib.sha256((' '.join(map(str, values)) + '\n').encode()).hexdigest()
                if expected is None:
                    expected = checksum
                assert checksum == expected, (mode, name)
                if trial:
                    samples[name].append(elapsed)
        result['workloads'][mode] = {'rows': n + m - 1, 'columns': m, 'convex_length': n,
                                    'input_sha256': hashlib.sha256(data).hexdigest(), 'output_sha256': expected,
                                    'brute_force_checked_rows': sorted(checks),
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
