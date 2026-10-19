import argparse
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--cxx', default='g++')
parser.add_argument('--base', default='d6ef542f834dbf09b335af241503a5a9a5ae7c71')
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--trials', type=int, default=7)
parser.add_argument('--native', action='store_true')
parser.add_argument('--cases', nargs='+', choices=['ntt', 'crt', 'sieve', 'quad', 'wavelet'])
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter']
if args.native:
    flags += ['-march=native', '-ftrivial-auto-var-init=zero']
environment = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
               'flags': flags, 'base': args.base, 'trials': args.trials,
               'affinity': sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None}
inputs = ['components/ntt_mod.hpp', 'components/quad_bit_vector.hpp', 'math/number_theory/prime_sieve.hpp',
          'convolution/convolution_mod.hpp', 'data_structure/wavelet_matrix/wavelet_matrix.hpp',
          'test/benchmark/gcc_audit.cpp']
environment['sha256'] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in inputs}
for name, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
    environment[name] = subprocess.check_output(command, text=True)
(args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
with tempfile.TemporaryDirectory(prefix='cp-gcc-audit-') as directory:
    directory = Path(directory)
    baseline = directory / 'baseline'
    for name in ['components/ntt_mod.hpp', 'math/number_theory/prime_sieve.hpp', 'convolution/convolution_mod.hpp',
                 'components/quad_bit_vector.hpp', 'data_structure/wavelet_matrix/wavelet_matrix.hpp']:
        target = baseline / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(subprocess.check_output(['git', 'show', f'{args.base}:{name}'], cwd=ROOT))
    binaries = {}
    for variant, include in [('before', baseline), ('after', ROOT)]:
        binary = directory / variant
        subprocess.run([args.cxx, *flags, '-I', str(include), '-I', str(ROOT),
                        str(ROOT / 'test/benchmark/gcc_audit.cpp'), '-o', str(binary)], check=True)
        binaries[variant] = binary
    cases = [('ntt', 16, 10000), ('ntt', 1024, 100), ('ntt', 262144, 4),
             ('crt', 32, 100), ('crt', 4096, 4), ('crt', 131072, 1),
             ('sieve', 1000, 10000), ('sieve', 1000000, 10), ('sieve', 10000000, 2),
             ('quad', 200000, 100), ('wavelet', 200000, 5)]
    rows = []
    sample_file = args.output / 'samples.csv'
    if args.cases and sample_file.exists():
        with sample_file.open(newline='') as file:
            previous = csv.reader(file)
            next(previous)
            rows = [row for row in previous if row[0] not in args.cases]
    for kind, n, repeat in cases:
        if args.cases and kind not in args.cases:
            continue
        samples = {'before': [], 'after': []}
        checksum = None
        for trial in range(args.trials):
            for variant in (['before', 'after'] if trial % 2 == 0 else ['after', 'before']):
                result = subprocess.check_output([str(binaries[variant]), kind, str(n), str(repeat)], text=True)
                fields = next(csv.reader(io.StringIO(result)))
                if checksum is not None and checksum != fields[4]:
                    raise RuntimeError(f'checksum mismatch: {kind} {n}')
                checksum = fields[4]
                samples[variant].append(float(fields[3]))
                rows.append([kind, n, repeat, trial, variant, fields[3], checksum])
        before, after = (statistics.median(samples[key]) for key in ['before', 'after'])
        print(f'{kind} n={n}: {before:.3f} -> {after:.3f} ms ({before / after:.2f}x)', flush=True)
    with sample_file.open('w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['kind', 'n', 'repetitions', 'trial', 'variant', 'ms', 'checksum'])
        writer.writerows(rows)
