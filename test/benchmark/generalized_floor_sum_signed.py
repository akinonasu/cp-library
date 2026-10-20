import argparse
import csv
from datetime import datetime, timezone
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
    args.output.mkdir(parents=True, exist_ok=True)
    reference = root / 'test/benchmark/results/generalized_floor_sum_yuki/reference.json'
    if not reference.exists():
        url = 'https://yukicoder.me/submissions/1081475/source'
        with urllib.request.urlopen(url, timeout=30) as response:
            source = response.read()
        reference.write_text(json.dumps({'retrieved_at': datetime.now(timezone.utc).isoformat(),
                                         'statistics': 'https://yukicoder.me/problems/no/2996/statistics',
                                         'submission': 'https://yukicoder.me/submissions/1081475',
                                         'user': 'akakimidori', 'time_ms': 49, 'language': 'Rust',
                                         'source': url, 'sha256': hashlib.sha256(source).hexdigest()}, indent=2) + '\n')
    baseline = subprocess.check_output(['git', 'show', '73dff6c:math/floor_sum/generalized_floor_sum.hpp'], cwd=root)
    baseline_hash = hashlib.sha256(baseline).hexdigest()
    baseline = baseline.replace(b'GENERALIZED_FLOOR_SUM_HPP', b'BASELINE_GENERALIZED_FLOOR_SUM_HPP').replace(b'generalized_floor_sum', b'baseline_generalized_floor_sum')
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    names = ['math/floor_sum/generalized_floor_sum.hpp', 'math/mint.hpp', 'test/benchmark/generalized_floor_sum_signed.cpp']
    metadata = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'flags': flags, 'affinity': [2],
                'baseline_commit': '73dff6c', 'baseline_sha256': baseline_hash, 'seed': 20261020, 'rounds': 7,
                'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names}}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='floor-signed-bench-') as temporary:
        (Path(temporary) / 'baseline.hpp').write_bytes(baseline)
        executable = str(Path(temporary) / 'benchmark')
        subprocess.run(['g++', *flags, '-I', temporary, str(root / 'test/benchmark/generalized_floor_sum_signed.cpp'), '-o', executable], check=True)
        for degree, count in [(4, 10000), (8, 2000), (20, 100)]:
            expected = None
            for trial in range(7):
                modes = ['base', 'unsigned', 'signed']
                rng.shuffle(modes)
                for mode in modes:
                    ns, checksum = map(int, subprocess.check_output([executable, mode, str(degree)], text=True, timeout=60).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (degree, mode)
                    rows.append({'degree': degree, 'count': count, 'trial': trial, 'mode': mode, 'ns': ns, 'checksum': checksum})
            record = {'degree': degree, 'count': count}
            for mode in ['base', 'unsigned', 'signed']:
                record[mode + '_ms'] = statistics.median(r['ns'] for r in rows if r['degree'] == degree and r['mode'] == mode) / 1e6
            summary.append(record)
            print(record, flush=True)
    with (args.output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader(); writer.writerows(rows)
    (args.output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')


if __name__ == '__main__':
    main()
