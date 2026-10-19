import argparse
import fcntl
import gzip
import hashlib
import io
import json
import random
import shutil
import statistics
import subprocess
import tarfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--build', action='store_true')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--official', type=Path, default=Path('/workspace/cp-fps-scratch-round3/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--seed', type=int, default=20261123)
    args = parser.parse_args()
    meta = json.loads((HERE / 'metadata.json').read_text())
    manifest = json.loads((HERE / 'manifest.json').read_text())
    for name, expected in manifest.items():
        if sha(HERE / name) != expected:
            raise ValueError('artifact mismatch: ' + name)
    work = args.work.resolve()
    if work.exists():
        raise ValueError('work already exists')
    work.mkdir(parents=True)
    data = subprocess.check_output(['git', 'archive', meta['baseline'], 'fps', 'components', 'math', 'convolution', 'test/fps_io.hpp', 'test/convolution.test.cpp'], cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        archive.extractall(work / 'snapshot', filter='data')
    for name, expected in meta['baseline_sources'].items():
        if sha(work / 'snapshot' / name) != expected:
            raise ValueError('baseline mismatch: ' + name)
    shutil.copytree(work / 'snapshot', work / 'candidate')
    subprocess.run(['patch', '-s', '-p1', '-d', str(work / 'candidate'), '-i', str(HERE / 'unbalanced.patch')], check=True)
    unit = work / 'candidate/test/unit/fps_convolution_unbalanced.cpp'
    unit.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(HERE / 'fps_convolution_unbalanced.cpp', unit)
    for name, expected in meta['candidate_sources'].items():
        if sha(work / 'candidate' / name) != expected:
            raise ValueError('candidate mismatch: ' + name)
    reference = work / '409465.cpp'
    reference.write_bytes(gzip.decompress((HERE / '409465.cpp.gz').read_bytes()))
    if sha(reference) != meta['reference']['sha256']:
        raise ValueError('reference mismatch')
    print('Artifact, baseline, candidate, reference SHA-256 checks passed.', flush=True)
    if not (args.build or args.check or args.measure):
        return
    flags = meta['flags']
    commands = [
        ['g++', *flags, 'snapshot/test/convolution.test.cpp', '-o', 'before'],
        ['g++', *[x.replace('Isnapshot', 'Icandidate') for x in flags], 'candidate/test/convolution.test.cpp', '-o', 'current'],
        ['g++', *flags, '409465.cpp', '-o', 'reference']]
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        for command in commands:
            subprocess.run(command, cwd=work, check=True)
        if args.check:
            subprocess.run(['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-Icandidate', 'candidate/test/unit/fps_convolution_unbalanced.cpp', '-o', 'unit'], cwd=work, check=True)
            subprocess.run([str(work / 'unit')], check=True)
            for inp in sorted((args.official / 'in').glob('*.in')):
                out = work / 'check.out'
                with inp.open('rb') as src, out.open('wb') as dest:
                    subprocess.run([str(work / 'current')], stdin=src, stdout=dest, check=True)
                subprocess.run([str(args.official / 'checker'), str(inp), str(out), str(args.official / 'out' / (inp.stem + '.out'))], check=True, stdout=subprocess.DEVNULL)
        fcntl.flock(lock, fcntl.LOCK_UN)
    if not args.measure:
        return
    rng = random.Random(args.seed)
    result = {'seed': args.seed, 'trials': args.trials, 'cpu': 2, 'harness_sha256': sha(Path(__file__)), 'binary_sha256': {name: sha(work / name) for name in ['before', 'current', 'reference']}, 'cases': []}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for case in ['small_and_large_00', 'small_and_large_01', 'small_and_large_02', 'small_and_large_03']:
            inp, ans = args.official / 'in' / (case + '.in'), args.official / 'out' / (case + '.out')
            row = {'case': case, 'input_sha256': sha(inp), 'answer_sha256': sha(ans), 'samples_ns': {name: [] for name in ['before', 'current', 'reference']}, 'order_including_warmup': [], 'checks': []}
            for trial in range(-1, args.trials):
                order = ['before', 'reference', 'current']
                rng.shuffle(order)
                row['order_including_warmup'].append(order)
                for name in order:
                    out = work / (case + '_' + name + '.out')
                    with inp.open('rb') as src, out.open('wb') as dest:
                        start = time.perf_counter_ns()
                        subprocess.run(['taskset', '-c', '2', str(work / name)], stdin=src, stdout=dest, check=True)
                        elapsed = time.perf_counter_ns() - start
                    check = subprocess.run([str(args.official / 'checker'), str(inp), str(out), str(ans)], capture_output=True, text=True)
                    row['checks'].append({'trial': trial, 'binary': name, 'returncode': check.returncode, 'output_sha256': sha(out), 'stderr': check.stderr})
                    if check.returncode:
                        raise RuntimeError(row['checks'][-1])
                    if trial >= 0:
                        row['samples_ns'][name].append(elapsed)
            row['medians_ns'] = {name: statistics.median(values) for name, values in row['samples_ns'].items()}
            result['cases'].append(row)
            (work / 'reproduced.json').write_text(json.dumps(result, indent=2) + '\n')
            print(case, row['medians_ns'], flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)


if __name__ == '__main__':
    main()
