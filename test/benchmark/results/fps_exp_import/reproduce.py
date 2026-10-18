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
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description='Reconstruct and optionally check/measure exp import candidates.')
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-exp-import-reproduction'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/exp_of_formal_power_series/409662.cpp'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--verify', action='store_true')
    parser.add_argument('--measure', action='store_true')
    args = parser.parse_args()
    if args.prepare_only and (args.verify or args.measure):
        parser.error('--prepare-only excludes --verify and --measure')
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or repo in work.parents:
        parser.error('--work must be outside the repository')
    recorded = json.loads((HERE / 'metadata.json').read_text())
    raw = {v: json.loads((HERE / (v + '.json')).read_text()) for v in ('inline', 'outlined', 'confirm')}
    work.mkdir(parents=True, exist_ok=True)
    trees = {v: work / v for v in ('baseline', 'inline', 'outlined')}
    result = dict(baseline=recorded['baseline'], flags=FLAGS, headers={}, builds={}, checks=[], measurements=[])
    binaries = {v: work / ('whole_' + v) for v in ('baseline', 'inline', 'outlined', 'reference')}
    def save():
        (work / 'reproduction.json').write_text(json.dumps(result, indent=2) + '\n')
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        archive = subprocess.check_output(['git', 'archive', recorded['baseline'], 'fps', 'components', 'math', 'convolution',
                                           'test/fps_exp.test.cpp', 'test/fps_io.hpp', 'test/unit/fps_exp_blocked.cpp'], cwd=repo)
        for tree in trees.values():
            if tree.exists():
                shutil.rmtree(tree)
        with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
            stream.extractall(trees['baseline'], filter='data')
        for variant in ('inline', 'outlined'):
            shutil.copytree(trees['baseline'], trees[variant])
            subprocess.run(['patch', '-p1', '-i', str(HERE / (variant + '.patch'))], cwd=trees[variant], check=True)
        expected = dict(baseline=recorded['baseline_header_sha256'], inline=raw['inline']['source_hashes']['candidate'],
                        outlined=recorded['outlined_header_sha256'])
        for variant, tree in trees.items():
            result['headers'][variant] = sha(tree / 'components/fps_exp_blocked.hpp')
            if result['headers'][variant] != expected[variant]:
                raise RuntimeError('Restored header differs: ' + variant)
        save()
        if not args.verify and not args.measure:
            print('All three header SHA-256 values match the measured sources.')
            return
        result['compiler'] = subprocess.check_output(['g++', '--version'], text=True).splitlines()[0]
        result['reference_sha256'] = sha(args.reference)
        if result['reference_sha256'] != recorded['reference_sha256']:
            raise RuntimeError('Reference source differs')
        for variant, tree in trees.items():
            command = ['g++', *FLAGS, '-I', str(tree), str(tree / 'test/fps_exp.test.cpp'), '-o', str(binaries[variant])]
            subprocess.run(command, check=True)
            result['builds'][variant] = dict(command=command, binary_sha256=sha(binaries[variant]))
        command = ['g++', *FLAGS, str(args.reference.resolve()), '-o', str(binaries['reference'])]
        subprocess.run(command, check=True)
        result['builds']['reference'] = dict(command=command, binary_sha256=sha(binaries['reference']))
        if args.verify:
            environment = os.environ.copy()
            environment.update(ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
            tests = [('existing', trees['outlined'] / 'test/unit/fps_exp_blocked.cpp', []),
                     ('import', HERE / 'unit.cpp', []), ('sanitize', HERE / 'unit.cpp', ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g'])]
            for name, source, extra in tests:
                binary = work / ('unit_' + name)
                command = ['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1', '-I', str(trees['outlined']), *extra, str(source), '-o', str(binary)]
                subprocess.run(command, check=True, env=environment)
                checked = subprocess.run([str(binary)], check=True, env=environment, capture_output=True, text=True, timeout=120)
                result['checks'].append(dict(name=name, command=command, returncode=checked.returncode, stdout=checked.stdout, stderr=checked.stderr))
            save()
        if not args.measure:
            return
        folder = args.official_root.resolve() / 'polynomial/exp_of_formal_power_series'
        inp, answer = folder / 'in/max_random_00.in', folder / 'out/max_random_00.out'
        expected_case = raw['inline']['cases'][0]
        if sha(inp) != expected_case['input_sha256'] or sha(answer) != expected_case['answer_sha256']:
            raise RuntimeError('Official input/answer differs')
        micro = work / 'import_kernel'
        command = ['g++', *FLAGS, '-I', str(trees['outlined']), str(HERE / 'import_kernel.cpp'), '-o', str(micro)]
        subprocess.run(command, check=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        for experiment, candidate, seed in (('inline', 'inline', 20261022), ('outlined', 'outlined', 20261024), ('confirm', 'outlined', 20261025)):
            variants = ['baseline', candidate, 'reference']
            samples = {v: [] for v in variants}
            rng = random.Random(seed)
            for rep in range(22):
                order = variants.copy()
                rng.shuffle(order)
                for variant in order:
                    output = work / (experiment + '_' + variant + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(binaries[variant])], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if rep:
                        samples[variant].append(elapsed)
            row = dict(experiment=experiment, shuffle_seed=seed, variants={v: dict(samples_ms=s, median_ms=statistics.median(s)) for v, s in samples.items()})
            result['measurements'].append(row)
            save()
            print(experiment, {v: statistics.median(s) for v, s in samples.items()}, flush=True)
        samples = {v: [] for v in ('old', 'new')}
        checksums = set()
        rng = random.Random(20261023)
        for rep in range(10):
            order = list(samples)
            rng.shuffle(order)
            for variant in order:
                output = subprocess.check_output([str(micro), variant], text=True).split()
                checksums.add(output[1])
                if rep:
                    samples[variant].append(float(output[0]))
        if len(checksums) != 1:
            raise RuntimeError('Import checksums differ')
        result['import'] = dict(samples_ms=samples, median_ms={v: statistics.median(s) for v, s in samples.items()}, checksum=next(iter(checksums)))
        save()


if __name__ == '__main__':
    main()
