import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--acl', type=Path, required=True)
parser.add_argument('--cxx', default='g++')
parser.add_argument('--cpu', type=int, default=2)
parser.add_argument('--trials', type=int, default=9)
parser.add_argument('--variants', nargs='+', choices=['generic', 'native'], default=['generic', 'native'])
parser.add_argument('--n', type=int, default=200000)
parser.add_argument('--q', type=int, default=500000)
parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/acl_structures/benchmark.json')
args = parser.parse_args()
os.chdir(ROOT)
args.acl = args.acl.resolve()
if args.cpu not in os.sched_getaffinity(0):
    parser.error('--cpu is unavailable')
os.sched_setaffinity(0, {args.cpu})
soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
args.output.parent.mkdir(parents=True, exist_ok=True)
flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT), '-isystem', str(args.acl)]
files = ['data_structure/union_find.hpp', 'data_structure/fenwick_tree.hpp', 'data_structure/segment_tree/lazy_segtree.hpp', 'components/scc_core.hpp', 'graph/connectivity/scc.hpp', 'graph/two_sat.hpp', 'test/benchmark/acl_structures.cpp', 'test/benchmark/acl_structures.py']
hashes = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in files}
report = {'started_at': datetime.now(timezone.utc).isoformat(), 'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu], 'acl_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.acl, text=True).strip(), 'stack_limit': list(resource.getrlimit(resource.RLIMIT_STACK)), 'trials': args.trials, 'n': args.n, 'q': args.q, 'sources': hashes, 'flags': {}, 'results': [], 'completed': False}
samples = []

def save():
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    with args.output.with_suffix('.csv').open('w') as stream:
        writer = csv.DictWriter(stream, fieldnames=['variant', 'scenario', 'implementation', 'trial', 'ms', 'checksum'], lineterminator='\n')
        writer.writeheader()
        writer.writerows(samples)

scenarios = ['dsu_mixed', 'dsu_queries', 'dsu_groups', 'fenwick_random', 'fenwick_local', 'fenwick_prefix', 'lazy_mixed', 'lazy_read_heavy', 'lazy_point', 'scc_random', 'scc_dag', 'scc_cycle', 'scc_path', 'sat_sat', 'sat_unsat', 'sat_units']
rng = random.Random(20261021)
save()
with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy, tempfile.TemporaryDirectory(prefix='acl-benchmark-') as temporary:
    fcntl.flock(lock, fcntl.LOCK_EX)
    fcntl.flock(legacy, fcntl.LOCK_EX)
    for variant in args.variants:
        extra = ['-march=native'] if variant == 'native' else []
        report['flags'][variant] = flags + extra
        exe = Path(temporary) / variant
        subprocess.run([args.cxx, *flags, *extra, 'test/benchmark/acl_structures.cpp', '-o', str(exe)], check=True)
        for scenario in scenarios:
            implementations = ['acl', 'ours', 'unpadded'] if scenario.startswith('fenwick') else ['acl', 'ours']
            values = {implementation: [] for implementation in implementations}
            checksums = set()
            for trial in range(args.trials):
                order = implementations.copy()
                rng.shuffle(order)
                for implementation in order:
                    ms, checksum = subprocess.check_output([str(exe), implementation, scenario, str(args.n), str(args.q)], text=True, timeout=60).split()
                    values[implementation].append(float(ms))
                    checksums.add(checksum)
                    samples.append({'variant': variant, 'scenario': scenario, 'implementation': implementation, 'trial': trial, 'ms': float(ms), 'checksum': checksum})
            assert len(checksums) == 1, (scenario, checksums)
            medians = {k: statistics.median(v) for k, v in values.items()}
            row = {'variant': variant, 'scenario': scenario, 'median_ms': medians, 'acl_over_ours': medians['acl'] / medians['ours'], 'checksum': checksums.pop()}
            report['results'].append(row)
            save()
            print(variant, scenario, medians, 'ACL/ours', row['acl_over_ours'], flush=True)
for p, expected in hashes.items():
    assert hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == expected, p
report['completed'] = True
report['completed_at'] = datetime.now(timezone.utc).isoformat()
save()
