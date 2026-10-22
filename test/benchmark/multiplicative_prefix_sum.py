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
    parser.add_argument('--checker-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=5)
    parser.add_argument('--cpu', type=int, default=min(os.sched_getaffinity(0)))
    args = parser.parse_args()
    references, checker = args.references.resolve(), args.checker_root.resolve()
    flags = ['-std=c++23', '-O2', '-I', str(ROOT)]
    report = {'utc': datetime.now(timezone.utc).isoformat(), 'flags': flags, 'constexpr': constexpr_flags('g++'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True), 'cpu': subprocess.check_output(['lscpu'], text=True),
              'cpu_pin': args.cpu, 'repeats': args.repeats, 'seed': 20261022, 'kernel': [], 'process': [], 'totient': []}
    os.sched_setaffinity(0, {args.cpu})
    sources = {'kernel': ROOT / 'test/benchmark/multiplicative_prefix_sum.cpp',
               'current': ROOT / 'test/multiplicative_prefix_sum.test.cpp',
               'totient': ROOT / 'test/multiplicative_prefix_sum_totient.test.cpp',
               '400957': references / '400957.cpp', '405757': references / '405757.cpp', '199449': references / '199449.cpp',
               'min25': checker / 'number_theory/sum_of_multiplicative_function/sol/min_25_sieve.cpp'}
    report['source_sha256'] = {str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sources.values()}
    randomizer = random.Random(report['seed'])
    with tempfile.TemporaryDirectory(prefix='multiplicative-bench-') as directory:
        binaries = {name: str(Path(directory) / name) for name in sources}
        for name, source in sources.items():
            limits = constexpr_flags('g++') if name in ('kernel', 'current', 'totient') else []
            subprocess.run(['g++', *flags, *limits, str(source), '-o', binaries[name]], check=True)
        for n in (10**6, 10**8, 10**10, 10**11, 10**12):
            reference = None
            values = {'scalar': [], 'table': []}
            for trial in range(-1, args.repeats):
                order = list(values)
                randomizer.shuffle(order)
                for mode in order:
                    answer = subprocess.check_output([binaries['kernel'], mode, str(n)], text=True).split()
                    if reference is None:
                        reference = int(answer[0])
                    assert int(answer[0]) == reference
                    if trial >= 0:
                        values[mode].append([float(x) for x in answer[1:]])
            medians = {mode: statistics.median(sum(row) for row in rows) for mode, rows in values.items()}
            report['kernel'].append({'n': n, 'answer': reference, 'values_ms': values, 'median_total_ms': medians})
            print('kernel', n, medians, flush=True)
        large = subprocess.check_output([binaries['405757']], input=b'1\n1000000000000 7 11\n', stderr=subprocess.DEVNULL)
        assert int(large) == report['kernel'][-1]['answer']
        report['large_reference_check'] = {'submission': 405757, 'n': 10**12, 'answer': int(large), 'status': 'passed'}
        for n in (10**8, 10**10, 10**11):
            values = {name: [] for name in ('current', 'min25', '400957')}
            reference = next(row['answer'] for row in report['kernel'] if row['n'] == n)
            for trial in range(-1, args.repeats):
                order = list(values)
                randomizer.shuffle(order)
                for name in order:
                    start = time.perf_counter()
                    answer = subprocess.check_output([binaries[name]], input=f'1\n{n} 7 11\n'.encode(), stderr=subprocess.DEVNULL)
                    elapsed = (time.perf_counter() - start) * 1000
                    assert int(answer) == reference, (name, n, answer, reference)
                    if trial >= 0:
                        values[name].append(elapsed)
            report['process'].append({'n': n, 'values_ms': values, 'median_ms': {name: statistics.median(rows) for name, rows in values.items()}})
        for n in (10**8, 10**10):
            values = {name: [] for name in ('totient', '199449')}
            reference = None
            for trial in range(-1, args.repeats):
                order = list(values)
                randomizer.shuffle(order)
                for name in order:
                    start = time.perf_counter()
                    answer = subprocess.check_output([binaries[name]], input=f'{n}\n'.encode())
                    elapsed = (time.perf_counter() - start) * 1000
                    if reference is None:
                        reference = int(answer)
                    assert int(answer) == reference
                    if trial >= 0:
                        values[name].append(elapsed)
            report['totient'].append({'n': n, 'answer': reference, 'values_ms': values, 'median_ms': {name: statistics.median(rows) for name, rows in values.items()}})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
