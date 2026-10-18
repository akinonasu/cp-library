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
    'compose': ('composition_of_formal_power_series', 'fps_compose.test.cpp', 409336),
    'compose_large': ('composition_of_formal_power_series_large', 'fps_compose_large.test.cpp', 409535),
    'inverse': ('compositional_inverse_of_formal_power_series', 'fps_compositional_inverse.test.cpp', 409399),
    'inverse_large': ('compositional_inverse_of_formal_power_series_large', 'fps_compositional_inverse_large.test.cpp', 409549),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-composition-speed'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--operations', nargs='+', default=list(PROBLEMS))
    parser.add_argument('--sizes', nargs='+', type=int, default=[4096, 32768, 131072])
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline=subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), flags=flags, cpu=args.cpu,
                  trials=args.trials, harness_sha256=sha(Path(__file__)), kernel=[], judge=[], sources={})
    def save():
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        before, current = args.work / 'before', args.work / 'current'
        for path in (before, current):
            if path.exists(): shutil.rmtree(path)
        data = subprocess.check_output(['git', 'archive', args.baseline, 'fps', 'components', 'math', 'convolution', 'test/fps_io.hpp'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive: archive.extractall(before, filter='data')
        for folder in ('fps', 'components', 'math', 'convolution'): shutil.copytree(ROOT / folder, current / folder)
        (current / 'test').mkdir()
        shutil.copy2(ROOT / 'test/fps_io.hpp', current / 'test/fps_io.hpp')
        builds = {}
        fcntl.flock(lock, fcntl.LOCK_UN)
        def build(name, src, include):
            source, exe = args.work / (name + '.cpp'), args.work / name
            source.write_text(src)
            fingerprint = hashlib.sha256((src + json.dumps({str(p.relative_to(include)): sha(p) for folder in ('fps', 'components', 'math', 'convolution') for p in (include / folder).rglob('*.hpp')}, sort_keys=True) + repr(flags)).encode()).hexdigest()
            key = args.work / (name + '.key')
            command = ['g++', *flags, '-I', str(include), '-I', str(include / 'test'), str(source), '-o', str(exe)]
            if not exe.exists() or not key.exists() or key.read_text() != fingerprint:
                fcntl.flock(lock, fcntl.LOCK_SH)
                try:
                    subprocess.run(command, check=True, capture_output=True)
                    key.write_text(fingerprint)
                finally:
                    fcntl.flock(lock, fcntl.LOCK_UN)
            builds[name] = exe
            result['sources'][name] = dict(source_sha256=sha(source), binary_sha256=sha(exe), command=command)
            print('built', name, flush=True)
        driver = (ROOT / 'test/benchmark/fps_tuning_advanced.cpp').read_text()
        for variant, tree in [('before', before), ('current', current)]:
            result['sources'][variant + '_headers'] = {str(p.relative_to(tree)): sha(p) for folder in ('fps', 'components', 'math', 'convolution') for p in (tree / folder).rglob('*.hpp')}
            build('kernel_' + variant, driver, tree)
        for op in args.operations:
            problem, filename, refid = PROBLEMS[op]
            for variant, tree in [('before', before), ('current', current)]:
                build(op + '_' + variant, (ROOT / 'test' / filename).read_text(), tree)
            ref = args.references / problem / (str(refid) + '.cpp')
            build(op + '_reference', ref.read_text(), current)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        rng = random.Random(20261019)
        for op in ('compose', 'inverse', 'projection', 'projection_weighted'):
            for n in args.sizes:
                row = dict(operation=op, n=n, variants={})
                sums, samples = {}, {v: [] for v in ('before', 'current')}
                for rep in range(args.trials + 1):
                    order = list(samples); rng.shuffle(order)
                    for v in order:
                        text = subprocess.check_output([str(builds['kernel_' + v]), op, 'current', str(n)], text=True)
                        ms, checksum = text.split(); sums[v] = checksum
                        if rep: samples[v].append(float(ms))
                assert len(set(sums.values())) == 1, (op, n, sums)
                for v, values in samples.items(): row['variants'][v] = dict(median_ms=statistics.median(values), samples_ms=values, checksum=sums[v])
                result['kernel'].append(row); save(); print('kernel', op, n, {v:x['median_ms'] for v,x in row['variants'].items()}, flush=True)
        for op in args.operations:
            problem, _, _ = PROBLEMS[op]
            folder = args.official_root / 'polynomial' / problem
            path = folder / 'in/max_random_00.in'
            answer = folder / 'out/max_random_00.out'
            row = dict(problem=problem, input_sha256=sha(path), answer_sha256=sha(answer), variants={})
            samples = {v: [] for v in ('before', 'current', 'reference')}
            for rep in range(args.trials + 1):
                order = list(samples); rng.shuffle(order)
                for v in order:
                    output = args.work / (op + '_' + v + '.out')
                    with path.open('rb') as inp, output.open('wb') as out:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(builds[op + '_' + v])], stdin=inp, stdout=out, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    checked = subprocess.run([str(folder / 'checker'), str(path), str(output), str(answer)], capture_output=True, text=True)
                    if checked.returncode: raise RuntimeError(checked.stderr)
                    if rep: samples[v].append(elapsed)
            for v, values in samples.items(): row['variants'][v] = dict(median_ms=statistics.median(values), samples_ms=values)
            result['judge'].append(row); save(); print('judge', problem, {v:x['median_ms'] for v,x in row['variants'].items()}, flush=True)


if __name__ == '__main__': main()
