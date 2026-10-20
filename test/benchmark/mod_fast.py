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
    paths = ['components/mod_fast_core.hpp', 'math/number_theory/mod_inv.hpp', 'math/number_theory/mod_pow.hpp',
             'math/mint.hpp', 'math/number_theory/is_prime.hpp', 'components/montgomery.hpp',
             'components/splitmix64.hpp', 'test/benchmark/mod_fast.cpp', 'test/benchmark/mod_fast.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'seed': 731, 'trials': args.trials, 'affinity': [args.cpu], 'queries': 1000000,
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    rows = []
    rng = random.Random(731)
    with tempfile.TemporaryDirectory(prefix='cp-mod-fast-') as directory:
        binary = Path(directory) / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / paths[-2]), '-o', str(binary)], check=True)
        for p in [998244353, 1000000007]:
            for workload in ['inv', 'pow', 'pow64']:
                variants = ['binary', 'barrett', 'library']
                if workload == 'inv': variants.append('euclid')
                if p == 998244353 and workload != 'pow64': variants.append('mint')
                expected = None
                for trial in range(args.trials):
                    order = variants.copy()
                    rng.shuffle(order)
                    for variant in order:
                        build, elapsed, result = subprocess.check_output([str(binary), variant, workload, str(p), '1000000']).splitlines()
                        if expected is None: expected = result
                        assert result == expected, (p, workload, variant)
                        rows.append({'mod': p, 'workload': workload, 'variant': variant, 'trial': trial,
                                     'build_seconds': float(build), 'seconds': float(elapsed), 'checksum': result.decode()})
                print(p, workload, 'checksums match', flush=True)
    summary = []
    for p, workload, variant in dict.fromkeys((r['mod'], r['workload'], r['variant']) for r in rows):
        group = [r for r in rows if (r['mod'], r['workload'], r['variant']) == (p, workload, variant)]
        summary.append({'mod': p, 'workload': workload, 'variant': variant,
                        'build_seconds': statistics.median(r['build_seconds'] for r in group),
                        'seconds': statistics.median(r['seconds'] for r in group)})
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
