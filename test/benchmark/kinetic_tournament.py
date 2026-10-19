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
import urllib.request


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    reference = json.loads((root / 'test/benchmark/results/kinetic_tournament/reference.json').read_text())['kinetic']
    cache = root / '.verify-helper/cache/kinetic_tournament_references'
    if not (cache / 'ekzhang.cpp').exists():
        cache.mkdir(parents=True, exist_ok=True)
        with urllib.request.urlopen(reference['url'], timeout=60) as response:
            (cache / 'ekzhang.cpp').write_bytes(response.read())
    assert hashlib.sha256((cache / 'ekzhang.cpp').read_bytes()).hexdigest() == reference['sha256']
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root), '-isystem', str(cache)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    names = ['data_structure/segment_tree/kinetic_tournament.hpp', 'test/benchmark/kinetic_tournament.cpp']
    metadata = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [2], 'seed': 20261020, 'rounds': 7,
                'n': 100000, 'q': 100000, 'reference': reference,
                'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names}}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='kinetic-bench-') as temporary:
        executable = str(Path(temporary) / 'benchmark')
        subprocess.run(['g++', *flags, str(root / 'test/benchmark/kinetic_tournament.cpp'), '-o', executable], check=True)
        for kind in ['random', 'envelope', 'same_slope', 'jump', 'concurrent']:
            expected = None
            for trial in range(7):
                modes = ['reference', 'library', 'library64']
                rng.shuffle(modes)
                for mode in modes:
                    ns, checksum = map(int, subprocess.check_output([executable, mode, kind], text=True, timeout=60).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (kind, mode)
                    rows.append({'case': kind, 'trial': trial, 'mode': mode, 'ns': ns, 'checksum': checksum})
            record = {'case': kind}
            for mode in ['reference', 'library', 'library64']:
                record[mode + '_ms'] = statistics.median(r['ns'] for r in rows if r['case'] == kind and r['mode'] == mode) / 1e6
            summary.append(record)
            print(record, flush=True)
    args.output.mkdir(parents=True, exist_ok=True)
    with (args.output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader(); writer.writerows(rows)
    (args.output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')


if __name__ == '__main__':
    main()
