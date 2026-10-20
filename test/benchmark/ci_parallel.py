import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys
import tempfile
import time


root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--trials', type=int, default=2)
args = parser.parse_args()
headers = ['components/barrett32.hpp', 'tree/dsu_on_tree.hpp',
           'math/linear_algebra/matrix.hpp', 'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp']
sources = ['test/unit/' + name + '.cpp' for name in ['barrett32', 'dsu_on_tree', 'matrix', 'dynamic_wavelet_matrix']]
report = {
    'platform': platform.platform(), 'python': platform.python_version(), 'cpu_count': os.cpu_count(),
    'compilers': {name: subprocess.check_output([name, '--version'], text=True).splitlines()[0]
                  for name in ['g++', 'clang++']},
    'runner_sha256': hashlib.sha256((root / 'test/verify.py').read_bytes()).hexdigest(),
    'headers': headers, 'unit': sources, 'trials': args.trials, 'seconds': {'1': [], '2': []},
}
with tempfile.TemporaryDirectory() as temporary:
    selection = Path(temporary) / 'selection.json'
    selection.write_text(json.dumps({'headers': headers, 'unit': sources}))
    for trial in range(args.trials):
        for jobs in ([1, 2] if trial % 2 == 0 else [2, 1]):
            command = [sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'g++', 'clang++',
                       '--jobs', str(jobs), '--selection', str(selection)]
            start = time.perf_counter()
            subprocess.run(command, check=True, cwd=root, stdout=subprocess.DEVNULL)
            elapsed = time.perf_counter() - start
            report['seconds'][str(jobs)].append(elapsed)
            print(f'jobs={jobs}: {elapsed:.3f}s', flush=True)
report['median_seconds'] = {jobs: statistics.median(times) for jobs, times in report['seconds'].items()}
report['parallel_over_serial'] = report['median_seconds']['2'] / report['median_seconds']['1']
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(report, indent=2) + '\n')
