#!/usr/bin/env python3
"""Compile once, freeze sources, run a JSON plan, and preserve every timing."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
SOURCES = ['test/benchmark/warshall_floyd.cpp', 'test/benchmark/warshall_floyd.py',
           'graph/warshall_floyd.hpp', 'components/warshall_floyd_kernel.hpp',
           'components/fast_io.hpp']
VARIANTS = {'auto': [], 'avx2': ['-DWARSHALL_FLOYD_DISABLE_AVX512'],
            'scalar': ['-DWARSHALL_FLOYD_DISABLE_SIMD'], 'baseline': []}


def sha(path):
    h = hashlib.sha256()
    with open(path, 'rb') as stream:
        for block in iter(lambda: stream.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--plan', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    plan = json.loads(args.plan.read_text())
    frozen = {path: sha(ROOT / path) for path in SOURCES}
    manifest_path = args.work / 'manifest.json'
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-I', str(ROOT)]
    commands = {name: [args.cxx, *flags, *extra, str(ROOT / SOURCES[0]), '-o', str(args.work / name)]
                for name, extra in VARIANTS.items() if name != 'baseline'}
    if manifest_path.exists():
        manifest = json.loads(manifest_path.read_text())
        assert manifest['sources'] == frozen, 'source changed; use another work directory'
        assert manifest['commands'] == commands
        assert manifest['binaries'] == {name: sha(args.work / name) for name in commands}
    else:
        with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            for command in commands.values():
                subprocess.run(command, check=True)
        manifest = {'utc': str(datetime.datetime.now(datetime.timezone.utc)), 'sources': frozen,
                    'commands': commands, 'binaries': {name: sha(args.work / name) for name in commands},
                    'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
                    'uname': list(os.uname()), 'cpu': subprocess.check_output(['lscpu'], text=True),
                    'pin': args.cpu, 'seed': 20261020, 'single_thread': True}
        manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    result = {'manifest': manifest, 'plan': plan, 'cases': []}
    rng = random.Random(20261020)
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for case in plan:
            shape, n = case['shape'], case['n']
            io = case.get('io', False)
            modes = case.get('modes', ['auto'])
            rows = {mode: [] for mode in modes}
            input_path, output_path = args.work / 'input.txt', args.work / 'output.txt'
            metadata = {}
            if io:
                with input_path.open('wb') as stream:
                    subprocess.run(['taskset', '-c', str(args.cpu), str(args.work / 'auto'), 'generate', str(n), shape],
                                   stdout=stream, check=True)
                metadata = {'input_sha256': sha(input_path), 'input_bytes': input_path.stat().st_size}
            # One full warm-up per variant, then deterministic shuffled rounds.
            for trial in range(-1, case.get('repeats', 7)):
                order = modes.copy()
                rng.shuffle(order)
                for mode in order:
                    binary = 'auto' if mode == 'baseline' else mode
                    command = ['taskset', '-c', str(args.cpu), str(args.work / binary),
                               'io' if io else ('baseline' if mode == 'baseline' else 'memory'), str(n), shape]
                    if io:
                        with input_path.open('rb') as source, output_path.open('wb') as destination:
                            start = time.perf_counter_ns()
                            run = subprocess.run(command, stdin=source, stdout=destination, stderr=subprocess.PIPE, check=True)
                            wall = (time.perf_counter_ns() - start) / 1e9
                        api = int(run.stderr) / 1e9
                        digest = sha(output_path)
                        size = output_path.stat().st_size
                        if 'output_sha256' in metadata:
                            assert (digest, size) == (metadata['output_sha256'], metadata['output_bytes'])
                        metadata.update(output_sha256=digest, output_bytes=size)
                    else:
                        start = time.perf_counter_ns()
                        run = subprocess.run(command, capture_output=True, check=True)
                        wall = (time.perf_counter_ns() - start) / 1e9
                        elapsed, digest = run.stdout.decode().split()
                        api = int(elapsed) / 1e9
                        if 'checksum' in metadata:
                            assert digest == metadata['checksum'], (case, mode, 'wrong answer')
                        metadata['checksum'] = digest
                    if trial >= 0:
                        rows[mode].append({'trial': trial, 'api_seconds': api, 'process_seconds': wall})
            summaries = {}
            for mode, runs in rows.items():
                summaries[mode] = {key: {'median': statistics.median(row[key] for row in runs),
                                         'min': min(row[key] for row in runs),
                                         'max': max(row[key] for row in runs)}
                                   for key in ('api_seconds', 'process_seconds')}
            record = {**case, **metadata, 'samples': rows, 'summary': summaries}
            result['cases'].append(record)
            assert frozen == {path: sha(ROOT / path) for path in SOURCES}
            args.output.write_text(json.dumps(result, indent=2) + '\n')
            print(json.dumps({**case, 'summary': summaries}), flush=True)
            if io:
                input_path.unlink()
                output_path.unlink()


if __name__ == '__main__':
    main()
