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

from dynamic_connectivity_variants import variants, generate

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--n', type=int, default=100000)
    parser.add_argument('--q', type=int, default=300000)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--variants', nargs='+', default=list(variants()))
    parser.add_argument('--kinds', nargs='+', default=['forest', 'cycle', 'dense', 'star', 'query'])
    parser.add_argument('--modes', nargs='+', type=int, default=[1, 0])
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=gnu++23', '-O2', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE']
    paths = ['graph/connectivity/dynamic_connectivity.hpp', 'components/splitmix64.hpp',
             'test/benchmark/dynamic_connectivity_baseline.hpp', 'test/benchmark/dynamic_connectivity.cpp',
             'test/benchmark/dynamic_connectivity_variants.py', 'test/benchmark/dynamic_connectivity.py']
    environment = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
                   'affinity': [args.cpu], 'n': args.n, 'q': args.q, 'trials': args.trials, 'seed': 713,
                   'hash_seeds': [713 + i * 101 for i in range(args.trials)],
                   'variants': args.variants, 'kinds': args.kinds, 'modes': args.modes,
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
                   'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True)}
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-dynamic-connectivity-') as temporary:
        directory = Path(temporary)
        generate(directory)
        includes = ''.join(f'#include "{name}.hpp"\n' for name in args.variants)
        declarations = ('struct Input;\nint dispatch(const Input &, int, const std::string &, const std::string &, bool);\n')
        (directory / 'variants.hpp').write_text('#include <string>\n#include <cstdint>\ninline std::uint64_t dynamic_connectivity_benchmark_seed = 713;\n' + includes + declarations)
        source = (ROOT / 'test/benchmark/dynamic_connectivity.cpp').read_text()
        source += '\nint dispatch(const Input &input, int n, const std::string &name, const std::string &kind, bool aggregate) {\n'
        for name in args.variants:
            source += f'if (name == "{name}") {{ if (aggregate) run<DynamicConnectivity_{name}, true>(input, n, name, kind); else run<DynamicConnectivity_{name}, false>(input, n, name, kind); return 0; }}\n'
        source += 'return 1;\n}\n'
        (directory / 'benchmark.cpp').write_text(source)
        binary = directory / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(directory), str(directory / 'benchmark.cpp'), '-o', str(binary)], check=True)
        order_random = random.Random(32947)
        for mode in args.modes:
            for kind in args.kinds:
                samples = {name: [] for name in args.variants}
                expected = None
                for trial in range(args.trials):
                    order = args.variants.copy()
                    order_random.shuffle(order)
                    for name in order:
                        row = json.loads(subprocess.check_output([str(binary), name, kind, str(args.n), str(args.q), str(mode), str(713 + trial * 101)], text=True))
                        if expected is None:
                            expected = row['digest']
                        if row['digest'] != expected:
                            raise RuntimeError(f'Wrong answer: {mode} {kind} {name}')
                        row['trial'] = trial
                        rows.append(row)
                        samples[name].append(row['seconds'])
                for name, values in samples.items():
                    row = {'kind': kind, 'aggregate': mode, 'variant': name, 'median_ms': statistics.median(values) * 1000,
                           'min_ms': min(values) * 1000, 'max_ms': max(values) * 1000}
                    summary.append(row)
                    print(json.dumps(row), flush=True)
    for name, values in [('samples', rows), ('summary', summary)]:
        with (args.output / f'{name}.csv').open('w', newline='') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(values[0]))
            writer.writeheader()
            writer.writerows(values)


if __name__ == '__main__':
    main()
