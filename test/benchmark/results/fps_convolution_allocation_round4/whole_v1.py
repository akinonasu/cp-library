import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-root', type=Path, required=True)
    parser.add_argument('--variant', action='append', default=[])
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
    parser.add_argument('--cases', nargs='+', default=['max_random_00', 'max_random_01', 'max_ans_zero_00',
                                                     'fft_killer_09', 'small_and_large_03', 'medium_02'])
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {2})
    trees = {'before': args.baseline_root.resolve()}
    for variant in args.variant:
        name, path = variant.split('=', 1)
        if name in trees or name == 'reference':
            raise ValueError('duplicate variant')
        trees[name] = Path(path).resolve()
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(baseline='7521ea8703a362aaf3ccd370188fccab86050848', flags=flags, cpu=2,
                  trials=args.trials, seed=20261041, harness_sha256=sha(Path(__file__)),
                  compiler=subprocess.check_output(['g++', '--version'], text=True),
                  hardware=subprocess.check_output(['lscpu'], text=True), sources={}, builds={}, checks=[], results=[])
    for variant, tree in trees.items():
        result['sources'][variant] = {str(p.relative_to(tree)): sha(p)
                                      for folder in ['fps', 'components', 'math', 'convolution']
                                      for p in sorted((tree / folder).rglob('*.hpp'))}
    binaries = {}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        for variant in [*trees, 'reference']:
            source = args.reference if variant == 'reference' else trees[variant] / 'test/convolution.test.cpp'
            binary = args.output / variant
            command = ['g++', *flags]
            if variant != 'reference':
                command += ['-I' + str(trees[variant])]
            command += [str(source), '-o', str(binary)]
            signature = dict(command=command, headers=result['sources'].get(variant), source_sha256=sha(source))
            manifest = binary.with_suffix('.build.json')
            if not binary.exists() or not manifest.exists() or json.loads(manifest.read_text()) != signature:
                fcntl.flock(lock, fcntl.LOCK_SH)
                subprocess.run(command, check=True)
                fcntl.flock(lock, fcntl.LOCK_UN)
                manifest.write_text(json.dumps(signature, indent=2) + '\n')
            binaries[variant] = binary
            result['builds'][variant] = dict(**signature, binary_sha256=sha(binary))
            print('built', variant, flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        rng = random.Random(20261041)
        for filename in args.cases:
            inp = args.official / 'in' / (filename + '.in')
            samples = {name: [] for name in binaries}
            orders = []
            for trial in range(args.trials + 1):
                order = list(binaries)
                rng.shuffle(order)
                orders.append(order)
                for variant in order:
                    output = args.output / 'actual.out'
                    with inp.open('rb') as src, output.open('wb') as dst:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(binaries[variant])], stdin=src, stdout=dst, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    if trial:
                        samples[variant].append(elapsed)
                    subprocess.run([str(args.official / 'checker'), str(inp), str(output),
                                    str(args.official / 'out' / inp.with_suffix('.out').name)],
                                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            deltas = {name: [a - b for a, b in zip(samples['before'], values)]
                      for name, values in samples.items() if name not in ['before', 'reference']}
            row = dict(input=filename, input_sha256=sha(inp), answer_sha256=sha(args.official / 'out' / (filename + '.out')), checker_sha256=sha(args.official / 'checker'), orders=orders, samples_ms=samples,
                       median_ms={name: statistics.median(values) for name, values in samples.items()},
                       paired_saving_ms={name: statistics.median(values) for name, values in deltas.items()},
                       wins={name: sum(x > 0 for x in values) for name, values in deltas.items()})
            result['results'].append(row)
            print(filename, row['median_ms'], row['paired_saving_ms'], row['wins'], flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
    (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
