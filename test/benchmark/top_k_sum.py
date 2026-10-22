import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--reference', type=Path)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--trials', type=int, default=5)
parser.add_argument('--cxx', default='g++')
args = parser.parse_args()
flags = ['-std=c++23', '-O2', '-march=haswell', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
if args.reference:
    flags.append('-DTOP_K_REFERENCE_HEADER="' + str(args.reference.resolve()) + '"')
variants = ['library', 'tree'] + (['heap'] if args.reference else [])
scenarios = ['updates', 'window', 'duplicates', 'long_stream']
rows = []
with tempfile.TemporaryDirectory(prefix='top-k-sum-') as directory:
    exe = Path(directory) / 'benchmark'
    subprocess.run([args.cxx, *flags, str(Path(__file__).with_suffix('.cpp')), '-o', str(exe)], check=True)
    for scenario in scenarios:
        schedule = variants * args.trials
        random.Random(306281).shuffle(schedule)
        for variant in schedule:
            output = subprocess.check_output([str(exe), variant, scenario], text=True).split()
            rows.append({'scenario': scenario, 'variant': variant, 'ms': float(output[0]), 'checksum': output[1]})
results = []
for scenario in scenarios:
    group = [row for row in rows if row['scenario'] == scenario]
    assert len({row['checksum'] for row in group}) == 1
    results.append({'scenario': scenario, 'median_ms': {variant: statistics.median(row['ms'] for row in group if row['variant'] == variant) for variant in variants}})
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
report = {'completed': True, 'timestamp': datetime.now(timezone.utc).isoformat(), 'compiler': subprocess.check_output([args.cxx, '--version'], text=True).splitlines()[0],
          'flags': flags, 'affinity': sorted(os.sched_getaffinity(0)), 'trials': args.trials,
          'source_sha256': {name: sha(root / name) for name in ['data_structure/top_k_sum.hpp', 'test/benchmark/top_k_sum.cpp', 'test/benchmark/top_k_sum.py']},
          'reference_sha256': sha(args.reference) if args.reference else None, 'samples': rows, 'results': results}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(report, indent=2) + '\n')
for row in results: print(row)
