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
BASELINE = 'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0'
NATIVE_UNITS = {
    'test/unit/fps.cpp',
    'test/unit/fps_io.cpp',
    'test/unit/fps_exp_tail.cpp',
    'test/unit/fps_bostan.cpp',
    'test/unit/fps_convolution_borrow.cpp',
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')


def load_ci(root):
    spec = importlib.util.spec_from_file_location('fps_round3_ci', root / '.verify-helper/ci.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-tree', type=Path, default=Path('/tmp/cp-fps-round3-base'))
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round3_validation')
    parser.add_argument('--gcc', type=Path, default=Path('/usr/bin/g++'))
    parser.add_argument('--clang', type=Path, default=Path('/tmp/fps-clang/toolchain/usr/bin/clang++-19'))
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-scratch-round3/validation'))
    parser.add_argument('--execute', action='store_true')
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    args.work = args.work.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    compilers = {'gcc14': args.gcc.absolute(), 'clang19': args.clang.absolute()}
    versions = {name: subprocess.check_output([str(path), '--version'], text=True)
                for name, path in compilers.items()}
    environment = json.dumps(dict(scope='fps-round3-validation', compilers=versions,
                                  compiler_paths={k: str(v) for k, v in compilers.items()}), sort_keys=True)
    ci = load_ci(ROOT)
    baseline = ci.build_plan(args.baseline_tree, environment=environment)
    selected = ci.build_plan(ROOT, previous=baseline['state'], environment=environment)
    changed_units = [name for name in selected['unit']
                     if not (args.baseline_tree / name).is_file()
                     or sha(ROOT / name) != sha(args.baseline_tree / name)]
    selected['sanitize'] = sorted(NATIVE_UNITS & set(selected['unit']))
    result = dict(baseline=BASELINE, baseline_tree=str(args.baseline_tree.resolve()), current=str(ROOT),
                  environment=environment, versions=versions,
                  compilers={name: dict(path=str(path), sha256=sha(path.resolve()))
                             for name, path in compilers.items()},
                  harness_sha256=sha(Path(__file__)),
                  started=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  status='prepared', commands=[], dependency_commands=[], changed_units=changed_units,
                  sanitize_policy='selected unit intersection with native focused units',
                  work=str(args.work),
                  selected_counts={group: len(selected[group]) for group in ci.GROUPS})
    write_json(args.output / 'baseline-plan.json', baseline)
    write_json(args.output / 'selection.json', selected)
    write_json(args.output / 'conditions.json', result)
    print(json.dumps({group: selected[group] for group in ['headers', 'unit', 'sanitize']}, indent=2), flush=True)
    if not args.execute:
        print('Prepared only; no compiler, unit or sanitizer checks were run.', flush=True)
        return

    controls = {'.verify-helper/ci.py', 'test/verify.py',
                str(Path(__file__).resolve().relative_to(ROOT))}
    controls.update(str(path.relative_to(ROOT)) for path in (ROOT / '.github/workflows').glob('*.y*ml'))
    dependencies = set(controls)
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-DDEBUG=1', '-I', str(ROOT)]
    process_environment = os.environ.copy()
    process_environment['TMPDIR'] = str(args.work)
    process_environment['ASAN_OPTIONS'] = 'detect_leaks=0'
    process_environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
    result['runtime_environment'] = {key: process_environment[key] for key in ['ASAN_OPTIONS', 'UBSAN_OPTIONS', 'TMPDIR']}
    result['status'] = 'running'
    result['flags_headers'] = flags + ['-Werror', '-fsyntax-only']
    result['flags_units'] = flags
    result['flags_sanitize'] = flags + ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']

    def checkpoint():
        write_json(args.output / 'result.json', result)

    def execute(name, command):
        row = dict(name=name, command=command, cwd=str(ROOT), log=name + '.log')
        result['commands'].append(row)
        checkpoint()
        begin = time.monotonic()
        with open('/tmp/cp-fps-tuning.lock', 'w') as lock, (args.output / row['log']).open('w') as log:
            fcntl.flock(lock, fcntl.LOCK_SH)
            completed = subprocess.run(command, cwd=ROOT, env=process_environment,
                                       stdout=log, stderr=subprocess.STDOUT)
        row.update(returncode=completed.returncode, seconds=time.monotonic() - begin,
                   log_sha256=sha(args.output / row['log']))
        checkpoint()
        print(name, row['returncode'], row['seconds'], flush=True)
        return completed.returncode == 0

    checkpoint()
    with tempfile.TemporaryDirectory(prefix='fps-round3-dependencies-', dir=args.work) as folder:
        for compiler_name, compiler in compilers.items():
            with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
                fcntl.flock(lock, fcntl.LOCK_SH)
                for name in sorted(set(selected['headers'] + selected['unit'])):
                    if name.endswith('.hpp'):
                        source = Path(folder) / 'header.cpp'
                        source.write_text('#include "' + name + '"\n#include "' + name + '"\nint main() {}\n')
                    else:
                        source = ROOT / name
                    command = [str(compiler), *flags, '-MM', str(source)]
                    completed = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
                    row = dict(compiler=compiler_name, target=name, command=command,
                               returncode=completed.returncode, stdout=completed.stdout, stderr=completed.stderr)
                    result['dependency_commands'].append(row)
                    if completed.returncode:
                        result['status'] = 'dependency scan failed'
                        checkpoint()
                        raise SystemExit(1)
                    for token in shlex.split(completed.stdout.replace('\\\n', ' ').partition(':')[2]):
                        path = Path(token)
                        path = path.resolve() if path.is_absolute() else (ROOT / path).resolve()
                        if path.is_relative_to(ROOT):
                            dependencies.add(path.relative_to(ROOT).as_posix())
            print('dependencies scanned', compiler_name, flush=True)
    hashes_before = {name: sha(ROOT / name) for name in sorted(dependencies)}
    write_json(args.output / 'dependencies-before.json', hashes_before)
    checkpoint()
    success = True
    for compiler_name, compiler in compilers.items():
        command = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx', str(compiler),
                   '--selection', str(args.output / 'selection.json')]
        success = execute(compiler_name, command) and success
    sanitize_command = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx',
                        str(compilers['gcc14']), '--sanitize', '--selection', str(args.output / 'selection.json')]
    success = execute('gcc14-sanitize', sanitize_command) and success
    hashes_after = {name: sha(ROOT / name) for name in sorted(dependencies)}
    write_json(args.output / 'dependencies-after.json', hashes_after)
    result['dependency_hashes_match'] = hashes_before == hashes_after
    after = ci.build_plan(ROOT, previous=baseline['state'], environment=environment)
    result['plan_fingerprints_match'] = after['state'] == selected['state']
    write_json(args.output / 'after-plan.json', after)
    result['status'] = ('passed' if success and result['dependency_hashes_match']
                        and result['plan_fingerprints_match'] else 'failed')
    result['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    checkpoint()
    print(result['status'], flush=True)
    if result['status'] != 'passed':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
