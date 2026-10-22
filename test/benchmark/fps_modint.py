import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shlex
import statistics
import subprocess


ROOT = Path(__file__).resolve().parents[2]
DRIVER = ROOT / 'test/benchmark/fps_modint.cpp'
OPERATIONS = ['multiply', 'square', 'inv', 'log', 'exp', 'sqrt', 'pow',
              'div_series', 'divmod', 'scale', 'diff', 'integral', 'eval']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def execute(executable, operation, n, dump=False):
    command = [str(executable), operation, str(n)]
    if dump:
        command.append('--dump')
    output = subprocess.check_output(command, timeout=120)
    header, _, coefficients = output.partition(b'\n')
    elapsed, size, remainder, checksum = header.split()
    return dict(ms=float(elapsed), answer=[int(size), int(remainder), int(checksum)]), coefficients


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--acl', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--sizes', type=int, nargs='+', default=[1024, 32768, 131073, 524288])
    args = parser.parse_args()
    if args.trials < 3 or min(args.sizes) < 2:
        parser.error('trials >= 3 and sizes >= 2 are required')
    args.acl = args.acl.resolve()
    if not (args.acl / 'atcoder/modint').is_file():
        parser.error('--acl must contain atcoder/modint')
    args.build_dir.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('--cpu is unavailable')
    flags = ['-std=c++23', '-O2', '-march=haswell', '-DEVAL', '-DONLINE_JUDGE',
             '-ftrivial-auto-var-init=zero', '-Wall', '-Wextra', '-Werror',
             '-fconstexpr-depth=1024', '-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152',
             '-I', str(ROOT), '-I', str(args.acl)]
    dependencies = subprocess.check_output(['g++', *flags, '-MM', str(DRIVER)], text=True)
    paths = [Path(p).resolve() for p in shlex.split(dependencies.replace('\\\n', ' '))[1:]]
    sources = {str(path.relative_to(ROOT) if path.is_relative_to(ROOT)
                   else Path('ac-library') / path.relative_to(args.acl)): sha(path) for path in paths}
    sources['test/benchmark/fps_modint.py'] = sha(Path(__file__))
    report = dict(started_at=datetime.now(timezone.utc).isoformat(),
                  revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  acl_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.acl, text=True).strip(),
                  acl_dirty=bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=args.acl)),
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  cpu_model=next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                 if line.startswith('model name')),
                  flags=flags, affinity=[args.cpu], trials=args.trials, sizes=args.sizes,
                  input_seed=731, order_seed=20261022, source_sha256=sources,
                  timing='fresh process; construction and I/O excluded; roots not prewarmed; allocation included; '
                         'linear operations average max(4,131072/n) repetitions with a compiler memory barrier',
                  validation=[], results=[], binaries={}, completed=False)
    samples = []

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        with args.output.with_suffix('.csv').open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=['operation', 'n', 'variant', 'trial', 'ms', 'answer'])
            writer.writeheader()
            writer.writerows(samples)

    save()
    binaries = {}
    for variant in ['custom', 'acl']:
        executable = args.build_dir / variant
        command = ['g++', *flags, *(['-DBENCH_ACL_MODINT'] if variant == 'acl' else []),
                   str(DRIVER), '-o', str(executable)]
        subprocess.run(command, check=True)
        binaries[variant] = executable
        report['binaries'][variant] = dict(command=command, sha256=sha(executable))
    os.sched_setaffinity(0, {args.cpu})
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy:
        fcntl.flock(lock, fcntl.LOCK_EX)
        fcntl.flock(legacy, fcntl.LOCK_EX)
        for n in [2, 31, 32, 33, 127, 256, 511, 512, 513, 1024, 4097, max(args.sizes)]:
            for operation in OPERATIONS:
                left, a = execute(binaries['custom'], operation, n, dump=True)
                right, b = execute(binaries['acl'], operation, n, dump=True)
                if left['answer'] != right['answer'] or a != b:
                    raise RuntimeError(f'Coefficient mismatch: {operation} {n}')
                report['validation'].append(dict(operation=operation, n=n, answer=left['answer'],
                                                 coefficient_sha256=hashlib.sha256(a).hexdigest()))
        save()
        order = random.Random(20261022)
        for n in args.sizes:
            for operation in OPERATIONS:
                values = {variant: [] for variant in binaries}
                answers = set()
                for trial in range(args.trials):
                    variants = list(binaries)
                    order.shuffle(variants)
                    for variant in variants:
                        row, _ = execute(binaries[variant], operation, n)
                        samples.append(dict(operation=operation, n=n, variant=variant, trial=trial, **row))
                        values[variant].append(row['ms'])
                        answers.add(tuple(row['answer']))
                if len(answers) != 1:
                    raise RuntimeError(f'Checksum mismatch: {operation} {n}')
                medians = {variant: statistics.median(v) for variant, v in values.items()}
                ratio = medians['acl'] / medians['custom']
                report['results'].append(dict(operation=operation, n=n, samples_ms=values,
                                              median_ms=medians, acl_over_custom=ratio, answer=list(answers.pop())))
                save()
                print(operation, n, medians, f'ACL/custom={ratio:.3f}', flush=True)
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    save()


if __name__ == '__main__':
    main()
