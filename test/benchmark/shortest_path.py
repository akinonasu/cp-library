import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '91a7388afe28532d7dd8c175ddda718c42f428c2'
HEADER = 'graph/dijkstra.hpp'
DRIVER = 'test/benchmark/shortest_path.cpp'
SEED = 20261019


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def snapshot(destination, header=HEADER, source=None, revision=None):
    destination.mkdir(parents=True, exist_ok=True)
    source = ROOT if source is None else source
    pending = [Path(header)]
    copied = {}
    while pending:
        relative = pending.pop()
        relative = Path(os.path.normpath(relative))
        if str(relative) in copied:
            continue
        if revision:
            data = subprocess.check_output(['git', 'show', revision + ':' + relative.as_posix()], cwd=source)
        else:
            data = (source / relative).read_bytes()
        text = data.decode().replace('\r\n', '\n')
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text)
        copied[str(relative)] = sha(target)
        for include in re.findall(r'^\s*#include\s+"([^"]+)"', text, re.M):
            pending.append(relative.parent / include)
    return copied


def synthetic_cases(destination):
    destination.mkdir(parents=True, exist_ok=True)
    rng = random.Random(SEED)

    def write(name, n, edges):
        edges = list(edges)
        path = destination / (name + '.in')
        with path.open('w') as stream:
            stream.write(f'{n} {len(edges)} 0 {n-1}\n')
            stream.writelines(f'{u} {v} {w}\n' for u, v, w in edges)
        return path

    cases = []
    n = 500000
    cases.append(write('synthetic_line', n, ((v-1, v, 10**9) for v in range(1, n))))
    n, m = 200000, 500000
    for name, bound in [('random', 10**9), ('zero', 0), ('binary', 1)]:
        edges = [(v-1, v, rng.randrange(bound+1)) for v in range(1, n)]
        edges += [(rng.randrange(n), rng.randrange(n), rng.randrange(bound+1)) for _ in range(m-len(edges))]
        cases.append(write('synthetic_' + name, n, edges))
    n = 707
    cases.append(write('synthetic_dense', n, ((u, v, rng.randrange(10**9)) for u in range(n) for v in range(n) if u != v)))
    n = 1000
    cases.append(write('synthetic_relaxation', n, ((u, v, 2*(v-u)-1) for u in range(n) for v in range(u+1, n))))
    n, reached, m = 500000, 100000, 500000
    edges = [(v-1, v, 1) for v in range(1, reached)]
    edges += [(rng.randrange(reached), rng.randrange(reached), rng.randrange(10**9)) for _ in range(m-len(edges))]
    cases.append(write('synthetic_disconnected', n, edges))
    return cases


