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
BEFORE = 'fd41ae5df33960b93288ae98f9edac4e9fdb166f'


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=15)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/fps_tuning_advanced.cpp'
    with tempfile.TemporaryDirectory(prefix='fps-tuning-advanced-') as temporary:
        work = Path(temporary)
        roots = {}
        hashes = {}
        for variant in ('current', 'before'):
            base = work / variant
            roots[variant] = base
            if variant == 'current':
                for directory in ('fps', 'components', 'math'):
                    shutil.copytree(ROOT / directory, base / directory)
            else:
                shutil.copytree(roots['current'], base)
                header = base / 'components/ntt.hpp'
                text = header.read_text()
                header.write_text(text)
                wrapper = []
                for operation in ('compose', 'power_projection', 'linear_recurrence', 'compositional_inverse'):
                    path = f'fps/{operation}.hpp'
                    text = subprocess.check_output(['git', 'show', f'{BEFORE}:{path}'], cwd=ROOT, text=True)
                    text = text.replace('#include "fps.hpp"', '#include "fps/fps.hpp"')
                    text = text.replace('#include "bostan_mori.hpp"', '#include "fps/bostan_mori.hpp"')
                    text = text.replace('CP_FPS_', 'CP_FPS_ADVANCED_BEFORE_')
                    text = text.replace(operation, operation + '_before')
                    text = text.replace('plan.compose_before(', 'plan.compose(')
                    name = f'{operation}_before.hpp'
                    (base / name).write_text(text)
                    wrapper.append(f'#include "{name}"')
                (base / 'advanced_baseline.hpp').write_text('\n'.join(wrapper) + '\n')
            extra = ['-DFPS_ADVANCED_BASELINE="advanced_baseline.hpp"'] if variant == 'before' else []
            subprocess.run(['g++', *flags, *extra, '-I', str(base), str(source), '-o', str(work / (variant + '.bin'))], check=True)
            hashes[variant] = {str(path.relative_to(base)): digest(path) for path in base.rglob('*.hpp')}
        result = {
            'measured_at': datetime.now(timezone.utc).isoformat(), 'before_public_headers': BEFORE,
            'comparison': 'identical frozen shared FPS/NTT implementation; baseline restores compose, power_projection, linear_recurrence and unchanged compositional_inverse from before_public_headers',
            'compiler': subprocess.check_output(['g++', '--version'], text=True),
            'kernel': subprocess.check_output(['uname', '-a'], text=True),
            'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
            'cpu_affinity': [args.cpu], 'flags': flags, 'trials': args.trials, 'warmup': 1,
            'input_seed': 731, 'schedule_seed': 731, 'binary_filesystem': 'Linux /tmp',
            'metric': 'in-process wall time including public API copies, allocations and transform setup; excludes input generation, checksum and I/O',
            'source_sha256': {str(source.relative_to(ROOT)): digest(source), 'test/benchmark/fps_tuning_advanced.py': digest(Path(__file__))},
            'frozen_headers_sha256': hashes, 'workloads': []}
        rng = random.Random(731)
        workloads = [(operation, n) for operation in ('compose', 'projection') for n in (4096, 8000, 16384, 131072)]
        workloads += [(operation, 131072) for operation in ('compose_stride', 'compose_valuation', 'projection_weighted', 'projection_stride', 'recurrence', 'recurrence_tail')]
        for operation, n in workloads:
            samples = {variant: [] for variant in roots}
            expected = None
            for trial in range(args.trials + 1):
                order = list(roots)
                rng.shuffle(order)
                for variant in order:
                    output = work / 'output.txt'
                    with output.open('w') as stream:
                        subprocess.run([str(work / (variant + '.bin')), operation, variant, str(n)], stdout=stream, check=True)
                    text = output.read_text()
                    elapsed, checksum = text.split()
                    if expected is None:
                        expected = checksum
                    assert checksum == expected, (operation, n, variant, checksum, expected)
                    if trial:
                        samples[variant].append(float(elapsed))
            entry = {'operation': operation, 'size': n, 'milliseconds': samples,
                     'median_ms': {name: statistics.median(values) for name, values in samples.items()}, 'checksum': expected}
            result['workloads'].append(entry)
            print(operation, n, entry['median_ms'], flush=True)
        (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
