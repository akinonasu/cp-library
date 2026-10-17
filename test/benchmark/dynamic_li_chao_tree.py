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
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['data_structure/dynamic_li_chao_tree.hpp', 'test/benchmark/dynamic_li_chao_tree.cpp',
             'test/benchmark/dynamic_li_chao_tree.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'seed': 731, 'trials': args.trials, 'affinity': [args.cpu], 'updates': 200000, 'queries': 200000,
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    rows = []
    rng = random.Random(731)
    shapes = ['random_line', 'hull', 'same_slope', 'segments', 'points', 'dominated', 'wide']
    with tempfile.TemporaryDirectory(prefix='cp-dynamic-li-chao-') as directory:
        binary = Path(directory) / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / paths[1]), '-o', str(binary)], check=True)
        for shape in shapes:
            expected = None
            for trial in range(args.trials):
                order = ['baseline', 'library'] + ([] if shape == 'wide' else ['library64'])
                rng.shuffle(order)
                for variant in order:
                    elapsed, result, nodes = subprocess.check_output([str(binary), variant, shape, '200000']).splitlines()
                    if expected is None: expected = result
                    assert result == expected, (shape, variant)
                    rows.append({'workload': shape, 'variant': variant, 'trial': trial,
                                 'seconds': float(elapsed), 'nodes': int(nodes), 'checksum': result.decode()})
            print(shape, 'checksums match', flush=True)
    summary = [{'workload': shape, 'variant': variant,
                'seconds': statistics.median(r['seconds'] for r in rows if (r['workload'], r['variant']) == (shape, variant)),
                'nodes': next(r['nodes'] for r in rows if (r['workload'], r['variant']) == (shape, variant))}
               for shape in shapes for variant in ['baseline', 'library'] + ([] if shape == 'wide' else ['library64'])]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
