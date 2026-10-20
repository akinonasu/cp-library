import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile


root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--native', action='store_true')
args = parser.parse_args()
flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
if args.native:
    flags += ['-march=native', '-ftrivial-auto-var-init=zero']
affinity = sorted(os.sched_getaffinity(0))
cpu = 2 if 2 in affinity else affinity[0]
sources = ['test/benchmark/persistent_segtree.cpp', 'data_structure/segment_tree/persistent_segtree.hpp']
with tempfile.TemporaryDirectory(prefix='persistent-segtree-') as temporary:
    binary = Path(temporary) / 'bench'
    subprocess.run(['g++', *flags, '-I', str(root), sources[0], '-o', str(binary)], cwd=root, check=True)
    output = subprocess.check_output(['taskset', '-c', str(cpu), str(binary)], text=True, cwd=root)
samples = [json.loads(line) for line in output.splitlines()]
summary = {}
for name in sorted({row['case'] for row in samples}):
    rows = [row for row in samples if row['case'] == name]
    before = statistics.median(row['baseline_ms'] for row in rows)
    after = statistics.median(row['candidate_ms'] for row in rows)
    summary[name] = {'baseline_ms': before, 'candidate_ms': after, 'ratio': after / before,
                     'nodes': rows[0]['nodes'], 'reserved_bytes': rows[0]['bytes']}
report = {
    'platform': platform.platform(), 'cpu': next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
    'affinity_cpu': cpu, 'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
    'flags': flags, 'seed': 20261021, 'n': 131073, 'timed_operations': 160000, 'trials': 5,
    'read_heavy_warmup': 20000, 'answers_compared': 'Every answer and final node count on every trial',
    'source_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in sources},
    'summary': summary, 'samples': samples,
}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(summary, indent=2))
