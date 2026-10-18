from pathlib import Path
import argparse
import hashlib
import json
import shutil
import subprocess


def sha(path):
	return hashlib.sha256(path.read_bytes()).hexdigest()


parser = argparse.ArgumentParser()
parser.add_argument('repo', type=Path)
parser.add_argument('work', type=Path)
parser.add_argument('--judge-root', type=Path, default=Path('/tmp/library-checker-problems/polynomial'))
args = parser.parse_args()
pack = Path(__file__).resolve().parent
repo, work, judge = args.repo.resolve(), args.work.resolve(), args.judge_root.resolve()
work.mkdir(parents=True, exist_ok=False)
baseline = '144ae32387675fa2a889ea3e8cd19b03ad5016bf'
before = work / 'baseline'
before.mkdir()
archive = subprocess.Popen(['git', '-C', str(repo), 'archive', baseline], stdout=subprocess.PIPE)
try:
	subprocess.run(['tar', '-xf', '-', '-C', str(before)], stdin=archive.stdout, check=True)
finally:
	archive.stdout.close()
if archive.wait():
	raise RuntimeError('git archive failed')

metadata = json.loads((pack / 'metadata.json').read_text())
fused = json.loads((pack / 'fused_metadata.json').read_text())
scaled = json.loads((pack / 'scaled_metadata.json').read_text())
expected = {
	'baseline': metadata['sources']['baseline_headers'],
	'candidate': metadata['sources']['candidate_headers'],
	'fused': fused['sources']['fused_headers'],
	'combined': fused['sources']['combined_headers'],
	'scaled': scaled['sources']['scaled_headers'],
}
for name, patch in [('candidate', 'live-batch.patch'), ('fused', 'fused.patch'), ('combined', 'combined.patch'), ('scaled', 'scaled.patch')]:
	tree = work / name
	shutil.copytree(before, tree)
	with (pack / patch).open('rb') as source:
		subprocess.run(['patch', '--batch', '-p1'], cwd=tree, stdin=source, check=True)

record = {'baseline': baseline, 'headers': {}, 'source_hashes': {}, 'official_inputs': {}}
for name, hashes in expected.items():
	tree = work / name
	actual = {str(p.relative_to(tree)): sha(p) for p in tree.rglob('*.hpp')}
	if actual != hashes:
		changed = sorted(p for p in actual.keys() | hashes.keys() if actual.get(p) != hashes.get(p))
		raise RuntimeError(f'header mismatch: {name}: {changed}')
	record['headers'][name] = len(actual)

integrated_checks = json.loads((pack / 'integrated-checks.json').read_text())
integrated = work / 'integrated'
shutil.copytree(work / 'scaled', integrated)
with (pack / 'unit.patch').open('rb') as source:
	subprocess.run(['patch', '--batch', '-p1'], cwd=integrated, stdin=source, check=True)
unit = integrated / 'test/unit/fps_inverse_blocked.cpp'
if sha(unit) != integrated_checks['unit_sha256'] or sha(unit) != sha(pack / 'integrated-unit.cpp'):
	raise RuntimeError('integrated unit snapshot mismatch')
if sha(integrated / 'components/fps_inverse_blocked.hpp') != integrated_checks['header_sha256']:
	raise RuntimeError('integrated inverse header mismatch')
record['integrated_unit_sha256'] = sha(unit)
record['integrated_header_sha256'] = integrated_checks['header_sha256']

helpers = ['kernel.cpp', 'append_kernel.cpp', 'append_check.cpp', 'fused_check.cpp', 'ref_inverse_tail.cpp', 'ref_sqrt_tail.cpp']
scripts = ['build.py', 'bench.py', 'fused_build.py', 'fused_bench.py', 'scaled_build.py', 'repeat.py', 'scaled_reference.py']
for name in helpers + scripts:
	shutil.copy2(pack / name, work / name)
	record['source_hashes'][name] = sha(pack / name)
shutil.copytree(pack / 'references', work / 'references')
for op, problem, number, marker in [
	('inverse', 'inv_of_formal_power_series', 409370, 'namespace {\n\n// g = 1 / f'),
	('sqrt', 'sqrt_of_formal_power_series', 409728, 'namespace {\n\nconstexpr std::uint32_t kP = poly::kModulus;'),
]:
	source = work / 'references' / problem / f'{number}.cpp'
	whole = metadata['sources'][f'reference_{op}_whole']
	if sha(source) != whole['source_sha256']:
		raise RuntimeError(f'reference whole source mismatch: {op}')
	text = source.read_text()
	kernel = work / f'reference_{op}_kernel.cpp'
	kernel.write_text(text[:text.index(marker)] + (pack / f'ref_{op}_tail.cpp').read_text())
	if sha(kernel) != metadata['sources'][f'reference_{op}_kernel']['source_sha256']:
		raise RuntimeError(f'reference kernel source mismatch: {op}')
	record['source_hashes'][f'reference_{op}_whole'] = sha(source)
	record['source_hashes'][f'reference_{op}_kernel'] = sha(kernel)

paths = {
	'/tmp/fps-round2-inverse-probe': work,
	'/tmp/cp-fps-round2-base': before,
	'/tmp/fps-round2-inverse-candidate': work / 'candidate',
	'/tmp/fps-round2-inverse-fused-candidate': work / 'fused',
	'/tmp/fps-round2-inverse-combined-candidate': work / 'combined',
	'/tmp/fps-round2-inverse-scaled/candidate': work / 'scaled',
	'/tmp/fps-reference': work / 'references',
	'/tmp/library-checker-problems/polynomial': judge,
}
for problem in ['inv_of_formal_power_series', 'sqrt_of_formal_power_series']:
	paths['/tmp/library-checker-problems/polynomial/' + problem] = judge / problem
for name in scripts:
	text = (pack / name).read_text()
	for original, replacement in paths.items():
		text = text.replace(f'Path({original!r})', f'Path({str(replacement)!r})')
	local = work / (Path(name).stem + '_local.py')
	local.write_text(text)

for filename in ['results.json', 'fused_results.json', 'repeat.json']:
	if not (pack / filename).is_file():
		continue
	data = json.loads((pack / filename).read_text())
	for op, cases in data.get('whole', {}).items():
		problem = 'inv_of_formal_power_series' if op == 'inverse' else 'sqrt_of_formal_power_series'
		for name, row in cases.items():
			inp = judge / problem / 'in' / f'{name}.in'
			if inp.is_file():
				if sha(inp) != row['input_sha256']:
					raise RuntimeError(f'official input mismatch: {problem}/{name}')
				record['official_inputs'][f'{problem}/{name}'] = sha(inp)

(work / 'reconstruction.json').write_text(json.dumps(record, indent=2) + '\n')
print(json.dumps(record, indent=2))
