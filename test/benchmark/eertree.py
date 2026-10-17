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


def workloads(n):
    rng = random.Random(393513)
    zimin = b'a'
    for c in range(ord('b'), ord('z') + 1):
        zimin += bytes([c]) + zimin
        if len(zimin) >= n:
            break
    return {'uniform': b'a' * n, 'alternating': (b'ab' * ((n + 1) // 2))[:n],
            'random2': bytes(rng.randrange(2) + 97 for _ in range(n)),
            'random26': bytes(rng.randrange(26) + 97 for _ in range(n)),
            'zimin': zimin[:n],
            'long_fallback': b'a' * (n - 1) + b'b'}


def canonical(output):
    return hashlib.sha256(b' '.join(output.split())).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--size', type=int, default=1000000)
    args = parser.parse_args()
    assert args.trials > 0 and 1 <= args.size <= 1000000
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/eertree-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/eertree.cpp', [f'-DEERTREE_DENSE={variant}'])
                for name, variant in [('library', 0), ('dense26', 1)]}
    for identity in [406240, 400577]:
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
              'input_seed': 393513, 'schedule_seed': 458333,
              'metric': 'wall time including process startup, input, construction, output and exit',
              'input_transport': 'regular file for every program',
              'sources': {name: digest(source) for name, (source, _) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in
                          ['string/eertree.hpp', 'components/fast_io.hpp', 'test/benchmark/eertree_baseline.hpp']},
              'workloads': {}}
    rng = random.Random(458333)
    for mode, text in workloads(args.size).items():
        data = text + b'\n'
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
                checksum = canonical(run.stdout)
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
        result['workloads'][mode] = {'length': len(text), 'input_sha256': hashlib.sha256(data).hexdigest(),
                                    'output_sha256': expected, 'peak_rss_kib': memory,
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, memory, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
