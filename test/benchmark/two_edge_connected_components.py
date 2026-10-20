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
    n = 200000
    rng = random.Random(891333)
    yield 'path', n, [(v - 1, v) for v in range(1, n)]
    yield 'star', n, [(0, v) for v in range(1, n)]
    yield 'cycle', n, [(v, (v + 1) % n) for v in range(n)]
    yield 'random', n, [(rng.randrange(n), rng.randrange(n)) for _ in range(n)]
    edges = [(v + a, v + b) for v in range(0, 150000, 3) for a, b in [(0, 1), (1, 2), (2, 0)]]
    edges += [(v - 1, v) for v in range(3, 150000, 3)]
    yield 'triangles_chain', 150000, edges
    yield 'parallel', n, [(v, v + 1) for v in range(0, n, 2) for _ in range(2)]
    yield 'loops', n, [(v, v) for v in range(n)]


def canonical(output, n):
    values = list(map(int, output.split()))
    assert values
    count, offset = values[0], 1
    labels = [-1] * n
    for _ in range(count):
        size = values[offset]
        assert size > 0
        members = values[offset + 1:offset + size + 1]
        assert len(members) == size
        label = min(members)
        for v in members:
            assert 0 <= v < n and labels[v] == -1
            labels[v] = label
        offset += size + 1
    assert offset == len(values) and all(v >= 0 for v in labels)
    return hashlib.sha256((' '.join(map(str, labels))).encode()).hexdigest()


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
    cache = ROOT / '.verify-helper/cache/two-edge-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/two_edge_connected_components.cpp', [f'-DTWO_EDGE_BASELINE={variant}'])
                for name, variant in [('library', 0), ('low_link_dsu', 1)]}
    for identity in [305681, 215566]:
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
              'stack_limit_bytes': 64 * 1024 * 1024,
              'input_seed': 891333, 'schedule_seed': 368931,
              'metric': 'wall time including process startup, input, construction, output and exit',
              'input_transport': 'regular file for every program',
              'sources': {name: digest(source) for name, (source, _) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in
                          ['graph/connectivity/two_edge_connected_components.hpp', 'graph/connectivity/low_link.hpp', 'components/undirected_csr.hpp',
                           'components/default_init_allocator.hpp', 'components/fast_io.hpp',
                           'test/benchmark/two_edge_connected_components_baseline.hpp']},
              'workloads': {}}
    rng = random.Random(368931)
    for mode, n, edges in workloads():
        data = (f'{n} {len(edges)}\n' + ''.join(f'{u} {v}\n' for u, v in edges)).encode()
        path = cache / (mode + '.in')
        path.write_bytes(data)
        expected = None
        samples = {name: [] for name in binaries}
        for trial in range(args.trials + 1):
            names = list(binaries)
            rng.shuffle(names)
            for name in names:
                with path.open('rb') as input_file:
                    start = time.perf_counter_ns()
                    run = subprocess.run([binaries[name]], stdin=input_file, capture_output=True, check=True, timeout=60)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                checksum = canonical(run.stdout, n)
                if expected is None:
                    expected = checksum
                assert checksum == expected, (mode, name)
                if trial:
                    samples[name].append(elapsed)
        memory = {}
        for name in binaries:
            log = cache / (name + '.memory')
            with path.open('rb') as input_file:
                subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(log), binaries[name]],
                               stdin=input_file, stdout=subprocess.DEVNULL, check=True)
            memory[name] = int(log.read_text())
        result['workloads'][mode] = {'n': n, 'm': len(edges), 'input_sha256': hashlib.sha256(data).hexdigest(),
                                    'partition_sha256': expected, 'peak_rss_kib': memory,
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, memory, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
