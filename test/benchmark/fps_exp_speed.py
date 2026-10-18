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

ROOT = Path(__file__).resolve().parents[2]
PROBLEMS = {'exp': 409662, 'pow': 409718}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-exp-speed'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--sizes', nargs='+', type=int, default=[32768, 131073, 500000, 524288])
    parser.add_argument('--cache-record', type=Path)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    result = dict(baseline=baseline, flags=flags, cpu=args.cpu, trials=args.trials, seed=20261019,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), harness_sha256=sha(Path(__file__)),
                  sources={}, binaries={}, official=[], kernel=[], judge=[])
    cached = json.loads(args.cache_record.read_text()) if args.cache_record else {}
    if cached and (cached['baseline'] != baseline or cached['flags'] != flags or cached['compiler'] != result['compiler']):
        raise RuntimeError('Cache conditions differ')

    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        before, current = args.work / 'before', args.work / 'final_candidate'
        if before.exists():
            shutil.rmtree(before)
        data = subprocess.check_output(['git', 'archive', baseline, 'fps', 'components', 'math', 'convolution',
                                        'test/fps_io.hpp', 'test/fps_exp.test.cpp', 'test/fps_pow.test.cpp'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(before, filter='data')
        if current.exists():
            shutil.rmtree(current)
        for folder in ['fps', 'components', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, current / folder)
        (current / 'test').mkdir()
        for file in ['fps_io.hpp', 'fps_exp.test.cpp', 'fps_pow.test.cpp']:
            shutil.copy2(ROOT / 'test' / file, current / 'test' / file)
        kernel_source = args.work / 'kernel.cpp'
        shutil.copy2(ROOT / 'test/benchmark/fps_tuning_basic.cpp', kernel_source)
        result['driver_sha256'] = sha(kernel_source)
        for name, tree in [('before', before), ('current', current)]:
            sources = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}
            result['sources'][name] = sources
            for operation, source in [('kernel', kernel_source)] + [(op, tree / f'test/fps_{op}.test.cpp') for op in PROBLEMS]:
                binary = args.work / (('final' if name == 'current' else name) + '_' + operation)
                cache_ok = (name == 'before' and sources == cached.get('sources', {}).get('before') and
                            binary.is_file() and sha(binary) == cached.get('binaries', {}).get(binary.name))
                if not cache_ok:
                    subprocess.run(['g++', *flags, '-I', str(tree), str(source), '-o', str(binary)], check=True)
                result['binaries'][name + '_' + operation] = dict(path=str(binary), sha256=sha(binary))
                print('built', name, operation, flush=True)
        for operation, reference in PROBLEMS.items():
            problem = operation + '_of_formal_power_series'
            source = args.references / problem / (str(reference) + '.cpp')
            binary = args.work / ('reference_' + operation)
            result['sources'][str(reference)] = sha(source)
            cache_ok = (sha(source) == cached.get('sources', {}).get(str(reference)) and binary.is_file() and
                        sha(binary) == cached.get('binaries', {}).get(binary.name))
            if not cache_ok:
                subprocess.run(['g++', *flags, str(source), '-o', str(binary)], check=True)
            result['binaries']['reference_' + operation] = dict(path=str(binary), sha256=sha(binary))
            folder = args.official_root / 'polynomial' / problem
            cases = []
            for path in sorted((folder / 'in').glob('*.in')):
                output = args.work / 'actual.out'
                with path.open('rb') as inp, output.open('wb') as out:
                    subprocess.run([str(args.work / ('final_' + operation))], stdin=inp, stdout=out, check=True)
                subprocess.run([str(folder / 'checker'), str(path), str(folder / 'out' / (path.stem + '.out')), str(output)],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                cases.append(dict(input=path.name, sha256=sha(path)))
            result['official'].append(dict(operation=operation, cases=cases))
            print('official', operation, len(cases), flush=True)
        save()
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        rng = random.Random(20261019)
        for operation in PROBLEMS:
            for n in args.sizes:
                samples, checks = {'before': [], 'current': []}, set()
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for name in order:
                        binary = result['binaries'][name + '_kernel']['path']
                        elapsed, checksum = subprocess.check_output([binary, operation, str(n)], text=True).split()
                        checks.add(checksum)
                        if trial:
                            samples[name].append(float(elapsed) * 1000)
                assert len(checks) == 1, (operation, n, checks)
                row = dict(operation=operation, n=n, checksum=checks.pop(), samples_ms=samples,
                           median_ms={k: statistics.median(v) for k, v in samples.items()})
                result['kernel'].append(row)
                print('kernel', operation, n, row['median_ms'], flush=True)
                save()
            folder = args.official_root / 'polynomial' / (operation + '_of_formal_power_series')
            path = folder / 'in/max_random_00.in'
            samples = {'before': [], 'current': [], 'reference': []}
            for trial in range(args.trials + 1):
                order = list(samples)
                rng.shuffle(order)
                for name in order:
                    output = args.work / 'actual.out'
                    with path.open('rb') as inp, output.open('wb') as out:
                        begin = time.perf_counter_ns()
                        subprocess.run([result['binaries'][name + '_' + operation]['path']], stdin=inp, stdout=out, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(path), str(folder / 'out/max_random_00.out'), str(output)],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                    if trial:
                        samples[name].append(elapsed)
            row = dict(operation=operation, input=path.name, input_sha256=sha(path), samples_ms=samples,
                       median_ms={k: statistics.median(v) for k, v in samples.items()})
            result['judge'].append(row)
            print('judge', operation, row['median_ms'], flush=True)
            save()


if __name__ == '__main__':
    main()
