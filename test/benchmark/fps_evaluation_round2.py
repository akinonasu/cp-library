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

from fps_evaluation import PROBLEMS

ROOT = Path(__file__).resolve().parents[2]
CANDIDATES = Path(__file__).with_suffix('')
BASELINE = '144ae32387675fa2a889ea3e8cd19b03ad5016bf'
GROUPS = {
    'evaluation': (['eval', 'interpolate'], ['before', 'evaluation_boundary', 'reference']),
    'taylor_fused': (['taylor'], ['before', 'taylor_input', 'taylor_both', 'reference']),
    'taylor_vectors': (['taylor'], ['before', 'vectors1', 'vectors2', 'reference']),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--baseline-tree', type=Path)
    parser.add_argument('--groups', choices=GROUPS, nargs='+', default=list(GROUPS))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--reference-root', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline=args.baseline, flags=flags, cpu=args.cpu, trials=args.trials,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  hardware=subprocess.check_output(['lscpu'], text=True), seed=20261019,
                  harness_sha256=sha(Path(__file__)), sources={}, builds={}, checks=[], results=[])
    needed = {v for group in args.groups for v in GROUPS[group][1] if v != 'reference'}
    before = args.output / 'before'
    if before.exists():
        shutil.rmtree(before)
    if args.baseline_tree:
        shutil.copytree(args.baseline_tree, before)
    else:
        data = subprocess.check_output(['git', 'archive', args.baseline, 'fps', 'components', 'math', 'convolution', 'test'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(before, filter='data')
    trees = {'before': before}
    for variant in sorted(needed - {'before'}):
        tree = args.output / variant
        if tree.exists():
            shutil.rmtree(tree)
        shutil.copytree(before, tree)
        if variant.startswith('vectors'):
            p = tree / 'components/fps_taylor_blocked.hpp'
            p.write_text(p.read_text().replace('kVectors = 4, kLanes = 8 * kVectors',
                                              'kVectors = ' + variant[-1] + ', kLanes = 8 * kVectors'))
        else:
            patch = CANDIDATES / (variant + '.patch')
            subprocess.run(['patch', '-s', '-p1', '-d', str(tree), '-i', str(patch)], check=True)
            result['sources'][variant + '_patch'] = sha(patch)
        trees[variant] = tree
    for variant, tree in trees.items():
        result['sources'][variant] = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*.hpp'))}
    jobs = {(op, variant) for group in args.groups for op in GROUPS[group][0] for variant in GROUPS[group][1]}
    binaries = {}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        for op, variant in sorted(jobs):
            problem, driver, sid = PROBLEMS[op]
            binary = args.output / (op + '_' + variant)
            source = args.reference_root / problem / (str(sid) + '.cpp') if variant == 'reference' else trees[variant] / 'test' / driver
            command = ['g++', *flags]
            if variant != 'reference':
                command += ['-I' + str(trees[variant])]
            command += [str(source), '-o', str(binary)]
            fcntl.flock(lock, fcntl.LOCK_SH)
            subprocess.run(command, check=True)
            fcntl.flock(lock, fcntl.LOCK_UN)
            binaries[op, variant] = binary
            result['builds'][op + '_' + variant] = dict(command=command, source_sha256=sha(source), binary_sha256=sha(binary))
            print('built', op, variant, flush=True)
        for op, variant in sorted(jobs):
            if variant in ['before', 'reference']:
                continue
            problem = PROBLEMS[op][0]
            folder = args.official_root / 'polynomial' / problem
            count = 0
            fcntl.flock(lock, fcntl.LOCK_SH)
            for inp in sorted((folder / 'in').glob('*.in')):
                output = args.output / 'actual.out'
                with inp.open('rb') as src, output.open('wb') as dst:
                    subprocess.run([str(binaries[op, variant])], stdin=src, stdout=dst, check=True)
                subprocess.run([str(folder / 'checker'), str(inp), str(output), str(folder / 'out' / inp.with_suffix('.out').name)],
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                count += 1
            fcntl.flock(lock, fcntl.LOCK_UN)
            result['checks'].append(dict(op=op, variant=variant, cases=count))
        fcntl.flock(lock, fcntl.LOCK_EX)
        rng = random.Random(20261019)
        for group in args.groups:
            operations, variants = GROUPS[group]
            for op in operations:
                folder = args.official_root / 'polynomial' / PROBLEMS[op][0]
                for name in ['max_random_00.in', 'max_random_01.in']:
                    inp = folder / 'in' / name
                    samples = {v: [] for v in variants}
                    orders = []
                    for trial in range(args.trials + 1):
                        order = variants[:]
                        rng.shuffle(order)
                        orders.append(order)
                        for variant in order:
                            output = args.output / 'actual.out'
                            with inp.open('rb') as src, output.open('wb') as dst:
                                start = time.perf_counter_ns()
                                subprocess.run([str(binaries[op, variant])], stdin=src, stdout=dst, check=True)
                                elapsed = (time.perf_counter_ns() - start) / 1e6
                            if trial:
                                samples[variant].append(elapsed)
                            subprocess.run([str(folder / 'checker'), str(inp), str(output), str(folder / 'out' / inp.with_suffix('.out').name)],
                                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    deltas = {v: [a - b for a, b in zip(samples['before'], samples[v])]
                              for v in variants if v not in ['before', 'reference']}
                    row = dict(group=group, op=op, input=name, input_sha256=sha(inp), orders=orders, samples_ms=samples,
                               median_ms={v: statistics.median(s) for v, s in samples.items()}, paired_deltas_ms=deltas,
                               paired_median_saving_ms={v: statistics.median(s) for v, s in deltas.items()},
                               candidate_wins={v: sum(x > 0 for x in s) for v, s in deltas.items()})
                    result['results'].append(row)
                    print(group, op, name, row['median_ms'], flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
