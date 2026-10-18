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


COMPARISON_CASES = [
    ('product', 'dense', 64, 64, 64), ('product', 'dense', 256, 256, 256),
    ('product', 'dense', 512, 512, 512), ('product', 'dense', 1024, 1024, 1024),
    ('product', 'rectangular', 32, 512, 65), ('product', 'tail', 513, 257, 63),
    ('product', 'vector', 513, 257, 1), ('product', 'row', 1, 512, 65),
    ('product', 'rows2', 2, 512, 512), ('product', 'rows3', 3, 512, 512),
    ('product', 'rows6', 6, 512, 512), ('small_product', 'dense', 4, 4, 4),
]


def compare_revisions(args):
    import math
    import re
    import time

    def git(*command):
        return subprocess.check_output(['git', '-C', str(ROOT), *command], timeout=30)

    refs = {'baseline': args.base_ref, 'candidate': args.candidate_ref}
    for ref in refs.values():
        if not ref or not re.fullmatch('[0-9a-f]{40}', ref):
            raise ValueError('Comparison requires two exact commit IDs')
        if git('rev-parse', '--verify', ref + '^{commit}').decode().strip() != ref:
            raise ValueError('Cannot resolve exact source commit')
    if git('rev-parse', 'HEAD').decode().strip() != args.candidate_ref:
        raise ValueError('Comparison checkout is not the exact candidate')
    git('merge-base', '--is-ancestor', args.base_ref, args.candidate_ref)
    if git('diff', '--name-only', 'HEAD', '--') or git('ls-files', '--others', '--exclude-standard'):
        raise ValueError('Comparison checkout must be clean')
    if git('show', args.candidate_ref + ':test/benchmark/matrix.py') != Path(__file__).read_bytes():
        raise ValueError('Comparison script differs from candidate source')
    compiler = '/usr/bin/g++-15'
    if subprocess.check_output([compiler, '-dumpfullversion'], timeout=30).strip() != b'15.2.0':
        raise ValueError('Exact GCC 15.2.0 is required; do not install or substitute')
    cpu = min(os.sched_getaffinity(0)) if args.cpu is None else args.cpu
    os.sched_setaffinity(0, {cpu})
    flags = ['-O2', '-std=gnu++23', '-ftrivial-auto-var-init=zero', '-fmodules',
             '-DONLINE_JUDGE', '-DATCODER', '-fno-lto', '-march=native', '-Wall', '-Wextra', '-Werror']
    paths = ['components/matrix_arithmetic.hpp', 'math/matrix.hpp', 'math/mint.hpp',
             'test/benchmark/matrix.cpp']
    source_hashes, binaries = {}, {}
    with tempfile.TemporaryDirectory(prefix='cp-matrix-compare-') as directory:
        directory = Path(directory)
        environment = {'PATH': '/usr/bin:/bin', 'HOME': str(directory), 'TMPDIR': str(directory),
                       'LC_ALL': 'C', 'TZ': 'UTC'}
        macros = subprocess.check_output([compiler, *flags, '-dM', '-E', '-x', 'c++', '-'],
                                         input=b'#include <cassert>\n', timeout=30, env=environment)
        if b'#define __AVX2__ 1\n' not in macros or re.search(rb'^#define NDEBUG\b', macros, re.MULTILINE):
            raise ValueError('Comparison requires native AVX2 and active assertions')
        for name, ref in refs.items():
            snapshot = directory / name
            source_hashes[name] = {}
            for path in paths:
                data = git('show', ref + ':' + path)
                target = snapshot / path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
                source_hashes[name][path] = hashlib.sha256(data).hexdigest()
            binary = directory / (name + '.bin')
            subprocess.run([compiler, *flags, '-I', str(snapshot),
                            str(snapshot / 'test/benchmark/matrix.cpp'), '-o', str(binary)],
                           check=True, timeout=120, env=environment)
            binaries[name] = binary
        if source_hashes['baseline']['test/benchmark/matrix.cpp'] != source_hashes['candidate']['test/benchmark/matrix.cpp']:
            raise ValueError('Both revisions must use the same existing benchmark driver')
        print(json.dumps({'event': 'matrix_comparison', 'refs': refs,
              'trees': {name: git('rev-parse', ref + '^{tree}').decode().strip() for name, ref in refs.items()},
              'compiler': compiler, 'version': '15.2.0', 'flags': flags, 'cpu': cpu,
              'cpu_model': next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'uname': list(os.uname()), 'seed': 713, 'pairs_per_case': 6,
              'case_count': len(COMPARISON_CASES), 'source_sha256': source_hashes,
              'binary_sha256': {name: hashlib.sha256(binary.read_bytes()).hexdigest() for name, binary in binaries.items()},
              'core_scope': 'allocation, val conversion, packing, multiplication, raw output conversion',
              'process_scope': 'startup, generated input, core, complete text output to memory'}, sort_keys=True), flush=True)
        for operation, kind, n, inner, m in COMPARISON_CASES:
            expected, samples = None, []
            for pair in range(6):
                order = ['baseline', 'candidate'] if pair % 2 == 0 else ['candidate', 'baseline']
                for name in order:
                    begin = time.perf_counter()
                    output = subprocess.check_output([str(binaries[name]), 'library', operation, kind,
                                                      str(n), str(inner), str(m)], timeout=120, env=environment)
                    process_seconds = time.perf_counter() - begin
                    elapsed, result = output.split(b'\n', 1)
                    seconds = float(elapsed)
                    if not math.isfinite(seconds) or seconds <= 0:
                        raise ValueError('Invalid core timing')
                    if expected is None:
                        expected = result
                    if result != expected:
                        raise RuntimeError(f'Full result mismatch: {operation} {kind} {n} {inner} {m} {name}')
                    samples.append({'variant': name, 'pair': pair, 'core_seconds': seconds,
                                    'process_seconds': process_seconds})
            summary = {}
            for name in refs:
                summary[name] = {key: {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
                                 for key in ('core_seconds', 'process_seconds')
                                 for values in ([sample[key] for sample in samples if sample['variant'] == name],)}
            print(json.dumps({'event': 'matrix_case', 'operation': operation, 'kind': kind,
                  'n': n, 'inner': inner, 'm': m, 'samples': samples, 'summary': summary,
                  'result_sha256': hashlib.sha256(expected).hexdigest(), 'result_bytes': len(expected),
                  'full_outputs_equal': True}, sort_keys=True), flush=True)
        print(json.dumps({'event': 'matrix_comparison_completed', 'refs': refs,
              'cases': len(COMPARISON_CASES), 'pairs': 6 * len(COMPARISON_CASES),
              'full_outputs_equal': True, 'artifacts_uploaded': False, 'cache_written': False}, sort_keys=True), flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--quick', action='store_true')
    parser.add_argument('--cpu', type=int)
    parser.add_argument('--base-ref')
    parser.add_argument('--candidate-ref')
    args = parser.parse_args()
    if args.base_ref or args.candidate_ref:
        compare_revisions(args)
        return
    if args.output is None:
        parser.error('--output is required outside exact-revision comparison')
    if args.cpu is None:
        args.cpu = 2
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['components/matrix_arithmetic.hpp', 'math/matrix.hpp', 'math/mint.hpp', 'test/benchmark/matrix.cpp', 'test/benchmark/matrix.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags, 'seed': 713,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'affinity': [args.cpu], 'trials': args.trials, 'sha256': {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    cases = [('product', 'dense', 64, 64, 64), ('product', 'dense', 256, 256, 256), ('product', 'dense', 512, 512, 512),
             ('product', 'rectangular', 32, 512, 65), ('det', 'dense', 512, 0, 512), ('det', 'singular', 512, 0, 512),
             ('inverse', 'dense', 384, 0, 384), ('rank', 'rectangular', 256, 0, 512), ('solve', 'rectangular', 256, 0, 512),
             ('characteristic', 'dense', 256, 0, 256), ('adjugate', 'singular', 192, 0, 192), ('pow', 'dense', 64, 0, 64)]
    if args.quick:
        cases = [cases[0], cases[1], ('det', 'dense', 256, 0, 256), ('inverse', 'dense', 128, 0, 128)]
        cases += [('small_product', 'dense', n, n, n) for n in (1, 2, 4, 8, 16, 32)]
    rows = []
    with tempfile.TemporaryDirectory(prefix='cp-matrix-bench-') as tmp:
        binaries = {}
        for name in ('library', 'scalar'):
            binary = Path(tmp, name)
            extra = ['-DMATRIX_DISABLE_AVX2'] if name == 'scalar' else []
            subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(ROOT / 'test/benchmark/matrix.cpp'), '-o', str(binary)], check=True)
            binaries[name] = binary
        rng = random.Random(713)
        for operation, kind, n, inner, m in cases:
            expected = None
            for trial in range(args.trials):
                order = ['baseline', 'scalar', 'library']
                rng.shuffle(order)
                for name in order:
                    binary = binaries['scalar' if name == 'scalar' else 'library']
                    output = subprocess.check_output([str(binary), name, operation, kind, str(n), str(inner), str(m)])
                    elapsed, result = output.split(b'\n', 1)
                    if expected is None:
                        expected = result
                    if result != expected:
                        raise RuntimeError(f'Full result mismatch: {operation} {kind} {name}')
                    rows.append({'operation': operation, 'kind': kind, 'n': n, 'inner': inner, 'm': m, 'variant': name, 'trial': trial,
                                 'seconds': float(elapsed), 'result_sha256': hashlib.sha256(result).hexdigest()})
            print(operation, kind, n, 'full outputs match', flush=True)
        with (args.output / 'samples.csv').open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=rows[0].keys()); writer.writeheader(); writer.writerows(rows)
        summary = []
        for operation, kind, n, inner, m in cases:
            for name in ('baseline', 'scalar', 'library'):
                sample = [r['seconds'] for r in rows if (r['operation'], r['kind'], r['n'], r['variant']) == (operation, kind, n, name)]
                summary.append({'operation': operation, 'kind': kind, 'n': n, 'inner': inner, 'm': m, 'variant': name, 'seconds': statistics.median(sample)})
        with (args.output / 'summary.csv').open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=summary[0].keys()); writer.writeheader(); writer.writerows(summary)


if __name__ == '__main__':
    main()
