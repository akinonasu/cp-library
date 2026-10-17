import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-cache', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['data_structure/generic_segtree_beats.hpp', 'data_structure/segtree_beats.hpp',
             'components/default_init_allocator.hpp', 'test/fixtures/beats.hpp',
             'test/benchmark/generic_beats.cpp', 'test/benchmark/generic_beats.py']
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'schedule_seed': 593710, 'input_seed': 814239, 'binary_filesystem': 'Linux /tmp (WSL ext4)',
              'source_sha256': {path: digest(ROOT / path) for path in paths},
              'reference': {'id': 204482, 'source_sha256': digest(args.reference_cache / 'reference-source.cpp'),
                            'adapter_sha256': digest(args.reference_cache / 'reference.hpp'),
                            'adaptation': 'test/benchmark/segtree_beats_reference.py; extracted core and half-open range adapter; assignment excluded'},
              'metric': {'build': 'tree construction including conversion to S and temporary destruction',
                         'queries': 'in-process operations and checksum; excludes input generation, I/O and final object destruction'},
              'cases': []}
    rng = random.Random(593710)
    with tempfile.TemporaryDirectory(prefix='cp-generic-beats-') as folder:
        binary = Path(folder) / 'kernel'
        subprocess.run(['g++', *flags, '-I', str(ROOT), '-isystem', str(args.reference_cache),
                        str(ROOT / 'test/benchmark/generic_beats.cpp'), '-o', str(binary)], check=True)
        for workload in ('mixed', 'short', 'assign', 'uniform', 'division_mixed', 'division_short', 'division_decay'):
            division = workload.startswith('division')
            n, q = (500000, 100000) if division else (200000, 200000)
            methods = ['generic', 'uniform'] if division else ['generic', 'dedicated'] + ([] if workload == 'assign' else ['reference'])
            samples = {method: {'build_ms': [], 'query_ms': []} for method in methods}
            expected = None
            for trial in range(args.trials + 1):
                order = methods.copy()
                rng.shuffle(order)
                for method in order:
                    build, elapsed, checksum = subprocess.check_output([str(binary), workload, method, str(n), str(q)], text=True, timeout=120).split()
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (workload, method)
                    if trial:
                        samples[method]['build_ms'].append(float(build))
                        samples[method]['query_ms'].append(float(elapsed))
            item = {'workload': workload, 'n': n, 'q': q, 'checksum': expected, 'milliseconds': samples,
                    'median_ms': {method: {metric: statistics.median(values) for metric, values in metrics.items()}
                                  for method, metrics in samples.items()}}
            result['cases'].append(item)
            print(workload, item['median_ms'], flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
