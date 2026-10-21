import argparse
import csv
from datetime import datetime, timezone
import fcntl
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

from fps_light import ROOT, expand, sha

OPERATIONS = ['taylor_shift', 'multipoint_eval', 'interpolate', 'linear_recurrence']


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--sizes', nargs='+', type=int, default=[1024, 32768])
    parser.add_argument('--target', choices=['generic', 'haswell', 'native'], default='haswell')
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_light/algorithms.json')
    args = parser.parse_args()
    if args.trials < 1 or min(args.sizes) < 2:
        parser.error('positive trials and sizes >= 2 are required')
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('--cpu is unavailable')
    os.chdir(ROOT)
    os.sched_setaffinity(0, {args.cpu})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-Wall', '-Wextra']
    if args.target != 'generic':
        flags += ['-march=' + args.target]
    hashes = {}
    for name in ['fps/fps_light_algorithms.hpp', 'fps/taylor_shift.hpp', 'fps/multipoint_eval.hpp', 'fps/interpolate.hpp', 'fps/linear_recurrence.hpp']:
        expand('#include "' + name + '"\n', hashes)
    for name in ['test/benchmark/fps_light_algorithms.cpp', 'test/benchmark/fps_light_algorithms.py', 'test/benchmark/fps_light.py']:
        hashes[name] = sha(ROOT / name)
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(), 'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu], 'flags': flags, 'trials': args.trials, 'sizes': args.sizes, 'seed': 731, 'execution_order_seed': 20261021, 'source_sha256': hashes, 'initialization': 'fresh process for every sample; no transform prewarming; input construction outside timing; distinct points 0..n-1; shift17; recurrence k=10^18', 'results': [], 'binaries': {}, 'completed': False}
    samples = []

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        with args.output.with_suffix('.csv').open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=['operation', 'n', 'variant', 'trial', 'ms', 'result_size', 'checksum'], lineterminator='\n')
            writer.writeheader()
            writer.writerows(samples)

    save()
    order = random.Random(20261021)
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy, tempfile.TemporaryDirectory(prefix='fps-light-algorithms-benchmark-') as temporary:
        fcntl.flock(lock, fcntl.LOCK_EX)
        fcntl.flock(legacy, fcntl.LOCK_EX)
        binaries = {}
        for variant in ['existing', 'light']:
            exe = Path(temporary) / variant
            extra = ['-DLIGHT_FPS'] if variant == 'light' else []
            command = [args.cxx, *flags, *extra, '-I', str(ROOT), 'test/benchmark/fps_light_algorithms.cpp', '-o', str(exe)]
            built = subprocess.run(command, capture_output=True, text=True, check=True)
            report['binaries'][variant] = {'command': command, 'sha256': sha(exe), 'bytes': exe.stat().st_size, 'compiler_stdout': built.stdout, 'compiler_stderr': built.stderr}
            binaries[variant] = exe
        for n in args.sizes:
            for operation in OPERATIONS:
                values = {variant: [] for variant in binaries}
                answers = set()
                for trial in range(args.trials):
                    variants = list(binaries)
                    order.shuffle(variants)
                    for variant in variants:
                        ms, result_size, checksum = subprocess.check_output([str(binaries[variant]), operation, str(n)], text=True, timeout=180).split()
                        values[variant].append(float(ms))
                        answers.add((result_size, checksum))
                        samples.append({'operation': operation, 'n': n, 'variant': variant, 'trial': trial, 'ms': float(ms), 'result_size': int(result_size), 'checksum': checksum})
                assert len(answers) == 1, (operation, n, answers)
                medians = {variant: statistics.median(ms) for variant, ms in values.items()}
                row = {'operation': operation, 'n': n, 'median_ms': medians, 'light_over_existing': medians['light'] / medians['existing'], 'answer': list(answers.pop())}
                report['results'].append(row)
                save()
                print(operation, n, medians, 'light/existing', row['light_over_existing'], flush=True)
    for name, digest in hashes.items():
        assert sha(ROOT / name) == digest, name
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()


if __name__ == '__main__':
    main()
