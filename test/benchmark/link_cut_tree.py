import argparse
import csv
import hashlib
import itertools
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

from link_cut_tree_variants import variants

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--variants', nargs='+', default=list(variants()))
    parser.add_argument('--n', type=int, default=200000)
    parser.add_argument('--q', type=int, default=200000)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--shapes', nargs='+', default=['line', 'star', 'binary', 'random', 'broom'])
    parser.add_argument('--modes', nargs='+', default=['query', 'mixed', 'update'])
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--ndebug', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    if 'baseline' not in args.variants:
        parser.error('--variants must include baseline')
    if hasattr(os, 'sched_setaffinity'):
        os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    if args.ndebug:
        flags += ['-DNDEBUG']
    paths = ['data_structure/link_cut_tree.hpp', 'math/mint.hpp',
             'test/benchmark/link_cut_tree_baseline.hpp', 'test/benchmark/link_cut_tree.cpp',
             'test/benchmark/link_cut_tree_variants.py', 'test/benchmark/link_cut_tree.py']
    environment = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
                   'flags': flags, 'affinity': [args.cpu], 'seed': 827493,
                   'n': args.n, 'q': args.q, 'trials': args.trials,
                   'variants': args.variants,
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    for key, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
        environment[key] = subprocess.check_output(command, text=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows = []
    summary = []
    with tempfile.TemporaryDirectory(prefix='cp-lct-') as temporary:
        directory = Path(temporary)
        subprocess.run(['python3', str(ROOT / 'test/benchmark/link_cut_tree_variants.py'),
                        str(directory)], check=True)
        binary = directory / 'benchmark'
        subprocess.run([args.cxx, *flags, '-I', str(ROOT), '-I', str(directory),
                        str(ROOT / 'test/benchmark/link_cut_tree.cpp'), '-o', str(binary)], check=True)
        order_random = random.Random(92647)
        for monoid, shape, mode in itertools.product(['sum', 'affine'], args.shapes, args.modes):
            expected = None
            samples = {name: [] for name in args.variants}
            for trial in range(args.trials):
                order = args.variants.copy()
                order_random.shuffle(order)
                for name in order:
                    answer = directory / 'answers.bin'
                    command = [str(binary), name, monoid, shape, mode, str(args.n),
                               str(args.q), str(environment['seed']), str(answer)]
                    row = json.loads(subprocess.check_output(command, text=True))
                    actual = answer.read_bytes()
                    if expected is None:
                        expected = actual
                    elif actual != expected:
                        raise RuntimeError(f'answer mismatch: {monoid} {shape} {mode} {name}')
                    row['trial'] = trial
                    rows.append(row)
                    samples[name].append(row)
            for name in args.variants:
                build = statistics.median(row['build_s'] for row in samples[name])
                duration = statistics.median(row['operations_s'] for row in samples[name])
                baseline = statistics.median(row['operations_s'] for row in samples['baseline'])
                summary.append({'monoid': monoid, 'shape': shape, 'mode': mode, 'variant': name,
                                'build_s': build, 'operations_s': duration, 'speedup': baseline / duration})
            print(f'{monoid} {shape} {mode}: all answers match', flush=True)
            for filename, data in [('samples.csv', rows), ('summary.csv', summary)]:
                with (args.output / filename).open('w', newline='') as file:
                    writer = csv.DictWriter(file, fieldnames=list(data[0]))
                    writer.writeheader()
                    writer.writerows(data)


if __name__ == '__main__':
    main()
