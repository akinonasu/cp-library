from pathlib import Path
import argparse
import hashlib
import io
import json
import shutil
import subprocess
import tarfile


def sha(path):
	return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('repo', type=Path)
	parser.add_argument('work', type=Path)
	parser.add_argument('--judge-dir', type=Path, default=Path('/tmp/library-checker-problems/polynomial/log_of_formal_power_series'))
	args = parser.parse_args()
	pack = Path(__file__).resolve().parent
	repo = args.repo.resolve()
	work = args.work.resolve()
	if work.exists():
		raise SystemExit('work directory must not exist: ' + str(work))
	work.mkdir(parents=True)
	baseline = '4765ceb5f995801d27b23840392c9c354ecdecb9'
	archive = subprocess.check_output(['git', '-C', str(repo), 'archive', baseline])
	before = work / 'before'
	before.mkdir()
	with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
		tar.extractall(before, filter='data')
	record = {'baseline': baseline, 'headers': {}, 'source_hashes': {}}
	for name, patch in [('after', 'split-input.patch'), ('after_layout', 'old-layout.patch'), ('after_shared', 'shared-input.patch')]:
		tree = work / name
		shutil.copytree(before, tree)
		with (pack / patch).open('rb') as source:
			subprocess.run(['patch', '--batch', '-p1'], cwd=tree, stdin=source, check=True)
	for file, names in [('results.json', ['before', 'after']), ('layout.json', ['before', 'after', 'after_layout']), ('shared.json', ['before', 'after_shared'])]:
		measured = json.loads((pack / file).read_text())
		for name in names:
			expected = measured['sources'][name + '_headers']
			for relative, digest in expected.items():
				actual = sha(work / name / relative)
				if actual != digest:
					raise SystemExit('header mismatch: ' + name + '/' + relative)
			record['headers'][file + ':' + name] = len(expected)
	for name in ['kernel.cpp', 'log-boundary-unit.cpp', 'bench.py', 'layout.py', 'shared_bench.py', 'repeat.py']:
		shutil.copy2(pack / name, work / name)
		record['source_hashes'][name] = sha(pack / name)
	ref = work / 'reference409452.cpp'
	shutil.copy2(pack / ref.name, ref)
	expected_ref = json.loads((pack / 'results.json').read_text())['sources']['reference_whole']['source_sha256']
	if sha(ref) != expected_ref:
		raise SystemExit('reference source mismatch')
	record['source_hashes'][ref.name] = sha(ref)
	for name in ['bench.py', 'layout.py', 'shared_bench.py', 'repeat.py']:
		text = (pack / name).read_text().replace("Path('/tmp/fps-native-source')", repr(work).replace('PosixPath', 'Path'))
		text = text.replace("Path('/tmp/library-checker-problems/polynomial/log_of_formal_power_series')", repr(args.judge_dir.resolve()).replace('PosixPath', 'Path'))
		text = text.replace("Path('/tmp/fps-reference/log_of_formal_power_series/409452.cpp')", repr(ref).replace('PosixPath', 'Path'))
		(work / (Path(name).stem + '_local.py')).write_text(text)
	for name in ['max_random_00', 'max_random_01']:
		inp = args.judge_dir / 'in' / (name + '.in')
		if inp.exists():
			expected = json.loads((pack / 'results.json').read_text())['whole'][name]['input_sha256']
			if sha(inp) != expected:
				raise SystemExit('official input mismatch: ' + name)
			record['source_hashes']['official:' + name] = sha(inp)
	(work / 'reconstruction.json').write_text(json.dumps(record, indent=2) + '\n')
	print(json.dumps(record, indent=2))


if __name__ == '__main__':
	main()
