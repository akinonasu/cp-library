import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shutil
import subprocess
import tempfile
import threading
import time

ROOT = Path(__file__).resolve().parents[2]
QUOTE = re.compile(r'^\s*#\s*include\s*"([^"\r\n]+)"', re.MULTILINE)
PUBLIC = [
    'fps/fps.hpp', 'fps/bostan_mori.hpp', 'fps/compose.hpp',
    'fps/compositional_inverse.hpp', 'fps/interpolate.hpp',
    'fps/linear_recurrence.hpp', 'fps/multipoint_eval.hpp',
    'fps/polynomial_gcd.hpp', 'fps/power_projection.hpp', 'fps/taylor_shift.hpp',
]
SANITIZE = {
    'test/unit/fps.cpp', 'test/unit/fps_algorithms.cpp',
    'test/unit/fps_bostan.cpp', 'test/unit/fps_bostan_workspace.cpp',
    'test/unit/fps_composition_blocked.cpp', 'test/unit/fps_convolution_borrow.cpp',
    'test/unit/fps_convolution_constant.cpp', 'test/unit/fps_convolution_unbalanced.cpp',
    'test/unit/fps_division_blocked.cpp', 'test/unit/fps_evaluation.cpp',
    'test/unit/fps_exp_blocked.cpp', 'test/unit/fps_exp_import.cpp',
    'test/unit/fps_exp_tail.cpp', 'test/unit/fps_inverse_blocked.cpp',
    'test/unit/fps_io.cpp', 'test/unit/fps_log.cpp', 'test/unit/fps_public_headers.cpp',
    'test/unit/fps_workspace.cpp', 'test/unit/fps_tuning_advanced.cpp',
    'test/unit/polynomial_gcd.cpp', 'test/unit/power_projection.cpp',
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def inventory(baseline):
    names = set(git('ls-tree', '-r', '--name-only', baseline).decode().splitlines())
    names.update(str(path.relative_to(ROOT)) for folder in ('components', 'convolution', 'fps', 'math', 'test')
                 for path in (ROOT / folder).rglob('*') if path.suffix in ('.hpp', '.cpp'))
    old, current, dependency_cache = {}, {}, {}

    def content(name, before):
        cache = old if before else current
        if name not in cache:
            if before:
                found = subprocess.run(['git', 'show', baseline + ':' + name], cwd=ROOT,
                                       capture_output=True)
                cache[name] = found.stdout if found.returncode == 0 else None
            else:
                path = ROOT / name
                cache[name] = path.read_bytes() if path.is_file() else None
        return cache[name]

    def dependencies(name, before):
        key = name, before
        if key in dependency_cache:
            return dependency_cache[key]
        dependency_cache[key] = result = {name}
        data = content(name, before)
        if data is not None:
            for include in QUOTE.findall(data.decode()):
                choices = [os.path.normpath(str(Path(name).parent / include)), include]
                target = next((path for path in choices if content(path, before) is not None), None)
                if target is None:
                    raise FileNotFoundError((name, before, include))
                result.update(dependencies(target, before))
        return result

    focus = {name for name in names if (name.startswith('fps/') or name.startswith('components/fps'))
             and name.endswith('.hpp') and 'light' not in name}
    for name in PUBLIC:
        focus.update(dependencies(name, True) | dependencies(name, False))
    changed = {name for name in names if name.endswith(('.hpp', '.cpp')) and content(name, True) != content(name, False)}
    candidate, selected = {}, {}
    for group, prefix, suffix in [('headers', '', '.hpp'), ('unit', 'test/unit/', '.cpp'),
                                   ('verification', 'test/', '.test.cpp')]:
        targets = sorted(name for name in names if name.startswith(prefix) and name.endswith(suffix)
                         and content(name, False) is not None)
        if group == 'headers':
            targets = [name for name in targets if not name.startswith('test/')]
        candidate[group], selected[group] = [], []
        for name in targets:
            before, after = dependencies(name, True), dependencies(name, False)
            if (before | after) & focus:
                candidate[group].append(name)
                if (before | after) & changed:
                    selected[group].append(name)
    selected['sanitize'] = sorted(set(selected['unit']) & SANITIZE)
    watched = {Path(__file__).resolve().relative_to(ROOT).as_posix(), 'test/benchmark/fps_refactor_precision.cpp'}
    watched.update(dependencies('test/benchmark/fps_refactor_precision.cpp', False))
    for names_in_group in selected.values():
        for name in names_in_group:
            watched.update(dependencies(name, False))
    for name in PUBLIC:
        watched.update(dependencies(name, False))
    precision = {}
    for name in sorted(focus):
        data = content(name, True)
        if data is not None:
            rows = [dict(line=i, text=line.strip()) for i, line in enumerate(data.decode().splitlines(), 1)
                    if re.search(r'(?:1U?\s*<<\s*2[1234]|deg\s*[<>]=|assert\(.*deg)', line)]
            if rows:
                precision[name] = rows
    return candidate, selected, sorted(changed), {name: sha(content(name, False)) for name in sorted(watched)}, precision


def run(args):
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.output.exists():
        raise FileExistsError('Use a new output path for each validation run: ' + str(args.output))
    baseline = git('rev-parse', args.baseline).decode().strip()
    candidates, selected, changed, source_hashes, precision = inventory(baseline)
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), baseline_commit=baseline,
                  revision=git('rev-parse', 'HEAD').decode().strip(), script_sha256=sha(Path(__file__).read_bytes()),
                  candidate_inventory=candidates, selection=selected, changed_sources=changed,
                  source_sha256=source_hashes, baseline_precision_statements=precision,
                  completed=False, prepared_only=args.prepare_only, requested_stages=args.stages,
                  jobs=args.jobs, timeout_seconds=args.timeout, commands=[], standalone=[], unit=[],
                  integration=[], precision=[], official=[], skipped_external=[], cases={})

    report_lock = threading.RLock()

    def save():
        with report_lock:
            args.output.write_text(json.dumps(report, indent=2) + '\n')

    save()
    print('selected', {group: len(paths) for group, paths in selected.items()}, flush=True)
    if args.prepare_only:
        return
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
    report['stack_limit'] = list(resource.getrlimit(resource.RLIMIT_STACK))
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1', '-I', str(ROOT)]
    report['flags'] = flags
    active_compilers = sorted(set(args.cxx + ([args.sanitize_cxx] if 'sanitize' in args.stages else [])))
    report['compiler_sha256'] = {cxx: sha(Path(shutil.which(cxx) or cxx).resolve().read_bytes())
                                 for cxx in active_compilers}
    report['compiler_versions'] = {cxx: subprocess.check_output([cxx, '--version'], text=True)
                                   for cxx in active_compilers}
    args.work.mkdir(parents=True, exist_ok=True)
    args.work = Path(tempfile.mkdtemp(prefix=args.output.stem + '-', dir=args.work))
    report['work'] = str(args.work)

    def execute(command, label, **kwargs):
        with report_lock:
            log = args.work / (f'{len(report["commands"]):04d}-' + label + '.log')
            row = dict(command=command, log=str(log))
            report['commands'].append(row)
            save()
        started = time.monotonic()
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock, log.open('wb') as output:
            fcntl.flock(lock, fcntl.LOCK_SH)
            completed = subprocess.run(command, cwd=ROOT, stdout=output, stderr=subprocess.STDOUT,
                                       timeout=args.timeout, **kwargs)
        with report_lock:
            row.update(returncode=completed.returncode, seconds=time.monotonic() - started,
                       log_sha256=sha(log.read_bytes()))
            save()
        if completed.returncode:
            raise RuntimeError((command, log.read_text()[-6000:]))
        return row

    with tempfile.TemporaryDirectory(prefix='fps-refactor-check-', dir=args.work) as directory:
        work = Path(directory)

        def compile_run(cxx, source, label, extra=(), sanitize=False):
            key = sha(json.dumps([cxx, list(extra), sanitize]).encode())[:12]
            executable = work / (label + '-' + key)
            command = [cxx, *flags, *extra]
            if sanitize:
                command += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
            command += [str(source), '-o', str(executable)]
            execute(command, label + '-compile')
            environment = os.environ.copy()
            if sanitize:
                environment['ASAN_OPTIONS'] = 'detect_leaks=0'
                environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
            result = execute([str(executable)], label + '-run', env=environment)
            return dict(compiler=cxx, source_sha256=sha(source.read_bytes()), command=command,
                        binary_sha256=sha(executable.read_bytes()), run_log_sha256=result['log_sha256'])

        if 'standalone' in args.stages:
            for cxx in args.cxx:
                for name in selected['headers']:
                    source = work / 'header.cpp'
                    source.write_text('#include "' + name + '"\n#include "' + name + '"\n'
                                      '#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)\n'
                                      '#error user_macro_dependency\n#endif\nint main() {}\n')
                    command = [cxx, *flags, '-DNDEBUG', '-fsyntax-only', str(source)]
                    execute(command, 'standalone')
                    report['standalone'].append(dict(compiler=cxx, header=name, command=command))
                    save()
                print('standalone', cxx, 'passed', flush=True)
        for stage in ['unit', 'sanitize']:
            if stage not in args.stages:
                continue
            compilers = args.cxx if stage == 'unit' else [args.sanitize_cxx]
            tasks = [(cxx, name) for cxx in compilers
                     for name in selected['unit' if stage == 'unit' else 'sanitize']]

            def unit_check(task):
                cxx, name = task
                source = ROOT / name
                row = compile_run(cxx, source, source.stem, sanitize=stage == 'sanitize')
                row.update(source=name, sanitized=stage == 'sanitize')
                return row

            with ThreadPoolExecutor(max_workers=args.jobs) as pool:
                futures = [pool.submit(unit_check, task) for task in tasks]
                for future in as_completed(futures):
                    row = future.result()
                    with report_lock:
                        report['unit'].append(row)
                        save()
                    print(stage, row['compiler'], row['source'], 'passed', flush=True)
        if 'integration' in args.stages:
            source = ROOT / 'test/unit/fps_public_headers.cpp'
            if args.acl:
                report['acl_commit'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.acl, text=True).strip()
            variants = [([], 'default'), (['-DCP_FPS_HEADERS_IO_FIRST'], 'io_first'),
                        (['-DCP_FPS_HEADERS_REVERSE'], 'reverse'),
                        (['-DCP_FPS_HEADERS_REVERSE', '-DCP_FPS_HEADERS_IO_FIRST'], 'reverse_io_first'),
                        (['-DCP_FPS_HEADERS_USER_MACROS', '-include', 'bits/stdc++.h'], 'user_macros'),
                        (['-U__linux__'], 'portable_dispatch')]
            if args.acl:
                variants.append((['-DCP_FPS_HEADERS_ACL', '-isystem', str(args.acl)], 'actual_acl_modint'))
            for cxx in args.cxx:
                for extra, label in variants:
                    row = compile_run(cxx, source, label, extra=extra)
                    row['variant'] = label
                    report['integration'].append(row)
                    save()
                    print('integration', cxx, label, 'passed', flush=True)
        if 'precision' in args.stages:
            source = ROOT / 'test/benchmark/fps_refactor_precision.cpp'
            row = compile_run(args.cxx[0], source, 'maximum_precision')
            report['precision_scope'] = 'Balanced convolution exercises an actual dense NTT of length 2^23; inverse/log/exp use sparse inputs, sqrt uses a quadratic input, and compose/reversion/projection use monomial inputs at their output limits.'
            report['precision'].append(row)
            save()
            print('precision', args.cxx[0], 'passed', flush=True)
        if 'official' in args.stages:
            report['generator_commit'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.problems, text=True).strip()
            for name in selected['verification']:
                source = ROOT / name
                url = re.search(r'#define PROBLEM "([^"]+)"', source.read_text())[1]
                if not url.startswith('https://judge.yosupo.jp/problem/'):
                    report['skipped_external'].append(dict(driver=name, url=url, reason='External judge; covered by unit tests only'))
                    save()
                    continue
                problem = url.rsplit('/', 1)[1]
                found = list(args.problems.glob('*/' + problem + '/info.toml'))
                assert len(found) == 1, (problem, found)
                folder = found[0].parent
                inputs = sorted((folder / 'in').glob('*.in'))
                assert inputs and (folder / 'checker').is_file(), 'Generate official cases first: ' + problem
                assets = dict(checker_sha256=sha((folder / 'checker').read_bytes()),
                    inputs={case.name: dict(input_sha256=sha(case.read_bytes()),
                        answer_sha256=sha((folder / 'out' / (case.stem + '.out')).read_bytes())) for case in inputs})
                if problem in report['cases']:
                    assert report['cases'][problem] == assets, 'Official assets changed: ' + problem
                report['cases'][problem] = assets
                for cxx in args.cxx:
                    executable = work / source.stem
                    command = [cxx, *flags, '-DEVAL', '-DONLINE_JUDGE', str(source), '-o', str(executable)]
                    execute(command, source.stem + '-official-compile')
                    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
                        fcntl.flock(lock, fcntl.LOCK_SH)
                        for case in inputs:
                            actual = work / 'actual.out'
                            with case.open('rb') as inp, actual.open('wb') as out:
                                subprocess.run([str(executable)], stdin=inp, stdout=out, cwd=ROOT,
                                               timeout=args.timeout, check=True)
                            expected = folder / 'out' / (case.stem + '.out')
                            checked = subprocess.run([str(folder / 'checker'), str(case), str(actual), str(expected)],
                                                     capture_output=True, text=True, timeout=args.timeout)
                            assert checked.returncode == 0, (name, cxx, case.name, checked.stdout, checked.stderr)
                    report['official'].append(dict(driver=name, problem=problem, compiler=cxx,
                        cases_passed=len(inputs), cases=[case.name for case in inputs], command=command,
                        binary_sha256=sha(executable.read_bytes())))
                    save()
                    print('official', name, cxx, len(inputs), 'passed', flush=True)
    for problem, assets in report['cases'].items():
        folder = next(args.problems.glob('*/' + problem + '/info.toml')).parent
        assert sha((folder / 'checker').read_bytes()) == assets['checker_sha256']
        assert sorted(case.name for case in (folder / 'in').glob('*.in')) == sorted(assets['inputs'])
        for name, expected in assets['inputs'].items():
            assert sha((folder / 'in' / name).read_bytes()) == expected['input_sha256']
            assert sha((folder / 'out' / (Path(name).stem + '.out')).read_bytes()) == expected['answer_sha256']
    report['official_assets_unchanged'] = True
    for name, expected in source_hashes.items():
        assert sha((ROOT / name).read_bytes()) == expected, 'Source changed while checking: ' + name
    assert report['script_sha256'] == sha(Path(__file__).read_bytes())
    for compiler, expected in report['compiler_sha256'].items():
        assert sha(Path(shutil.which(compiler) or compiler).resolve().read_bytes()) == expected
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    save()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='32f6930')
    parser.add_argument('--problems', type=Path)
    parser.add_argument('--acl', type=Path)
    parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
    parser.add_argument('--sanitize-cxx', default='clang++')
    parser.add_argument('--stages', nargs='+', choices=['unit', 'sanitize', 'standalone', 'integration', 'precision', 'official'],
                        default=['unit', 'sanitize', 'standalone', 'integration', 'official'])
    parser.add_argument('--timeout', type=int, default=180)
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-refactor-validation'))
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_refactor/validation.json')
    args = parser.parse_args()
    if not args.prepare_only and 'official' in args.stages and args.problems is None:
        parser.error('--problems is required for official checks')
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    args.work, args.output = args.work.resolve(), args.output.resolve()
    if args.problems:
        args.problems = args.problems.resolve()
    if args.acl:
        args.acl = args.acl.resolve()
    run(args)


if __name__ == '__main__':
    main()
