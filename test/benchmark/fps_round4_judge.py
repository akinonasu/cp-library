import argparse
from concurrent.futures import ThreadPoolExecutor
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shlex
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
FLAGS = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-DEVAL', '-DONLINE_JUDGE', '-I', str(ROOT)]
LOCK = '/tmp/cp-fps-tuning.lock'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def source_guard(selection):
    paths = {ROOT / name for name in selection['verification']}
    for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'math', 'string']:
        paths.update((ROOT / folder).rglob('*.hpp'))
    paths.update(ROOT.glob('*.hpp'))
    paths.update((ROOT / 'test').glob('*.hpp'))
    paths.update([Path(__file__), ROOT / 'test/benchmark/fps_speed_verify.py'])
    return {str(path.relative_to(ROOT)): sha(path) for path in sorted(paths)}


def official_guard(official, folders, problems):
    paths = {official / 'generate.py', official / 'common/testlib.h'}
    for problem in problems:
        folder = folders[problem]
        paths.update(folder / name for name in ['info.toml', 'checker', 'checker.cpp'] if (folder / name).is_file())
        paths.update((folder / 'in').glob('*.in'))
        paths.update((folder / 'out').glob('*.out'))
    return {'commit': subprocess.check_output(['git', '-C', str(official), 'rev-parse', 'HEAD'], text=True).strip(),
            'files': {str(path.relative_to(official)): sha(path) for path in sorted(paths)}}


