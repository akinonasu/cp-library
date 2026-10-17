import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline', default='22c50c36e5066c07c2e6c34ab2c81a1896ce53ae')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--size', type=int, default=100000)
    parser.add_argument('--queries', type=int, default=100000)
    args = parser.parse_args()
    if min(args.trials, args.size) < 1 or args.queries < 0:
        parser.error('invalid size')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/dynamic-wavelet-count-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    headers = ['data_structure/dynamic_wavelet_matrix.hpp', 'components/dynamic_bit_vector.hpp']
    for name in headers:
        path = cache / 'baseline' / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(subprocess.check_output(['git', 'show', args.baseline + ':' + name], cwd=ROOT))
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/dynamic_wavelet_count.cpp'
    binaries = {}
    extras = {'before': [f'-DDYNAMIC_WAVELET_COUNT_HEADER="{cache / "baseline" / headers[0]}"'], 'after': []}
    for name, extra in extras.items():
        binary = cache / name
        subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'input_seed': 18106471, 'schedule_seed': 28063,
              'metric': 'in-process wall time for queries and interleaved updates; excludes generation, construction, final validation, I/O',
              'baseline': args.baseline, 'extra_flags': extras, 'source_sha256': digest(source),
              'headers': {name: {'before': digest(cache / 'baseline' / name), 'after': digest(ROOT / name)} for name in headers},
              'workloads': {}}
    rng = random.Random(28063)
    for mode in ['random20', 'short20', 'single20', 'full64', 'missing64', 'narrow64', 'duplicates64', 'uniform64', 'mixed20']:
        samples = {name: [] for name in ['before_count', 'after_count', 'range_freq']}
        expected = None
        for trial in range(args.trials + 1):
            names = list(samples)
            rng.shuffle(names)
            for name in names:
                binary = binaries['before' if name == 'before_count' else 'after']
                output = subprocess.check_output([binary, mode, 'range_freq' if name == 'range_freq' else 'count',
                                                  str(args.size), str(args.queries)], text=True, timeout=120)
                elapsed, checksum = output.split()
                if expected is None:
                    expected = checksum
                assert checksum == expected, (mode, name, checksum, expected)
                if trial:
                    samples[name].append(float(elapsed))
        result['workloads'][mode] = {'initial_size': args.size, 'operations': args.queries, 'checksum': expected,
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
