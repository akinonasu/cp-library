import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-cache', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/power-projection-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    runtime = tempfile.TemporaryDirectory(prefix='cp-power-projection-')
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/power_projection.cpp'
    binaries = {}
    references = {}
    extras = {'library': []}
    for identity in (409399, 409549):
        original = args.reference_cache / f'{identity}.cpp'
        text = original.read_text()
        marker = text.rfind('namespace {\n\nvoid solve()')
        assert marker > 0
        adapted = cache / f'{identity}.hpp'
        adapted.write_text(text[:marker])
        name = str(identity)
        extras[name] = ['-mavx2', f'-DPOWER_PROJECTION_REFERENCE="{adapted}"']
        references[name] = {'source_sha256': digest(original), 'adapted_sha256': digest(adapted),
                            'adaptation': 'remove the final solve/main/preinit section; call poly::power_projection directly'}
    for name, extra in extras.items():
        binary = Path(runtime.name) / name
        subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    if args.build_only:
        return
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'extra_flags': extras, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'binary_filesystem': 'Linux /tmp (WSL ext4)', 'simd_warmup': '4096 AVX2 mul/add iterations before timing',
              'input_seed': 718497, 'schedule_seed': 72967,
              'metric': 'in-process wall time including public API allocation and transform setup, excluding input generation, export, checksum and I/O; reference arena destruction excluded',
              'references': references,
              'source_sha256': {name: digest(ROOT / name) for name in (
                  'fps/power_projection.hpp', 'components/ntt.hpp', 'components/fps_operations.hpp', 'fps/fps.hpp',
                  'math/mint.hpp', 'test/benchmark/power_projection.cpp', 'test/benchmark/power_projection.py')},
              'workloads': []}
    rng = random.Random(72967)
    workloads = [('coefficient', n, ['library', '409399', '409549']) for n in (8000, 32768, 131072, 262144)]
    workloads += [(mode, 131072, ['library']) for mode in ('weighted', 'constant', 'short', 'monomial')]
    workloads += [('weighted', n, ['library', 'naive']) for n in (256, 512)]
    for mode, n, names in workloads:
        samples = {name: [] for name in names}
        expected = None
        for trial in range(args.trials + 1):
            order = names.copy()
            rng.shuffle(order)
            for name in order:
                binary = binaries.get(name, binaries['library'])
                algorithm = 'reference' if name.isdigit() else name
                elapsed, checksum = subprocess.check_output([binary, mode, algorithm, str(n)], text=True, timeout=120).split()
                if expected is None:
                    expected = checksum
                assert checksum == expected, (mode, n, name, checksum, expected)
                if trial:
                    samples[name].append(float(elapsed))
        entry = {'mode': mode, 'size': n, 'checksum': expected, 'milliseconds': samples,
                 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        result['workloads'].append(entry)
        print(mode, n, {name: round(value, 3) for name, value in entry['median_ms'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
