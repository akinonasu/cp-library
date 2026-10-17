import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import resource
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--fixed-reference', type=Path, required=True)
    parser.add_argument('--reroot-reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    stack_limit = resource.getrlimit(resource.RLIMIT_STACK)[1]
    resource.setrlimit(resource.RLIMIT_STACK, (stack_limit, stack_limit))
    cache = ROOT / '.verify-helper/cache/static-top-tree-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    sources = {
        'fixed_library': ROOT / 'test/point_set_tree_path_composite_sum_fixed_root.test.cpp',
        'reroot_library': ROOT / 'test/point_set_tree_path_composite_sum.test.cpp',
        'fixed_reference_205356': args.fixed_reference.resolve(),
        'reroot_reference_369678': args.reroot_reference.resolve(),
    }
    bins = {}
    for name, source in sources.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        bins[name] = str(binary)
    rng = random.Random(761825)
    n, q, mod = 200000, 200000, 998244353
    result = {
        'platform': platform.platform(),
        'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
        'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
        'flags': flags, 'cpu_affinity': [2], 'stack_limit': stack_limit,
        'trials': 7, 'seed': 761825, 'n': n, 'q': q,
        'metric': 'wall time including startup, input, construction, queries, output and exit',
        'sources': {name: sha(path) for name, path in sources.items()},
        'headers': {name: sha(ROOT / name) for name in [
            'graph/static_top_tree.hpp', 'components/static_top_tree_structure.hpp',
            'graph/hld.hpp', 'components/fast_io.hpp', 'math/mint.hpp',
            'test/static_top_tree_dp.hpp']}, 'workloads': {},
    }
    for shape in ['random', 'chain', 'star', 'balanced', 'broom']:
        parent = [rng.randrange(v) if shape == 'random' else v - 1 if shape == 'chain' else
                  0 if shape == 'star' else (v - 1) // 2 if shape == 'balanced' else
                  v - 1 if v < n // 2 else v - n // 2 for v in range(1, n)]
        prefix = f'{n} {q}\n' + ' '.join(str(rng.randrange(mod)) for _ in range(n)) + '\n'
        prefix += ''.join(f'{u} {v} {rng.randrange(mod)} {rng.randrange(mod)}\n' for v, u in enumerate(parent, 1))
        queries = [(f'0 {rng.randrange(n)} {rng.randrange(mod)}' if rng.randrange(2) else
                    f'1 {rng.randrange(n-1)} {rng.randrange(mod)} {rng.randrange(mod)}', rng.randrange(n)) for _ in range(q)]
        for kind in ['fixed', 'reroot']:
            data = (prefix + ''.join(line + (f' {root}\n' if kind == 'reroot' else '\n') for line, root in queries)).encode()
            programs = {name: command for name, command in bins.items() if name.startswith(kind + '_')}
            samples = {name: [] for name in programs}
            output_hash = None
            for command in programs.values():
                output = subprocess.check_output([command], input=data, timeout=30)
                current = hashlib.sha256(output).hexdigest()
                if output_hash is None:
                    output_hash = current
                assert current == output_hash, (shape, kind)
            schedule = list(programs) * 7
            rng.shuffle(schedule)
            for name in schedule:
                start = time.perf_counter()
                output = subprocess.check_output([programs[name]], input=data, timeout=30)
                samples[name].append((time.perf_counter() - start) * 1000)
                assert hashlib.sha256(output).hexdigest() == output_hash
            item = {'input_sha256': hashlib.sha256(data).hexdigest(), 'input_bytes': len(data),
                    'output_sha256': output_hash, 'programs': {
                        name: {'milliseconds': values, 'median_ms': statistics.median(values)} for name, values in samples.items()}}
            result['workloads'][f'{kind}_{shape}'] = item
            print(kind, shape, {name: round(values['median_ms'], 3) for name, values in item['programs'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
