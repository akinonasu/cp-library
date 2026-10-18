import argparse
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import random
import shutil
import statistics
import subprocess
import tarfile
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--official-root', type=Path, required=True)
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--series-size', type=int, default=500000)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    rng = random.Random(20261019)
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    official = args.official_root.resolve() / 'polynomial'
    reference = args.references.resolve()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result = dict(baseline=baseline, flags=flags, trials=args.trials, seed=20261019, cpu_affinity=[args.cpu],
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), platform=platform.platform(),
                  driver_sha256=sha(ROOT / 'test/benchmark/fps_division.cpp'), harness_sha256=sha(Path(__file__)),
                  metric='whole process wall milliseconds; computation includes allocations and excludes I/O',
                  sources={}, references={}, workloads={})
    with tempfile.TemporaryDirectory(prefix='fps-division-') as directory, open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        work = Path(directory)
        fcntl.flock(lock, fcntl.LOCK_SH)
        before, current = work / 'baseline', work / 'current'
        archive = subprocess.check_output(['git', 'archive', baseline, 'fps', 'components', 'math', 'convolution', 'test/fps_io.hpp'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
            tar.extractall(before, filter='data')
        for folder in ['fps', 'components', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, current / folder)
        (current / 'test').mkdir()
        shutil.copy2(ROOT / 'test/fps_io.hpp', current / 'test/fps_io.hpp')
        binaries = {}
        for variant, tree in [('baseline', before), ('current', current)]:
            result['sources'][variant] = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*.hpp'))}
            for mode, operation in enumerate(['division', 'inverse_mod', 'gcd', 'series']):
                name = operation + '_' + variant
                binary = work / name
                subprocess.run(['g++', *flags, '-I', str(tree), '-DMODE=' + str(mode), str(ROOT / 'test/benchmark/fps_division.cpp'), '-o', str(binary)], check=True)
                binaries[name] = binary
                print('built', name, flush=True)
        for operation, source_name, expected_sha in [
            ('division', 'division_of_polynomials/409561.cpp', 'fd06ec63bfe326a8f8c2b25292ecf38d77c4fd0114fbaa2565e4c667b8b4eeb1'),
            ('inverse_mod', 'inv_of_polynomials/409589.cpp', '274d6c6107676bf6043aa5984e4b5788617ae7c8ff0ae1f157b6b0954e895306')]:
            source = reference / source_name
            if sha(source) != expected_sha:
                raise RuntimeError('Reference source changed: ' + str(source))
            name = operation + '_reference'
            binary = work / name
            subprocess.run(['g++', *flags, str(source), '-o', str(binary)], check=True)
            binaries[name] = binary
            result['references'][name] = dict(source=source_name, sha256=sha(source))
            print('built', name, flush=True)
        cases = []
        for name in ['v_random_00', 'v_random_01', 'v_random_02', 'v_random_03', 'u_eq_0_00']:
            cases.append(('division_' + name, official / 'division_of_polynomials/in' / (name + '.in'),
                          ['division_baseline', 'division_current', 'division_reference'], 'division_of_polynomials'))
        maximum = official / 'inv_of_polynomials/in/max_random_00.in'
        cases.append(('inverse_max', maximum, ['inverse_mod_baseline', 'inverse_mod_current', 'inverse_mod_reference'], 'inv_of_polynomials'))
        cases.append(('gcd_max', maximum, ['gcd_baseline', 'gcd_current'], None))
        n = args.series_size
        series = work / 'series.in'
        series.write_text(f'{n} {n}\n' + ' '.join(str(rng.randrange(998244353)) for _ in range(n)) + '\n1 ' +
                          ' '.join(str(rng.randrange(998244353)) for _ in range(n - 1)) + '\n')
        cases.append(('series_max', series, ['series_baseline', 'series_current'], None))
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        for name, inp, programs, problem in cases:
            expected = None
            whole = {p: [] for p in programs}
            computation = {p: [] for p in programs}
            schedule = programs.copy()
            order = programs * args.trials
            rng.shuffle(order)
            schedule.extend(order)
            for index, program in enumerate(schedule):
                output = work / (program + '.txt')
                with inp.open('rb') as f, output.open('wb') as g:
                    start = time.perf_counter_ns()
                    process = subprocess.run([str(binaries[program])], stdin=f, stdout=g, stderr=subprocess.PIPE, check=True)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                if problem:
                    folder = official / problem
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(folder / 'out' / (inp.stem + '.out'))],
                                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                else:
                    tokens = b' '.join(output.read_bytes().split())
                    if expected is None:
                        expected = tokens
                    if expected != tokens:
                        raise RuntimeError((name, program, 'Output differs'))
                if index >= len(programs):
                    whole[program].append(elapsed)
                    if process.stderr:
                        computation[program].append(float(process.stderr))
            item = dict(input=inp.name, input_sha256=sha(inp), input_header=inp.open().readline().strip(), binaries={})
            for program in programs:
                item['binaries'][program] = dict(binary_sha256=sha(binaries[program]), whole_ms=whole[program],
                    median_ms=statistics.median(whole[program]), computation_ms=computation[program],
                    compute_median_ms=statistics.median(computation[program]) if computation[program] else None)
            result['workloads'][name] = item
            print(name, {p: round(item['binaries'][p]['median_ms'], 3) for p in programs}, flush=True)
            args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
