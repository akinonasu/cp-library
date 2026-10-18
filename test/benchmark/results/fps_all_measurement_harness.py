import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import re
import shutil
import statistics
import subprocess
import tarfile
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-all-speed'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--problems', nargs='+')
    parser.add_argument('--old-io', action='store_true')
    args = parser.parse_args()
    previous = json.loads(args.output.read_text()) if args.output.exists() else {}
    metadata = json.loads((ROOT / 'test/benchmark/results/fps_speed_reference.json').read_text())
    problems = args.problems or [name for name, row in metadata['problems'].items() if row.get('references')]
    folders = {p.parent.name: p.parent for p in args.official_root.glob('*/*/info.toml')}
    drivers = {}
    for p in sorted((ROOT / 'test').glob('*.test.cpp')):
        match = re.search(r'#define PROBLEM "https://judge.yosupo.jp/problem/([^\"]+)"', p.read_text())
        if match:
            drivers.setdefault(match[1], p)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline=subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), flags=flags, cpu=args.cpu, trials=args.trials,
                  official_commit=subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip(),
                  harness_sha256=sha(Path(__file__)), sources={}, judge=[])
    args.work.mkdir(parents=True, exist_ok=True)

    def save():
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        trees = {v: args.work / v for v in ('before', 'current')}
        for path in trees.values():
            if path.exists():
                shutil.rmtree(path)
        data = subprocess.check_output(['git', 'archive', args.baseline, 'fps', 'components', 'math', 'convolution', 'test/fps_io.hpp', *dict.fromkeys(str(drivers[p].relative_to(ROOT)) for p in problems)], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(trees['before'], filter='data')
        for folder in ('fps', 'components', 'math', 'convolution', 'test'):
            shutil.copytree(ROOT / folder, trees['current'] / folder, ignore=shutil.ignore_patterns('results', '__pycache__'))
        if args.old_io:
            trees['current_old_io'] = args.work / 'current_old_io'
            if trees['current_old_io'].exists():
                shutil.rmtree(trees['current_old_io'])
            shutil.copytree(trees['current'], trees['current_old_io'])
            shutil.copy2(trees['before'] / 'components/fps_io.hpp', trees['current_old_io'] / 'components/fps_io.hpp')
        binaries = {}
        for v, tree in trees.items():
            result['sources'][v] = {str(p.relative_to(tree)): sha(p) for folder in ('fps', 'components', 'math', 'convolution') for p in (tree / folder).glob('*.hpp')}
        jobs = []
        for problem in problems:
            ref = next(row for row in metadata['problems'][problem]['references'] if row['lang'] == 'cpp')
            for v in [*trees, 'reference']:
                source = args.references / problem / (str(ref['id']) + '.cpp') if v == 'reference' else trees[v] / drivers[problem].relative_to(ROOT)
                tree = trees['current'] if v == 'reference' else trees[v]
                jobs.append((problem, v, source, tree))

        def build(job):
            problem, v, source, tree = job
            binary = args.work / (problem + '_' + v)
            command = ['g++', *flags, '-I', str(tree), str(source), '-o', str(binary)]
            old = previous.get('sources', {}).get(problem + '_' + v, {})
            cacheable = (previous.get('flags') == flags and previous.get('compiler') == result['compiler']
                         and previous.get('baseline') == result['baseline'] and old.get('source_sha256') == sha(source)
                         and old.get('command') == command and binary.exists() and old.get('binary_sha256') == sha(binary))
            if v == 'reference':
                cacheable &= not re.search(r'^\s*#\s*include\s*"', source.read_text(), re.M)
            else:
                cacheable &= previous.get('sources', {}).get(v) == result['sources'][v]
            if not cacheable:
                subprocess.run(command, check=True)
            return problem, v, binary, dict(command=command, source_sha256=sha(source), binary_sha256=sha(binary), cached=bool(cacheable))

        with ThreadPoolExecutor(max_workers=3) as pool:
            for problem, v, binary, info in pool.map(build, jobs):
                binaries[problem, v] = binary
                result['sources'][problem + '_' + v] = info
                print('built', problem, v, flush=True)
        save()
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        rng = random.Random(20261019)
        for problem in problems:
            folder = folders[problem]
            case = 'max_random_01' if problem == 'sqrt_of_formal_power_series' else 'max_random_00'
            inp, answer = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
            if not inp.exists():
                inp = max((folder / 'in').glob('*.in'), key=lambda p: p.stat().st_size)
                answer = folder / 'out' / (inp.stem + '.out')
            row = dict(problem=problem, input=inp.name, input_sha256=sha(inp), answer_sha256=sha(answer),
                       reference=metadata['problems'][problem]['references'][0], variants={})
            samples = {v: [] for v in [*trees, 'reference']}
            for rep in range(args.trials + 1):
                order = list(samples)
                rng.shuffle(order)
                for v in order:
                    output = args.work / (problem + '_' + v + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(binaries[problem, v])], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if rep:
                        samples[v].append(elapsed)
            row['variants'] = {v: dict(median_ms=statistics.median(s), samples_ms=s, output_sha256=sha(args.work / (problem + '_' + v + '.out'))) for v, s in samples.items()}
            result['judge'].append(row)
            save()
            print(problem, {v: r['median_ms'] for v, r in row['variants'].items()}, flush=True)


if __name__ == '__main__':
    main()
