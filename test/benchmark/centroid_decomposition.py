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
    paths = ['graph/centroid_decomposition.hpp', 'test/benchmark/centroid_decomposition.cpp',
             'test/benchmark/centroid_decomposition.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'seed': 731, 'trials': args.trials, 'affinity': [args.cpu], 'vertices': 200000,
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    rows = []
    rng = random.Random(731)
    workloads = [(shape, cache) for shape in ['path', 'star', 'balanced', 'broom', 'random', 'random_labels']
                 for cache in [0, 1]]
    with tempfile.TemporaryDirectory(prefix='cp-centroid-') as directory:
        binary = Path(directory) / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / paths[1]), '-o', str(binary)], check=True)
        for shape, cache in workloads:
            expected = None
            for trial in range(args.trials):
                order = ['baseline', 'library']
                rng.shuffle(order)
                for variant in order:
                    elapsed, result = subprocess.check_output([str(binary), variant, shape, '200000', str(cache)]).splitlines()
                    if expected is None:
                        expected = result
                    assert result == expected, (shape, cache, variant)
                    rows.append({'shape': shape, 'distances': cache, 'variant': variant, 'trial': trial,
                                 'seconds': float(elapsed), 'checksum': result.decode()})
            print(shape, cache, 'checksums match', flush=True)
    summary = [{'shape': shape, 'distances': cache, 'variant': variant,
                'seconds': statistics.median(r['seconds'] for r in rows
                                            if (r['shape'], r['distances'], r['variant']) == (shape, cache, variant))}
               for shape, cache in workloads for variant in ['baseline', 'library']]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
