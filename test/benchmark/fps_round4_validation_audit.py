import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shlex


ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'bd29f70c70e24e27770b9e9a3777b6cf85664b63'
ARTIFACT = 'test/benchmark/results/fps_convolution_allocation_round4/sources/convolution.test.cpp'
ARTIFACT_SHA = '51e8bb757f35275e6632f3ea4f521f9959aea7e1baa5ad4895b5e3294fc1c043'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round4_validation')
    args = parser.parse_args()
    out = args.output.resolve()
    read = lambda name: json.loads((out / name).read_text())
    original, final = read('selection.json'), read('selection-final.json')
    refinement, contract, result = read('scope-refinement.json'), read('scope-contract.json'), read('result.json')
    assert result['baseline'] == BASELINE == refinement['baseline'] == contract['baseline']
    assert sha(out / 'selection.json') == result['selection_sha256'] == contract['selection_sha256']
    assert sha(out / 'scope-contract.json') == result['scope_contract_sha256']
    assert sha(out / 'selection.json') == refinement['original_selection_sha256']
    assert sha(out / 'selection-final.json') == refinement['final_selection_sha256']
    assert refinement['old_artifact'] == ARTIFACT and refinement['new_artifact'] == ARTIFACT + '.txt'
    assert not (ROOT / ARTIFACT).exists()
    assert sha(ROOT / (ARTIFACT + '.txt')) == sha(ROOT / 'test/convolution.test.cpp') == ARTIFACT_SHA
    assert refinement['artifact_sha256'] == ARTIFACT_SHA
    assert all(original[group] == names for group, names in contract['groups'].items())
    for group in original['state']['targets']:
        if group == 'verification':
            assert final[group] == [name for name in original[group] if name != ARTIFACT]
            assert final['state']['targets'][group] == {
                name: digest for name, digest in original['state']['targets'][group].items() if name != ARTIFACT}
        else:
            assert final[group] == original[group]
            assert final['state']['targets'][group] == original['state']['targets'][group]

    assert result['status'] == 'failed' and result['plan_fingerprints_match'] is False
    gates = ['dependency_scan_hashes_match', 'dependency_hashes_match', 'selection_hash_matches',
             'contract_hash_matches', 'compiler_hashes_match']
    assert all(result[name] is True for name in gates)
    before, after = read('dependencies-before.json'), read('dependencies-after.json')
    assert before == after and all(sha(ROOT / name) == digest for name, digest in after.items())
    assert sha(ROOT / 'test/benchmark/fps_round4_validation.py') == result['harness_sha256']
    for name, row in result['compilers'].items():
        assert sha(Path(row['path']).resolve()) == row['sha256'], name

    spec = importlib.util.spec_from_file_location('round4_audit_ci', ROOT / '.verify-helper/ci.py')
    ci = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ci)
    baseline = read('baseline-plan.json')
    current = ci.build_plan(ROOT, previous=baseline['state'], environment=result['environment'])
    current['sanitize'] = final['sanitize']
    assert current['state'] == final['state'] == read('after-plan.json')['state']
    assert all(current[group] == final[group] for group in ci.GROUPS)

    scans = result['dependency_commands']
    expected_scans = [(compiler, 'normal', target) for compiler in ['gcc14', 'clang19']
                      for target in sorted(set(original['headers'] + original['unit']))]
    expected_scans += [('gcc14', 'sanitize', target) for target in original['sanitize']]
    assert [(row['compiler'], row['mode'], row['target']) for row in scans] == expected_scans
    for row in scans:
        assert row['returncode'] == 0
        paths = set()
        for token in shlex.split(row['stdout'].replace('\\\n', ' ').partition(':')[2]):
            path = Path(token)
            path = path.resolve() if path.is_absolute() else (ROOT / path).resolve()
            if path.is_relative_to(ROOT):
                paths.add(path.relative_to(ROOT).as_posix())
        assert row['dependency_hashes'] == {name: before[name] for name in sorted(paths)}

    commands = result['commands']
    assert [row['name'] for row in commands] == ['gcc14', 'clang19', 'gcc14-sanitize']
    coverage = {}
    for row in commands:
        assert row['returncode'] == 0
        command = row['command']
        assert Path(command[command.index('--selection') + 1]).resolve() == out / 'selection.json'
        compiler = command[command.index('--cxx') + 1]
        if '--sanitize' in command:
            expected = [compiler + ': ' + Path(name).name for name in original['sanitize']]
        else:
            expected = [compiler + ': standalone ' + name for name in original['headers']]
            expected += [compiler + ': ' + Path(name).name for name in original['unit']]
        path = out / row['log']
        assert sha(path) == row['log_sha256']
        lines = path.read_text().splitlines()
        observed = [line for line in lines if line.startswith(compiler + ': ')]
        assert observed == expected and lines[-1] == 'Verification passed'
        assert all(result['coverage'][row['name']][key] for key in
                   ['exact_order', 'verification_passed', 'log_sha256_matches'])
        coverage[row['name']] = dict(expected=len(expected), observed=len(observed), exact_order=True)

    audit = dict(status='passed', baseline=BASELINE, historical_runner_status=result['status'],
                 historical_runner_result_sha256=sha(out / 'result.json'),
                 original_selection_sha256=sha(out / 'selection.json'),
                 final_selection_sha256=sha(out / 'selection-final.json'),
                 scope_refinement_sha256=sha(out / 'scope-refinement.json'),
                 refinement='Only an archived byte-identical driver copy was renamed out of CI discovery; production and all executed core target fingerprints are unchanged.',
                 final_selected_counts={group: len(final[group]) for group in ci.GROUPS},
                 dependency_files=len(after), actual_dependency_scans=len(scans),
                 actual_dependencies_unchanged=True, current_source_hashes_match=True,
                 core_target_fingerprints_unchanged=True, current_plan_matches_final=True,
                 full_log_coverage=coverage, all_compile_and_test_commands_passed=True,
                 normalization_exceptions=[], harness_sha256=sha(Path(__file__)))
    (out / 'scope-final-audit.json').write_text(json.dumps(audit, indent=2, sort_keys=True) + '\n')
    print(json.dumps(audit, indent=2, sort_keys=True))


if __name__ == '__main__':
    main()
