import argparse
import csv
import hashlib
import io
import json
import math
import os
from pathlib import Path
import re
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASE = 'bce99d05caeea138a0598eab170f1110c1c6f786'
REF = 'refs/heads/codex/edit-distance-trim-20261018'
HEADER = 'string/edit_distance.hpp'
DRIVER = 'test/benchmark/edit_distance_trim.cpp'
CALLER = 'test/benchmark/edit_distance_trim_call.cpp'
CASES = {
    'identical200k': (2000, 1), 'last_sub200k': (2000, 1),
    'first_sub200k': (2000, 1), 'middle_sub200k': (2000, 1),
    'prefix199999_200000': (2000, 1), 'random26_4096': (200, 1),
    'disjoint200k': (1000, 1), 'short_corpus': (1000, 1344),
}
COMMON = ['-O2', '-std=gnu++23', '-ftrivial-auto-var-init=zero', '-fmodules',
          '-DONLINE_JUDGE', '-DATCODER', '-fno-lto', '-Wall', '-Wextra', '-Werror']
MODES = {'generic': ['-march=x86-64', '-mtune=generic'], 'native': ['-march=native']}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate_samples(raw):
    columns = ['family', 'pair', 'order', 'implementation', 'repeats', 'cases', 'calls',
               'elapsed_ns', 'ns_per_call', 'checksum', 'expected_checksum']
    reader = csv.DictReader(io.StringIO(raw.decode()))
    require(reader.fieldnames == columns, 'Unexpected benchmark CSV schema')
    rows = list(reader)
    require(len(rows) == len(CASES) * 4 * 2, 'Incomplete fixed benchmark screen')
    samples = []
    index = 0
    for family, (repeats, cases) in CASES.items():
        for pair in range(4):
            order = 'AB' if pair % 2 == 0 else 'BA'
            for implementation in order:
                row = rows[index]
                index += 1
                require(row['family'] == family and row['pair'] == str(pair)
                        and row['order'] == order and row['implementation'] == implementation,
                        'Fixed family/order schedule changed')
                parsed = {key: int(row[key]) for key in ('repeats', 'cases', 'calls', 'elapsed_ns',
                                                       'checksum', 'expected_checksum')}
                require((parsed['repeats'], parsed['cases'], parsed['calls'])
                        == (repeats, cases, repeats * cases), 'Frozen repeat/corpus counts changed')
                ns = float(row['ns_per_call'])
                require(parsed['elapsed_ns'] > 0 and math.isfinite(ns) and ns > 0
                        and math.isclose(ns, parsed['elapsed_ns'] / parsed['calls'], rel_tol=1e-9),
                        'Invalid API timing')
                require(parsed['checksum'] == parsed['expected_checksum'] and parsed['checksum'] > 0,
                        'API checksum differs from oracle')
                samples.append({'family': family, 'pair': pair, 'order': order,
                                'implementation': implementation, 'ns_per_call': ns, **parsed})
    for family in CASES:
        require(len({row['checksum'] for row in samples if row['family'] == family}) == 1,
                'Cross-implementation checksum mismatch')
    return samples


