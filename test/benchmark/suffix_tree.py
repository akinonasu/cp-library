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
import tarfile
import urllib.request


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--rounds', type=int, default=7)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    cache = root / '.verify-helper/cache/suffix_tree_references'
    reference = json.loads((root / 'test/benchmark/results/suffix_tree/reference.json').read_text())['acl'][0]
    if not (cache / 'acl_string.hpp').exists():
        cache.mkdir(parents=True, exist_ok=True)
        with urllib.request.urlopen(reference['url'], timeout=60) as response:
            (cache / 'acl_string.hpp').write_bytes(response.read())
    assert hashlib.sha256((cache / 'acl_string.hpp').read_bytes()).hexdigest() == reference['sha256']
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root), '-isystem', str(cache)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    metadata = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [2], 'rounds': args.rounds,
                'n': 500000, 'queries': 20000, 'seed': 20261020, 'acl': reference}
    names = ['string/suffix_tree.hpp', 'components/suffix_array.hpp', 'components/sequence_compare.hpp',
             'components/static_range_min.hpp', 'data_structure/range_query/sparse_table.hpp',
             'components/default_init_allocator.hpp', 'test/benchmark/suffix_tree.cpp']
    metadata['sources'] = {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}
    baseline = root / 'test/benchmark/results/suffix_tree/initial_sources.tar.gz'
    metadata['baseline_archive_sha256'] = hashlib.sha256(baseline.read_bytes()).hexdigest()
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='suffix-tree-bench-') as temporary:
        with tarfile.open(baseline) as archive:
            source = archive.extractfile('string/suffix_tree.hpp').read()
        source = source.replace(b'SUFFIX_TREE_HPP', b'SUFFIX_TREE_BASE_HPP').replace(b'SuffixTree', b'BaseSuffixTree').replace(b'"../components/', b'"components/')
        (Path(temporary) / 'suffix_tree_base.hpp').write_bytes(source)
        executable = str(Path(temporary) / 'benchmark')
        subprocess.run(['g++', *flags, '-I', temporary, str(root / 'test/benchmark/suffix_tree.cpp'), '-o', executable], check=True)
        for kind in ['binary', 'letters', 'bytes', 'equal', 'periodic']:
            for operation, variants in [('build', ['acl', 'induced', 'base', 'tree']), ('query', ['binary_search', 'base_find', 'find'])]:
                expected = None
                for trial in range(args.rounds):
                    order = variants.copy()
                    rng.shuffle(order)
                    for variant in order:
                        ns, checksum = map(int, subprocess.check_output([executable, variant, kind, '500000'], text=True, timeout=60).split())
                        if expected is None:
                            expected = checksum
                        assert checksum == expected, (kind, operation, variant)
                        rows.append({'case': kind, 'operation': operation, 'trial': trial, 'variant': variant, 'ns': ns, 'checksum': checksum})
                record = {'case': kind, 'operation': operation}
                for variant in variants:
                    record[variant + '_ms'] = statistics.median(row['ns'] for row in rows if row['case'] == kind and row['operation'] == operation and row['variant'] == variant) / 1e6
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
