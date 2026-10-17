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

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '0841637ff8d5f59b4021775e812f66abef82cbdd'


def main():
    parser = argparse.ArgumentParser()
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
    baseline = subprocess.check_output(['git', 'show', BASELINE + ':math/fps.hpp'], cwd=ROOT)
    paths = [p.relative_to(ROOT).as_posix() for p in sorted((ROOT / 'fps').glob('*.hpp'))]
    paths += ['components/fps_operations.hpp', 'components/fps_product_tree.hpp', 'components/ntt.hpp',
              'math/mint.hpp', 'test/benchmark/fps_split.cpp', 'test/benchmark/fps_split.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'baseline_commit': BASELINE, 'baseline_sha256': hashlib.sha256(baseline).hexdigest(),
           'seed': 731, 'trials': args.trials, 'affinity': [args.cpu],
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    rows = []
    rng = random.Random(731)
    operations = ['bostan_mori', 'linear_recurrence', 'compose', 'compositional_inverse', 'multipoint_eval',
                  'interpolate', 'taylor_shift', 'inv', 'exp']
    with tempfile.TemporaryDirectory(prefix='cp-fps-split-') as directory:
        tmp = Path(directory)
        old_header = tmp / 'old.hpp'
        old_header.write_bytes(baseline.replace(b'../components/ntt.hpp', b'components/ntt.hpp'))
        binaries = {}
        for variant in ['before', 'after']:
            binary = tmp / variant
            variant_flags = [f'-DFPS_SPLIT_BASELINE="{old_header}"'] if variant == 'before' else []
            subprocess.run(['g++', *flags, *variant_flags, '-I', str(ROOT), str(ROOT / 'test/benchmark/fps_split.cpp'),
                            '-o', str(binary)], check=True)
            binaries[variant] = binary
        for operation in operations:
            n = 32768
            expected = None
            for trial in range(args.trials):
                order = ['before', 'after']
                rng.shuffle(order)
                for variant in order:
                    elapsed, result = subprocess.check_output([str(binaries[variant]), operation, str(n)]).splitlines()
                    if expected is None:
                        expected = result
                    assert result == expected, (operation, variant)
                    rows.append({'operation': operation, 'size': n, 'variant': variant, 'trial': trial,
                                 'seconds': float(elapsed), 'checksum': result.decode()})
            print(operation, 'checksums match', flush=True)
    summary = [{'operation': op, 'variant': variant,
                'seconds': statistics.median(r['seconds'] for r in rows if (r['operation'], r['variant']) == (op, variant))}
               for op in operations for variant in ['before', 'after']]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
