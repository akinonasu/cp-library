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

from shortest_path import check_synthetic, synthetic_cases

ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'f4aa74365d7802810c5c04b57eb01ed6e3aa5438'
DRIVER = 'test/benchmark/shortest_path_round3.cpp'
SEED = 20261019


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def freeze(destination, driver, source=ROOT, revision=None):
    destination.mkdir(parents=True, exist_ok=True)
    pending = [Path('graph/dijkstra.hpp'), Path('components/fast_io.hpp')]
    copied = {}
    for include in re.findall(r'^\s*#include\s+"([^"\r\n]+)"', driver, re.M):
        pending.append(Path(include))
    while pending:
        relative = Path(os.path.normpath(pending.pop()))
        key = relative.as_posix()
        if key in copied:
            continue
        if revision:
            result = subprocess.run(['git', 'show', revision + ':' + key], cwd=ROOT, capture_output=True)
            if result.returncode:
                continue
            data = result.stdout
        else:
            if not (source / relative).exists():
                continue
            data = (source / relative).read_bytes()
        data = data.replace(b'\r\n', b'\n')
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        copied[key] = sha(target)
        for include in re.findall(rb'^\s*#include\s+"([^"\r\n]+)"', data, re.M):
            pending.append(relative.parent / include.decode())
    return copied


