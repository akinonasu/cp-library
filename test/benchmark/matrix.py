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

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--quick', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['components/matrix_arithmetic.hpp', 'math/matrix.hpp', 'math/mint.hpp', 'test/benchmark/matrix.cpp', 'test/benchmark/matrix.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags, 'seed': 713,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'affinity': [args.cpu], 'trials': args.trials, 'sha256': {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    cases = [('product', 'dense', 64, 64, 64), ('product', 'dense', 256, 256, 256), ('product', 'dense', 512, 512, 512),
             ('product', 'rectangular', 32, 512, 65), ('det', 'dense', 512, 0, 512), ('det', 'singular', 512, 0, 512),
             ('inverse', 'dense', 384, 0, 384), ('rank', 'rectangular', 256, 0, 512), ('solve', 'rectangular', 256, 0, 512),
             ('characteristic', 'dense', 256, 0, 256), ('adjugate', 'singular', 192, 0, 192), ('pow', 'dense', 64, 0, 64)]
    if args.quick:
        cases = [cases[0], cases[1], ('det', 'dense', 256, 0, 256), ('inverse', 'dense', 128, 0, 128)]
        cases += [('small_product', 'dense', n, n, n) for n in (1, 2, 4, 8, 16, 32)]
    rows = []
    with tempfile.TemporaryDirectory(prefix='cp-matrix-bench-') as tmp:
        binaries = {}
        for name in ('library', 'scalar'):
            binary = Path(tmp, name)
            extra = ['-DMATRIX_DISABLE_AVX2'] if name == 'scalar' else []
            subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(ROOT / 'test/benchmark/matrix.cpp'), '-o', str(binary)], check=True)
            binaries[name] = binary
        rng = random.Random(713)
        for operation, kind, n, inner, m in cases:
            expected = None
            for trial in range(args.trials):
                order = ['baseline', 'scalar', 'library']
                rng.shuffle(order)
                for name in order:
                    binary = binaries['scalar' if name == 'scalar' else 'library']
                    output = subprocess.check_output([str(binary), name, operation, kind, str(n), str(inner), str(m)])
                    elapsed, result = output.split(b'\n', 1)
                    if expected is None:
                        expected = result
                    if result != expected:
                        raise RuntimeError(f'Full result mismatch: {operation} {kind} {name}')
                    rows.append({'operation': operation, 'kind': kind, 'n': n, 'inner': inner, 'm': m, 'variant': name, 'trial': trial,
                                 'seconds': float(elapsed), 'result_sha256': hashlib.sha256(result).hexdigest()})
            print(operation, kind, n, 'full outputs match', flush=True)
        with (args.output / 'samples.csv').open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=rows[0].keys()); writer.writeheader(); writer.writerows(rows)
        summary = []
        for operation, kind, n, inner, m in cases:
            for name in ('baseline', 'scalar', 'library'):
                sample = [r['seconds'] for r in rows if (r['operation'], r['kind'], r['n'], r['variant']) == (operation, kind, n, name)]
                summary.append({'operation': operation, 'kind': kind, 'n': n, 'inner': inner, 'm': m, 'variant': name, 'seconds': statistics.median(sample)})
        with (args.output / 'summary.csv').open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=summary[0].keys()); writer.writeheader(); writer.writerows(summary)


if __name__ == '__main__':
    main()
