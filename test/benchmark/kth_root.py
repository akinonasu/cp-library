import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


ROOT = Path(__file__).resolve().parents[2]
MAXIMUM = (1 << 64) - 1


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def root(n, k):
    if k == 1 or n <= 1:
        return n
    if k == 2:
        return math.isqrt(n)
    r = int(n ** (1 / k))
    while r ** k > n:
        r -= 1
    while (r + 1) ** k <= n:
        r += 1
    return r


def workloads(size):
    rng = random.Random(64329614)
    boundaries = []
    for k in range(2, 65):
        top = root(MAXIMUM, k)
        for i in range(200):
            r = max(1, top - i) if i < 100 else rng.randint(1, top)
            p = r ** k
            for n in [p - 1, p, p + 1]:
                if n <= MAXIMUM:
                    boundaries.append((n, k))
    result = {}
    for mode in ['mixed', 'small_k', 'square', 'cube', 'boundaries']:
        queries = []
        for i in range(size):
            if mode == 'boundaries':
                n, k = rng.choice(boundaries)
            else:
                k = 2 if mode == 'square' else 3 if mode == 'cube' else rng.randint(2, 15) if mode == 'small_k' else rng.randint(1, 64)
                n = rng.getrandbits(64 if i % 4 else rng.randint(0, 64))
            queries.append((n, k))
        data = (str(size) + '\n' + ''.join(f'{n} {k}\n' for n, k in queries)).encode()
        expected = [root(n, k) for n, k in queries]
        result[mode] = data, expected
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--size', type=int, default=500000)
    args = parser.parse_args()
    if args.trials < 1 or args.size < 1:
        parser.error('trials and size must be positive')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/kth-root-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/kth_root.cpp', [f'-DKTH_ROOT_VARIANT={variant}'])
                for name, variant in [('library', 0), ('binary', 1), ('pow_corrected', 2)]}
    for identity in [229185, 209944, 393720]:
        programs[str(identity)] = (args.references.resolve() / f'{identity}.cpp',
                                  ['-UNDEBUG', '-Wno-template-body'] if identity == 229185 else
                                  ['-mavx2'] if identity == 393720 else [])
    binaries = {}
    for name, (source, extra) in programs.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-DONLINE_JUDGE', *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    cases = workloads(args.size)
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'input_seed': 64329614, 'schedule_seed': 28063,
              'metric': 'wall time including process startup, input, computation, output and exit',
              'input_transport': 'regular file for every program',
              'sources': {name: {'sha256': digest(source), 'extra_flags': extra} for name, (source, extra) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in ['math/number_theory/kth_root.hpp', 'components/fast_io.hpp']},
              'workloads': {}}
    rng = random.Random(28063)
    for mode, (data, expected) in cases.items():
        input_path = cache / (mode + '.in')
        input_path.write_bytes(data)
        samples = {name: [] for name in binaries}
        for trial in range(args.trials + 1):
            names = list(binaries)
            rng.shuffle(names)
            for name in names:
                with input_path.open('rb') as input_file:
                    start = time.perf_counter_ns()
                    run = subprocess.run([binaries[name]], stdin=input_file, capture_output=True, check=True, timeout=60)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                assert list(map(int, run.stdout.split())) == expected, (mode, name)
                if trial:
                    samples[name].append(elapsed)
        result['workloads'][mode] = {'queries': len(expected), 'input_sha256': hashlib.sha256(data).hexdigest(),
                                    'output_sha256': hashlib.sha256(('\n'.join(map(str, expected)) + '\n').encode()).hexdigest(),
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
