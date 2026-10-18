from pathlib import Path
import hashlib
import json
import re


ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
OFFICIAL = Path('/tmp/library-checker-problems')


def sha(path):
	return hashlib.sha256(path.read_bytes()).hexdigest()


def read(name):
	return json.loads((HERE / name).read_text())


selection = read('selection.json')
before = read('judge_sources_before.json')
plan = read('judge_plan.json')
verification = selection['verification']
files = set()
for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'math', 'string']:
	files.update((ROOT / folder).rglob('*.hpp'))
files.update(ROOT.glob('*.hpp'))
files.update((ROOT / 'test').glob('*.hpp'))
files.update(ROOT / name for name in verification)
files.add(ROOT / 'test/benchmark/fps_speed_verify.py')
files.add(HERE / 'selection.json')
files.update(HERE.glob('judge_selection_part*.json'))
after = {str(path.relative_to(ROOT)): sha(path) for path in sorted(files)}
assert after == before['sources'], 'Sources changed during final official verification'
assert sha(HERE / 'selection.json') == plan['selection_sha256'] == before['selection_sha256']
folders = {path.parent.name: path.parent for path in OFFICIAL.glob('*/*/info.toml')}
parts = [read(f'judge_part{i}.json') for i in range(3)]
tests = {}
skipped = {}
for i, part in enumerate(parts):
	assert part['mode'] == 'judge' and part['case_limit'] is None
	for key in ['mode', 'case_limit', 'compiler', 'flags', 'official_commit', 'sources']:
		assert part[key] == parts[0][key], (i, key)
	expected = read(f'judge_selection_part{i}.json')['verification']
	assert set(part['tests']) | set(part['skipped']) == set(expected)
	assert not set(part['tests']) & set(part['skipped'])
	assert not (set(tests) | set(skipped)) & set(expected)
	for name, row in part['tests'].items():
		assert row['source_sha256'] == before['sources'][name] == after[name]
		url = re.search(r'#define PROBLEM "([^"]+)"', (ROOT / name).read_text())[1]
		assert url.startswith('https://judge.yosupo.jp/problem/')
		problem = url.rsplit('/', 1)[1]
		cases = sorted(path.name for path in (folders[problem] / 'in').glob('*.in'))
		assert row['cases'] == cases and cases, name
	for name, url in part['skipped'].items():
		assert plan['external_skips'][name]['url'] == url
	tests.update(part['tests'])
	skipped.update(part['skipped'])
assert set(tests) | set(skipped) == set(verification)
assert set(skipped) == set(plan['external_skips'])
assert set(tests) == set(plan['official'])
merged = {key: parts[0][key] for key in ['mode', 'case_limit', 'compiler', 'flags', 'official_commit', 'sources']}
merged.update({
	'tests': {name: tests[name] for name in verification if name in tests},
	'skipped': {name: skipped[name] for name in verification if name in skipped},
	'skip_reasons': plan['external_skips'],
	'parts': [f'judge_part{i}.json' for i in range(3)],
	'selection_sha256': plan['selection_sha256'],
	'checker_order': ['input', 'actual_output', 'official_answer'],
	'source_hashes_unchanged': True,
})
audit = {
	'selection_sha256': plan['selection_sha256'],
	'selected_drivers': len(verification),
	'passed_drivers': len(tests),
	'passed_cases': sum(len(row['cases']) for row in tests.values()),
	'skipped_drivers': len(skipped),
	'skip_reasons': plan['external_skips'],
	'coverage_matches_final_selection': True,
	'full_official_cases': True,
	'checker_order': merged['checker_order'],
	'source_hashes_unchanged': True,
	'source_files_checked': len(after),
	'part_source_maps_equal': True,
	'merge_script_sha256': sha(Path(__file__)),
}
(HERE / 'judge_sources_after.json').write_text(json.dumps({'phase': 'after', 'sources': after}, indent=2) + '\n')
(HERE / 'judge.json').write_text(json.dumps(merged, indent=2) + '\n')
(HERE / 'judge_audit.json').write_text(json.dumps(audit, indent=2) + '\n')
print(json.dumps(audit, indent=2))
