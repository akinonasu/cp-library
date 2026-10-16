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
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--count', type=int, default=300000)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--skip-check', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    if hasattr(os, 'sched_setaffinity'):
        os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['math/fraction.hpp', 'test/benchmark/fraction_baseline.hpp',
             'test/benchmark/fraction.cpp', 'test/benchmark/fraction.py']
    environment = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
                   'flags': flags, 'seed': 713, 'affinity': [args.cpu], 'count': args.count, 'trials': args.trials,
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    for key, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
        environment[key] = subprocess.check_output(command, text=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-fraction-') as temporary:
        directory = Path(temporary)
        original = (ROOT / 'math/fraction.hpp').read_text()
        variants = []
        for name, replacement in [('Euclid', 'while (b) { U r = a % b; a = b; b = r; }\n\t\treturn a;'),
                                  ('Hybrid', 'if (a < b) { U t = a; a = b; b = t; }\n\t\tif (b) a %= b;\n\t\treturn std::gcd(a, b);')]:
            source = original.replace('FRACTION_HPP', 'FRACTION_' + name.upper() + '_HPP')
            source = source.replace('Fraction', 'Fraction' + name).replace('using fraction =', 'using fraction_' + name + ' =')
            source = source.replace('return std::gcd(a, b);', replacement)
            (directory / (name + '.hpp')).write_text(source)
            variants.append(f'#include "{name}.hpp"\n')
        (directory / 'variants.hpp').write_text(''.join(variants))
        binary = directory / 'benchmark'
        subprocess.run([args.cxx, *flags, '-I', str(ROOT), '-I', str(directory),
                        str(ROOT / 'test/benchmark/fraction.cpp'), '-o', str(binary)], check=True)
        order_random = random.Random(32947)
        for kind in ['random', 'same_den', 'integer', 'cancel', 'wide', 'fibonacci']:
            if not args.skip_check:
                subprocess.run([str(binary), 'check', 'compare', kind, '20000', '713'], check=True)
            operations = ['construct', 'compare', 'continued']
            if kind not in ['wide', 'fibonacci']:
                operations += ['add', 'sub', 'mul', 'div']
            for operation in operations:
                names = ['library'] if operation == 'continued' else ['baseline', 'library', 'euclid', 'hybrid']
                samples = {name: [] for name in names}
                expected = None
                for trial in range(args.trials):
                    order = names.copy()
                    order_random.shuffle(order)
                    for name in order:
                        row = json.loads(subprocess.check_output([
                            str(binary), name, operation, kind, str(args.count), '713'], text=True))
                        if expected is None:
                            expected = row['digest']
                        elif row['digest'] != expected:
                            raise RuntimeError(f'checksum mismatch: {kind} {operation} {name}')
                        row['trial'] = trial
                        rows.append(row)
                        samples[name].append(row['seconds'])
                for name in names:
                    median = statistics.median(samples[name])
                    summary.append({'kind': kind, 'operation': operation, 'variant': name,
                                    'seconds': median})
                for filename, data in [('samples.csv', rows), ('summary.csv', summary)]:
                    with (args.output / filename).open('w', newline='') as file:
                        writer = csv.DictWriter(file, fieldnames=list(data[0]))
                        writer.writeheader()
                        writer.writerows(data)
                print(f'{kind} {operation}: measured', flush=True)


if __name__ == '__main__':
    main()
