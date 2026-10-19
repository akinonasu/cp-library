import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
REFERENCES = {'convolution_mod': 409465, 'inv_of_formal_power_series': 409370,
              'sqrt_of_formal_power_series': 409728, 'log_of_formal_power_series': 409452}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def headers(root):
    return {str(p.relative_to(root)): sha(p) for p in sorted(root.rglob('*.hpp'))}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-root', type=Path, default=Path('/tmp/fps-avx512-top-base'))
    parser.add_argument('--variant', action='append', default=[])
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--kernel', nargs='*', default=[])
    parser.add_argument('--sizes', nargs='+', type=int, default=[262144, 500000])
    parser.add_argument('--whole', nargs='*', default=[])
    parser.add_argument('--cases', nargs='+', default=['max_random_00', 'max_random_01'])
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--seed', type=int, default=20261028)
    args = parser.parse_args()
    variants = {'base': args.baseline_root, **{name: Path(tree) for name, tree in (v.split('=', 1) for v in args.variant)}}
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result = dict(baseline='bd951dce6fdcb3ea9db4fcef342147012a8ab0c0', flags=FLAGS,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), cpu=2, trials=args.trials,
                  seed=args.seed, harness_sha256=sha(Path(__file__)),
                  headers_before={v: headers(tree) for v, tree in variants.items()}, sources={}, results=[])
    jobs = []
    if args.kernel:
        for v, tree in variants.items():
            source = args.baseline_root / 'test/benchmark/fps_tuning_basic.cpp'
            binary = args.work / (v + '_kernel')
            jobs.append((v + '_kernel', source, binary, ['g++', *FLAGS, '-I', str(tree), str(source), '-o', str(binary)]))
    folders = {p.parent.name: p.parent for p in args.official_root.glob('*/*/info.toml')}
    for problem in args.whole:
        for v, tree in variants.items():
            source = next(p for p in sorted((tree / 'test').glob('*.test.cpp'))
                          if f'"https://judge.yosupo.jp/problem/{problem}"' in p.read_text())
            binary = args.work / (problem + '_' + v)
            jobs.append((problem + '_' + v, source, binary, ['g++', *FLAGS, '-I', str(tree), str(source), '-o', str(binary)]))
        source = args.references / problem / (str(REFERENCES[problem]) + '.cpp')
        binary = args.work / (problem + '_reference')
        jobs.append((problem + '_reference', source, binary, ['g++', *FLAGS, str(source), '-o', str(binary)]))

    def build(job):
        name, source, binary, command = job
        subprocess.run(command, check=True)
        return name, dict(command=command, source_sha256=sha(source), binary_sha256=sha(binary))

    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        with ThreadPoolExecutor(max_workers=2) as pool:
            for key, data in pool.map(build, jobs):
                result['sources'][key] = data
                save()
                print('built', key, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        print('waiting EX', flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        print('EX acquired', flush=True)
        os.sched_setaffinity(0, {2})
        rng = random.Random(args.seed)
        for op in args.kernel:
            for n in args.sizes:
                samples = {v: [] for v in variants}
                checksums = set()
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for v in order:
                        elapsed, checksum = subprocess.check_output([str(args.work / (v + '_kernel')), op, str(n)], text=True).split()
                        checksums.add(checksum)
                        if trial:
                            samples[v].append(float(elapsed) * 1000)
                assert len(checksums) == 1, (op, n, checksums)
                row = dict(kind='kernel', op=op, n=n, checksum=checksums.pop(),
                           variants={v: dict(median_ms=statistics.median(s), samples_ms=s) for v, s in samples.items()})
                result['results'].append(row)
                save()
                print(op, n, {v: data['median_ms'] for v, data in row['variants'].items()}, flush=True)
        for problem in args.whole:
            folder = folders[problem]
            for case in args.cases:
                inp, answer = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
                if not inp.exists():
                    continue
                samples = {v: [] for v in [*variants, 'reference']}
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for v in order:
                        output = args.work / (problem + '_' + case + '_' + v + '.out')
                        with inp.open('rb') as fin, output.open('wb') as fout:
                            start = time.perf_counter_ns()
                            subprocess.run([str(args.work / (problem + '_' + v))], stdin=fin, stdout=fout, check=True)
                            elapsed = (time.perf_counter_ns() - start) / 1e6
                        subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        if trial:
                            samples[v].append(elapsed)
                row = dict(kind='whole', problem=problem, case=case, input_sha256=sha(inp),
                           answer_sha256=sha(answer), checker_sha256=sha(folder / 'checker'),
                           variants={v: dict(median_ms=statistics.median(s), samples_ms=s,
                                            output_sha256=sha(args.work / (problem + '_' + case + '_' + v + '.out')))
                                     for v, s in samples.items()})
                result['results'].append(row)
                save()
                print(problem, case, {v: data['median_ms'] for v, data in row['variants'].items()}, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
    result['headers_after'] = {v: headers(tree) for v, tree in variants.items()}
    result['dependencies_unchanged'] = result['headers_before'] == result['headers_after']
    assert result['dependencies_unchanged']
    save()


if __name__ == '__main__':
    main()
