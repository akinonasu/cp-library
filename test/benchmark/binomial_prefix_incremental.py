import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/arc190-b'
OUT = ROOT / 'test/benchmark/results/binomial_prefix_incremental'
BASE = '555b183'


def run(command, **kwargs):
    return subprocess.run(command, cwd=ROOT, check=True, **kwargs)


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--quick', action='store_true')
    parser.add_argument('--repeats', type=int, default=7)
    args = parser.parse_args()
    CACHE.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    for name in ['components/binomial_prefix_core.hpp', 'components/fps_factorials.hpp', 'math/mint.hpp']:
        original_component = subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=ROOT)
        assert original_component.replace(b'\r\n', b'\n') == (ROOT / name).read_bytes().replace(b'\r\n', b'\n'), name
    old = subprocess.check_output(['git', 'show', BASE + ':math/combinatorics/binomial_prefix_sum_incremental.hpp'], cwd=ROOT, text=True)
    old = old.replace('BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP', 'BASELINE_BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP')
    old = old.replace('IncrementalBinomialPrefixSum', 'BaselineIncrementalBinomialPrefixSum').replace('../../components/', 'components/')
    (CACHE / 'baseline.hpp').write_text(old)
    report = {'base': BASE, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': list(os.uname()), 'cpu': subprocess.check_output(['lscpu'], text=True),
              'cpu_pin': 2, 'seed': 20261020, 'repeats': args.repeats, 'micro': []}
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-I', str(ROOT)]
    configurations = {'generic': [], 'native': ['-march=native', '-ftrivial-auto-var-init=zero']}
    if args.quick:
        configurations = {'native': configurations['native']}
    randomizer = random.Random(20261020)
    for name, extra in configurations.items():
        binaries = {}
        sources = {'micro': ROOT / 'test/benchmark/binomial_prefix_incremental.cpp'}
        for mode, source in sources.items():
            binary = CACHE / (mode + '-' + name)
            run(['g++', *flags, *extra, str(source), '-o', str(binary)])
            binaries[mode] = str(binary)
        for shape in ['rows', 'diagonal', 'columns', 'walk', 'jumps', 'arc190']:
            n = 10000 if shape == 'jumps' else 1000000
            values = {mode: [] for mode in ['baseline', 'current', 'shared']}
            checksum = None
            for trial in range(-1, args.repeats):
                modes = list(values)
                randomizer.shuffle(modes)
                for mode in modes:
                    parts = subprocess.check_output(['taskset', '-c', '2', binaries['micro'], mode, shape, str(n)], text=True).split()
                    if checksum is None:
                        checksum = parts[2]
                    assert checksum == parts[2], (shape, mode, parts)
                    if trial >= 0:
                        values[mode].append({'build_ms': float(parts[0]), 'query_ms': float(parts[1]), 'peak_kib': int(parts[3])})
            report['micro'].append({'configuration': name, 'flags': flags + extra, 'shape': shape, 'n': n, 'values': values})
            print(name, shape, {m: round(statistics.median(x['query_ms'] for x in v), 3) for m, v in values.items()}, flush=True)
    sources = ['math/combinatorics/binomial_prefix_sum_incremental.hpp', 'components/binomial_prefix_core.hpp',
               'components/fps_factorials.hpp', 'math/mint.hpp', 'test/unit/binomial_prefix_sum_incremental.cpp',
               'test/benchmark/binomial_prefix_incremental.cpp', 'test/benchmark/binomial_prefix_incremental.py']
    report['sources'] = {name: sha(ROOT / name) for name in sources}
    (OUT / 'micro_measurements.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
