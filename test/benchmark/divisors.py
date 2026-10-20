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
import urllib.parse
import urllib.request


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = args.output
    output.mkdir(parents=True, exist_ok=True)
    reference = root / 'test/benchmark/results/divisors/reference.json'
    if not reference.exists():
        query = urllib.parse.urlencode({'problem': 'factorize', 'status': 'AC', 'order': '+time', 'limit': 3})
        with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions?' + query, timeout=30) as response:
            listing = json.load(response)
        cache = root / '.verify-helper/cache/divisors_references'
        cache.mkdir(parents=True, exist_ok=True)
        entries = []
        for item in listing['submissions']:
            with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id']), timeout=30) as response:
                detail = json.load(response)
            source = detail['source'].encode()
            (cache / (str(item['id']) + '.cpp')).write_bytes(source)
            entries.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                            'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
        reference.parent.mkdir(parents=True, exist_ok=True)
        reference.write_text(json.dumps({'retrieved_at': datetime.now(timezone.utc).isoformat(),
                                         'listing': listing, 'references': entries}, indent=2) + '\n')
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    names = ['math/number_theory/factorize.hpp', 'math/number_theory/is_prime.hpp', 'components/montgomery.hpp',
             'components/splitmix64.hpp', 'test/benchmark/divisors.cpp']
    environment = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
                   'cpu': subprocess.check_output(['lscpu'], text=True), 'flags': flags, 'affinity': [2],
                   'seed': 20261020, 'rounds': 7,
                   'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names}}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='divisors-bench-') as temporary:
        executable = str(Path(temporary) / 'benchmark')
        subprocess.run(['g++', *flags, str(root / 'test/benchmark/divisors.cpp'), '-o', executable], check=True)
        for kind in ['small', 'random32']:
            expected = None
            modes = ['trial', 'sorted', 'unsorted', 'reused', 'reused_unsorted']
            for trial in range(7):
                order = modes.copy()
                rng.shuffle(order)
                for mode in order:
                    ns, checksum = map(int, subprocess.check_output([executable, mode, kind], text=True, timeout=60).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (kind, mode)
                    rows.append({'case': kind, 'trial': trial, 'mode': mode, 'ns': ns, 'checksum': checksum})
            record = {'case': kind, 'count': 10000 if kind == 'small' else 2000}
            for mode in modes:
                record[mode + '_ms'] = statistics.median(r['ns'] for r in rows if r['case'] == kind and r['mode'] == mode) / 1e6
            summary.append(record)
            print(record, flush=True)
    with (output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader(); writer.writerows(rows)
    (output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')


if __name__ == '__main__':
    main()