def freeze_reference(destination, source, revision=None):
    destination.mkdir(parents=True, exist_ok=True)
    copied = {}
    files = {}

    def copy(path):
        path = path.resolve()
        if path in files:
            return files[path]
        target = destination / (str(len(files)) + path.suffix)
        files[path] = target
        data = subprocess.check_output(['git', 'show', revision + ':' + path.relative_to(ROOT).as_posix()], cwd=ROOT).decode().replace('\r\n', '\n') if revision else path.read_text()
        key = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
        copied[key] = hashlib.sha256(data.encode()).hexdigest()

        def include(match):
            included = path.parent / match[2]
            if not included.exists():
                included = ROOT / match[2]
            if not included.exists():
                raise FileNotFoundError(included)
            return match[1] + copy(included).name + match[3]

        data = re.sub(r'(^\s*#include\s+")([^"\r\n]+)(")', include, data, flags=re.M)
        target.write_text(data)
        return target

    translation = copy(source)
    manifest = {str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path): target.name for path, target in files.items()}
    return translation, copied, manifest


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--problem', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path')
    parser.add_argument('--variant', action='append', default=[], metavar='NAME=DIRECTORY')
    parser.add_argument('--reference', action='append', default=[], metavar='NAME=SOURCE')
    parser.add_argument('--option', action='append', default=[], metavar='NAME=FLAG')
    parser.add_argument('--reference-only', action='store_true')
    parser.add_argument('--submissions', action='store_true')
    parser.add_argument('--topological', action='store_true')
    parser.add_argument('--only', nargs='*')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--official-only', action='store_true')
    parser.add_argument('--synthetic-only', action='store_true')
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/shortest-path/round3'
    cache.mkdir(parents=True, exist_ok=True)
    temporary = tempfile.TemporaryDirectory(prefix='cp-shortest-round3-')
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
    extra_options = {}
    for item in args.option:
        name, option = item.split('=', 1)
        extra_options.setdefault(name, []).append(option)
    driver = (ROOT / DRIVER).read_text()
    variants = [('baseline', ROOT, BASELINE), ('current', ROOT, None)]
    if args.topological:
        variants += [('baseline_topo', ROOT, BASELINE), ('current_topo', ROOT, None)]
    for item in args.variant:
        name, directory = item.split('=', 1)
        variants.append((name, Path(directory).resolve(), None))
    if args.reference_only:
        variants = []
    binaries = {}
    compile_jobs = []
    sources = {}
    references = {}
    reference_snapshots = {}
    for name, source, revision in variants:
        if name in binaries:
            raise ValueError('Duplicate name: ' + name)
        snapshot = output / 'sources' / name
        sources[name] = freeze(snapshot, driver, source, revision)
        translation = snapshot / 'driver.cpp'
        translation.write_text(driver)
        options = [*flags, *extra_options.get(name, []), '-I', str(snapshot)]
        if revision:
            options.append('-DSP_BASELINE=1')
        if name.endswith('_topo'):
            options.append('-DSP_TOPOLOGICAL=1')
        binary = work / name
        compile_jobs.append(['g++', *options, str(translation), '-o', str(binary)])
        binaries[name] = binary
    reference_settings = []
    if args.submissions:
        reference_settings += [('own_baseline', ROOT / 'test/dijkstra_path.test.cpp', BASELINE),
                               ('own_path', ROOT / 'test/dijkstra_path.test.cpp', None),
                               ('own_topo', ROOT / 'test/dijkstra_topological.test.cpp', None)]
    for item in args.reference:
        name, path = item.split('=', 1)
        reference_settings.append((name, Path(path).resolve(), None))
    for name, source, revision in reference_settings:
        if name in binaries:
            raise ValueError('Duplicate name: ' + name)
        archive = source.is_relative_to(ROOT) and not source.is_relative_to(ROOT / '.verify-helper/cache/shortest-path/reference')
        snapshot = (output / 'sources' / name) if archive else (work / ('reference-' + name))
        key = source, revision
        if key not in reference_snapshots:
            reference_snapshots[key] = freeze_reference(snapshot, source, revision)
        translation, copied, manifest = reference_snapshots[key]
        binary = work / name
        compile_jobs.append(['g++', *flags, *extra_options.get(name, []), str(translation), '-o', str(binary)])
        binaries[name] = binary
        source_key = str(source.relative_to(ROOT)) if source.is_relative_to(ROOT) else str(source)
        references[name] = dict(source=str(source), revision=revision, sha256=copied[source_key], dependencies=copied,
                               entry=str(translation.relative_to(output)) if archive else None,
                               archived_dependencies=manifest if archive else None)
    if set(extra_options) - binaries.keys():
        raise ValueError('Options for unknown variant')
    live_sources = {}
    for name, source, revision in variants:
        if source == ROOT and revision is None:
            for key, digest in sources[name].items():
                assert key not in live_sources or live_sources[key] == digest
                live_sources[key] = digest
    for name, source, revision in reference_settings:
        if revision is None and source.is_relative_to(ROOT) and not source.is_relative_to(ROOT / '.verify-helper/cache'):
            for key, digest in references[name]['dependencies'].items():
                assert key not in live_sources or live_sources[key] == digest
                live_sources[key] = digest
    assert all(sha(ROOT / key) == digest for key, digest in live_sources.items())
    environment = dict(measured_at=datetime.now(timezone.utc).isoformat(), baseline=BASELINE,
        flags=flags, extra_options=extra_options, compiler=subprocess.check_output(['g++', '--version'], text=True),
        cpu=subprocess.check_output(['lscpu'], text=True), kernel=subprocess.check_output(['uname', '-a'], text=True),
        affinity=[args.cpu], trials=args.trials, warmup=1, order_seed=SEED, sources=sources,
        driver_sha256=sha(ROOT / DRIVER), runner_sha256=sha(Path(__file__)), references=references,
        utility_sha256=sha(ROOT / 'test/benchmark/shortest_path.py'),
        live_sources=live_sources,
        archived_sources={str(path.relative_to(output)): sha(path) for path in (output / 'sources').rglob('*') if path.is_file()},
        method='shared exclusive /tmp/cp-wm-opt.lock; CPU affinity; one warmup; fresh processes; randomized order; binaries and regular-file input/output on Linux /tmp; phases match the submission graph type and construction; whole includes process startup/exit and output; checker is outside timing')
    revision = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.problem, text=True, capture_output=True) if args.problem.exists() else None
    environment['problem_revision'] = revision.stdout.strip() if revision is not None and revision.returncode == 0 else None
    (output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    print('Frozen sources', environment['sources'].get('current', {}), flush=True)
    for command in compile_jobs:
        with exclusive():
            subprocess.run(command, check=True)
    cases = [] if args.synthetic_only else sorted((args.problem / 'in').glob('*.in'))
    if not args.official_only:
        with exclusive():
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
        synthetic_edges = None
        with exclusive():
            for trial in range(-1, args.trials):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    answer_path = work / 'output.out'
                    with inp.open('rb') as stream, answer_path.open('wb') as destination:
                        start = time.perf_counter()
                        completed = subprocess.run([binaries[name]], stdin=stream, stdout=destination, stderr=subprocess.PIPE, check=True)
                        whole_ms = (time.perf_counter() - start) * 1000
                    value = dict(distance=int(answer_path.read_bytes().split(maxsplit=1)[0])) if name in references else json.loads(completed.stderr)
                    if expected is None:
                        expected = value['distance']
                    assert expected == value['distance'], (case.name, name, expected, value)
                    if trial == -1:
                        if case.stem.startswith('synthetic_'):
                            synthetic_edges = check_synthetic(inp, answer_path, expected, synthetic_edges)
                        else:
                            subprocess.run([args.problem / 'checker', inp, answer_path, args.problem / 'out' / (case.stem + '.out')], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if trial >= 0:
                        value['whole_ms'] = whole_ms
                        record['samples'].setdefault(name, []).append(value)
        for name, samples in record['samples'].items():
            keys = ['whole_ms'] if name in references else ['parse_ms', 'build_ms', 'solve_ms', 'output_ms', 'whole_ms']
            record['median_ms'][name] = {key: statistics.median(sample[key] for sample in samples) for key in keys}
        record['distance'] = expected
        results[case.stem] = record
        (output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
        print(case.stem, {name: round(row['whole_ms'], 3) for name, row in record['median_ms'].items()}, flush=True)
    assert sha(ROOT / DRIVER) == environment['driver_sha256']
    assert all(sha(ROOT / key) == digest for key, digest in live_sources.items())
    assert all(sha(output / path) == digest for path, digest in environment['archived_sources'].items())
    print('All distances and paths passed', flush=True)


if __name__ == '__main__':
    main()
