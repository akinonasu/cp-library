import argparse
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
REVISION = '520e97212aa2fb7f277accbcd0b764213dce5d93'
OFFICIAL_REVISION = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
HEADER = 'components/fps_convolution.hpp'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def check_hash(path, expected):
    actual = sha(path)
    if actual != expected:
        raise RuntimeError(f'SHA-256 mismatch: {path}: {actual} != {expected}')
    return actual


def main():
    parser = argparse.ArgumentParser(description='Reconstruct and optionally measure the two rejected input-fusion candidates.')
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-conv-fused-reproduction'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
    parser.add_argument('--lock', type=Path, default=Path('/tmp/cp-fps-tuning.lock'))
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--measure', action='store_true')
    args = parser.parse_args()
    if args.prepare_only and args.measure:
        parser.error('--prepare-only and --measure are mutually exclusive')
    if args.trials < 1:
        parser.error('--trials must be positive')
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or repo in work.parents:
        parser.error('--work must be outside the repository')
    work.mkdir(parents=True, exist_ok=True)
    recorded = {v: json.loads((HERE / f'{v}.json').read_text()) for v in ('canonical', 'lazy')}
    result = dict(base_revision=REVISION, official_revision=OFFICIAL_REVISION, flags=FLAGS,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  cpu=args.cpu, trials=args.trials, source_hashes={}, build_commands={}, verification={}, measurements=[])
    trees = {v: work / v for v in ('base', 'candidate', 'lazy')}
    binaries = {v: work / ('whole_' + v) for v in ('base', 'candidate', 'lazy', 'reference')}

    def save():
        (work / 'reproduction.json').write_text(json.dumps(result, indent=2) + '\n')

    with args.lock.open('w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        archive = subprocess.check_output(['git', 'archive', REVISION, 'fps', 'components', 'math', 'convolution',
                                           'test/convolution.test.cpp', 'test/fps_io.hpp', 'test/benchmark/fps_tuning_basic.cpp'], cwd=repo)
        for tree in trees.values():
            if tree.exists():
                shutil.rmtree(tree)
        with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
            stream.extractall(trees['base'], filter='data')
        for variant, patch in (('candidate', 'canonical.patch'), ('lazy', 'lazy.patch')):
            shutil.copytree(trees['base'], trees[variant])
            subprocess.run(['patch', '-p1', '-i', str(HERE / patch)], cwd=trees[variant], check=True)
        expected = dict(recorded['canonical']['source_hashes'], **recorded['lazy']['source_hashes'])
        for variant, tree in trees.items():
            check_hash(tree / HEADER, expected[variant])
            result['source_hashes'][variant] = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}
        result['source_hashes_match_recorded'] = True
        if args.prepare_only:
            save()
            print('Prepared all three trees; recorded header SHA-256 values match.')
            return
        result['reference_source_sha256'] = check_hash(args.reference, recorded['canonical']['reference_source_sha256'])
        for variant, tree in trees.items():
            command = ['g++', *FLAGS, '-I', str(tree), str(tree / 'test/convolution.test.cpp'), '-o', str(binaries[variant])]
            subprocess.run(command, check=True)
            result['build_commands'][variant] = command
        command = ['g++', *FLAGS, str(args.reference.resolve()), '-o', str(binaries['reference'])]
        subprocess.run(command, check=True)
        result['build_commands']['reference'] = command
        result['binary_hashes'] = {v: sha(path) for v, path in binaries.items()}
        for variant in ('candidate', 'lazy'):
            binary = work / ('verify_' + variant)
            command = ['g++', *FLAGS, '-Wall', '-Wextra', '-Werror', '-I', str(trees[variant]), str(HERE / 'verify.cpp'), '-o', str(binary)]
            subprocess.run(command, check=True)
            checked = subprocess.run([str(binary)], text=True, capture_output=True, check=True)
            result['verification'][variant] = dict(command=command, stdout=checked.stdout, stderr=checked.stderr, returncode=checked.returncode)
        save()
        if not args.measure:
            print('Built all variants; both boundary/convolution/input-preservation checks passed.')
            return
        folder = args.official_root.resolve() / 'convolution/convolution_mod'
        inp, answer = folder / 'in/max_random_00.in', folder / 'out/max_random_00.out'
        measured_case = recorded['canonical']['cases'][0]
        result['input_sha256'] = check_hash(inp, measured_case['input_sha256'])
        result['answer_sha256'] = check_hash(answer, measured_case['answer_sha256'])
        result['official_checkout'] = subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip()
        if result['official_checkout'] != OFFICIAL_REVISION:
            raise RuntimeError('Use the recorded Library Checker commit: ' + OFFICIAL_REVISION)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        for name, candidate in (('canonical', 'candidate'), ('lazy', 'lazy')):
            variants = ['base', candidate, 'reference']
            samples = {v: [] for v in variants}
            rng = random.Random(20261020)
            for rep in range(args.trials + 1):
                order = variants.copy()
                rng.shuffle(order)
                for variant in order:
                    output = work / (name + '_' + variant + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(binaries[variant])], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if rep:
                        samples[variant].append(elapsed)
            row = dict(experiment=name, variants={v: dict(median_ms=statistics.median(s), samples_ms=s,
                       output_sha256=sha(work / (name + '_' + v + '.out'))) for v, s in samples.items()})
            result['measurements'].append(row)
            save()
            print(name, {v: statistics.median(s) for v, s in samples.items()}, flush=True)


if __name__ == '__main__':
    main()
