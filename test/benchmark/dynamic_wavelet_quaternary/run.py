#!/usr/bin/env python3
"""Freeze sources and compare streaming solvers and instrumented APIs on ABC343 F."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics
import subprocess
import sys
import time

BASE = '2559803022319333756173295947577d6f5f9b2e'
PUBLIC = 'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp'
BASE_HEADERS = ['components/dynamic_bit_vector.hpp', PUBLIC]
FINAL_HEADERS = [*BASE_HEADERS, 'components/dynamic_quad_vector.hpp', 'components/dynamic_quad_wavelet.hpp', 'components/fast_io.hpp']
SHAPES = ['random30', 'duplicates16', 'uniform_same', 'no_op_random', 'clustered', 'short']


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def cpu_now():
    value = resource.getrusage(resource.RUSAGE_CHILDREN)
    return value.ru_utime + value.ru_stime


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def load_reference(directory, api=False):
    """Accept fetch_nim_reference.py output or the original measurement metadata."""
    directory = Path(directory)
    source_name = 'abc343f_api.nim' if api else 'BundledMain.nim'
    binary_name = 'nim-api' if api else 'abc343f-bundled'
    reconstruction = directory / 'reconstruction.json'
    if reconstruction.exists():
        record = json.loads(reconstruction.read_text())
        if not record.get('compiled'):
            raise RuntimeError('Nim reference was not compiled; run fetch_nim_reference.py with --compile')
        expected_source = record['bundle_sha256'][source_name]
        expected_binary = record['binary_sha256'][binary_name]
        build = {'mode': 'fetch-helper-reconstruction', 'command': record['commands'][binary_name],
                 'nim_version': record['nim_version'], 'nim_binary_sha256': record['nim_binary_sha256'],
                 'source_sha256': expected_source, 'binary_sha256': expected_binary,
                 'reconstruction': record}
        extras = ['reconstruction.json', 'LICENSE']
        provenance = record
    else:
        manifest_name = 'nim-api-validation.json' if api else 'bundled-build-validation.json'
        build = json.loads((directory / manifest_name).read_text())
        expected_source = build['source_sha256'] if api else build['sources'][source_name]
        expected_binary = build['binary_sha256'] if api else build['solver_binary_sha256']
        extras = [manifest_name] if api else [manifest_name, 'provenance.json', 'LICENSE']
        provenance = json.loads((directory / 'provenance.json').read_text()) if not api else None
    if sha(directory / source_name) != expected_source:
        raise RuntimeError('Nim source hash mismatch: ' + str(directory / source_name))
    if sha(directory / binary_name) != expected_binary:
        raise RuntimeError('Nim binary hash mismatch: ' + str(directory / binary_name))
    for extra in extras:
        if not (directory / extra).is_file():
            raise RuntimeError('Missing Nim provenance file: ' + str(directory / extra))
    return {'directory': directory, 'source_name': source_name, 'binary_name': binary_name,
            'source_sha256': expected_source, 'binary_sha256': expected_binary,
            'build': build, 'provenance': provenance, 'extras': extras}


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--repo', type=Path, default=Path.cwd())
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--driver', type=Path, required=True)
    parser.add_argument('--large-cases', type=Path, required=True)
    parser.add_argument('--small-cases', type=Path, required=True)
    parser.add_argument('--nim-reference', type=Path, required=True)
    parser.add_argument('--nim-api-directory', type=Path)
    parser.add_argument('--base', default=BASE)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--repeats', type=int, default=7)
    parser.add_argument('--seed', type=int, default=20261020)
    parser.add_argument('--optimized-consumer', action='store_true')
    parser.add_argument('--expected-final', type=Path)
    args = parser.parse_args()
    args.repo = args.repo.resolve()
    if args.nim_api_directory is None and (args.nim_reference / 'reconstruction.json').exists():
        reconstruction = json.loads((args.nim_reference / 'reconstruction.json').read_text())
        if reconstruction.get('compiled') and 'nim-api' in reconstruction.get('binary_sha256', {}):
            args.nim_api_directory = args.nim_reference
    primary_reference = load_reference(args.nim_reference)
    api_reference = load_reference(args.nim_api_directory, api=True) if args.nim_api_directory else None
    args.work.mkdir(parents=True, exist_ok=True)
    args.work = args.work.resolve()
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('Selected CPU is unavailable')
    if args.repeats < 1:
        parser.error('--repeats must be positive')
    output_path = args.work / 'results.json'
    if output_path.exists():
        parser.error('Existing results would be overwritten; use a new work directory')
    cxx = shutil.which(args.cxx)
    if cxx is None:
        parser.error('C++ compiler was not found')
    flags = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero', '-UNDEBUG', '-fno-lto', '-Wall', '-Wextra', '-Werror']
    final_hashes = {path: sha(args.repo / path) for path in FINAL_HEADERS}
    if args.expected_final:
        for path, expected in json.loads(args.expected_final.read_text()).items():
            assert final_hashes[path] == expected, (path, 'unexpected production source')
    metadata = {
        'started_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'invocation': sys.argv, 'base_commit': args.base,
        'working_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.repo, text=True).strip(),
        'runner_sha256': sha(__file__), 'driver_sha256': sha(args.driver),
        'compiler': subprocess.check_output([cxx, '--version'], text=True),
        'compiler_path': cxx, 'compiler_sha256': sha(cxx), 'cpp_flags': flags,
        'uname': list(os.uname()), 'lscpu': subprocess.check_output(['lscpu'], text=True),
        'pin': args.cpu, 'allowed_cpus': sorted(os.sched_getaffinity(0)),
        'repeats': args.repeats, 'shuffle_seed': args.seed,
        'warmups_per_large_case_and_binary': 1,
        'timing': 'blocking process wait; wall perf_counter_ns; CPU RUSAGE_CHILDREN delta; regular input/output files; fsync excluded',
        'cpp_semantics': 'unchanged original signed int32/32-bit caller: kth_largest(0), prev_value, range_freq(value,value+1)',
        'nim_semantics': 'faithful original unsigned/nonnegative H=30-bit matrix caller: kth_largest(0), prev_value, count',
        'api_timing': 'input/operation loading and answer allocation precede build/operation timers; complete answers are output after timers',
        'sources': {}, 'binaries': {}, 'commands': [], 'reference': {},
    }
    result = {'metadata': metadata, 'cases': [], 'validation': [], 'warmups': [], 'samples': [], 'summary': {}}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        for name, headers in [('baseline', BASE_HEADERS), ('final', FINAL_HEADERS)]:
            directory = args.work / 'sources' / name
            hashes = {}
            for header in headers:
                path = directory / header
                path.parent.mkdir(parents=True, exist_ok=True)
                data = subprocess.check_output(['git', 'show', f'{args.base}:{header}'], cwd=args.repo) if name == 'baseline' else (args.repo / header).read_bytes()
                path.write_bytes(data)
                hashes[header] = sha(path)
            metadata['sources'][name] = hashes
        driver = args.work / 'sources' / 'abc343f.cpp'
        shutil.copyfile(args.driver, driver)
        configurations = [('baseline_stream', 'baseline', {}, 'stream'), ('final_stream', 'final', {}, 'stream')]
        if args.optimized_consumer:
            configurations += [('final_fastio_count', 'final', {'DWM_FAST_IO': 1, 'DWM_COUNT_DIRECT': 1}, 'stream')]
        if args.nim_api_directory:
            configurations += [('baseline_api', 'baseline', {'DWM_INSTRUMENT': 1}, 'api'), ('final_api', 'final', {'DWM_INSTRUMENT': 1}, 'api')]
        for tag, source_name, defines, family in configurations:
            binary = args.work / tag
            command = [cxx, *flags, *[f'-D{key}={value}' for key, value in defines.items()], '-I', str(args.work / 'sources' / source_name), str(driver), '-o', str(binary)]
            subprocess.run(command, check=True)
            metadata['commands'].append(command)
            metadata['binaries'][tag] = {'path': str(binary), 'sha256': sha(binary), 'source': source_name, 'family': family, 'defines': defines, 'caller': 'fast I/O + count control' if tag == 'final_fastio_count' else 'original range_freq caller'}
            print('compiled', tag, flush=True)
        reference = args.work / 'reference'
        reference.mkdir(exist_ok=True)
        for src_name in [primary_reference['source_name'], *primary_reference['extras']]:
            shutil.copyfile(args.nim_reference / src_name, reference / src_name)
        shutil.copyfile(args.nim_reference / primary_reference['binary_name'], reference / 'nim-stream')
        (reference / 'nim-stream').chmod(0o755)
        assert sha(reference / 'nim-stream') == primary_reference['binary_sha256']
        assert sha(reference / 'BundledMain.nim') == primary_reference['source_sha256']
        metadata['reference']['stream_build'] = primary_reference['build']
        metadata['reference']['provenance'] = primary_reference['provenance']
        metadata['binaries']['nim_stream'] = {'path': str(reference / 'nim-stream'), 'sha256': sha(reference / 'nim-stream'), 'source': 'reference Nim primary, scoped checks-off before main only', 'family': 'stream', 'caller': 'original count caller'}
        if api_reference:
            for src_name in [api_reference['source_name'], *api_reference['extras']]:
                shutil.copyfile(args.nim_api_directory / src_name, reference / src_name)
            shutil.copyfile(args.nim_api_directory / api_reference['binary_name'], reference / 'nim-api')
            (reference / 'nim-api').chmod(0o755)
            assert sha(reference / 'nim-api') == api_reference['binary_sha256']
            assert sha(reference / 'abc343f_api.nim') == api_reference['source_sha256']
            metadata['reference']['api_build'] = api_reference['build']
            metadata['binaries']['nim_api'] = {'path': str(reference / 'nim-api'), 'sha256': sha(reference / 'nim-api'), 'source': 'instrumented faithful Nim library source', 'family': 'api', 'caller': 'original count caller'}
        metadata['sources']['reference'] = {path.name: sha(path) for path in reference.iterdir() if path.is_file() and path.suffix in ['.nim', '.json']}
        small = [json.loads(path.read_text()) for path in sorted(args.small_cases.glob('small*.json'))]
        large = [json.loads((args.large_cases / f'{shape}.json').read_text()) for shape in SHAPES]
        result['cases'] = large
        for case in small + large:
            assert sha(case['input']['path']) == case['input']['sha256']
            assert sha(case['expected']['path']) == case['expected']['sha256']
            expected = Path(case['expected']['path']).read_bytes()
            for tag, binary in metadata['binaries'].items():
                actual = args.work / 'preflight-output.txt'
                command = ['taskset', '-c', str(args.cpu), binary['path']]
                with open(case['input']['path'], 'rb') as source_input, actual.open('wb') as destination:
                    run = subprocess.run(command, stdin=source_input, stdout=destination, stderr=subprocess.PIPE, check=True, timeout=120)
                assert actual.read_bytes() == expected, (case['name'], tag, 'wrong answer')
                timers = json.loads(run.stderr) if binary['family'] == 'api' else None
                if timers:
                    assert timers['answers'] == len(expected.split())
                    for field in ['build_cpu', 'build_wall', 'operation_cpu', 'operation_wall']:
                        assert timers[field] >= 0, (tag, field)
                result['validation'].append({'case': case['name'], 'variant': tag, 'input_sha256': case['input']['sha256'], 'answer_sha256': sha(actual), 'answers': len(expected.split()), 'api': timers, 'command': command})
            print('validated', case['name'], len(metadata['binaries']), flush=True)
            save(output_path, result)
        assert final_hashes == {path: sha(args.repo / path) for path in FINAL_HEADERS}, 'production changed before timing'
    def measured(case, tag, binary, trial, sequence):
        command = ['taskset', '-c', str(args.cpu), binary['path']]
        actual = args.work / 'measured-output.txt'
        with open(case['input']['path'], 'rb') as source_input, actual.open('wb') as destination:
            start_wall, start_cpu = time.perf_counter_ns(), cpu_now()
            run = subprocess.run(command, stdin=source_input, stdout=destination, stderr=subprocess.PIPE, check=True)
            process_cpu = cpu_now() - start_cpu
            process_wall = (time.perf_counter_ns() - start_wall) / 1e9
        digest = sha(actual)
        assert digest == case['expected']['sha256'], (case['name'], tag, 'wrong timed answer')
        timers = json.loads(run.stderr) if binary['family'] == 'api' else None
        if timers:
            assert timers['answers'] == case['stats']['query']
            timers['total_cpu'] = timers['build_cpu'] + timers['operation_cpu']
            timers['total_wall'] = timers['build_wall'] + timers['operation_wall']
        return {'case': case['name'], 'variant': tag, 'family': binary['family'], 'trial': trial, 'sequence': sequence, 'wall_seconds': process_wall, 'cpu_seconds': process_cpu, 'answer_sha256': digest, 'api': timers, 'command': command}
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        rng = random.Random(args.seed)
        sequence = 0
        cases_order = large.copy()
        rng.shuffle(cases_order)
        metadata['case_order'] = [case['name'] for case in cases_order]
        for case in cases_order:
            warmup_order = list(metadata['binaries'])
            rng.shuffle(warmup_order)
            for tag in warmup_order:
                result['warmups'].append(measured(case, tag, metadata['binaries'][tag], -1, sequence))
                sequence += 1
            for trial in range(args.repeats):
                order = list(metadata['binaries'])
                rng.shuffle(order)
                for tag in order:
                    result['samples'].append(measured(case, tag, metadata['binaries'][tag], trial, sequence))
                    sequence += 1
                save(output_path, result)
            print('timed', case['name'], args.repeats, 'rounds', flush=True)
            assert final_hashes == {path: sha(args.repo / path) for path in FINAL_HEADERS}, 'production changed during timing'
        for tag, binary in metadata['binaries'].items():
            assert sha(binary['path']) == binary['sha256'], (tag, 'binary changed')
        for name in ['baseline', 'final']:
            for path, expected in metadata['sources'][name].items():
                assert sha(args.work / 'sources' / name / path) == expected, (name, path, 'snapshot changed')
    for case in large:
        table = {}
        for tag, binary in metadata['binaries'].items():
            samples = [row for row in result['samples'] if row['case'] == case['name'] and row['variant'] == tag]
            metrics = {'wall_seconds': [row['wall_seconds'] for row in samples], 'cpu_seconds': [row['cpu_seconds'] for row in samples]}
            if binary['family'] == 'api':
                for key in ['build_cpu', 'operation_cpu', 'build_wall', 'operation_wall', 'total_cpu', 'total_wall']:
                    metrics['api_' + key] = [row['api'][key] for row in samples]
            table[tag] = {key: {'median': statistics.median(values), 'min': min(values), 'max': max(values)} for key, values in metrics.items()}
        result['summary'][case['name']] = table
    metadata['completed_utc'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save(output_path, result)
    for case, table in result['summary'].items():
        print(case, flush=True)
        for tag, metrics in table.items():
            key = 'api_operation_cpu' if metadata['binaries'][tag]['family'] == 'api' else 'wall_seconds'
            print(tag, key, round(metrics[key]['median'] * 1000, 3), flush=True)


if __name__ == '__main__':
    main()
