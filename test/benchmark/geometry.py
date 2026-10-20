import argparse
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--n', type=int, default=200000)
    parser.add_argument('--trials', type=int, default=5)
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    if args.n < 2 or args.trials < 1:
        parser.error('n >= 2 and trials >= 1 are required')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    sources = ['geometry/point.hpp', 'geometry/linear.hpp', 'geometry/polygon.hpp', 'geometry/convex.hpp',
               'test/benchmark/geometry.cpp', 'test/benchmark/geometry.py']
    environment = {
        'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
        'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu],
        'n': args.n, 'trials': args.trials, 'seeds': [731 + trial for trial in range(args.trials)],
        'sha256': {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in sources},
        'scope': 'library call, internal allocation, result traversal/checksum and result destruction; input generation excluded',
    }
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    samples, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-geometry-') as temporary:
        binary = Path(temporary) / 'geometry'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / 'test/benchmark/geometry.cpp'), '-o', str(binary)], check=True)
        for operation in ['hull', 'closest', 'furthest']:
            for kind in ['random', 'collinear', 'grid', 'duplicates', 'parabola']:
                rows = []
                for trial in range(args.trials):
                    row = json.loads(subprocess.check_output([str(binary), operation, kind, str(args.n), str(731 + trial)], text=True))
                    row['trial'] = trial
                    rows.append(row)
                    samples.append(row)
                times = [row['milliseconds'] for row in rows]
                result = {'operation': operation, 'kind': kind, 'median_ms': statistics.median(times),
                          'min_ms': min(times), 'max_ms': max(times)}
                summary.append(result)
                print(json.dumps(result), flush=True)
    (args.output / 'samples.json').write_text(json.dumps(samples, indent=2) + '\n')
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')


if __name__ == '__main__':
    main()
