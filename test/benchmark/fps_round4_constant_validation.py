import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile
import time

import fps_round4_validation as shared


ROOT = Path(__file__).resolve().parents[2]
HEADERS = ['components/fps_convolution.hpp', 'components/fps_convolution_constant.hpp',
           'convolution/convolution.hpp', 'fps/fps.hpp']
UNIT = 'test/unit/fps_convolution_constant.cpp'
UNITS = ['test/unit/fps.cpp', UNIT]
sha, write_json = shared.sha, shared.write_json


def dependency_paths(stdout, root):
    result = set()
    for token in shlex.split(stdout.replace('\\\n', ' ').partition(':')[2]):
        path = Path(token)
        path = path.resolve() if path.is_absolute() else (root / path).resolve()
        if path.is_relative_to(root):
            result.add(path.relative_to(root).as_posix())
    return result


def reuse_check(path, mode, current, compiler, flags, run_environment):
    evidence = json.loads(path.read_text())
    row = next(item for item in evidence['checks'] if item['name'] == mode)
    assert row['passed'] and row['compile_returncode'] == row['run_returncode'] == 0
    assert row['source_sha256'] == evidence['unit_sha256'] == sha(ROOT / UNIT)
    assert row['compiler']['sha256'] == compiler['sha256']
    resolved = Path(compiler['resolved_path'])
    assert sha(resolved) == row['compiler']['sha256']
    version = subprocess.check_output([str(resolved), '--version'], text=True)
    assert version == row['compiler']['version']
    normalized = [str(ROOT) if arg == row['project_root'] else arg for arg in row['flags']]
    assert normalized == flags
    assert row['command'][1:] == row['flags'] + [row['source_path'], '-o', row['command'][-1]]
    assert all(row['run_env'][key] == run_environment[key] for key in ['ASAN_OPTIONS', 'UBSAN_OPTIONS'])
    bound = {str(path): sha(path)}
    for stage in ['dependencies_before', 'dependencies_after']:
        record = row[stage]
        depfile = path.parent / record['raw_dependency_file']
        assert sha(depfile) == record['raw_dependency_sha256']
        bound[str(depfile)] = sha(depfile)
        mapped = {UNIT if item['source'] else item['tree_relative']: item['sha256'] for item in record['files']}
        assert mapped == current
        raw_paths = shlex.split(depfile.read_text().replace('\\\n', ' ').partition(':')[2])
        assert {str(Path(name).resolve()) for name in raw_paths} == {item['path'] for item in record['files']}
        for item in record['files']:
            source = Path(item['path'])
            assert sha(source) == item['sha256']
            bound[str(source)] = item['sha256']
    assert {UNIT if name == '@unit' else name: digest for name, digest in row['dependency_hashes'].items()} == current
    log = path.parent / row['log_path']
    assert sha(log) == row['log_sha256']
    bound[str(log)] = sha(log)
    binary = Path(row['command'][-1])
    assert sha(binary) == row['binary_sha256']
    bound[str(binary)] = sha(binary)
    return dict(target=UNIT, mode=mode, compiler='gcc14', evidence=str(path), evidence_sha256=sha(path),
                dependency_files=len(current), flags_match_after_project_root_mapping=True,
                current_dependency_hashes_match=True, compiler_sha256=compiler['sha256'],
                resolved_compiler_version=version, successful_compile_and_run=True, bound_files=bound)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--previous', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_validation')
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_constant_validation')
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-round4/constant-validation'))
    parser.add_argument('--reuse-evidence', type=Path, default=ROOT / 'test/benchmark/results/fps_convolution_constant_round4/private_validation/reuse_checks.json')
    parser.add_argument('--execute', action='store_true')
    args = parser.parse_args()
    args.previous, args.output, args.work = args.previous.resolve(), args.output.resolve(), args.work.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    args.work.mkdir(parents=True, exist_ok=True)
    assert not (args.output / 'result.json').exists(), 'Use a new output directory for a new execution'
    previous = json.loads((args.previous / 'selection-final.json').read_text())
    previous_audit = json.loads((args.previous / 'scope-final-audit.json').read_text())
    previous_result = json.loads((args.previous / 'result.json').read_text())
    assert previous_audit['status'] == 'passed'
    assert previous_audit['final_selection_sha256'] == sha(args.previous / 'selection-final.json')
    assert previous_audit['historical_runner_result_sha256'] == sha(args.previous / 'result.json')
    old = json.loads((args.previous / 'dependencies-after.json').read_text())
    preserved = {name: digest for name, digest in old.items() if name != 'components/fps_convolution.hpp'}
    assert all(sha(ROOT / name) == digest for name, digest in preserved.items())
    convolution = (ROOT / 'components/fps_convolution.hpp').read_text()
    include = '#include "fps_convolution_constant.hpp"\n'
    dispatch = r'\tif \(constant::fits\(a, b\)\)(?: \[\[unlikely\]\])? return constant::multiply\(a, b\);\n'
    assert convolution.count(include) == 1 and len(re.findall(dispatch, convolution)) == 1
    restored = re.sub(dispatch, '', convolution.replace(include, ''))
    assert hashlib.sha256(restored.encode()).hexdigest() == old['components/fps_convolution.hpp']

    compiler_info = previous_result['compilers']
    compilers = {name: Path(row['path']) for name, row in compiler_info.items()}
    assert all(sha(Path(row['resolved_path'])) == row['sha256'] for row in compiler_info.values())
    flags, sanitize_flags = previous_result['flags_units'], previous_result['flags_sanitize']
    environment = previous_result['environment']
    ci = shared.load_ci(ROOT)
    full = ci.build_plan(ROOT, previous=previous['state'], environment=environment)
    assert set(HEADERS) <= set(full['headers']) and set(UNITS) <= set(full['unit'])
    core = dict(headers=HEADERS, unit=UNITS, sanitize=UNITS, verification=[], bundles=[], state=full['state'],
                reason='New constant helper, both public gateways, and direct plus FPS integration tests; the previous complete frozen snapshot is preserved separately.')
    write_json(args.output / 'selection-full.json', full)
    write_json(args.output / 'selection-core.json', core)
    write_json(args.output / 'previous-selection.json', previous)
    result = dict(status='prepared', started=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  previous=str(args.previous), previous_audit_sha256=sha(args.previous / 'scope-final-audit.json'),
                  previous_result_sha256=sha(args.previous / 'result.json'), previous_state=previous['state'],
                  compilers=compiler_info, flags_headers=flags + ['-Werror', '-fsyntax-only'],
                  flags_units=flags, flags_sanitize=sanitize_flags, environment=environment,
                  harness_sha256=sha(Path(__file__)), shared_helper_sha256=sha(ROOT / 'test/benchmark/fps_round4_validation.py'),
                  selection_full_sha256=sha(args.output / 'selection-full.json'),
                  selection_core_sha256=sha(args.output / 'selection-core.json'),
                  full_counts={group: len(full[group]) for group in ci.GROUPS},
                  core_counts={group: len(core[group]) for group in ci.GROUPS},
                  unchanged_previous_dependency_files=len(preserved), prior_convolution_restored_exactly=True,
                  scope_reason=core['reason'], dependency_commands=[], commands=[], reused=[], work=str(args.work))
    contract = {key: result[key] for key in ['previous_audit_sha256', 'previous_result_sha256',
                'selection_full_sha256', 'selection_core_sha256', 'full_counts', 'core_counts', 'scope_reason']}
    write_json(args.output / 'scope-contract.json', contract)
    result['scope_contract_sha256'] = sha(args.output / 'scope-contract.json')
    write_json(args.output / 'conditions.json', result)
    print('selected', result['full_counts'], 'core', result['core_counts'], flush=True)
    if not args.execute:
        return

    env = os.environ | dict(TMPDIR=str(args.work), ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
    result['runtime_environment'] = {key: env[key] for key in ['TMPDIR', 'ASAN_OPTIONS', 'UBSAN_OPTIONS']}
    controls = {'.verify-helper/ci.py', 'test/verify.py', 'test/benchmark/fps_round4_validation.py',
                str(Path(__file__).relative_to(ROOT))}
    controls.update(str(path.relative_to(ROOT)) for path in (ROOT / '.github/workflows').glob('*.y*ml'))
    dependencies = set(controls)
    before_all = shared.repository_hashes()
    def checkpoint():
        write_json(args.output / 'result.json', result)
    result['status'] = 'running'
    checkpoint()
    with tempfile.TemporaryDirectory(prefix='constant-deps-', dir=args.work) as folder:
        scans = [(name, compiler, 'normal', HEADERS + UNITS, flags) for name, compiler in compilers.items()]
        scans.append(('gcc14', compilers['gcc14'], 'sanitize', UNITS, sanitize_flags))
        for name, compiler, mode, targets, scan_flags in scans:
            with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
                fcntl.flock(lock, fcntl.LOCK_SH)
                for target in targets:
                    source = ROOT / target
                    if target.endswith('.hpp'):
                        source = Path(folder) / 'header.cpp'
                        source.write_text('#include "' + target + '"\n#include "' + target + '"\nint main() {}\n')
                    command = [str(compiler), *scan_flags, '-MM', str(source)]
                    done = subprocess.run(command, cwd=ROOT, env=env, text=True, capture_output=True)
                    assert done.returncode == 0, done.stderr
                    names = dependency_paths(done.stdout, ROOT)
                    hashes = {path: before_all[path] for path in sorted(names)}
                    dependencies.update(names)
                    result['dependency_commands'].append(dict(compiler=name, mode=mode, target=target, command=command,
                        returncode=done.returncode, stdout=done.stdout, stderr=done.stderr, dependency_hashes=hashes))
            print('dependencies', name, mode, flush=True)
    before = {name: before_all[name] for name in sorted(dependencies)}
    write_json(args.output / 'dependencies-before.json', before)
    assert all(sha(ROOT / name) == digest for name, digest in before.items())
    for mode, expected_flags in [('normal', flags), ('sanitize', sanitize_flags)]:
        scan = next(row for row in result['dependency_commands'] if row['compiler'] == 'gcc14' and row['mode'] == mode and row['target'] == UNIT)
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            result['reused'].append(reuse_check(args.reuse_evidence, mode, scan['dependency_hashes'], compiler_info['gcc14'], expected_flags, env))
    write_json(args.output / 'reuse-assessment.json', dict(eligible=True, checks=result['reused']))
    checkpoint()
    executions = []
    for name, compiler in compilers.items():
        execution = dict(core)
        execution['unit'] = [target for target in UNITS if not (name == 'gcc14' and target == UNIT)]
        executions.append((name, compiler, False, execution))
    execution = dict(core, headers=[], unit=['test/unit/fps.cpp'], sanitize=['test/unit/fps.cpp'])
    executions.append(('gcc14-sanitize', compilers['gcc14'], True, execution))
    success = True
    coverage = {}
    for name, compiler, sanitize, execution in executions:
        selection_path = args.output / ('selection-execute-' + name + '.json')
        write_json(selection_path, execution)
        command = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx', str(compiler)]
        if sanitize:
            command.append('--sanitize')
        command += ['--selection', str(selection_path)]
        log = args.output / (name + '.log')
        row = dict(name=name, command=command, log=log.name, execution_selection_sha256=sha(selection_path))
        result['commands'].append(row)
        checkpoint()
        start = time.monotonic()
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock, log.open('w') as stream:
            fcntl.flock(lock, fcntl.LOCK_SH)
            done = subprocess.run(command, cwd=ROOT, env=env, stdout=stream, stderr=subprocess.STDOUT)
        row.update(returncode=done.returncode, seconds=time.monotonic() - start, log_sha256=sha(log))
        expected = [] if sanitize else [str(compiler) + ': standalone ' + target for target in execution['headers']]
        targets = execution['sanitize'] if sanitize else execution['unit']
        expected += [str(compiler) + ': ' + Path(target).name for target in targets]
        lines = log.read_text().splitlines()
        observed = [line for line in lines if line.startswith(str(compiler) + ': ')]
        passed = done.returncode == 0 and observed == expected and lines[-1] == 'Verification passed'
        reused = int(name in ['gcc14', 'gcc14-sanitize'])
        coverage[name] = dict(executed=len(expected), observed=len(observed), exact_order=observed == expected,
                              reused=reused, logical_total=len(expected) + reused, passed=passed)
        success = success and passed
        checkpoint()
        print(name, done.returncode, row['seconds'], flush=True)

    after = {name: sha(ROOT / name) for name in before}
    write_json(args.output / 'dependencies-after.json', after)
    plan_after = ci.build_plan(ROOT, previous=previous['state'], environment=environment)
    write_json(args.output / 'after-plan.json', plan_after)
    gates = dict(dependencies_unchanged=before == after, full_plan_fingerprints_unchanged=plan_after['state'] == full['state'],
                 full_selection_unchanged=sha(args.output / 'selection-full.json') == result['selection_full_sha256'],
                 core_selection_unchanged=sha(args.output / 'selection-core.json') == result['selection_core_sha256'],
                 scope_contract_unchanged=sha(args.output / 'scope-contract.json') == result['scope_contract_sha256'],
                 previous_record_unchanged=sha(args.previous / 'scope-final-audit.json') == result['previous_audit_sha256'] and sha(args.previous / 'result.json') == result['previous_result_sha256'],
                 reused_evidence_unchanged=all(sha(Path(path)) == digest for row in result['reused'] for path, digest in row['bound_files'].items()),
                 compiler_hashes_unchanged=all(sha(Path(row['resolved_path'])) == row['sha256'] for row in compiler_info.values()))
    result.update(gates=gates, coverage=coverage, status='passed' if success and all(gates.values()) else 'failed',
                  finished=datetime.datetime.now(datetime.timezone.utc).isoformat())
    checkpoint()
    audit = dict(status=result['status'], previous_audit_sha256=result['previous_audit_sha256'],
                 selection_full_sha256=result['selection_full_sha256'], selection_core_sha256=result['selection_core_sha256'],
                 core_counts=result['core_counts'], full_counts=result['full_counts'], gates=gates, coverage=coverage,
                 dependency_files=len(before), actual_dependency_scans=len(result['dependency_commands']),
                 reused_checks=len(result['reused']), result_sha256=sha(args.output / 'result.json'), harness_sha256=sha(Path(__file__)))
    write_json(args.output / 'scope-audit.json', audit)
    print(result['status'], flush=True)
    if result['status'] != 'passed':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
