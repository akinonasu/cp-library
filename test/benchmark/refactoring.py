import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess


ROOT = Path(__file__).resolve().parents[2]
BASE = '7a67c6e'
TARGETS = ['data_structure/range_query/sparse_table.hpp', 'data_structure/range_query/disjoint_sparse_table.hpp',
           'data_structure/segment_tree/dynamic_segtree.hpp', 'data_structure/weighted_union_find.hpp', 'tree/rerooting.hpp']


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--fraction-only', action='store_true')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    lock = open('/tmp/cp-wm-opt.lock', 'w')
    fcntl.flock(lock, fcntl.LOCK_EX)
    cache = ROOT / '.verify-helper/cache/refactoring'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-ftrivial-auto-var-init=zero']
    if not args.generic:
        flags.append('-march=native')
    sources = {}
    targets = ['math/fraction.hpp'] if args.fraction_only else TARGETS
    program = 'refactoring_fraction.cpp' if args.fraction_only else 'refactoring.cpp'
    for name in ['before', 'after']:
        directory = cache / name
        for folder in (['math'] if args.fraction_only else ['components', 'data_structure', 'tree']):
            for path in (ROOT / folder).rglob('*.hpp'):
                target = directory / path.relative_to(ROOT)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, target)
        if name == 'before':
            for path in targets:
                (directory / path).write_bytes(subprocess.check_output(['git', 'show', f'{BASE}:{path}']))
        sources[name] = {p: hashlib.sha256((directory / p).read_bytes()).hexdigest() for p in targets}
        for variant, extra in ([('release', ['-DNDEBUG'])] if args.fraction_only else [('release', ['-DNDEBUG']), ('debug', [])]):
            subprocess.run(['g++', *flags, *extra, '-I' + str(directory), '-I' + str(ROOT),
                            str(ROOT / 'test/benchmark' / program), '-o', str(directory / variant)], check=True)
    cases = []
    for mode, size in [('sparse', 200000), ('disjoint', 200000), ('sparse_strings', 20000)]:
        for reuse in [0, 1]:
            cases.append(('release', [mode, 'random', str(size), '5', str(reuse)]))
    for shape in ['random', 'cluster', 'increasing', 'decreasing']:
        cases.append(('release', ['dynamic', shape, '200000', '1', '0']))
    for shape in ['random', 'path', 'star']:
        for reuse in [0, 1]:
            cases.append(('release', ['rerooting', shape, '200000', '5', str(reuse)]))
    for variant in ['release', 'debug']:
        cases.append((variant, ['weighted_union_find', 'random', '200000', '1', '0']))
    if args.fraction_only:
        cases = [('release', [])]
    result = dict(created_at=datetime.now(timezone.utc).isoformat(), baseline_commit=BASE, flags=flags,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  system=subprocess.check_output(['uname', '-a'], text=True).strip(),
                  cpu=next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),
                  affinity=[2], trials=args.trials, sources=sources,
                  driver=program, driver_sha256=hashlib.sha256((ROOT / 'test/benchmark' / program).read_bytes()).hexdigest(),
                  cases=[])
    rng = random.Random(20261020)
    for variant, command in cases:
        values = {'before': [], 'after': []}
        checksum = None
        for trial in range(args.trials + 1):
            order = ['before', 'after']
            rng.shuffle(order)
            for name in order:
                raw = dict(x.split('=') for x in subprocess.check_output([str(cache / name / variant), *command], text=True).split())
                if checksum is None:
                    checksum = raw['checksum']
                if raw['checksum'] != checksum:
                    raise RuntimeError((command, name, checksum, raw))
                if trial:
                    values[name].append({k: float(v) for k, v in raw.items() if k != 'checksum'})
        row = dict(variant=variant, command=command, checksum=checksum, milliseconds=values,
                   median_ms={name: {key: statistics.median(x[key] for x in trials) for key in trials[0]}
                              for name, trials in values.items()})
        result['cases'].append(row)
        print(variant, ' '.join(command), row['median_ms'], flush=True)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
