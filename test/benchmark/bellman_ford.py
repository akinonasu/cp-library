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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--rounds', type=int, default=7)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    metadata = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [2], 'rounds': args.rounds, 'seed': 20261020}
    names = ['graph/bellman_ford.hpp', 'components/shortest_path_scc.hpp', 'components/RadixHeap.hpp',
             'components/directed_csr.hpp', 'test/benchmark/bellman_ford.cpp']
    metadata['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='negative-path-bench-') as temporary:
        executable = str(Path(temporary) / 'benchmark')
        subprocess.run(['g++', *flags, str(root / 'test/benchmark/bellman_ford.cpp'), '-o', executable], check=True)
        for kind in ['sparse', 'dense', 'dag', 'components', 'positive_bridge', 'augmented_gor']:
            expected = None
            variants = ['basic', 'fifo', 'slf', 'gor', 'scc', 'library']
            for trial in range(args.rounds):
                order = variants.copy()
                rng.shuffle(order)
                for variant in order:
                    ns, checksum, n, m = map(int, subprocess.check_output([executable, variant, kind], text=True, timeout=60).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (kind, variant)
                    rows.append({'case': kind, 'trial': trial, 'variant': variant, 'ns': ns, 'checksum': checksum, 'n': n, 'm': m})
            record = {'case': kind, 'n': n, 'm': m}
            for variant in variants:
                record[variant + '_ms'] = statistics.median(row['ns'] for row in rows if row['case'] == kind and row['variant'] == variant) / 1e6
            summary.append(record)
            print(record, flush=True)
    args.output.mkdir(parents=True, exist_ok=True)
    with (args.output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader(); writer.writerows(rows)
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    (args.output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')


if __name__ == '__main__':
    main()
