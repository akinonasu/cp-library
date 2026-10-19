import argparse
import fcntl
import gzip
import hashlib
import json
import os
import random
import shutil
import statistics
import subprocess
import tarfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--build', action='store_true')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--screen9', action='store_true')
    parser.add_argument('--trials', type=int)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    args = parser.parse_args()
    manifest = json.loads((HERE / 'manifest.json').read_text())
    for name, expected in manifest.items():
        if sha(HERE / name) != expected:
            raise ValueError('artifact hash mismatch: ' + name)
    work = args.work.resolve()
    if work.exists():
        raise ValueError('work already exists')
    work.mkdir(parents=True)
    before = work / 'before'
    with tarfile.open(HERE / 'baseline.tar.gz') as archive:
        archive.extractall(before, filter='data')
    snapshot = json.loads((HERE / 'snapshot.json').read_text())
    for name, expected in snapshot['sources'].items():
        if sha(before / name) != expected:
            raise ValueError('baseline mismatch: ' + name)
    for variant, patch in [('candidate', 'constant.patch'), ('candidate_noinline', 'constant_noinline.patch')]:
        shutil.copytree(before, work / variant)
        subprocess.run(['patch', '-s', '-p1', '-d', str(work / variant), '-i', str(HERE / patch)], check=True)
    metadata = json.loads((HERE / 'metadata.json').read_text())
    if sha(work / 'candidate_noinline/components/fps_convolution_constant.hpp') != metadata['adopted_header_sha256']:
        raise ValueError('adopted header mismatch')
    reference = work / '409465.cpp'
    reference.write_bytes(gzip.decompress((HERE / '409465.cpp.gz').read_bytes()))
    if sha(reference) != metadata['reference_sha256']:
        raise ValueError('reference mismatch')
    screen = json.loads((HERE / 'screen9.json').read_text())
    for tree in ['candidate', 'candidate_noinline']:
        if sha(work / tree / 'components/fps_convolution.hpp') != screen['source_sha256']['candidate/components/fps_convolution.hpp']:
            raise ValueError('integration header mismatch')
    if sha(work / 'candidate/components/fps_convolution_constant.hpp') != screen['source_sha256']['candidate/components/fps_convolution_constant.hpp']:
        raise ValueError('inline header mismatch')
    unit = work / 'unit.cpp'
    shutil.copyfile(HERE / 'private_validation/unit.cpp', unit)
    if sha(unit) != metadata['final_unit_sha256']:
        raise ValueError('unit mismatch')
    print('Manifest, baseline, both patches, adopted header, reference, and unit verified.', flush=True)
    if not (args.build or args.check or args.sanitize or args.measure):
        return
    flags = json.loads((HERE / 'build.json').read_text())['flags']
    variants = ['before', 'candidate', 'reference'] if args.screen9 else ['before', 'candidate', 'noinline', 'reference']
    bins = {name: work / (name + '.bin') for name in variants}
    commands = []
    for name in variants:
        if name == 'reference':
            command = ['g++', *flags, str(reference), '-o', str(bins[name])]
        else:
            tree = work / ('candidate_noinline' if name == 'noinline' else name)
            command = ['g++', *flags, '-I' + str(tree), str(tree / 'test/convolution.test.cpp'), '-o', str(bins[name])]
        commands.append(command)
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        for command in commands:
            subprocess.run(command, check=True)
        if args.check or args.sanitize:
            unit_flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-DDEBUG=1', '-I', str(work / 'candidate_noinline')]
            if args.sanitize:
                unit_flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
            subprocess.run(['g++', *unit_flags, str(unit), '-o', str(work / 'unit')], check=True)
            subprocess.run([str(work / 'unit')], check=True, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1'))
        fcntl.flock(lock, fcntl.LOCK_UN)
    if not args.measure:
        return
    trials = args.trials if args.trials is not None else 9 if args.screen9 else 21
    seed = 20261124 if args.screen9 else 20261125
    rng = random.Random(seed)
    result = {'seed': seed, 'trials': trials, 'cpu': 2, 'harness_sha256': sha(Path(__file__)), 'commands': commands,
              'binary_sha256': {name: sha(path) for name, path in bins.items()}, 'cases': []}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        for case in ['all_same_00', 'all_same_01', 'max_random_00', 'max_random_01']:
            inp, ans = args.official / 'in' / (case + '.in'), args.official / 'out' / (case + '.out')
            samples = {name: [] for name in variants}
            checks, orders = [], []
            for rep in range(-1, trials):
                order = variants[:]
                rng.shuffle(order)
                orders.append(order)
                for name in order:
                    out = work / (case + '_' + name + '.out')
                    with inp.open('rb') as src, out.open('wb') as dest:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(bins[name])], stdin=src, stdout=dest, check=True)
                        elapsed = time.perf_counter_ns() - begin
                    check = subprocess.run([str(args.official / 'checker'), str(inp), str(out), str(ans)], capture_output=True, text=True)
                    checks.append({'rep': rep, 'variant': name, 'returncode': check.returncode, 'stderr': check.stderr, 'output_sha256': sha(out)})
                    if check.returncode:
                        raise RuntimeError(checks[-1])
                    if rep >= 0:
                        samples[name].append(elapsed)
            row = {'case': case, 'input_sha256': sha(inp), 'answer_sha256': sha(ans), 'samples_ns': samples,
                   'orders_including_warmup': orders, 'checks': checks,
                   'median_ns': {name: statistics.median(values) for name, values in samples.items()}}
            result['cases'].append(row)
            (work / 'reproduced.json').write_text(json.dumps(result, indent=2) + '\n')
            print(case, row['median_ns'], flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)


if __name__ == '__main__':
    main()
