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
import time

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-io-pipeline'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--trials', type=int, default=11)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline=subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), flags=flags, cpu=args.cpu,
                  trials=args.trials, harness_sha256=digest(Path(__file__)), sources={}, variants={})
    folder = args.official_root / 'polynomial/log_of_formal_power_series'
    inp, answer = folder / 'in/max_random_00.in', folder / 'out/max_random_00.out'
    result.update(input_sha256=digest(inp), answer_sha256=digest(answer))
    binaries = {}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        current = args.work / 'current'
        if current.exists():
            shutil.rmtree(current)
        current.mkdir()
        for name in ['components', 'fps', 'math', 'convolution']:
            shutil.copytree(ROOT / name, current / name)
        result['headers'] = {str(p.relative_to(current)): digest(p) for p in current.rglob('*.hpp')}
        driver = args.work / 'driver.cpp'
        shutil.copy2(ROOT / 'test/benchmark/fps_io_pipeline.cpp', driver)
        old = subprocess.check_output(['git', 'show', args.baseline + ':components/fps_io.hpp'], cwd=ROOT, text=True)
        old = old.replace('#include "fast_io.hpp"', '#include "' + str(current / 'components/fast_io.hpp') + '"')
        old = old.replace('#include "../math/mint.hpp"', '#include "' + str(current / 'math/mint.hpp') + '"')
        old_header = args.work / 'old_io.hpp'
        old_header.write_text(old)
        for variant, header in [('old_io', old_header), ('current_io', current / 'components/fps_io.hpp')]:
            binary = args.work / variant
            command = ['g++', *flags, '-I', str(current), '-include', str(header), str(driver), '-o', str(binary)]
            subprocess.run(command, check=True)
            binaries[variant] = binary
            result['sources'][variant] = dict(command=command, header_sha256=digest(header),
                                              driver_sha256=digest(driver), binary_sha256=digest(binary))
            print('built', variant, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        samples = {v: [] for v in binaries}
        rng = random.Random(20261019)
        for trial in range(args.trials + 1):
            order = list(samples)
            rng.shuffle(order)
            for variant in order:
                output = args.work / (variant + '.out')
                with inp.open('rb') as fin, output.open('wb') as fout:
                    start = time.perf_counter_ns()
                    process = subprocess.run([str(binaries[variant])], stdin=fin, stdout=fout,
                                             stderr=subprocess.PIPE, text=True, check=True)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                if trial:
                    samples[variant].append([*map(float, process.stderr.split()), elapsed])
        fcntl.flock(lock, fcntl.LOCK_UN)
    result['columns'] = ['input_ms', 'kernel_ms', 'output_ms', 'whole_ms']
    for variant, values in samples.items():
        row = dict(samples_ms=values, median_ms=[statistics.median(x[k] for x in values) for k in range(4)])
        result['variants'][variant] = row
        print(variant, row['median_ms'], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
