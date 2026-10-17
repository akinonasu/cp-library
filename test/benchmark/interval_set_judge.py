import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--cases-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    metadata = json.loads((ROOT / 'test/benchmark/results/interval_set_reference.json').read_text())
    reference = metadata['problems']['predecessor_problem']['references'][0]
    reference_source = args.cache / (str(reference['id']) + '.cpp')
    assert hashlib.sha256(reference_source.read_bytes()).hexdigest() == reference['source_sha256']
    paths = ['data_structure/interval_set.hpp', 'components/fast_io.hpp',
             'test/interval_set_predecessor.test.cpp', 'test/benchmark/interval_set_judge.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'reference_flags': flags + ['-mavx2'], 'reference': reference,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'affinity': [args.cpu], 'trials': args.trials, 'cases': {},
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    rng = random.Random(731)
    rows = []
    cases = ['max_random_00', 'max_all1_00', 'max_sparse_00']
    with tempfile.TemporaryDirectory(prefix='cp-interval-set-judge-') as directory:
        binaries = {}
        for variant, source in [('library', ROOT / 'test/interval_set_predecessor.test.cpp'), ('reference', reference_source)]:
            binary = Path(directory) / variant
            compile_flags = env['reference_flags'] if variant == 'reference' else flags
            subprocess.run(['g++', *compile_flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
            binaries[variant] = binary
        for case in cases:
            input_path = args.cases_dir / (case + '.in')
            expected = (args.cases_dir / (case + '.out')).read_bytes().split()
            env['cases'][case] = {'input_sha256': hashlib.sha256(input_path.read_bytes()).hexdigest(),
                                  'canonical_output_sha256': hashlib.sha256(b' '.join(expected)).hexdigest()}
            for trial in range(args.trials):
                order = ['library', 'reference']
                rng.shuffle(order)
                for variant in order:
                    with input_path.open('rb') as file:
                        start = time.perf_counter()
                        output = subprocess.check_output([str(binaries[variant])], stdin=file)
                        elapsed = time.perf_counter() - start
                    assert output.split() == expected, (case, variant)
                    rows.append({'case': case, 'variant': variant, 'trial': trial, 'seconds': elapsed,
                                 'canonical_output_sha256': env['cases'][case]['canonical_output_sha256']})
            print(case, 'all answers match', flush=True)
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    summary = [{'case': case, 'variant': variant,
                'seconds': statistics.median(r['seconds'] for r in rows if (r['case'], r['variant']) == (case, variant))}
               for case in cases for variant in ['library', 'reference']]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
