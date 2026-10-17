import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def path(a, b):
    result = []
    while a != b:
        if a > b:
            k = (a - 1) // b
            result.extend(['R', str(k)])
            a -= k * b
        else:
            k = (b - 1) // a
            result.extend(['L', str(k)])
            b -= k * a
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    cache = ROOT / '.verify-helper/cache/stern-brocot-tree-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/stern_brocot_tree.test.cpp'
    references = ROOT / '.verify-helper/cache/stern_brocot_tree_reference'
    programs = {'uint32': (source, []), 'uint64': (source, ['-DBENCH_STERN_BROCOT_64'])}
    for identity in [405753, 327746]:
        programs[f'reference_{identity}'] = (references / f'{identity}.cpp', [])
    binaries = {}
    for name, (cpp, extra) in programs.items():
        binary = cache / name
        subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(cpp), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    rng = random.Random(519214)

    def fraction():
        a, b = rng.randrange(1, 10**9 + 1), rng.randrange(1, 10**9 + 1)
        g = math.gcd(a, b)
        return a // g, b // g

    workloads = {}
    for mode in ['ENCODE_PATH', 'DECODE_PATH', 'LCA', 'ANCESTOR', 'RANGE', 'MIXED', 'SKEWED', 'FIBONACCI']:
        queries = []
        fib = [1, 1]
        while fib[-1] + fib[-2] <= 10**9:
            fib.append(fib[-1] + fib[-2])
        for i in range(100000):
            command = mode if mode not in ('MIXED', 'SKEWED', 'FIBONACCI') else [
                'ENCODE_PATH', 'DECODE_PATH', 'LCA', 'ANCESTOR', 'RANGE'][i % 5]
            a, b = fraction()
            if mode == 'SKEWED':
                a, b = (rng.randrange(1, 10**9 + 1), 1) if i % 2 else (1, rng.randrange(1, 10**9 + 1))
            if mode == 'FIBONACCI':
                j = rng.randrange(len(fib) - 6, len(fib))
                a, b = fib[j], fib[j - 1]
                if i % 2: a, b = b, a
            if command == 'DECODE_PATH':
                runs = path(a, b)
                queries.append(f'{command} {len(runs) // 2}' + (' ' + ' '.join(runs) if runs else ''))
            elif command == 'LCA':
                c, d = fraction()
                if mode == 'FIBONACCI':
                    c, d = a, b
                queries.append(f'{command} {a} {b} {c} {d}')
            elif command == 'ANCESTOR':
                runs = path(a, b)
                depth = sum(map(int, runs[1::2]))
                k = rng.randrange(depth + 1) if i % 2 else depth + 1
                queries.append(f'{command} {k} {a} {b}')
            else:
                queries.append(f'{command} {a} {b}')
        workloads[mode.lower()] = (str(len(queries)) + '\n' + '\n'.join(queries) + '\n').encode()
    result = {'platform': platform.platform(),
              'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              'flags': flags, 'cpu_affinity': [2], 'trials': 7, 'seed': 519214,
              'source_hash_normalization': 'CRLF to LF',
              'metric': 'wall time including startup, input, queries, output and exit',
              'sources': {name: sha(ROOT / name) for name in ['test/stern_brocot_tree.test.cpp',
                  'math/stern_brocot_tree.hpp', 'components/stern_brocot_path.hpp', 'components/fast_io.hpp']},
              'references': {name: sha(cpp) for name, (cpp, _) in programs.items() if name.startswith('reference_')},
              'workloads': {}}
    for name, data in workloads.items():
        samples = {program: [] for program in programs}
        expected = None
        for program, binary in binaries.items():
            output = b' '.join(subprocess.check_output([binary], input=data, timeout=30).split())
            if expected is None: expected = output
            assert output == expected, (name, program)
        schedule = list(programs) * 7
        rng.shuffle(schedule)
        for program in schedule:
            start = time.perf_counter()
            output = subprocess.check_output([binaries[program]], input=data, timeout=30)
            samples[program].append((time.perf_counter() - start) * 1000)
            assert b' '.join(output.split()) == expected, (name, program)
        item = {'queries': 100000, 'input_sha256': hashlib.sha256(data).hexdigest(),
                'output_tokens_sha256': hashlib.sha256(expected).hexdigest(),
                'programs': {program: {'milliseconds': times, 'median_ms': statistics.median(times)}
                             for program, times in samples.items()}}
        result['workloads'][name] = item
        print(name, {p: round(v['median_ms'], 3) for p, v in item['programs'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
