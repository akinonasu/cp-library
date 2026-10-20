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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=9)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--strings-only', action='store_true')
    parser.add_argument('--long', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    cases = [('lcs', kind, 262144, 64) for kind in ['identical', 'near', 'prefix', 'suffix']]
    cases += [('restore', 'identical', 262144, 64)]
    cases += [('edit', kind, 262144, 128) for kind in ['identical', 'near', 'prefix', 'suffix']]
    cases += [(op, 'random', 4096, 32) for op in ['lcs', 'edit']]
    cases += [(op, 'random', 16, 100000) for op in ['lcs', 'edit']]
    cases += [('runs', kind, 100000, 3) for kind in ['random', 'periodic']]
    cases += [('prime', kind, 30000, 1) for kind in ['random32', 'random64', 'prime32', 'prime64']]
    cases += [('factor', 'random32', 5000, 1), ('factor', 'random64', 1000, 1),
              ('factor', 'semiprime', 2000, 1), ('factor', 'hard', 200, 1)]
    if args.strings_only:
        cases = [case for case in cases if case[0] not in ['prime', 'factor']]
    if args.long:
        cases = [(op, kind, size, repeats * (32 if op == 'lcs' and size > 4096 else
                                             16 if op == 'edit' and size > 4096 else
                                             16 if op == 'runs' else 4))
                 for op, kind, size, repeats in cases]
    metadata = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu],
                'baseline': json.loads((args.baseline.parent / 'baseline.json').read_text()),
                'rounds': args.rounds, 'samples': [], 'summary': []}
    metadata['sources'] = {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
                           for folder in ['components', 'string', 'math/number_theory'] for p in (root / folder).glob('*.hpp')}
    metadata['benchmark_sha256'] = hashlib.sha256((root / 'test/benchmark/shared_optimization.cpp').read_bytes()).hexdigest()
    with tempfile.TemporaryDirectory(prefix='shared-optimization-') as directory:
        binaries = {}
        for name, includes in [('before', [args.baseline.resolve(), root]), ('after', [root])]:
            binary = str(Path(directory) / name)
            include_flags = [v for path in includes for v in ['-I', str(path)]]
            subprocess.run(['g++', *flags, *include_flags, str(root / 'test/benchmark/shared_optimization.cpp'), '-o', binary], check=True)
            binaries[name] = binary
        rng = random.Random(20261019)
        for case in cases:
            samples = {'before': [], 'after': []}
            expected = None
            for trial in range(args.rounds):
                order = ['before', 'after']
                rng.shuffle(order)
                for name in order:
                    ns, checksum = map(int, subprocess.check_output([binaries[name], *map(str, case)], text=True).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (case, name, checksum, expected)
                    samples[name].append(ns)
                    metadata['samples'].append({'case': case, 'trial': trial, 'implementation': name, 'ns': ns, 'checksum': checksum})
            before, after = [statistics.median(samples[name]) / 1e6 for name in ['before', 'after']]
            metadata['summary'].append({'case': case, 'before_ms': before, 'after_ms': after, 'ratio': after / before})
            print(*case, f'{before:.3f} -> {after:.3f} ms ({after / before:.3f})', flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    samples = metadata.pop('samples')
    samples_path = args.output.with_suffix('.csv')
    with samples_path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=['case', 'trial', 'implementation', 'ns', 'checksum'])
        writer.writeheader()
        for row in samples:
            writer.writerow(dict(row, case='|'.join(map(str, row['case']))))
    metadata['samples_file'] = samples_path.name
    args.output.write_text(json.dumps(metadata, indent=2) + '\n')


if __name__ == '__main__':
    main()
