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
REFERENCES = {'inv': 409370, 'sqrt': 409728}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='ce81108')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-inverse-final'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--sizes', nargs='+', type=int, default=[32768, 131073, 500000, 524288])
    args = parser.parse_args()
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    args.work.mkdir(parents=True, exist_ok=True)
    result = dict(baseline=subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), flags=flags, cpu=args.cpu,
                  trials=args.trials, harness_sha256=digest(Path(__file__)), sources={}, kernel={}, whole={})
    binaries = {}

    def save():
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        baseline, current = args.work / 'baseline', args.work / 'current'
        for folder in [baseline, current]:
            if folder.exists():
                shutil.rmtree(folder)
            folder.mkdir()
        archive = subprocess.check_output(['git', 'archive', args.baseline], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
            tar.extractall(baseline, filter='data')
        for folder in ['components', 'convolution', 'fps', 'math']:
            shutil.copytree(ROOT / folder, current / folder)
        (current / 'test').mkdir()
        for filename in ['fps_io.hpp', 'fps_inv.test.cpp', 'fps_sqrt.test.cpp']:
            shutil.copy2(ROOT / 'test' / filename, current / 'test' / filename)
        driver = args.work / 'kernel.cpp'
        shutil.copy2(ROOT / 'test/benchmark/fps_inverse_sqrt.cpp', driver)

        def build(name, source, includes):
            binary = args.work / name
            command = ['g++', *flags, '-I', str(includes), str(source), '-o', str(binary)]
            subprocess.run(command, check=True)
            binaries[name] = binary
            result['sources'][name] = dict(source_sha256=digest(source), binary_sha256=digest(binary), command=command)
            print('built', name, flush=True)

        for variant, tree in [('baseline', baseline), ('current', current)]:
            result['sources'][variant + '_headers'] = {
                str(p.relative_to(tree)): digest(p)
                for folder in ['components', 'convolution', 'fps', 'math']
                for p in (tree / folder).rglob('*.hpp')
            }
            build(variant + '_kernel', driver, tree)
            for op in REFERENCES:
                build(variant + '_' + op, tree / 'test' / f'fps_{op}.test.cpp', tree)
        for op, sid in REFERENCES.items():
            source = args.references / (op + '_of_formal_power_series') / f'{sid}.cpp'
            build('reference_' + op, source, current)
            result['sources']['reference_' + op]['submission'] = f'https://judge.yosupo.jp/submission/{sid}'
        fcntl.flock(lock, fcntl.LOCK_UN)
        save()
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        rng = random.Random(20261019)
        for op in REFERENCES:
            for n in args.sizes:
                samples = {v: [] for v in ['baseline', 'current']}
                hashes = {v: set() for v in samples}
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for variant in order:
                        text = subprocess.check_output([str(binaries[variant + '_kernel']), op, str(n)], text=True)
                        elapsed, checksum = text.split()
                        hashes[variant].add(checksum)
                        if trial:
                            samples[variant].append(float(elapsed))
                assert len(set.union(*hashes.values())) == 1, (op, n, hashes)
                row = {v: dict(samples_ms=s, median_ms=statistics.median(s), checksum=next(iter(hashes[v])))
                       for v, s in samples.items()}
                result['kernel'][op + ':' + str(n)] = row
                save()
                print('kernel', op, n, {v: x['median_ms'] for v, x in row.items()}, flush=True)
            folder = args.official_root / 'polynomial' / (op + '_of_formal_power_series')
            filename = 'max_random_01' if op == 'sqrt' else 'max_random_00'
            inp, answer = folder / 'in' / (filename + '.in'), folder / 'out' / (filename + '.out')
            samples = {v: [] for v in ['baseline', 'current', 'reference']}
            for trial in range(args.trials + 1):
                order = list(samples)
                rng.shuffle(order)
                for variant in order:
                    output = args.work / (variant + '_' + op + '.out')
                    with inp.open('rb') as fin, output.open('wb') as fout:
                        start = time.perf_counter_ns()
                        subprocess.run([str(binaries[variant + '_' + op])], stdin=fin, stdout=fout, check=True)
                        elapsed = (time.perf_counter_ns() - start) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                    if trial:
                        samples[variant].append(elapsed)
            result['whole'][op] = dict(input=str(inp), input_sha256=digest(inp), answer_sha256=digest(answer),
                                      variants={v: dict(samples_ms=s, median_ms=statistics.median(s)) for v, s in samples.items()})
            save()
            print('whole', op, {v: x['median_ms'] for v, x in result['whole'][op]['variants'].items()}, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)


if __name__ == '__main__':
    main()
