import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    os.chdir(root)
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = root / '.verify-helper/cache/binomial-prefix-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    sources = {
        'online': root / 'test/binomial_prefix_sum_online.test.cpp',
        'offline': root / 'test/binomial_prefix_sum_offline.test.cpp',
        'reference_1110156': args.reference.resolve(),
    }
    binaries = {}
    for name, source in sources.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-I', str(root), str(source), '-o', str(binary)], check=True)
        binaries[name] = binary
    result = {
        'platform': platform.platform(),
        'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
        'affinity': sorted(os.sched_getaffinity(0)),
        'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
        'flags': flags, 'trials': args.trials, 'seed': 20261017,
        'metric': 'wall time including process startup, input, preprocessing, queries, output',
        'sources': {name: sha(path) for name, path in sources.items()},
        'headers': {name: sha(root / name) for name in [
            'math/binomial_prefix_sum.hpp', 'components/binomial_prefix_core.hpp',
            'components/mo_order.hpp', 'math/mint.hpp', 'components/fast_io.hpp']},
        'workloads': {},
    }
    rng = random.Random(20261017)
    nmax = q = 200000
    for shape in ['random', 'fixed_n', 'edges']:
        queries = []
        for _ in range(q):
            n = nmax if shape == 'fixed_n' else rng.randint(1, nmax)
            m = rng.randint(1, n) if shape != 'edges' else rng.choice([1, n, max(1, n - 1)])
            queries.append((n, m))
        queries[-1] = (nmax, queries[-1][1])
        data = (str(q) + '\n' + ''.join(f'{n} {m}\n' for n, m in queries)).encode()
        item = {'max_n': nmax, 'queries': q, 'input_sha256': hashlib.sha256(data).hexdigest(), 'programs': {}}
        expected = None
        for name, binary in binaries.items():
            output = subprocess.check_output([str(binary)], input=data)
            if expected is None:
                expected = output
            assert output == expected, (shape, name)
            item['programs'][name] = {'milliseconds': []}
        schedule = list(binaries) * args.trials
        rng.shuffle(schedule)
        for name in schedule:
            start = time.perf_counter()
            output = subprocess.check_output([str(binaries[name])], input=data)
            elapsed = (time.perf_counter() - start) * 1000
            assert output == expected, (shape, name)
            item['programs'][name]['milliseconds'].append(elapsed)
        item['output_sha256'] = hashlib.sha256(expected).hexdigest()
        for values in item['programs'].values():
            values['median_ms'] = statistics.median(values['milliseconds'])
        result['workloads'][shape] = item
        print(shape, {name: round(values['median_ms'], 3) for name, values in item['programs'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
