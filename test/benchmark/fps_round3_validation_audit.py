import argparse
import hashlib
import importlib.util
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EXPECTED_SELECTION = 'b2e965770fdc2e3709af7ff4cab6356f5b06e600997fc83d465a1e681bfc3e37'
BOSTAN_HEADER = 'components/fps_bostan.hpp'
ORIGINAL_BOSTAN = 'bbe4528ee226bcf0ea90d143f0b5832ef6f69b2162b448ec845d249349bc7be3'
CANONICAL_BOSTAN = 'bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_round3_validation')
    args = parser.parse_args()
    output = args.output.resolve()
    read = lambda name: json.loads((output / name).read_text())
    selection, contract, result = read('selection.json'), read('scope-contract.json'), read('result.json')
    assert sha(output / 'selection.json') == EXPECTED_SELECTION == contract['selection_sha256']
    assert all(selection[group] == names for group, names in contract['groups'].items())
    before, after = read('dependencies-before.json'), read('dependencies-after.json')
    assert before.keys() == after.keys()
    delta = {name: [before[name], after[name]] for name in before if before[name] != after[name]}
    assert result['dependency_hashes_match'] == (not delta)
    assert all(sha(ROOT / path) == digest for path, digest in after.items())
    normalization = None
    if delta:
        assert result['status'] == 'failed' and not result['plan_fingerprints_match']
        assert delta == {BOSTAN_HEADER: [ORIGINAL_BOSTAN, CANONICAL_BOSTAN]}
        preprocessing, codegen = read('docs_pragma_preprocessing.json'), read('docs_pragma_codegen.json')
        assert preprocessing['original']['file_sha256'] == ORIGINAL_BOSTAN
        assert preprocessing['candidate']['file_sha256'] == CANONICAL_BOSTAN
        assert preprocessing['normal_preprocessing_identical']
        assert (preprocessing['original']['normalized_preprocessed_sha256']
                == preprocessing['candidate']['normalized_preprocessed_sha256'])
        assert codegen['byte_identical_objects']
        assert codegen['original']['object_sha256'] == codegen['candidate']['object_sha256']
        canonical = (ROOT / BOSTAN_HEADER).read_bytes()
        directive = b'_Pragma("GCC target(\\"avx2\\")")'
        assert canonical.count(directive) == 1
        original = canonical.replace(directive, b'#pragma GCC target("avx2")', 1)
        assert hashlib.sha256(original).hexdigest() == ORIGINAL_BOSTAN
        spec = importlib.util.spec_from_file_location('fps_round3_audit_ci', ROOT / '.verify-helper/ci.py')
        ci = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(ci)
        baseline = read('baseline-plan.json')
        current = ci.build_plan(ROOT, previous=baseline['state'], environment=result['environment'])
        assert current['state'] == read('after-plan.json')['state']
        real_read_bytes = Path.read_bytes

        def initial_bytes(path):
            return original if path.resolve() == ROOT / BOSTAN_HEADER else real_read_bytes(path)

        try:
            Path.read_bytes = initial_bytes
            initial = ci.build_plan(ROOT, previous=baseline['state'], environment=result['environment'])
        finally:
            Path.read_bytes = real_read_bytes
        assert initial['state'] == selection['state']
        assert result['plan_fingerprints_match'] == (current['state'] == selection['state'])
        for group in ['headers', 'unit', 'verification', 'bundles']:
            assert current[group] == contract['groups'][group]
        normalization = dict(file=BOSTAN_HEADER, original_sha256=ORIGINAL_BOSTAN,
                             canonical_sha256=CANONICAL_BOSTAN, change='GCC target directive spelling only',
                             current_plan_matches_after=True, original_content_plan_matches_selection=True,
                             preprocessing_evidence_sha256=sha(output / 'docs_pragma_preprocessing.json'),
                             codegen_evidence_sha256=sha(output / 'docs_pragma_codegen.json'),
                             changed_plan_targets={group: [name for name, digest in current['state']['targets'][group].items()
                                                         if selection['state']['targets'][group].get(name) != digest]
                                                   for group in ci.GROUPS})
    else:
        assert result['status'] == 'passed' and result['plan_fingerprints_match']
    commands = result['commands']
    assert [row['name'] for row in commands] == ['gcc14', 'clang19', 'gcc14-sanitize']
    coverage = {}
    for row in commands:
        assert row['returncode'] == 0
        command = row['command']
        assert Path(command[command.index('--selection') + 1]).resolve() == output / 'selection.json'
        compiler = command[command.index('--cxx') + 1]
        if '--sanitize' in command:
            expected = [compiler + ': ' + Path(name).name for name in contract['groups']['sanitize']]
        else:
            expected = [compiler + ': standalone ' + name for name in contract['groups']['headers']]
            expected += [compiler + ': ' + Path(name).name for name in contract['groups']['unit']]
        path = output / row['log']
        assert sha(path) == row['log_sha256']
        lines = path.read_text().splitlines()
        observed = [line for line in lines if line.startswith(compiler + ': ')]
        assert observed == expected, row['name']
        assert lines[-1] == 'Verification passed', row['name']
        coverage[row['name']] = dict(expected=len(expected), observed=len(observed), exact_order=True)
    audit = dict(status='passed', selection_sha256=EXPECTED_SELECTION,
                 selection_groups_match_contract=True, full_log_coverage=coverage,
                 dependency_files=len(after), historical_runner_status=result['status'],
                 historical_dependency_hashes_match=result['dependency_hashes_match'],
                 historical_plan_fingerprints_match=result['plan_fingerprints_match'],
                 current_dependencies_match=True, allowed_normalization=normalization,
                 contract_sha256=sha(output / 'scope-contract.json'),
                 result_sha256=sha(output / 'result.json'), harness_sha256=sha(Path(__file__)))
    (output / 'scope-audit.json').write_text(json.dumps(audit, indent=2) + '\n')
    print(json.dumps(audit, indent=2))


if __name__ == '__main__':
    main()
