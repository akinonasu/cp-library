import hashlib
import importlib.util
import json
from pathlib import Path
import shlex


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'test/benchmark/results/fps_round4_constant_validation'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    read = lambda name: json.loads((OUT / name).read_text())
    result, audit, full, core = read('result.json'), read('scope-audit.json'), read('selection-full.json'), read('selection-core.json')
    assert result['status'] == audit['status'] == 'passed'
    assert sha(OUT / 'result.json') == audit['result_sha256']
    assert sha(ROOT / 'test/benchmark/fps_round4_constant_validation.py') == result['harness_sha256']
    assert sha(ROOT / 'test/benchmark/fps_round4_validation.py') == result['shared_helper_sha256']
    assert sha(OUT / 'selection-full.json') == result['selection_full_sha256']
    assert sha(OUT / 'selection-core.json') == result['selection_core_sha256']
    assert sha(OUT / 'scope-contract.json') == result['scope_contract_sha256']
    assert all(result['gates'].values())
    before, after = read('dependencies-before.json'), read('dependencies-after.json')
    assert before == after and all(sha(ROOT / name) == digest for name, digest in after.items())
    assert len(result['dependency_commands']) == 14
    for row in result['dependency_commands']:
        assert row['returncode'] == 0
        paths = set()
        for token in shlex.split(row['stdout'].replace('\\\n', ' ').partition(':')[2]):
            path = Path(token)
            path = path.resolve() if path.is_absolute() else (ROOT / path).resolve()
            if path.is_relative_to(ROOT):
                paths.add(path.relative_to(ROOT).as_posix())
        assert row['dependency_hashes'] == {name: before[name] for name in sorted(paths)}
    assert {row['mode'] for row in result['reused']} == {'normal', 'sanitize'}
    for row in result['reused']:
        assert row['target'] == 'test/unit/fps_convolution_constant.cpp' and row['compiler'] == 'gcc14'
        assert row['successful_compile_and_run'] and row['current_dependency_hashes_match']
        assert all(sha(Path(name)) == digest for name, digest in row['bound_files'].items())
    coverage = {}
    for row in result['commands']:
        assert row['returncode'] == 0
        command = row['command']
        compiler = command[command.index('--cxx') + 1]
        selection_path = Path(command[command.index('--selection') + 1])
        assert sha(selection_path) == row['execution_selection_sha256']
        selected = json.loads(selection_path.read_text())
        expected_selection = dict(core)
        if row['name'] == 'gcc14':
            expected_selection['unit'] = ['test/unit/fps.cpp']
        elif row['name'] == 'gcc14-sanitize':
            expected_selection.update(headers=[], unit=['test/unit/fps.cpp'], sanitize=['test/unit/fps.cpp'])
        else:
            assert row['name'] == 'clang19'
        assert selected == expected_selection
        log = OUT / row['log']
        assert sha(log) == row['log_sha256']
        expected = [] if '--sanitize' in command else [compiler + ': standalone ' + name for name in selected['headers']]
        units = selected['sanitize'] if '--sanitize' in command else selected['unit']
        expected += [compiler + ': ' + Path(name).name for name in units]
        lines = log.read_text().splitlines()
        assert [line for line in lines if line.startswith(compiler + ': ')] == expected
        assert lines[-1] == 'Verification passed' and result['coverage'][row['name']]['passed']
        coverage[row['name']] = result['coverage'][row['name']]
    previous = json.loads((Path(result['previous']) / 'selection-final.json').read_text())
    spec = importlib.util.spec_from_file_location('constant_audit_ci', ROOT / '.verify-helper/ci.py')
    ci = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ci)
    current = ci.build_plan(ROOT, previous=previous['state'], environment=result['environment'])
    assert current == read('after-plan.json') and current['state'] == full['state']
    final = dict(status='passed', source_and_actual_dependencies_unchanged=True,
                 actual_dependency_scans=14, dependency_files=len(after), reuse_bindings_verified=2,
                 execution_selections_match_core=True, full_log_coverage=coverage,
                 full_selection_sha256=sha(OUT / 'selection-full.json'),
                 core_selection_sha256=sha(OUT / 'selection-core.json'),
                 result_sha256=sha(OUT / 'result.json'), initial_audit_sha256=sha(OUT / 'scope-audit.json'),
                 harness_sha256=sha(Path(__file__)))
    (OUT / 'scope-final-audit.json').write_text(json.dumps(final, indent=2, sort_keys=True) + '\n')
    print(json.dumps(final, indent=2, sort_keys=True))


if __name__ == '__main__':
    main()
