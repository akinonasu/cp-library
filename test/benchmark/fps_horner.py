import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    names = ['serial', 'scalar4', 'scalar8', 'vector1', 'vector2', 'vector4', 'adopted']
    result = dict(cpu_affinity=[args.cpu], flags=flags, trials=args.trials, seed=20261019,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), harness_sha256=sha(Path(__file__)),
                  driver_sha256=sha(ROOT / 'test/benchmark/fps_horner.cpp'), metric='computation milliseconds', workloads={})
    rng = random.Random(20261019)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='fps-horner-') as directory, open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        work = Path(directory)
        fcntl.flock(lock, fcntl.LOCK_SH)
        for folder in ['fps', 'components', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, work / folder)
        result['headers'] = {str(p.relative_to(work)): sha(p) for p in sorted(work.rglob('*.hpp'))}
        binary = work / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(work), str(ROOT / 'test/benchmark/fps_horner.cpp'), '-o', str(binary)], check=True)
        result['binary_sha256'] = sha(binary)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        for n in [32, 64, 128, 1024, 32768, 500000]:
            samples = {name: [] for name in names}
            checksums = set()
            for mode in range(len(names)):
                raw = subprocess.check_output([str(binary), str(mode), str(n)], text=True).split()
                checksums.add(raw[1])
            order = list(range(len(names))) * args.trials
            rng.shuffle(order)
            for mode in order:
                raw = subprocess.check_output([str(binary), str(mode), str(n)], text=True).split()
                samples[names[mode]].append(float(raw[0]))
                checksums.add(raw[1])
            if len(checksums) != 1:
                raise RuntimeError((n, checksums))
            result['workloads'][n] = dict(samples_ms=samples, median_ms={name: statistics.median(v) for name, v in samples.items()}, checksum=next(iter(checksums)))
            print(n, result['workloads'][n]['median_ms'], flush=True)
            args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
