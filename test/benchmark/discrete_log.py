import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import statistics
import subprocess
import time


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def workloads():
    rng = random.Random(734762)
    result = {}
    for mode, count in [('prime_yes', 600), ('composite_mixed', 600), ('short_order', 3000),
                        ('even_impossible', 3000), ('full32', 300)]:
        queries = []
        known = []
        for i in range(count):
            if mode == 'prime_yes':
                mod = rng.choice([998244353, 1000000007, 999999937, 985661441])
                a = rng.randrange(2, mod)
                e = rng.randrange(mod)
                b = pow(a, e, mod)
            elif mode == 'composite_mixed':
                mod = rng.randrange(10**8, 10**9)
                a = rng.randrange(mod)
                e = rng.randrange(mod)
                b = pow(a, e, mod) if i % 2 else rng.randrange(mod)
            elif mode == 'short_order':
                mod = rng.randrange(10**8, 10**9)
                a = rng.choice([0, 1, mod - 1])
                e = rng.randrange(3)
                b = pow(a, e, mod)
                if i % 4 == 0:
                    b = 2
                    e = None
            elif mode == 'even_impossible':
                mod = 2 * rng.randrange(10**8, 5 * 10**8)
                a, b = 2 * rng.randrange(mod // 2), rng.randrange(mod // 2) * 2 + 1
                if b == 1:
                    b = 3
                e = None
            else:
                mod = rng.randrange(2**31, 2**32)
                a = rng.randrange(mod)
                e = rng.randrange(mod)
                b = pow(a, e, mod) if i % 2 else rng.randrange(mod)
            queries.append((a, b, mod))
            known.append(e if mode in ['prime_yes', 'short_order'] or i % 2 else None)
        data = (f'{count}\n' + ''.join(f'{a} {b} {m}\n' for a, b, m in queries)).encode()
        result[mode] = (data, queries, known)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    assert args.trials > 0
    os.sched_setaffinity(0, {args.cpu})
    resource.setrlimit(resource.RLIMIT_STACK, (64 * 1024 * 1024, resource.getrlimit(resource.RLIMIT_STACK)[1]))
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/discrete-log-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/discrete_log.cpp', [f'-DDISCRETE_LOG_BASELINE={variant}'])
                for name, variant in [('library', 0), ('unordered_map', 1)]}
    for identity in [361590, 405763]:
        programs[str(identity)] = (args.references.resolve() / f'{identity}.cpp', [])
    binaries = {}
    for name, (source, extra) in programs.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-DONLINE_JUDGE', *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'input_seed': 734762, 'schedule_seed': 482661,
              'metric': 'wall time including process startup, input, computation, output and exit',
              'input_transport': 'regular file for every program',
              'stack_limit_bytes': 64 * 1024 * 1024,
              'sources': {name: digest(source) for name, (source, _) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in
                          ['math/number_theory/discrete_log.hpp', 'components/barrett32.hpp', 'components/fast_io.hpp',
                           'math/number_theory/factorize.hpp', 'math/number_theory/is_prime.hpp', 'components/montgomery.hpp', 'components/splitmix64.hpp']},
              'workloads': {}}
    rng = random.Random(482661)
    for mode, (data, queries, known) in workloads().items():
        path = cache / (mode + '.in')
        path.write_bytes(data)
        expected = None
        selected = list(binaries) if mode != 'full32' else ['library', 'unordered_map']
        samples = {name: [] for name in selected}
        for trial in range(args.trials + 1):
            names = selected.copy()
            rng.shuffle(names)
            for name in names:
                with path.open('rb') as input_file:
                    start = time.perf_counter_ns()
                    run = subprocess.run([binaries[name]], stdin=input_file, capture_output=True, check=True, timeout=120)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                values = list(map(int, run.stdout.split()))
                assert len(values) == len(queries), (mode, name)
                for value, (a, b, mod), e in zip(values, queries, known):
                    assert value == -1 or 0 <= value < mod and pow(a, value, mod) == b, (mode, name, a, b, mod, value)
                    if e is not None:
                        assert 0 <= value <= e
                if expected is None:
                    expected = values
                assert values == expected, (mode, name)
                if trial:
                    samples[name].append(elapsed)
        result['workloads'][mode] = {'queries': len(queries), 'input_sha256': hashlib.sha256(data).hexdigest(),
                                    'output_sha256': hashlib.sha256(('\n'.join(map(str, expected)) + '\n').encode()).hexdigest(),
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
