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
    report = json.loads((root / 'test/benchmark/results/kinetic_tournament/reference.json').read_text())
    reference = report['problems']['range_linear_add_range_min']['references'][0]
    source = (root / '.verify-helper/cache/kinetic_range_references' / (str(reference['overview']['id']) + '.cpp')).read_bytes()
    assert hashlib.sha256(source).hexdigest() == reference['sha256']
    start = source.index(b'class KineticRangeLinearAddMinTree {')
    end = source.index(b'\n} // namespace toy', start)
    extracted = b'#include <algorithm>\n#include <cstdint>\n#include <limits>\n#include <vector>\nnamespace reference {\n' + source[start:end] + b'\n}\n'
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    names = ['data_structure/segment_tree/kinetic_tournament.hpp', 'components/kinetic_tournament_monoid.hpp',
             'data_structure/segment_tree/generic_segtree_beats.hpp', 'test/benchmark/kinetic_tournament.cpp',
             'test/benchmark/kinetic_tournament_jsc.cpp']
    environment = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                   'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [2], 'rounds': args.rounds,
                   'n': 100000, 'q': 100000, 'jsc_n': 500000, 'reference': reference,
                   'sources': {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names},
                   'extracted_reference_sha256': hashlib.sha256(extracted).hexdigest()}
    rows, summary = [], []
    rng = random.Random(20261020)
    with tempfile.TemporaryDirectory(prefix='kinetic-range-bench-') as temporary:
        temporary = Path(temporary)
        (temporary / 'reference.hpp').write_bytes(extracted)
        executable = str(temporary / 'benchmark')
        subprocess.run(['g++', *flags, '-I', str(temporary), 'test/benchmark/kinetic_tournament.cpp', '-o', executable], check=True, cwd=root)
        cases = [('tree', c, ['library', 'reference']) for c in ['random', 'partial', 'envelope', 'jump']]
        cases += [('jsc', c, ['jsc']) for c in ['equal', 'ascending', 'descending', 'alternating', 'random']]
        for group, case, variants in cases:
            expected = None
            for trial in range(args.rounds):
                order = variants.copy(); rng.shuffle(order)
                for variant in order:
                    ns, checksum = map(int, subprocess.check_output([executable, variant, case], text=True, timeout=60).split())
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (group, case, variant)
                    rows.append({'group': group, 'case': case, 'trial': trial, 'variant': variant, 'ns': ns, 'checksum': checksum})
            record = {'group': group, 'case': case}
            for variant in variants:
                record[variant + '_ms'] = statistics.median(r['ns'] for r in rows if r['group'] == group and r['case'] == case and r['variant'] == variant) / 1e6
            summary.append(record)
            print(record, flush=True)
    args.output.mkdir(parents=True, exist_ok=True)
    with (args.output / 'samples.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys()); writer.writeheader(); writer.writerows(rows)
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')


if __name__ == '__main__':
    main()
