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
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'binary_filesystem': 'Linux /tmp (WSL ext4)', 'schedule_seed': 513709, 'input_seed': 612409,
              'source_sha256': {p: digest(ROOT / p) for p in ('tree/auxiliary_tree.hpp', 'tree/lca.hpp',
                  'tree/hld.hpp', 'data_structure/range_query/sparse_table.hpp', 'test/benchmark/auxiliary_tree.cpp',
                  'test/benchmark/auxiliary_tree.py')},
              'metric': 'in-process construction and checksum of every result, including first allocations; excludes preprocessing, generation, presorting for sorted methods, I/O and final object destruction',
              'cases': []}
    rng = random.Random(513709)
    with tempfile.TemporaryDirectory(prefix='cp-virtual-tree-') as directory:
        binary = Path(directory) / 'kernel'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / 'test/benchmark/auxiliary_tree.cpp'), '-o', str(binary)], check=True)
        for shape in ('path', 'star', 'balanced', 'random'):
            for k, q in ((2, 300000), (32, 20000), (1024, 1000), (100000, 10)):
                samples = {name: [] for name in ('library', 'baseline', 'library_sorted', 'baseline_sorted', 'hld')}
                checksums = {}
                for trial in range(args.trials + 1):
                    names = list(samples)
                    rng.shuffle(names)
                    for name in names:
                        backend = 'hld' if name == 'hld' else 'lca'
                        method = 'library' if name == 'hld' else name
                        elapsed, checksum = subprocess.check_output([str(binary), shape, method, '200000', str(k), str(q), backend], text=True, timeout=120).split()
                        if checksums:
                            assert checksum == next(iter(checksums.values())), (shape, k, name)
                        if backend in checksums:
                            assert checksums[backend] == checksum, (shape, k, name)
                        else:
                            checksums[backend] = checksum
                        if trial:
                            samples[name].append(float(elapsed))
                item = {'shape': shape, 'n': 200000, 'k': k, 'queries': q, 'checksums': checksums,
                        'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
                result['cases'].append(item)
                print(shape, k, item['median_ms'], flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
