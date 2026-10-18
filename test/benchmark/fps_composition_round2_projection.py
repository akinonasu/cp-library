import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('/tmp/fps-composition-round2-projection')
WORK.mkdir(parents=True, exist_ok=True)
OUT = ROOT / 'test/benchmark/results/fps_composition_round2_projection.json'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
manifest = ROOT / 'test/benchmark/results/fps_composition_round2_tableonly.json'
source = ROOT / 'test/benchmark/fps_tuning_advanced.cpp'
flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
result = dict(baseline_commit='144ae32387675fa2a889ea3e8cd19b03ad5016bf', flags=flags, compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              cpu=2, trials=21, seed=20261024, manifest_sha256=sha(manifest), harness_sha256=sha(Path(__file__)), source_sha256=sha(source),
              metric='whole process wall milliseconds and API calculation time reported separately; warmup 1', builds={}, workloads=[])
trees = {'baseline': Path('/tmp/cp-fps-round2-base'), 'candidate': Path('/tmp/fps-composition-round2-tableonly/candidate')}
record = json.loads(manifest.read_text())
with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
    for name, tree in trees.items():
        hashes = {str(p.relative_to(tree)): sha(p) for folder in ('fps', 'components', 'math', 'convolution') for p in (tree / folder).rglob('*.hpp')}
        assert hashes == record['sources'][name + '_headers']
        binary = WORK / name
        cmd = ['g++', *flags, '-I', str(tree), str(source), '-o', str(binary)]
        fcntl.flock(lock, fcntl.LOCK_SH)
        try:
            p = subprocess.run(cmd, capture_output=True, text=True)
            (WORK / (name + '.build.log')).write_text(p.stdout + p.stderr)
            p.check_returncode()
        finally: fcntl.flock(lock, fcntl.LOCK_UN)
        result['builds'][name] = dict(command=cmd, binary_sha256=sha(binary))
        print('built', name, flush=True)
    fcntl.flock(lock, fcntl.LOCK_EX)
    os.sched_setaffinity(0, {2})
    rng = random.Random(result['seed'])
    for op in ('projection', 'projection_weighted'):
        for n in (32768, 131072):
            row = dict(operation=op, n=n, variants={}, orders=[])
            whole = {name: [] for name in trees}; kernel = {name: [] for name in trees}; sums = {}
            for rep in range(result['trials'] + 1):
                order = list(trees); rng.shuffle(order); row['orders'].append(order)
                for name in order:
                    begin = time.perf_counter_ns()
                    text = subprocess.check_output([str(WORK / name), op, 'current', str(n)], text=True)
                    wall = (time.perf_counter_ns() - begin) / 1e6
                    ms, checksum = text.split(); sums[name] = checksum
                    if rep: whole[name].append(wall); kernel[name].append(float(ms))
            assert len(set(sums.values())) == 1
            for name in trees: row['variants'][name] = dict(whole_median_ms=statistics.median(whole[name]), whole_samples_ms=whole[name], calculation_median_ms=statistics.median(kernel[name]), calculation_samples_ms=kernel[name], checksum=sums[name])
            result['workloads'].append(row); OUT.write_text(json.dumps(result, indent=2) + '\n')
            print(op, n, {name: (v['whole_median_ms'], v['calculation_median_ms']) for name, v in row['variants'].items()}, flush=True)
