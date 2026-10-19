import argparse
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    metadata = json.loads((HERE / 'metadata.json').read_text())
    parser = argparse.ArgumentParser()
    parser.add_argument('--variant', choices=list(metadata['candidates']), default='borrow32_assign')
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--trials', type=int, default=41)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
    args = parser.parse_args()
    args.work = args.work.resolve()
    if args.work.exists():
        raise ValueError('work directory already exists')
    for name, expected in metadata['files'].items():
        if sha(HERE / name) != expected:
            raise ValueError('artifact hash mismatch: ' + name)
    args.work.mkdir(parents=True)
    baseline = args.work / 'before'
    data = subprocess.check_output(['git', 'archive', metadata['baseline'], 'fps', 'components', 'math',
                                    'convolution', 'test'], cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        archive.extractall(baseline, filter='data')
    for name, expected in metadata['baseline_headers'].items():
        if sha(baseline / name) != expected:
            raise ValueError('baseline header hash mismatch: ' + name)
    candidate = args.work / 'candidate'
    shutil.copytree(baseline, candidate)
    info = metadata['candidates'][args.variant]
    patch = HERE / (args.variant + '.patch')
    if sha(patch) != info['patch_sha256']:
        raise ValueError('patch hash mismatch')
    subprocess.run(['patch', '-s', '-p1', '-d', str(candidate), '-i', str(patch)], check=True)
    if sha(candidate / info['header']) != info['candidate_sha256']:
        raise ValueError('candidate header hash mismatch')
    source = candidate / 'test/unit/fps_convolution_borrow.cpp'
    shutil.copyfile(HERE / 'fps_convolution_borrow.cpp', source)
    print('Prepared', args.variant, flush=True)
    if args.check:
        checks = []
        with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
            for name, flags in [('normal', ['-Werror']),
                                ('sanitize', ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g'])]:
                binary = args.work / ('unit_' + name)
                command = ['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', *flags,
                           '-I' + str(candidate), str(source), '-o', str(binary)]
                env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
                fcntl.flock(lock, fcntl.LOCK_SH)
                subprocess.run(command, check=True)
                subprocess.run([str(binary)], check=True, env=env)
                fcntl.flock(lock, fcntl.LOCK_UN)
                checks.append(dict(name=name, command=command, binary_sha256=sha(binary), passed=True))
        (args.work / 'unit_results.json').write_text(json.dumps(checks, indent=2) + '\n')
    if args.measure:
        subprocess.run([sys.executable, str(HERE / 'initial_whole.py'), '--baseline-root', str(baseline),
                        '--variant', args.variant + '=' + str(candidate), '--output', str(args.work / 'measurement'),
                        '--trials', str(args.trials), '--official', str(args.official), '--reference', str(args.reference),
                        '--cases', 'max_random_00', 'max_random_01', 'small_and_large_03', 'medium_02'], check=True)


if __name__ == '__main__':
    main()
