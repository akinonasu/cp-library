import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    cache = ROOT / '.verify-helper/cache/mod-linear-partition-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/mod_linear_partition.cpp'
    binary = cache / 'benchmark'
    subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
    rng = random.Random(78419)
    maximum = (1 << 64) - 1
    def random_cases(q, n):
        return [(n, m, rng.randrange(m), rng.randrange(m)) for m in [rng.randrange(1, 10**9+1) for _ in range(q)]]
    workloads = {
        'small': random_cases(50000, 64), 'medium': random_cases(1000, 10**6),
        'large': random_cases(100, 10**9),
        'simple': [(10**9, 10**9-7, a, 10**9-8) for a in [0, 1, 10**9-8] for _ in range(100)],
        'uint64': [(maximum, maximum, a, b) for a in [0, 1, maximum-1] for b in [0, maximum-1] for _ in range(100)],
    }
    result = {'platform': platform.platform(),
              'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              'flags': flags, 'cpu_affinity': [2], 'trials': 7, 'seed': 78419,
              'source_hash_normalization': 'CRLF to LF',
              'metric': 'wall time including startup, input, partition, allocation, checksum, output and exit',
              'sources': {name: sha(ROOT/name) for name in ['test/benchmark/mod_linear_partition.cpp',
                  'test/benchmark/mod_linear_partition_baseline.hpp', 'math/floor_sum/partition_mod_of_linear.hpp',
                  'math/floor_sum/prefix_min_of_mod_of_linear.hpp', 'components/mod_linear_prefix_min.hpp',
                  'components/fast_io.hpp']}, 'workloads': {}}
    for name, cases in workloads.items():
        data = (str(len(cases)) + '\n' + ''.join(' '.join(map(str, case)) + '\n' for case in cases)).encode()
        programs = ['callback', 'vector'] + ([] if name == 'uint64' else ['baseline'])
        samples = {p: [] for p in programs}
        expected = None
        for program in programs:
            output = subprocess.check_output([str(binary), program], input=data, timeout=30)
            if expected is None:
                expected = output
            assert output == expected, (name, program)
        schedule = programs*7
        rng.shuffle(schedule)
        for program in schedule:
            start = time.perf_counter()
            output = subprocess.check_output([str(binary), program], input=data, timeout=30)
            samples[program].append((time.perf_counter()-start)*1000)
            assert output == expected
        item = {'queries': len(cases), 'input_sha256': hashlib.sha256(data).hexdigest(),
                'output_sha256': hashlib.sha256(expected).hexdigest(),
                'programs': {p: {'milliseconds': values, 'median_ms': statistics.median(values)} for p, values in samples.items()}}
        result['workloads'][name] = item
        print(name, {p: round(v['median_ms'], 3) for p, v in item['programs'].items()}, flush=True)
    (args.output/'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
