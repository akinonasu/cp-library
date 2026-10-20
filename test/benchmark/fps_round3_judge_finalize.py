import hashlib
import json
import subprocess
from pathlib import Path
import tomllib

ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / 'test/benchmark/results/fps_round3_validation'
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
read = lambda name: json.loads((HERE / name).read_text())
plan, audit = read('judge_plan.json'), read('judge_audit.json')
assert plan['status'] == 'passed'
selection = read('selection.json')
assert sha(HERE / 'selection.json') == audit['selection_sha256']
freeze_path = HERE.parent / 'fps_round3_freeze.json'
assert sha(freeze_path) == audit['freeze_sha256']
freeze = json.loads(freeze_path.read_text())
assert all(sha(ROOT / name) == digest for name, digest in freeze['files'].items())
before = read('judge_sources_before.json')['sources']
after = read('judge_sources_after.json')['sources']
assert before == after and all(sha(ROOT / name) == digest for name, digest in after.items())
official = Path(plan['official_root'])
folders = {path.parent.name: path.parent for path in official.glob('*/*/info.toml')}
case_inventory = {}
registered_hashes = {}
official_before = read('judge_official_before.json')['files']
for problem in sorted(set(plan['official'].values())):
    folder = folders[problem]
    info = tomllib.loads((folder / 'info.toml').read_text())
    expected = sorted(f'{Path(row["name"]).stem}_{i:02}.in' for row in info['tests'] for i in range(row['number']))
    actual = sorted(path.name for path in (folder / 'in').glob('*.in'))
    assert expected == actual, problem
    assert all((folder / 'out' / (Path(name).stem + '.out')).is_file() for name in expected)
    case_inventory[problem] = len(expected)
    hash_path = folder / 'hash.json'
    committed = subprocess.check_output(['git', '-C', str(official), 'show', 'HEAD:' + hash_path.relative_to(official).as_posix()])
    assert hash_path.read_bytes() == committed
    registered = json.loads(hash_path.read_text())
    for name in expected:
        out = Path(name).stem + '.out'
        for kind, file in [('in', name), ('out', out)]:
            key = (folder / kind / file).relative_to(official).as_posix()
            assert official_before[key] == registered[file], (problem, file)
    registered_hashes[problem] = sha(hash_path)
previous = json.loads((ROOT / 'test/benchmark/results/fps_round2_validation/judge_sources_after.json').read_text())['sources']
changes = {}
for driver, row in plan['dependencies'].items():
    changes[driver] = [name for name, digest in row['dependencies'].items() if previous.get(name) != digest]
    assert changes[driver], f'An unchanged dependency check was repeated: {driver}'
assert set(plan['tests']) | set(plan['skipped']) == set(selection['verification'])
assert not set(plan['tests']) & set(plan['skipped'])
audit.update({'freeze_files_match': True, 'official_case_inventory_matches_info_toml': True,
              'official_problem_case_counts': case_inventory,
              'official_registered_case_hashes_match': True,
              'official_hash_json_matches_commit': True,
              'official_hash_json_sha256': registered_hashes,
              'changed_actual_dependencies_since_previous_success': changes,
              'all_executed_drivers_have_changed_dependencies': True,
              'finalize_harness_sha256': sha(Path(__file__))})
(HERE / 'judge_audit.json').write_text(json.dumps(audit, indent=2) + '\n')
merged = read('judge.json')
merged['audit'] = audit
(HERE / 'judge.json').write_text(json.dumps(merged, indent=2) + '\n')
manifest = {path.name: sha(path) for path in sorted(HERE.glob('judge*')) if path.is_file() and path.name != 'judge_manifest.json'}
(HERE / 'judge_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps({key: audit[key] for key in ['selected_drivers', 'passed_drivers', 'passed_cases', 'skipped_drivers',
      'source_hashes_unchanged', 'official_hashes_unchanged', 'official_case_inventory_matches_info_toml',
      'all_executed_drivers_have_changed_dependencies', 'freeze_files_match']}, indent=2))
