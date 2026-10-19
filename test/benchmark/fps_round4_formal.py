import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shlex
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
LOCK = '/tmp/cp-fps-tuning.lock'
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1048576), b''):
            h.update(block)
    return h.hexdigest()


def hashes(tree):
    return {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}


def old_headers(tree):
    return {str(p.relative_to(tree)): sha(p) for folder in ('fps', 'components', 'math', 'convolution', 'test') for p in (tree / folder).glob('*.hpp')}


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(data, indent=2) + '\n')
    temporary.replace(path)


def deps(path, cwd):
    text = path.read_text().replace('\\\n', '')
    return sorted({str((cwd / word).resolve()) for word in shlex.split(text.split(':', 1)[1])})


def compiler_identity(cxx):
    executable = Path(shutil.which(cxx)).resolve()
    tools = {'driver': executable}
    for name in ('cc1plus', 'collect2', 'as', 'ld'):
        name_path = subprocess.check_output([cxx, '-print-prog-name=' + name], text=True).strip()
        path = Path(name_path) if '/' in name_path else Path(shutil.which(name_path))
        tools[name] = path.resolve()
    return dict(version=subprocess.check_output([cxx, '--version'], text=True).splitlines()[0],
                tools={name: dict(path=str(path), sha256=sha(path)) for name, path in tools.items()})


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--initial', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_formal.json')
    parser.add_argument('--old-work', type=Path, default=Path('/workspace/cp-fps-round4/formal'))
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-round4/formal-constant'))
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_constant_formal.json')
    parser.add_argument('--source-pack', type=Path, default=ROOT / 'test/benchmark/results/fps_convolution_constant_final_round4')
    parser.add_argument('--prepared-source', type=Path)
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--seed', type=int, default=20261019)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    initial = json.loads(args.initial.read_text())
    source_metadata_path = args.source_pack / 'sources.json'
    source_metadata = json.loads(source_metadata_path.read_text())
    assert initial['flags'] == FLAGS and initial['baseline'] == source_metadata['baseline']
    assert len(initial['judge']) == 16
    folders = {p.parent.name: p.parent for p in args.official_root.glob('*/*/info.toml')}
    drivers = {}
    for row in initial['judge']:
        command = initial['sources'][row['problem'] + '_before']['command']
        drivers[row['problem']] = Path(command[-3]).relative_to(args.old_work / 'before').as_posix()
    expected = dict(source_metadata['current'])
    driver_hashes = {}
    for problem, driver in drivers.items():
        value = initial['sources'][problem + '_before']['source_sha256']
        if driver in expected:
            assert expected[driver] == value
        expected[driver] = driver_hashes[driver] = value
    assert len(expected) == 169 and len(driver_hashes) == 16
    result = dict(baseline=initial['baseline'], compiler=initial['compiler'], flags=FLAGS, cpu=args.cpu, trials=args.trials,
                  shuffle_seed=args.seed, hardware=subprocess.check_output(['lscpu'], text=True),
                  initial_result=dict(path=str(args.initial), sha256=sha(args.initial)),
                  source_pack=dict(path=str(args.source_pack), sources_sha256=sha(source_metadata_path),
                                   file_count=len(source_metadata['current'])),
                  source_snapshot_sha256=expected, additional_drivers=driver_hashes,
                  harness_sha256=sha(Path(__file__)), official_commit=initial['official_commit'],
                  legacy_provenance='The 32 before/reference executables are reused byte-for-byte after checking their original binary/source/command records, declared snapshot header maps, driver/reference files, compiler version string and flags. Compiler executable hashes and actual system-header dependencies were not recorded for those original builds; current measurements do not retroactively establish them.',
                  current_provenance='Only current executables are rebuilt. -MD captures actual project and system include dependencies; their bytes, the full source snapshot/live source set, compiler tools, all binaries, fixture/checker bytes and legacy sources are guarded before and after timing. This is not a claim that all historical compiler/link/runtime dependencies were captured.',
                  builds={}, reused={}, judge=[], status='building')
    snapshot = args.work / 'source/current'
    if snapshot.exists():
        raise RuntimeError('Use a fresh --work; source/current already exists')
    cases, binaries, dependencies = [], {}, {}
    initial_sha = sha(args.initial)
    pack_sha = sha(source_metadata_path)

    def save():
        write_json(args.output, result)

    def guard():
        actual = dict(snapshot=hashes(snapshot), old_declared_headers={v: old_headers(args.old_work / v) for v in ('before', 'current')},
                      legacy_source_sha256={key: sha(info['source']) for key, info in result['reused'].items()},
                      compiler=compiler_identity(args.cxx),
                      actual_md_dependencies={name: sha(name) for name in dependencies},
                      binaries={problem + '_' + variant: sha(binary) for (problem, variant), binary in binaries.items()},
                      fixtures={r['problem']: {key: sha(r[key]) for key in ('input', 'answer', 'checker')} for r in cases},
                      official_commit=subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip(),
                      initial_result_sha256=sha(args.initial), source_pack_sources_sha256=sha(source_metadata_path))
        assert actual['snapshot'] == expected
        for variant in ('before', 'current'):
            assert actual['old_declared_headers'][variant] == initial['sources'][variant]
        assert actual['compiler'] == result['compiler_build_identity']
        assert actual['actual_md_dependencies'] == dependencies
        assert actual['official_commit'] == initial['official_commit']
        assert actual['initial_result_sha256'] == initial_sha and actual['source_pack_sources_sha256'] == pack_sha
        for key, info in result['reused'].items():
            assert actual['legacy_source_sha256'][key] == info['source_sha256']
            assert actual['binaries'][key] == info['binary_sha256']
        for problem, info in result['builds'].items():
            assert actual['binaries'][problem + '_current'] == info['binary_sha256']
        for case in cases:
            assert actual['fixtures'][case['problem']] == {key: case[key + '_sha256'] for key in ('input', 'answer', 'checker')}
        if not args.prepared_source:
            live = {name: sha(ROOT / name) for name in expected}
            project_names = {str(p.relative_to(ROOT)) for p in ROOT.rglob('*.hpp') if '.git' not in p.parts and ('test' not in p.relative_to(ROOT).parts or p == ROOT / 'test/fps_io.hpp')}
            assert project_names | set(driver_hashes) == set(expected)
            assert live == expected, 'live source changed'
            actual['live_sources_sha256'] = live
        return actual

    try:
        with open(LOCK, 'a') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            identity = compiler_identity(args.cxx)
            assert identity['version'] == initial['compiler']
            result['compiler_build_identity'] = identity
            snapshot.mkdir(parents=True)
            source_tree = args.prepared_source or ROOT
            for name, value in expected.items():
                source = source_tree / name
                assert sha(source) == value, 'frozen source mismatch: ' + name
                target = snapshot / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
            assert hashes(snapshot) == expected
            for variant in ('before', 'current'):
                assert old_headers(args.old_work / variant) == initial['sources'][variant]
            for row in initial['judge']:
                problem = row['problem']
                for variant in ('before', 'reference'):
                    old = initial['sources'][problem + '_' + variant]
                    command = old['command']
                    assert command[1:1 + len(FLAGS)] == FLAGS
                    source, binary = Path(command[-3]), Path(command[-1])
                    assert source.is_file() and binary.is_file()
                    assert sha(source) == old['source_sha256'] and sha(binary) == old['binary_sha256']
                    if variant == 'reference':
                        assert not re.search(r'^\s*#\s*include\s*"', source.read_text(), re.M)
                        assert sha(source) == row['reference']['source_sha256']
                    binaries[problem, variant] = binary
                    result['reused'][problem + '_' + variant] = dict(original_record=old, original_record_sha256=hashlib.sha256(json.dumps(old, sort_keys=True).encode()).hexdigest(),
                        source=str(source), source_sha256=sha(source), binary=str(binary), binary_sha256=sha(binary),
                        compiler_executable_sha256_at_original_build=None, actual_md_dependencies_at_original_build=None)
                folder = folders[problem]
                inp, answer, checker = folder / 'in' / row['input'], folder / 'out' / (Path(row['input']).stem + '.out'), folder / 'checker'
                assert sha(inp) == row['input_sha256'] and sha(answer) == row['answer_sha256']
                cases.append(dict(problem=problem, name=Path(row['input']).stem, input=str(inp), answer=str(answer), checker=str(checker),
                                  input_sha256=sha(inp), answer_sha256=sha(answer), checker_sha256=sha(checker), reference=row['reference']))
            save()

            def build(problem):
                source = snapshot / drivers[problem]
                binary = args.work / (problem + '_current')
                depfile = args.work / (problem + '.d')
                command = [args.cxx, *FLAGS, '-MD', '-MF', str(depfile), '-I', str(snapshot), str(source), '-o', str(binary)]
                log = args.work / (problem + '.build.log')
                with log.open('w') as output:
                    subprocess.run(command, stdout=output, stderr=subprocess.STDOUT, check=True, cwd=args.work)
                paths = deps(depfile, args.work)
                assert not any(Path(p).is_relative_to(ROOT) for p in paths), 'live header leaked into current build'
                return problem, binary, dict(command=command, cwd=str(args.work), source=str(source), source_sha256=sha(source), binary=str(binary),
                    binary_sha256=sha(binary), depfile=str(depfile), depfile_sha256=sha(depfile), actual_dependencies=paths, build_log=log.read_text())

            with ThreadPoolExecutor(max_workers=3) as pool:
                for problem, binary, info in pool.map(build, drivers):
                    binaries[problem, 'current'] = binary
                    result['builds'][problem] = info
                    for path in info['actual_dependencies']:
                        value = sha(path)
                        assert dependencies.get(path, value) == value
                        dependencies[path] = value
                    save()
                    print('built current', problem, len(info['actual_dependencies']), 'actual dependencies', flush=True)
            result['guard_before'] = guard()
            result['status'] = 'timing'
            save()
            fcntl.flock(lock, fcntl.LOCK_UN)
            print('waiting EX', flush=True)
            fcntl.flock(lock, fcntl.LOCK_EX)
            os.sched_setaffinity(0, {args.cpu})
            rng = random.Random(args.seed)
            for case in cases:
                problem = case['problem']
                samples = {v: [] for v in ('before', 'current', 'reference')}
                runs, orders = [], []
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    orders.append(order)
                    for variant in order:
                        output = args.work / (problem + '_' + variant + '.out')
                        with Path(case['input']).open('rb') as source, output.open('wb') as destination:
                            begin = time.perf_counter_ns()
                            subprocess.run([str(binaries[problem, variant])], stdin=source, stdout=destination, check=True)
                            elapsed = time.perf_counter_ns() - begin
                        subprocess.run([case['checker'], case['input'], str(output), case['answer']], check=True,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        runs.append(dict(trial=trial, warmup=trial == 0, variant=variant, ns=elapsed, checker_pass=True))
                        if trial:
                            samples[variant].append(elapsed / 1e6)
                old_row = next(r for r in initial['judge'] if r['problem'] == problem)
                row = dict(case, orders=orders, runs=runs, variants={})
                for variant, values in samples.items():
                    output = args.work / (problem + '_' + variant + '.out')
                    output_sha = sha(output)
                    matches = output_sha == old_row['variants'][variant]['output_sha256']
                    if variant != 'current':
                        assert matches, 'reused executable output changed'
                    row['variants'][variant] = dict(median_ms=statistics.median(values), samples_ms=values, output=str(output), output_sha256=output_sha,
                                                   matches_initial_output_sha256=matches)
                result['judge'].append(row)
                save()
                print(problem, {v: x['median_ms'] for v, x in row['variants'].items()}, flush=True)
            fcntl.flock(lock, fcntl.LOCK_UN)
            fcntl.flock(lock, fcntl.LOCK_SH)
            result['guard_after'] = guard()
            result['guards_equal'] = result['guard_before'] == result['guard_after']
            assert result['guards_equal']
            result['checker_calls'] = sum(len(row['runs']) for row in result['judge'])
            assert result['checker_calls'] == 16 * 3 * (args.trials + 1)
            result['status'] = 'success'
            save()
            print('PASS', result['checker_calls'], 'checker calls; all guards unchanged', flush=True)
    except Exception as error:
        result['status'] = 'failed'
        result['error'] = repr(error)
        save()
        raise


if __name__ == '__main__':
    main()
