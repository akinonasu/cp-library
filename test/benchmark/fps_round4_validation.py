import argparse
import datetime
import fcntl
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import time


ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'bd29f70c70e24e27770b9e9a3777b6cf85664b63'
NATIVE_UNITS = {
    'test/unit/fps.cpp',
    'test/unit/fps_io.cpp',
    'test/unit/fps_convolution_unbalanced.cpp',
    'test/unit/fps_convolution_borrow.cpp',
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')


def inventory(tree):
    return {str(path.relative_to(tree)): sha(path)
            for path in sorted(tree.rglob('*')) if path.is_file()}


def load_ci(root):
    spec = importlib.util.spec_from_file_location('fps_round4_ci', root / '.verify-helper/ci.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def repository_hashes():
    names = subprocess.check_output(['git', 'ls-files', '-z', '-c', '-o', '--exclude-standard'],
                                    cwd=ROOT).decode().split('\0')
    return {name: sha(ROOT / name) for name in sorted(set(names)) if name and (ROOT / name).is_file()}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-round4/validation'))
    parser.add_argument('--baseline-tree', type=Path)
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_validation')
    parser.add_argument('--gcc', type=Path, default=Path('/usr/bin/g++'))
    parser.add_argument('--clang', type=Path, default=Path('/tmp/fps-clang/toolchain/usr/bin/clang++-19'))
    parser.add_argument('--execute', action='store_true')
    args = parser.parse_args()
    args.work = args.work.resolve()
    args.baseline_tree = (args.baseline_tree or args.work / 'baseline').resolve()
    args.output = args.output.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    provenance_path = args.baseline_tree.parent / 'baseline-provenance.json'
    provenance = json.loads(provenance_path.read_text())
    assert provenance['baseline'] == BASELINE
    assert inventory(args.baseline_tree) == provenance['files']
    assert not (args.output / 'result.json').exists(), 'Use a new output directory for a new run'

    compilers = {'gcc14': args.gcc.absolute(), 'clang19': args.clang.absolute()}
    compiler_info = {name: dict(path=str(path), resolved_path=str(path.resolve()),
                               sha256=sha(path.resolve()),
                               version=subprocess.check_output([str(path), '--version'], text=True))
                     for name, path in compilers.items()}
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-DDEBUG=1', '-I', str(ROOT)]
    sanitize_flags = flags + ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
    environment = json.dumps(dict(scope='fps-round4-validation', compilers=compiler_info,
                                  flags=flags, sanitize_flags=sanitize_flags), sort_keys=True)
    ci = load_ci(ROOT)
    baseline = ci.build_plan(args.baseline_tree, environment=environment)
    selected = ci.build_plan(ROOT, previous=baseline['state'], environment=environment)
    selected['sanitize'] = sorted(NATIVE_UNITS & set(selected['unit']))
    changed_units = [name for name in selected['unit']
                     if not (args.baseline_tree / name).is_file()
                     or sha(ROOT / name) != sha(args.baseline_tree / name)]
    result = dict(baseline=BASELINE, baseline_tree=str(args.baseline_tree), current=str(ROOT),
                  baseline_provenance_sha256=sha(provenance_path), environment=environment,
                  compilers=compiler_info, harness_sha256=sha(Path(__file__)),
                  started=datetime.datetime.now(datetime.timezone.utc).isoformat(), status='prepared',
                  commands=[], dependency_commands=[], changed_units=changed_units,
                  sanitize_policy='selected unit intersection with four native focused units',
                  reuse_policy='No private check is reused unless exact compiler, flags, dependencies and success evidence are bound before execution.',
                  work=str(args.work), selected_counts={group: len(selected[group]) for group in ci.GROUPS},
                  flags_headers=flags + ['-Werror', '-fsyntax-only'], flags_units=flags,
                  flags_sanitize=sanitize_flags)
    write_json(args.output / 'baseline-plan.json', baseline)
    write_json(args.output / 'selection.json', selected)
    result['selection_sha256'] = sha(args.output / 'selection.json')
    contract = dict(baseline=BASELINE, selection_sha256=result['selection_sha256'],
                    baseline_provenance_sha256=sha(provenance_path), environment=environment,
                    groups={group: selected[group] for group in ci.GROUPS},
                    state_sha256=hashlib.sha256(json.dumps(selected['state'], sort_keys=True).encode()).hexdigest())
    write_json(args.output / 'scope-contract.json', contract)
    result['scope_contract_sha256'] = sha(args.output / 'scope-contract.json')
    write_json(args.output / 'conditions.json', result)
    print(json.dumps({group: selected[group] for group in ci.GROUPS}, indent=2), flush=True)
    if not args.execute:
        print('Prepared only; no dependency, compiler, unit or sanitizer check was run.', flush=True)
        return

    process_environment = os.environ.copy()
    process_environment['TMPDIR'] = str(args.work)
    process_environment['ASAN_OPTIONS'] = 'detect_leaks=0'
    process_environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
    result['runtime_environment'] = {key: process_environment[key]
                                     for key in ['ASAN_OPTIONS', 'UBSAN_OPTIONS', 'TMPDIR']}
    result['status'] = 'running'
    controls = {'.verify-helper/ci.py', 'test/verify.py', str(Path(__file__).resolve().relative_to(ROOT))}
    controls.update(str(path.relative_to(ROOT)) for path in (ROOT / '.github/workflows').glob('*.y*ml'))
    dependencies = set(controls)
    source_hashes_before_scan = repository_hashes()

    def checkpoint():
        write_json(args.output / 'result.json', result)

    def execute(name, command):
        row = dict(name=name, command=command, cwd=str(ROOT), log=name + '.log')
        result['commands'].append(row)
        checkpoint()
        begin = time.monotonic()
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock, (args.output / row['log']).open('w') as log:
            fcntl.flock(lock, fcntl.LOCK_SH)
            completed = subprocess.run(command, cwd=ROOT, env=process_environment,
                                       stdout=log, stderr=subprocess.STDOUT)
        row.update(returncode=completed.returncode, seconds=time.monotonic() - begin,
                   log_sha256=sha(args.output / row['log']))
        checkpoint()
        print(name, row['returncode'], row['seconds'], flush=True)
        return completed.returncode == 0

    checkpoint()
    with tempfile.TemporaryDirectory(prefix='fps-round4-dependencies-', dir=args.work) as folder:
        scans = [(name, compiler, 'normal', sorted(set(selected['headers'] + selected['unit'])), flags)
                 for name, compiler in compilers.items()]
        scans.append(('gcc14', compilers['gcc14'], 'sanitize', selected['sanitize'], sanitize_flags))
        for compiler_name, compiler, mode, targets, scan_flags in scans:
            with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
                fcntl.flock(lock, fcntl.LOCK_SH)
                for name in targets:
                    if name.endswith('.hpp'):
                        source = Path(folder) / 'header.cpp'
                        source.write_text('#include "' + name + '"\n#include "' + name + '"\nint main() {}\n')
                    else:
                        source = ROOT / name
                    command = [str(compiler), *scan_flags, '-MM', str(source)]
                    completed = subprocess.run(command, cwd=ROOT, env=process_environment,
                                               text=True, capture_output=True)
                    row = dict(compiler=compiler_name, mode=mode, target=name, command=command,
                               returncode=completed.returncode, stdout=completed.stdout,
                               stderr=completed.stderr)
                    result['dependency_commands'].append(row)
                    if completed.returncode:
                        result['status'] = 'dependency scan failed'
                        checkpoint()
                        raise SystemExit(1)
                    names = set()
                    for token in shlex.split(completed.stdout.replace('\\\n', ' ').partition(':')[2]):
                        path = Path(token)
                        path = path.resolve() if path.is_absolute() else (ROOT / path).resolve()
                        if path.is_relative_to(ROOT):
                            names.add(path.relative_to(ROOT).as_posix())
                    dependencies.update(names)
                    row['dependency_hashes'] = {path: source_hashes_before_scan[path] for path in sorted(names)}
            print('dependencies scanned', compiler_name, mode, flush=True)
    hashes_before = {name: source_hashes_before_scan[name] for name in sorted(dependencies)}
    write_json(args.output / 'dependencies-before.json', hashes_before)
    result['dependency_scan_hashes_match'] = all(sha(ROOT / name) == digest for name, digest in hashes_before.items())
    assert result['dependency_scan_hashes_match'], 'Source changed during dependency scan'
    checkpoint()
    success = True
    for compiler_name, compiler in compilers.items():
        command = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx', str(compiler),
                   '--selection', str(args.output / 'selection.json')]
        success = execute(compiler_name, command) and success
    command = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx', str(compilers['gcc14']),
               '--sanitize', '--selection', str(args.output / 'selection.json')]
    success = execute('gcc14-sanitize', command) and success

    hashes_after = {name: sha(ROOT / name) for name in sorted(dependencies)}
    write_json(args.output / 'dependencies-after.json', hashes_after)
    result['dependency_hashes_match'] = hashes_before == hashes_after
    after = ci.build_plan(ROOT, previous=baseline['state'], environment=environment)
    result['plan_fingerprints_match'] = after['state'] == selected['state']
    write_json(args.output / 'after-plan.json', after)
    result['selection_hash_matches'] = sha(args.output / 'selection.json') == contract['selection_sha256']
    result['contract_hash_matches'] = sha(args.output / 'scope-contract.json') == result['scope_contract_sha256']
    result['compiler_hashes_match'] = all(sha(path.resolve()) == compiler_info[name]['sha256']
                                         for name, path in compilers.items())
    coverage = {}
    for row in result['commands']:
        command = row['command']
        compiler = command[command.index('--cxx') + 1]
        if '--sanitize' in command:
            expected = [compiler + ': ' + Path(name).name for name in selected['sanitize']]
        else:
            expected = [compiler + ': standalone ' + name for name in selected['headers']]
            expected += [compiler + ': ' + Path(name).name for name in selected['unit']]
        path = args.output / row['log']
        lines = path.read_text().splitlines()
        observed = [line for line in lines if line.startswith(compiler + ': ')]
        covered = observed == expected and bool(lines) and lines[-1] == 'Verification passed'
        coverage[row['name']] = dict(expected=len(expected), observed=len(observed), exact_order=observed == expected,
                                      verification_passed=covered, log_sha256_matches=sha(path) == row['log_sha256'])
        success = success and covered and coverage[row['name']]['log_sha256_matches']
    result['coverage'] = coverage
    gates = ['dependency_scan_hashes_match', 'dependency_hashes_match', 'plan_fingerprints_match',
             'selection_hash_matches', 'contract_hash_matches', 'compiler_hashes_match']
    result['status'] = 'passed' if success and all(result[name] for name in gates) else 'failed'
    result['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    checkpoint()
    audit = dict(status=result['status'], baseline=BASELINE, selection_sha256=result['selection_sha256'],
                 selected_counts=result['selected_counts'], dependency_files=len(hashes_after), coverage=coverage,
                 gates={name: result[name] for name in gates}, result_sha256=sha(args.output / 'result.json'),
                 harness_sha256=sha(Path(__file__)), normalization_exceptions=[])
    write_json(args.output / 'scope-audit.json', audit)
    print(result['status'], flush=True)
    if result['status'] != 'passed':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