def check_synthetic(inp, output, expected, edges):
    if edges is None:
        edges = {}
        with inp.open() as stream:
            n, m, source, target = map(int, next(stream).split())
            for line in stream:
                u, v, weight = map(int, line.split())
                edges[u, v] = min(weight, edges.get((u, v), weight))
        edges['endpoints'] = source, target
    values = list(map(int, output.read_bytes().split()))
    assert values[0] == expected
    if expected == -1:
        assert len(values) == 1
        return edges
    count = values[1]
    assert len(values) == 2 + count * 2
    source, target = edges['endpoints']
    total = 0
    vertex = source
    for i in range(count):
        u, v = values[2+i*2:4+i*2]
        assert u == vertex and (u, v) in edges
        total += edges[u, v]
        vertex = v
    assert vertex == target and total == expected
    return edges


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--problem', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path')
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--target-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--topological-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--topological-target-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--csr-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--csr-target-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--csr-topological-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--csr-topological-target-variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--reference', action='append', default=[], metavar='NAME=SOURCE')
    parser.add_argument('--reference-only', action='store_true')
    parser.add_argument('--only', nargs='*')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--no-heap', action='store_true')
    parser.add_argument('--kernel-only', action='store_true')
    parser.add_argument('--synthetic-only', action='store_true')
    parser.add_argument('--official-only', action='store_true')
    args = parser.parse_args()
    if args.kernel_only and args.reference:
        raise ValueError('References require whole-process timing')
    if args.reference_only and not args.reference:
        raise ValueError('No references')
    os.sched_setaffinity(0, {args.cpu})
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/shortest-path/benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    temporary = tempfile.TemporaryDirectory(prefix='cp-shortest-path-')
    work = Path(temporary.name)
    lock = open('/tmp/cp-wm-opt.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    flags = ['-std=gnu++23', '-O2', '-DNDEBUG', '-DATCODER', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero']
    if args.native:
        flags.append('-march=native')
    variants = [('baseline', None, False, False, False), ('current', ROOT, False, False, False)]
    for setting in args.variant:
        name, path = setting.split('=', 1)
        variants.append((name, Path(path).resolve(), False, False, False))
    for setting in args.target_variant:
        name, path = setting.split('=', 1)
        variants.append((name, Path(path).resolve(), True, False, False))
    for setting in args.topological_variant:
        name, path = setting.split('=', 1)
        variants.append((name, Path(path).resolve(), False, True, False))
    for setting in args.topological_target_variant:
        name, path = setting.split('=', 1)
        variants.append((name, Path(path).resolve(), True, True, False))
    for settings, target, topological in [(args.csr_variant, False, False), (args.csr_target_variant, True, False),
                                           (args.csr_topological_variant, False, True), (args.csr_topological_target_variant, True, True)]:
        for setting in settings:
            name, path = setting.split('=', 1)
            variants.append((name, Path(path).resolve(), target, topological, True))
    if not args.no_heap:
        variants.append(('binary_heap', ROOT, False, False, False))
    if args.reference_only:
        variants = []
    if len({name for name, _, _, _, _ in variants}) != len(variants):
        raise ValueError('Duplicate variant')
    sources = {}
    binaries = {}
    references = {}
    for name, source, target, topological, csr in variants:
        destination = cache / 'headers' / name
        sources[name] = snapshot(destination, source=source, revision=args.baseline if name == 'baseline' else None)
        binary = work / name
        options = [*flags, '-I', str(ROOT), '-DSP_BENCH_HEADER="' + str(destination / HEADER) + '"']
        if name == 'binary_heap':
            options.append('-DSP_REFERENCE_HEAP=1')
        if target:
            options.append('-DSP_TARGET_API=1')
        if topological:
            options.append('-DSP_TOPOLOGICAL=1')
        if csr:
            options.append('-DSP_CSR_GRAPH=1')
        with exclusive():
            subprocess.run(['g++', *options, str(ROOT / DRIVER), '-o', str(binary)], check=True)
        binaries[name] = binary
    for setting in args.reference:
        name, path = setting.split('=', 1)
        if name in binaries:
            raise ValueError('Duplicate reference')
        source = Path(path).resolve()
        binary = work / name
        with exclusive():
            subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = binary
        references[name] = dict(source=str(source), sha256=sha(source))
    environment = dict(measured_at=datetime.now(timezone.utc).isoformat(), baseline=args.baseline,
        flags=flags, compiler=subprocess.check_output(['g++', '--version'], text=True),
        cpu=subprocess.check_output(['lscpu'], text=True), kernel=subprocess.check_output(['uname', '-a'], text=True),
        affinity=[args.cpu], trials=args.trials, warmup=1, order_seed=SEED, sources=sources,
        driver_sha256=sha(ROOT / DRIVER), runner_sha256=sha(Path(__file__)), io_sha256=sha(ROOT / 'components/fast_io.hpp'),
        target_variants=[name for name, _, target, _, _ in variants if target],
        topological_variants=[name for name, _, _, topological, _ in variants if topological],
        csr_variants=[name for name, _, _, _, csr in variants if csr],
        references=references,
        method='shared exclusive /tmp/cp-wm-opt.lock; CPU affinity; one warmup; fresh processes; randomized order; binaries and regular-file input/output on Linux /tmp; solve excludes parsing, graph construction, separate BFS recovery, checksum, path validation and output; target solve includes parent recovery in dijkstra_path; whole includes process startup/exit, parsing, construction, solve, recovery, output and checksum; output checker is outside timing')
    revision = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.problem, text=True, capture_output=True) if args.problem.exists() else None
    environment['problem_revision'] = revision.stdout.strip() if revision is not None and revision.returncode == 0 else None
    if args.kernel_only:
        environment['method'] += '; kernel-only whole_ms also includes in-process path validation and emits only JSON'
    (output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    cases = [] if args.synthetic_only else sorted((args.problem / 'in').glob('*.in'))
    if not args.official_only:
        cases += synthetic_cases(cache / 'inputs')
    if args.only:
        cases = [case for case in cases if case.stem in args.only]
    if not cases:
        raise ValueError('No cases')
    rng = random.Random(SEED)
    results = {}
    for case in cases:
        inp = work / 'input.in'
        shutil.copyfile(case, inp)
        with inp.open() as stream:
            n, m, source, target = map(int, next(stream).split())
        record = dict(input_sha256=sha(inp), bytes=inp.stat().st_size, n=n, m=m, source=source, target=target, samples={}, median_ms={})
        expected = None
        expected_distance = None
        synthetic_edges = None
        with exclusive():
            for trial in range(-1, args.trials):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    with inp.open('rb') as stream, (work / 'output.out').open('wb') as destination:
                        start = time.perf_counter()
                        command = [binaries[name], 'kernel' if args.kernel_only else 'whole']
                        completed = subprocess.run(command, stdin=stream, stdout=subprocess.PIPE if args.kernel_only else destination, stderr=subprocess.PIPE, check=True)
                        whole_ms = (time.perf_counter() - start) * 1000
                    if name in references:
                        value = dict(distance=int((work / 'output.out').read_bytes().split(maxsplit=1)[0]))
                    else:
                        value = json.loads(completed.stdout if args.kernel_only else completed.stderr)
                    if expected_distance is None:
                        expected_distance = value['distance']
                    assert expected_distance == value['distance'], (case.name, name, expected_distance, value)
                    if name not in environment['target_variants'] and name not in references:
                        invariant = value['checksum'], value['distance'], value['reached']
                        if expected is None:
                            expected = invariant
                        assert expected == invariant, (case.name, name, expected, invariant)
                    answer = args.problem / 'out' / (case.stem + '.out')
                    if not args.kernel_only and not case.stem.startswith('synthetic_') and trial == -1:
                        subprocess.run([args.problem / 'checker', inp, work / 'output.out', answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    elif not args.kernel_only and trial == -1:
                        synthetic_edges = check_synthetic(inp, work / 'output.out', expected_distance, synthetic_edges)
                    if trial >= 0:
                        value['whole_ms'] = whole_ms
                        record['samples'].setdefault(name, []).append(value)
        for name, samples in record['samples'].items():
            keys = ['whole_ms'] if name in references else ['parse_ms', 'build_ms', 'solve_ms', 'recover_ms', 'output_ms', 'whole_ms']
            record['median_ms'][name] = {key: statistics.median(sample[key] for sample in samples) for key in keys}
        record['checksum'], record['distance'], record['reached'] = expected if expected is not None else (None, expected_distance, None)
        results[case.stem] = record
        (output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
        print(case.stem, {name: round(row.get('solve_ms', row['whole_ms']), 3) for name, row in record['median_ms'].items()}, flush=True)
    assert sha(ROOT / DRIVER) == environment['driver_sha256']
    assert sha(ROOT / 'components/fast_io.hpp') == environment['io_sha256']
    print('All checksums passed', flush=True)


if __name__ == '__main__':
    main()