def generator_stack():
    _, hard = resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--selection', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_validation/selection-final.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_validation')
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-round4/validation/official'))
    parser.add_argument('--execute', action='store_true')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    args.work.mkdir(parents=True, exist_ok=True)
    args.selection = args.selection.resolve()
    official = args.official_root.resolve()
    selection = json.loads(args.selection.read_text())
    names = selection['verification']
    assert len(names) == len(set(names))
    folders = {path.parent.name: path.parent for path in official.glob('*/*/info.toml')}
    urls = {name: re.search(r'#define PROBLEM "([^"]+)"', (ROOT / name).read_text())[1] for name in names}
    problems = {name: url.rsplit('/', 1)[1] for name, url in urls.items() if url.startswith('https://judge.yosupo.jp/problem/')}
    skips = {name: {'url': url, 'reason': 'Yukicoder token unavailable; skip as instructed'} for name, url in urls.items() if name not in problems}
    assert all(row['url'].startswith('https://yukicoder.me/problems/no/') for row in skips.values())
    plan = {'selection_sha256': sha(args.selection), 'baseline': 'bd29f70c70e24e27770b9e9a3777b6cf85664b63',
            'verification': names, 'official': problems, 'external_skips': skips,
            'parts': {str(i): names[i::3] for i in range(3)},
            'checker_order': ['input', 'actual_output', 'official_answer'], 'all_official_cases': True,
            'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
            'compiler_sha256': sha(Path('/usr/bin/g++').resolve()), 'flags': FLAGS,
            'harness_sha256': sha(Path(__file__)), 'runner_sha256': sha(ROOT / 'test/benchmark/fps_speed_verify.py'),
            'work': str(args.work), 'official_root': str(official), 'lock': LOCK,
            'shared_lock_scope': 'one dependency scan and one complete driver compile/check; released between drivers',
            'status': 'prepared', 'started': datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'generation_stack_limit_bytes': resource.getrlimit(resource.RLIMIT_STACK)[1],
            'driver_stack_limit_bytes': resource.getrlimit(resource.RLIMIT_STACK)[0],
            'generation': [], 'dependencies': {}, 'tests': {}, 'skipped': {}}
    for i in range(3): save(args.output / f'judge_selection_part{i}.json', {'verification': names[i::3]})
    save(args.output / 'judge_plan.json', plan)
    print('prepared', len(names), 'drivers,', len(problems), 'Yosupo,', len(skips), 'Yukicoder skips', flush=True)
    if not args.execute:
        return
    before = source_guard(selection)
    save(args.output / 'judge_sources_before.json', {'sources': before, 'selection_sha256': plan['selection_sha256']})
    env = os.environ.copy()
    env['TMPDIR'] = str(args.work)
    for problem in sorted(set(problems.values())):
        folder = folders[problem]
        inputs = list((folder / 'in').glob('*.in'))
        if inputs and (folder / 'checker').is_file() and all((folder / 'out' / (path.stem + '.out')).is_file() for path in inputs):
            continue
        command = [sys.executable, str(official / 'generate.py'), str(folder / 'info.toml')]
        log_path = args.output / ('judge_generate_' + problem + '.log')
        print('generate', problem, flush=True)
        start = time.monotonic()
        with open(LOCK, 'w') as lock, log_path.open('w') as log:
            fcntl.flock(lock, fcntl.LOCK_SH)
            completed = subprocess.run(command, env=env, cwd=official, stdout=log, stderr=subprocess.STDOUT, preexec_fn=generator_stack)
        plan['generation'].append({'problem': problem, 'command': command, 'seconds': time.monotonic() - start,
                                   'returncode': completed.returncode, 'log_sha256': sha(log_path)})
        save(args.output / 'judge_plan.json', plan)
        completed.check_returncode()
    with open(LOCK, 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        official_before = official_guard(official, folders, set(problems.values()))
    save(args.output / 'judge_official_before.json', official_before)
    assert before == source_guard(selection), 'Source changed during official generation'
    plan['status'] = 'running'
    save(args.output / 'judge_plan.json', plan)

    def driver(index):
        name = names[index]
        if name in skips:
            return name, None, skips[name], None
        single = args.output / f'judge_selection_driver{index:02}.json'
        save(single, {'verification': [name]})
        output = args.output / f'judge_driver{index:02}.json'
        log_path = args.output / f'judge_driver{index:02}.log'
        dep_command = ['g++', *FLAGS, '-MM', str(ROOT / name)]
        command = [sys.executable, str(ROOT / 'test/benchmark/fps_speed_verify.py'), '--official-root', str(official),
                   '--selection', str(single), '--output', str(output)]
        start = time.monotonic()
        with open(LOCK, 'w') as lock, log_path.open('w') as log:
            fcntl.flock(lock, fcntl.LOCK_SH)
            dependencies = subprocess.run(dep_command, cwd=ROOT, env=env, text=True, capture_output=True)
            log.write('dependency command: ' + shlex.join(dep_command) + '\n' + dependencies.stdout + dependencies.stderr)
            dependencies.check_returncode()
            paths = []
            for token in shlex.split(dependencies.stdout.replace('\\\n', ' ').partition(':')[2]):
                path = Path(token).resolve()
                if path.is_relative_to(ROOT): paths.append(path.relative_to(ROOT).as_posix())
            hashes = {path: sha(ROOT / path) for path in sorted(set(paths))}
            completed = subprocess.run(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
            returncode = completed.returncode
        row = {'command': command, 'dependency_command': dep_command, 'dependencies': hashes,
               'seconds': time.monotonic() - start, 'returncode': returncode, 'log_sha256': sha(log_path),
               'output_sha256': sha(output) if output.is_file() else None}
        completed.check_returncode()
        data = json.loads(output.read_text())
        assert data['mode'] == 'judge' and data['case_limit'] is None and data['flags'] == FLAGS
        assert data['compiler'] == plan['compiler'] and data['official_commit'] == official_before['commit']
        assert list(data['tests']) == [name] and not data['skipped']
        test = data['tests'][name]
        assert test['source_sha256'] == before[name]
        expected = sorted(path.name for path in (folders[problems[name]] / 'in').glob('*.in'))
        assert test['cases'] == expected and expected
        assert all(before[path] == digest for path, digest in hashes.items())
        print(name, len(expected), 'passed', flush=True)
        return name, test, None, row

    with ThreadPoolExecutor(max_workers=3) as pool:
        for first in range(0, len(names), 3):
            rows = list(pool.map(driver, range(first, min(first + 3, len(names)))))
            for name, test, skip, audit in rows:
                if skip is not None: plan['skipped'][name] = skip
                else: plan['tests'][name], plan['dependencies'][name] = test, audit
            save(args.output / 'judge_plan.json', plan)
    with open(LOCK, 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        official_after = official_guard(official, folders, set(problems.values()))
    after = source_guard(selection)
    save(args.output / 'judge_sources_after.json', {'sources': after})
    save(args.output / 'judge_official_after.json', official_after)
    assert before == after, 'Sources changed during verification'
    assert official_before == official_after, 'Official checker/data changed during verification'
    assert sha(args.selection) == plan['selection_sha256']
    assert sha(Path('/usr/bin/g++').resolve()) == plan['compiler_sha256']
    assert set(plan['tests']) == set(problems) and set(plan['skipped']) == set(skips)
    for part in range(3):
        selected = names[part::3]
        save(args.output / f'judge_part{part}.json', {'verification': selected,
             'tests': {name: plan['tests'][name] for name in selected if name in plan['tests']},
             'skipped': {name: plan['skipped'][name] for name in selected if name in plan['skipped']},
             'selection_sha256': plan['selection_sha256']})
        logs = [args.output / f'judge_driver{i:02}.log' for i in range(part, len(names), 3) if names[i] in problems]
        (args.output / f'judge_part{part}.log').write_text(''.join(path.read_text() for path in logs))
    audit = {'selection_sha256': plan['selection_sha256'], 'baseline': plan['baseline'],
             'selected_drivers': len(names), 'passed_drivers': len(plan['tests']),
             'passed_cases': sum(len(row['cases']) for row in plan['tests'].values()),
             'skipped_drivers': len(skips), 'skip_reasons': skips,
             'executed_drivers': len(plan['dependencies']), 'coverage_matches_final_selection': True,
             'full_official_cases': True, 'source_hashes_unchanged': True, 'official_hashes_unchanged': True,
             'actual_dependencies_unchanged': True, 'source_files_checked': len(before),
             'official_files_checked': len(official_before['files']), 'checker_order': plan['checker_order'],
             'compiler': plan['compiler'], 'compiler_sha256': plan['compiler_sha256'], 'flags': FLAGS,
             'harness_sha256': sha(Path(__file__)), 'runner_sha256': plan['runner_sha256']}
    plan['status'] = 'passed'
    plan['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save(args.output / 'judge_plan.json', plan)
    save(args.output / 'judge.json', {'tests': plan['tests'], 'skipped': plan['skipped'], 'audit': audit})
    save(args.output / 'judge_audit.json', audit)
    manifest = {path.name: sha(path) for path in sorted(args.output.glob('judge*')) if path.is_file() and path.name != 'judge_manifest.json'}
    save(args.output / 'judge_manifest.json', manifest)
    print(json.dumps(audit, indent=2), flush=True)


if __name__ == '__main__':
    main()
