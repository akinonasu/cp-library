import argparse
from array import array
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def make_input(path, mode, n, seed):
    rng = random.Random(seed)
    labels = list(range(n))
    rng.shuffle(labels)
    edges = []
    for v in range(1, n):
        parent = (v - 1 if mode == 'path' else 0 if mode == 'star' else (v - 1) // 2 if mode == 'balanced'
                  else (v - 1 if v < n // 2 else n // 2 - 1) if mode == 'broom' else rng.randrange(v))
        u, w = labels[parent], labels[v]
        if rng.randrange(2):
            u, w = w, u
        edges.append((u, w, rng.randrange(1, 998244353), rng.randrange(998244353)))
    rng.shuffle(edges)
    with path.open('w') as output:
        output.write(f'{n}\n')
        output.write(' '.join(str(rng.randrange(998244353)) for _ in range(n)) + '\n')
        for edge in edges:
            output.write(' '.join(map(str, edge)) + '\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-cache', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--size', type=int, default=200000)
    args = parser.parse_args()
    if args.size < 1 or args.trials < 1:
        parser.error('size and trials must be positive')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'binary_filesystem': 'Linux /tmp (WSL ext4)', 'schedule_seed': 771306,
              'kernel_seed': 731605, 'end_to_end_seed': 941607,
              'source_sha256': {p: digest(ROOT / p) for p in ('graph/rerooting.hpp', 'components/undirected_csr.hpp',
                  'math/mint.hpp', 'components/fast_io.hpp', 'test/rerooting.test.cpp',
                  'test/benchmark/rerooting.cpp', 'test/benchmark/rerooting.py')},
              'metric': {'kernel': 'in-process wall time including graph construction and DP; excludes generation, result checksum and I/O; library result destruction excluded, baseline work-vector destruction included',
                         'end_to_end': 'process wall time including startup, graph construction, DP, input and output; excludes input generation and output validation'},
              'references': {}, 'kernel': [], 'end_to_end': []}
    rng = random.Random(771306)
    with tempfile.TemporaryDirectory(prefix='cp-rerooting-') as folder:
        folder = Path(folder)
        sources = {'kernel': ROOT / 'test/benchmark/rerooting.cpp', 'library': ROOT / 'test/rerooting.test.cpp'}
        for identity in (346214, 346138):
            source = args.reference_cache / f'{identity}.cpp'
            sources[str(identity)] = source
            result['references'][str(identity)] = {'source_sha256': digest(source), 'adaptation': 'none'}
        binaries = {}
        for name, source in sources.items():
            binary = folder / name
            subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
            binaries[name] = binary
        for mode in ('path', 'star', 'balanced', 'broom', 'random'):
            samples = {name: [] for name in ('library', 'baseline')}
            expected = None
            for trial in range(args.trials + 1):
                names = list(samples)
                rng.shuffle(names)
                for name in names:
                    elapsed, checksum = subprocess.check_output([binaries['kernel'], mode, name, str(args.size)], text=True, timeout=120).split()
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (mode, name)
                    if trial:
                        samples[name].append(float(elapsed))
            item = {'shape': mode, 'size': args.size, 'checksum': expected, 'milliseconds': samples,
                    'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
            result['kernel'].append(item)
            print('kernel', mode, item['median_ms'], flush=True)
        for index, mode in enumerate(('path', 'star', 'balanced', 'broom', 'random')):
            source = folder / 'input.txt'
            make_input(source, mode, args.size, 941607 + index)
            samples = {name: [] for name in ('library', '346214', '346138')}
            expected = None
            for trial in range(args.trials + 1):
                names = list(samples)
                rng.shuffle(names)
                for name in names:
                    with source.open('rb') as input_file, tempfile.TemporaryFile() as output:
                        start = time.perf_counter_ns()
                        subprocess.run([binaries[name]], stdin=input_file, stdout=output, check=True)
                        elapsed = (time.perf_counter_ns() - start) / 1e6
                        output.seek(0)
                        values = array('I', map(int, output.read().split()))
                    assert len(values) == args.size
                    checksum = hashlib.sha256(values.tobytes()).hexdigest()
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (mode, name)
                    if trial:
                        samples[name].append(elapsed)
            item = {'shape': mode, 'size': args.size, 'input_sha256': digest(source), 'checksum_u32le': expected,
                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
            result['end_to_end'].append(item)
            print('end_to_end', mode, item['median_ms'], flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
