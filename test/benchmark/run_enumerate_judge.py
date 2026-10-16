import argparse
import csv
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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tests', type=Path, required=True)
    parser.add_argument('--reference-cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DNDEBUG']
    reference = args.reference_cache / '406234.cpp'
    sources = {'current': ROOT / 'test/run_enumerate.test.cpp', 'reference_avx2': reference}
    paths = ['components/sequence_match.hpp', 'components/default_init_allocator.hpp', 'components/fast_io.hpp',
             'string/run_enumerate.hpp', 'test/run_enumerate.test.cpp', 'test/benchmark/run_enumerate_judge.py']
    environment = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
                   'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu], 'trials': args.trials,
                   'timing': 'subprocess wall time, input/output as regular files; process launch and I/O included',
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
                   'reference_sha256': hashlib.sha256(reference.read_bytes()).hexdigest()}
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-run-judge-') as temporary:
        directory = Path(temporary)
        binaries = {}
        for name, source in sources.items():
            binary = directory / name
            subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
            binaries[name] = binary
        order_random = random.Random(32947)
        for source in sorted(args.tests.glob('*.in')):
            expected = source.with_suffix('.out').read_bytes().split()
            samples = {name: [] for name in sources}
            for trial in range(args.trials):
                order = list(sources)
                order_random.shuffle(order)
                for name in order:
                    output = directory / 'output'
                    with source.open('rb') as input_stream, output.open('wb') as output_stream:
                        begin = time.perf_counter()
                        subprocess.run([str(binaries[name])], stdin=input_stream, stdout=output_stream, check=True)
                        seconds = time.perf_counter() - begin
                    if output.read_bytes().split() != expected:
                        raise RuntimeError(f'Wrong answer: {source.name} {name}')
                    rows.append({'case': source.stem, 'variant': name, 'trial': trial, 'seconds': seconds})
                    samples[name].append(seconds)
            for name, values in samples.items():
                row = {'case': source.stem, 'variant': name, 'median_ms': statistics.median(values) * 1000,
                       'min_ms': min(values) * 1000, 'max_ms': max(values) * 1000}
                summary.append(row)
                print(json.dumps(row), flush=True)
    if not rows:
        raise RuntimeError('No official test cases found')
    for name, values in [('samples', rows), ('summary', summary)]:
        with (args.output / f'{name}.csv').open('w', newline='') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(values[0]))
            writer.writeheader()
            writer.writerows(values)


if __name__ == '__main__':
    main()
