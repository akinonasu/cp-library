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
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--rounds', type=int, default=5)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    cache = root / '.verify-helper/cache/dynamic_pattern_matching_references'
    reference = json.loads((root / 'test/benchmark/results/dynamic_pattern_matching/reference.json').read_text())
    for entry in reference['author_sources']:
        assert hashlib.sha256((cache / 'author' / entry['path']).read_bytes()).hexdigest() == entry['sha256']
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    names = ['string/dynamic_pattern_matching.hpp', 'components/persistent_rope.hpp', 'components/suffix_range.hpp',
             'string/suffix_tree.hpp', 'components/suffix_array.hpp', 'components/sequence_compare.hpp',
             'test/benchmark/dynamic_pattern_matching.cpp']
    metadata = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [2], 'rounds': args.rounds,
                'n': 100000, 'pattern_length': 20000, 'queries': 1000,
                'sources': {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names},
                'author_commit': reference['author_commit']}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='dynamic-pattern-bench-') as temporary:
        executable = str(Path(temporary) / 'benchmark')
        author = str(Path(temporary) / 'author')
        source = str(root / 'test/benchmark/dynamic_pattern_matching.cpp')
        subprocess.run(['g++', *flags, source, '-o', executable], check=True)
        subprocess.run(['g++', *flags, '-Wno-error', '-w', '-DAUTHOR_REFERENCE', '-I', str(cache), source, '-o', author], check=True)
        for kind in ['equal', 'binary', 'letters', 'periodic', 'fragmented']:
            for operation in ['build', 'edit', 'splice', 'move']:
                variants = ['dynamic'] if operation == 'build' else ['dynamic', 'kmp']
                if kind == 'equal' and operation == 'edit':
                    variants += ['author']
                expected = None
                for trial in range(args.rounds):
                    order = variants.copy()
                    rng.shuffle(order)
                    for variant in order:
                        command = [author if variant == 'author' else executable, variant, operation, kind, '100000', '1000']
                        ns, checksum, nodes = map(int, subprocess.check_output(command, text=True, timeout=60).split())
                        if expected is None:
                            expected = checksum
                        assert checksum == expected, (kind, operation, variant, checksum, expected)
                        rows.append({'case': kind, 'operation': operation, 'trial': trial, 'variant': variant,
                                     'ns': ns, 'checksum': checksum, 'nodes': nodes})
                record = {'case': kind, 'operation': operation}
                for variant in variants:
                    record[variant + '_ms'] = statistics.median(r['ns'] for r in rows if r['case'] == kind and r['operation'] == operation and r['variant'] == variant) / 1e6
                record['dynamic_nodes'] = next(r['nodes'] for r in rows if r['case'] == kind and r['operation'] == operation and r['variant'] == 'dynamic')
                summary.append(record)
                print(record, flush=True)
    args.output.mkdir(parents=True, exist_ok=True)
    with (args.output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader(); writer.writerows(rows)
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    (args.output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')


if __name__ == '__main__':
    main()
