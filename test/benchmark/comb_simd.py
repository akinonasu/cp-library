import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--cxx', default='g++')
parser.add_argument('--cpu', type=int, default=2)
parser.add_argument('--trials', type=int, default=9)
parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/comb_modint/simd.json')
args = parser.parse_args()
if args.trials < 1:
    parser.error('--trials must be positive')
os.chdir(ROOT)
if args.cpu not in os.sched_getaffinity(0):
    parser.error('--cpu is not available')
os.sched_setaffinity(0, {args.cpu})
args.output.parent.mkdir(parents=True, exist_ok=True)
flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE', '-I', str(ROOT)]
files = ['test/benchmark/comb_simd.cpp', 'test/benchmark/comb_simd.py', 'components/fps_factorials.hpp', 'components/ntt.hpp', 'math/mint.hpp']
hashes = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in files}
report = {
    'started_at': datetime.now(timezone.utc).isoformat(),
    'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
    'cpu': subprocess.check_output(['lscpu'], text=True),
    'kernel': subprocess.check_output(['uname', '-a'], text=True),
    'affinity': [args.cpu],
    'trials': args.trials,
    'order_seed': 20261021,
    'flags': {},
    'sources': hashes,
    'scope': 'Both complete factorial and inverse-factorial tables modulo 998244353, using the same mint and preallocated buffers. First generation and repeated average are separate. Allocation, process startup, I/O and exact verification are excluded. All entries are independently checked using uint64 modular arithmetic after timing.',
    'results': [],
    'completed': False,
}
samples = []

def save():
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    with args.output.with_suffix('.csv').open('w') as stream:
        writer = csv.DictWriter(stream, fieldnames=['variant', 'max_n', 'trial', 'method', 'repeats', 'first_ms', 'repeated_ms', 'checksum'], lineterminator='\n')
        writer.writeheader()
        writer.writerows(samples)

save()
rng = random.Random(report['order_seed'])
with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy_lock, tempfile.TemporaryDirectory(prefix='comb-simd-') as temporary:
    fcntl.flock(lock, fcntl.LOCK_EX)
    fcntl.flock(legacy_lock, fcntl.LOCK_EX)
    for variant, extra in [('generic', []), ('native', ['-march=native'])]:
        exe = Path(temporary) / variant
        report['flags'][variant] = flags + extra
        subprocess.run([args.cxx, *flags, *extra, 'test/benchmark/comb_simd.cpp', '-o', str(exe)], check=True)
        for max_n in [1000, 100000, 1000000, 10000000]:
            repeats = max(1, min(10000, 10000000 // (max_n + 1)))
            cells = {method: [] for method in ['naive', 'scalar', 'simd']}
            checksums = set()
            for trial in range(args.trials):
                order = list(cells)
                rng.shuffle(order)
                for method in order:
                    first, repeated, checksum = subprocess.check_output([str(exe), str(max_n), method, str(repeats)], text=True).split()
                    sample = {'variant': variant, 'max_n': max_n, 'trial': trial, 'method': method, 'repeats': repeats, 'first_ms': float(first), 'repeated_ms': float(repeated), 'checksum': checksum}
                    cells[method].append(sample)
                    samples.append(sample)
                    checksums.add(checksum)
            assert len(checksums) == 1, (variant, max_n)
            row = {'variant': variant, 'max_n': max_n, 'entries': max_n + 1, 'repeats': repeats, 'checksum': checksums.pop(), 'methods': {}}
            for method, values in cells.items():
                xs = [x['repeated_ms'] for x in values]
                row['methods'][method] = {'median_ms': statistics.median(xs), 'min_ms': min(xs), 'max_ms': max(xs), 'first_median_ms': statistics.median(x['first_ms'] for x in values)}
            median = {m: v['median_ms'] for m, v in row['methods'].items()}
            row['naive_over_simd'] = median['naive'] / median['simd']
            row['scalar_over_simd'] = median['scalar'] / median['simd']
            report['results'].append(row)
            save()
            print(variant, max_n, median, 'naive/simd', row['naive_over_simd'], 'scalar/simd', row['scalar_over_simd'], flush=True)
for path, expected in hashes.items():
    assert hashlib.sha256((ROOT / path).read_bytes()).hexdigest() == expected, path
report['completed'] = True
report['completed_at'] = datetime.now(timezone.utc).isoformat()
save()