def compare(base, candidate):
    def git(*args):
        return subprocess.check_output(['git', '-C', str(ROOT), *args], timeout=30)

    require(base == BASE, 'Only the reviewed exact main baseline is allowed')
    for ref in (base, candidate):
        require(isinstance(ref, str) and re.fullmatch('[0-9a-f]{40}', ref), 'Exact commit IDs required')
        require(git('rev-parse', '--verify', ref + '^{commit}').decode().strip() == ref,
                'Cannot resolve exact source commit')
    require(git('rev-parse', 'HEAD').decode().strip() == candidate, 'Checkout is not exact candidate')
    git('merge-base', '--is-ancestor', base, candidate)
    require(not git('diff', '--name-only', 'HEAD', '--')
            and not git('ls-files', '--others', '--exclude-standard'), 'Checkout must be clean')
    require(git('show', candidate + ':test/benchmark/edit_distance.py') == Path(__file__).read_bytes(),
            'Runner differs from exact candidate')
    require(os.environ.get('GITHUB_EVENT_NAME') == 'push'
            and os.environ.get('GITHUB_REPOSITORY') == 'qandori/cp-library'
            and os.environ.get('GITHUB_REF') == REF and os.environ.get('GITHUB_SHA') == candidate,
            'Comparison requires the reviewed exact branch push event')
    compiler = '/usr/bin/g++-15'
    require(subprocess.check_output([compiler, '-dumpfullversion'], timeout=30).strip() == b'15.2.0',
            'Exact GCC 15.2.0 is required; do not substitute or install')
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    deadline = time.monotonic() + 7 * 60
    with tempfile.TemporaryDirectory(prefix='cp-edit-distance-compare-') as temporary:
        directory = Path(temporary)
        environment = {'PATH': '/usr/bin:/bin', 'HOME': str(directory), 'TMPDIR': str(directory),
                       'LC_ALL': 'C', 'TZ': 'UTC'}

        def run(argv, source=None, seconds=120):
            remaining = min(seconds, deadline - time.monotonic())
            require(remaining > 0, 'Comparison runtime budget exhausted')
            process = subprocess.run([str(x) for x in argv], input=source, capture_output=True,
                                     cwd=directory, env=environment, timeout=remaining)
            require(len(process.stdout) + len(process.stderr) <= 128 * 1024, 'Output budget exceeded')
            if process.returncode:
                print(json.dumps({'event': 'command_failure', 'argv': [str(x) for x in argv],
                                  'returncode': process.returncode,
                                  'stdout': process.stdout.decode(errors='replace'),
                                  'stderr': process.stderr.decode(errors='replace')}), flush=True)
                raise RuntimeError('Comparison command failed; do not rerun or downgrade')
            return process.stdout, process.stderr

        driver, caller = directory / 'driver.cpp', directory / 'caller.cpp'
        driver.write_bytes(git('show', candidate + ':' + DRIVER))
        caller.write_bytes(git('show', candidate + ':' + CALLER))
        hashes = {'driver': sha(driver.read_bytes()), 'caller': sha(caller.read_bytes())}
        snapshots = {}
        for variant, ref in (('baseline', base), ('candidate', candidate)):
            snapshot = directory / variant
            path = snapshot / HEADER
            path.parent.mkdir(parents=True)
            path.write_bytes(git('show', ref + ':' + HEADER))
            snapshots[variant] = snapshot
            hashes[variant] = sha(path.read_bytes())
        print(json.dumps({'event': 'edit_distance_comparison', 'refs': {'baseline': base, 'candidate': candidate},
                          'trees': {name: git('rev-parse', ref + '^{tree}').decode().strip()
                                    for name, ref in (('baseline', base), ('candidate', candidate))},
                          'compiler': compiler, 'version': '15.2.0', 'cpu': cpu,
                          'affinity': sorted(os.sched_getaffinity(0)), 'uname': list(os.uname()),
                          'cpu_model': next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                            if line.startswith('model name')),
                          'seed': 1234, 'pairs': 4, 'cases': CASES, 'source_sha256': hashes,
                          'api_scope': 'complete edit_distance call including all dispatch/allocation, plus equal wrapper/checksum overhead',
                          'excluded': 'generation, DP oracle, pre-timing one-call correctness/warmup for every case',
                          'generic_runtime_dispatch': 'unchanged; AVX2/AVX512 target functions may be used at runtime',
                          'claims': 'warm-cache API screen, not judge-fastest or Mac performance evidence'}), flush=True)
        for mode, extra in MODES.items():
            flags = COMMON + extra
            macros, _ = run([compiler, *flags, '-dM', '-E', '-x', 'c++', '-'], b'#include <cassert>\n', 30)
            definitions = dict(re.findall(rb'^#define (\S+)(?: ([^\n]*))?$', macros, re.MULTILINE))
            require(b'NDEBUG' not in definitions and b'assert(expr)' in definitions, 'Assertions must remain active')
            for key, expected in ((b'__GNUC__', b'15'), (b'__GNUC_MINOR__', b'2'), (b'__GNUC_PATCHLEVEL__', b'0'),
                                  (b'__x86_64__', b'1'), (b'ONLINE_JUDGE', b'1'), (b'ATCODER', b'1')):
                require(definitions.get(key) == expected, 'Unexpected target/compiler macro: ' + key.decode())
            isa = {k.decode(): v.decode() for k, v in definitions.items() if k.startswith((b'__AVX', b'__BMI'))}
            require((not isa) if mode == 'generic' else isa.get('__AVX2__') == '1' and isa.get('__BMI2__') == '1',
                    'Target ISA differs; do not downgrade or rerun')
            objects = []
            for variant in ('baseline', 'candidate'):
                obj = directory / (variant + '-' + mode + '.o')
                run([compiler, *flags, '-I', snapshots[variant], '-DED_API=edit_distance_' + variant,
                     '-DED_WRAPPER=' + variant + '_call', '-c', caller, '-o', obj])
                objects.append(obj)
            binary = directory / ('screen-' + mode)
            run([compiler, *flags, driver, *objects, '-o', binary])
            print(json.dumps({'event': 'build', 'mode': mode, 'flags': flags, 'macro_sha256': sha(macros),
                              'isa_compile_macros': isa, 'binary_sha256': sha(binary.read_bytes()),
                              'assertions': True, 'lto': False, 'same_driver_and_caller_both_variants': True}), flush=True)
            start = time.monotonic()
            raw, stderr = run([binary, '1234'])
            process_seconds = time.monotonic() - start
            print(json.dumps({'event': 'raw_screen', 'mode': mode, 'csv': raw.decode(),
                              'process_seconds': process_seconds, 'stdout_sha256': sha(raw),
                              'stderr': stderr.decode().strip()}), flush=True)
            samples = validate_samples(raw)
            require(stderr.decode().strip() == 'seed=1234 corpus_hash=14700761369690958196 short_cases=1344',
                    'Fixed runtime corpus digest changed')
            for family in CASES:
                subset = [row for row in samples if row['family'] == family]
                ratios = [next(row['ns_per_call'] for row in subset if row['pair'] == p and row['implementation'] == 'A') /
                          next(row['ns_per_call'] for row in subset if row['pair'] == p and row['implementation'] == 'B')
                          for p in range(4)]
                print(json.dumps({'event': 'family', 'mode': mode, 'family': family,
                                  'paired_baseline_over_candidate': ratios,
                                  'median_ratio': statistics.median(ratios)}), flush=True)
        print(json.dumps({'event': 'edit_distance_comparison_completed', 'base': base, 'candidate': candidate,
                          'all_checksums_match': True, 'screen_runs_per_mode': 1,
                          'artifacts_uploaded': False, 'cache_written': False}), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--base-ref', required=True)
    parser.add_argument('--candidate-ref', required=True)
    args = parser.parse_args()
    compare(args.base_ref, args.candidate_ref)
