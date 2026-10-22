import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.verify-helper'))
from atcoder_flags import constexpr_flags


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--problem', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=min(os.sched_getaffinity(0)))
    parser.add_argument('--repeats', type=int, default=7)
    args = parser.parse_args()
    args.references = args.references.resolve()
    args.problem = args.problem.resolve()
    report = {'utc': datetime.now(timezone.utc).isoformat(), 'cpu_pin': args.cpu, 'repeats': args.repeats,
              'cpu': subprocess.check_output(['lscpu'], text=True), 'seed': 20261022, 'checks': [], 'kernel': [], 'process': []}
    compilers = {name: subprocess.check_output([name, '--version'], text=True).splitlines()[0] for name in ('g++', 'clang++')}
    report['compilers'] = compilers
    flags = ['-std=c++23', '-O2', '-I', str(ROOT)]
    report['flags'] = {name: flags + constexpr_flags(name) for name in compilers}
    known = {10**6: 78498, 10**8: 5761455, 10**10: 455052511, 10**11: 4118054813,
             10**12: 37607912018, 10**13: 346065536839}
    os.sched_setaffinity(0, {args.cpu})
    with tempfile.TemporaryDirectory(prefix='prime-count-bench-') as directory:
        work = Path(directory)
        programs = {}
        for compiler in compilers:
            binary = work / compiler
            subprocess.run([compiler, *flags, *constexpr_flags(compiler), str(ROOT / 'test/prime_count.test.cpp'), '-o', str(binary)], check=True)
            programs[compiler] = binary
        kernel = work / 'kernel'
        subprocess.run(['g++', *flags, *constexpr_flags('g++'), str(ROOT / 'test/benchmark/prime_count.cpp'), '-o', str(kernel)], check=True)
        for identity in (377988, 222872):
            source = args.references / (str(identity) + '.cpp')
            binary = work / str(identity)
            subprocess.run(['g++', *flags, '-DONLINE_JUDGE', str(source), '-o', str(binary)], check=True)
            programs[str(identity)] = binary
        for path in sorted((args.problem / 'in').glob('*.in')):
            expected = (args.problem / 'out' / path.with_suffix('.out').name).read_bytes().split()
            for compiler in compilers:
                actual = subprocess.check_output([programs[compiler]], input=path.read_bytes()).split()
                assert actual == expected, (compiler, path, actual, expected)
            report['checks'].append({'case': path.name, 'input_sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                                     'expected': expected[0].decode()})
        if not report['checks']:
            raise ValueError('Library Checker cases must be generated first')
        randomizer = random.Random(report['seed'])
        for n, expected in known.items():
            values = {'baseline': [], 'current': []}
            for trial in range(-1, args.repeats):
                order = list(values)
                randomizer.shuffle(order)
                for mode in order:
                    result = subprocess.check_output([kernel, mode, str(n)], text=True).split()
                    assert int(result[0]) == expected, (mode, n, result)
                    if trial >= 0:
                        values[mode].append(float(result[1]))
            report['kernel'].append({'n': n, 'values_ms': values, 'median_ms': {k: statistics.median(v) for k, v in values.items()}})
            print(n, report['kernel'][-1]['median_ms'], flush=True)
        for n in (10**6, 10**10, 10**11):
            values = {name: [] for name in ('g++', '377988', '222872')}
            for trial in range(-1, args.repeats):
                order = list(values)
                randomizer.shuffle(order)
                for name in order:
                    start = time.perf_counter()
                    output = subprocess.check_output([programs[name]], input=f'{n}\n'.encode())
                    elapsed = (time.perf_counter() - start) * 1000
                    assert int(output) == known[n], (name, n, output)
                    if trial >= 0:
                        values[name].append(elapsed)
            report['process'].append({'n': n, 'values_ms': values, 'median_ms': {k: statistics.median(v) for k, v in values.items()}})
        measured = [ROOT / name for name in ('math/number_theory/prime_count.hpp', 'test/unit/prime_count.cpp',
                                              'test/prime_count.test.cpp', 'test/benchmark/prime_count.cpp', 'test/benchmark/prime_count.py')]
        measured += [args.references / (str(identity) + '.cpp') for identity in (377988, 222872)]
        report['source_sha256'] = {str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in measured}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
