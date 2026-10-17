import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'fd41ae5df33960b93288ae98f9edac4e9fdb166f'
OWNED = ['components/polynomial_half_gcd.hpp', 'fps/polynomial_gcd.hpp',
         'fps/multipoint_eval.hpp', 'fps/interpolate.hpp', 'fps/taylor_shift.hpp',
         'components/fps_product_tree.hpp', 'components/fps_direct_eval.hpp']


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--operations', default='gcd,extended,inverse,eval,interpolate,taylor')
    parser.add_argument('--sizes', default='256,4096,32768,50000')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--repeat', type=int, default=1)
    parser.add_argument('--candidate', type=Path)
    parser.add_argument('--baseline', default=BASELINE)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero']
    headers = sorted(p for d in ['fps', 'components', 'math'] for p in (ROOT / d).glob('*.hpp'))
    frozen = {p.relative_to(ROOT).as_posix(): p.read_bytes() for p in headers}
    environment = {'created_at': datetime.now(timezone.utc).isoformat(),
                   'compiler': subprocess.check_output(['g++', '--version'], text=True),
                   'flags': flags, 'affinity': [2], 'seed': 294384,
                   'trials': args.trials, 'repeat': args.repeat, 'baseline': args.baseline,
                   'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
                   'sha256': {p: hashlib.sha256(source).hexdigest() for p, source in frozen.items()},
                   'driver_sha256': hashlib.sha256((ROOT / 'test/benchmark/fps_tuning_polynomial.cpp').read_bytes()).hexdigest(),
                   'variant_sha256': {}}
    rng = random.Random(294384)
    results = []
    with tempfile.TemporaryDirectory(prefix='cp-fps-tuning-poly-') as directory:
        tmp = Path(directory)
        binaries = {}
        for variant in ['before', 'after']:
            copy = tmp / variant
            for p, source in frozen.items():
                dest = copy / p
                dest.parent.mkdir(parents=True, exist_ok=True)
                dest.write_bytes(source)
            if variant == 'before':
                for p in OWNED:
                    (copy / p).write_bytes(subprocess.check_output(['git', 'show', args.baseline + ':' + p], cwd=ROOT))
            elif args.candidate:
                for p in OWNED:
                    source = args.candidate / p
                    if source.exists():
                        shutil.copyfile(source, copy / p)
            binary = copy / 'bench'
            environment['variant_sha256'][variant] = {p: hashlib.sha256((copy / p).read_bytes()).hexdigest() for p in OWNED}
            subprocess.run(['g++', *flags, '-I', str(copy), str(ROOT / 'test/benchmark/fps_tuning_polynomial.cpp'), '-o', str(binary)], check=True)
            binaries[variant] = binary
        print('built', flush=True)
        for operation in args.operations.split(','):
            for n in map(int, args.sizes.split(',')):
                samples = {v: [] for v in binaries}
                expected = None
                for trial in range(args.trials + 1):
                    order = list(binaries)
                    rng.shuffle(order)
                    for variant in order:
                        output = subprocess.check_output([str(binaries[variant]), operation, str(n), str(args.repeat)]).split()
                        if expected is None:
                            expected = output[1]
                        assert output[1] == expected, (operation, n, variant)
                        if trial:
                            samples[variant].append(float(output[0]))
                item = {'operation': operation, 'size': n, 'checksum': expected.decode(),
                        'samples': samples, 'median_seconds': {v: statistics.median(s) for v, s in samples.items()}}
                results.append(item)
                print(operation, n, {v: round(t * 1000, 4) for v, t in item['median_seconds'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps({'environment': environment, 'results': results}, indent=2) + '\n')


if __name__ == '__main__':
    main()
