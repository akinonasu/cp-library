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
PROBLEMS = {
    'eval': ('multipoint_evaluation', 'multipoint_eval.test.cpp', 409516),
    'interpolate': ('polynomial_interpolation', 'fps_interpolate.test.cpp', 409545),
    'taylor': ('polynomial_taylor_shift', 'fps_taylor_shift.test.cpp', 409534),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--cpu', default=2, type=int)
    parser.add_argument('--trials', default=11, type=int)
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--reference-root', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--operations', nargs='+', choices=PROBLEMS, default=list(PROBLEMS))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(environment=dict(baseline=baseline, cpu=args.cpu, trials=args.trials, flags=flags,
                                   seed=20261019, harness_sha256=sha(Path(__file__)),
                                   compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                                   hardware=subprocess.check_output(['lscpu'], text=True)),
                  sources={}, checks=[], results=[])
    before, current = args.output / 'before', args.output / 'current'
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        for tree in [before, current]:
            if tree.exists():
                shutil.rmtree(tree)
        data = subprocess.check_output(['git', 'archive', baseline, 'fps', 'components', 'math', 'convolution', 'test'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(before, filter='data')
        for folder in ['fps', 'components', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, current / folder)
        (current / 'test').mkdir()
        shutil.copy2(ROOT / 'test/fps_io.hpp', current / 'test/fps_io.hpp')
        for op in args.operations:
            driver = PROBLEMS[op][1]
            shutil.copy2(ROOT / 'test' / driver, current / 'test' / driver)
        fcntl.flock(lock, fcntl.LOCK_UN)
        for variant, tree in [('before', before), ('current', current)]:
            result['sources'][variant] = {p.relative_to(tree).as_posix(): sha(p) for p in tree.rglob('*.hpp')}
        binaries = {}
        for op in args.operations:
            problem, driver, sid = PROBLEMS[op]
            reference = args.reference_root / problem / (str(sid) + '.cpp')
            for variant, source, include in [('before', before / 'test' / driver, before),
                                             ('current', current / 'test' / driver, current),
                                             ('reference', reference, current)]:
                binary = args.output / (op + '_' + variant)
                signature = dict(source_sha256=sha(source), flags=flags,
                                 headers=result['sources'].get(variant, {}))
                signature_path = binary.with_suffix('.build.json')
                if not binary.exists() or not signature_path.exists() or json.loads(signature_path.read_text()) != signature:
                    fcntl.flock(lock, fcntl.LOCK_SH)
                    subprocess.run(['g++', *flags, '-I' + str(include), str(source), '-o', str(binary)], check=True)
                    fcntl.flock(lock, fcntl.LOCK_UN)
                    signature_path.write_text(json.dumps(signature, indent=2) + '\n')
                binaries[op, variant] = binary
                result['sources'][op + '_' + variant] = dict(source_sha256=sha(source), binary_sha256=sha(binary),
                                                             reference_submission=sid if variant == 'reference' else None)
                print('built', op, variant, flush=True)
        for op in args.operations:
            problem = PROBLEMS[op][0]
            official = args.official_root / 'polynomial' / problem
            fcntl.flock(lock, fcntl.LOCK_SH)
            count = 0
            for inp in sorted((official / 'in').glob('*.in')):
                output = args.output / (op + '.out')
                with inp.open('rb') as src, output.open('wb') as dst:
                    subprocess.run([str(binaries[op, 'current'])], stdin=src, stdout=dst, check=True)
                subprocess.run([str(official / 'checker'), str(inp), str(output),
                                str(official / 'out' / inp.with_suffix('.out').name)],
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                count += 1
            fcntl.flock(lock, fcntl.LOCK_UN)
            result['checks'].append(dict(op=op, cases=count))
            print('checked', op, count, flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        rng = random.Random(20261019)
        for op in args.operations:
            problem = PROBLEMS[op][0]
            official = args.official_root / 'polynomial' / problem
            for name in ['max_random_00.in', 'max_random_01.in']:
                inp = official / 'in' / name
                samples = {v: [] for v in ['before', 'current', 'reference']}
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for variant in order:
                        output = args.output / (op + '_' + variant + '.out')
                        with inp.open('rb') as src, output.open('wb') as dst:
                            start = time.perf_counter_ns()
                            subprocess.run([str(binaries[op, variant])], stdin=src, stdout=dst, check=True)
                            elapsed = (time.perf_counter_ns() - start) / 1e6
                        if trial:
                            samples[variant].append(elapsed)
                        subprocess.run([str(official / 'checker'), str(inp), str(output),
                                        str(official / 'out' / inp.with_suffix('.out').name)],
                                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                row = dict(op=op, input=name, input_sha256=sha(inp), samples_ms=samples,
                           median_ms={v: statistics.median(values) for v, values in samples.items()})
                result['results'].append(row)
                print(op, name, row['median_ms'], flush=True)
        (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
