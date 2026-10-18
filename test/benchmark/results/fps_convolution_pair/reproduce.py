from pathlib import Path
import argparse
import fcntl
import hashlib
import io
import json
import shutil
import subprocess
import tarfile


HERE = Path(__file__).resolve().parent


def sha(path):
	return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('--repo', type=Path, default=HERE.parents[3])
	parser.add_argument('--work', type=Path, required=True)
	parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
	parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
	parser.add_argument('--prepare-only', action='store_true')
	parser.add_argument('--measure', action='store_true')
	args = parser.parse_args()
	if args.prepare_only and args.measure:
		parser.error('--prepare-only and --measure are mutually exclusive')
	repo, work = args.repo.resolve(), args.work.resolve()
	if work.exists():
		parser.error('--work must not exist')
	work.mkdir(parents=True)
	meta = json.loads((HERE / 'metadata.json').read_text())
	raw = json.loads((HERE / 'results.json').read_text())
	base = work / 'base'
	base.mkdir()
	archive = subprocess.check_output(['git', '-C', str(repo), 'archive', meta['baseline']])
	with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
		tar.extractall(base, filter='data')
	candidate = work / 'candidate'
	shutil.copytree(base, candidate)
	with (HERE / 'candidate.patch').open('rb') as source:
		subprocess.run(['patch', '--batch', '-p1'], cwd=candidate, stdin=source, check=True)
	record = {'baseline': meta['baseline'], 'header_checks': {}, 'source_checks': {}}
	for name, tree in [('base', base), ('candidate', candidate)]:
		for relative, digest in meta['headers'][name].items():
			assert sha(tree / relative) == digest, (name, relative)
		record['header_checks'][name] = len(meta['headers'][name])
	for name in ['bench.py', 'verify.cpp']:
		shutil.copy2(HERE / name, work / name)
		record['source_checks'][name] = sha(HERE / name)
	assert sha(HERE / 'bench.py') == meta['harness_sha256']
	text = (HERE / 'bench.py').read_text()
	text = text.replace("Path('/tmp/fps-round2-conv-pair')", repr(work).replace('PosixPath', 'Path'))
	text = text.replace("Path('/tmp/cp-fps-round2-base')", repr(base).replace('PosixPath', 'Path'))
	text = text.replace("Path('/tmp/fps-reference/convolution_mod/409465.cpp')", repr(args.reference.resolve()).replace('PosixPath', 'Path'))
	judge = args.official_root.resolve() / 'convolution/convolution_mod'
	if not judge.exists():
		judge = args.official_root.resolve() / 'polynomial/convolution_mod'
	text = text.replace("Path('/tmp/library-checker-problems/convolution/convolution_mod')", repr(judge).replace('PosixPath', 'Path'))
	text = text.replace("Path('/tmp/library-checker-problems/polynomial/convolution_mod')", repr(judge).replace('PosixPath', 'Path'))
	(work / 'bench_local.py').write_text(text)
	if args.reference.exists():
		assert sha(args.reference) == raw['sources']['reference_whole']['source_sha256']
		record['source_checks']['reference'] = sha(args.reference)
	for name, case in raw['whole'].items():
		inp = judge / 'in' / (name + '.in')
		answer = judge / 'out' / (name + '.out')
		if inp.exists():
			assert sha(inp) == case['input_sha256']
			assert sha(answer) == case['answer_sha256']
			record['source_checks'][name] = sha(inp)
	(work / 'reconstruction.json').write_text(json.dumps(record, indent=2) + '\n')
	if args.prepare_only:
		print(json.dumps(record, indent=2))
		return
	with Path('/tmp/cp-fps-tuning.lock').open('w') as lock:
		fcntl.flock(lock, fcntl.LOCK_SH)
		binary = work / 'verify'
		subprocess.run(['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-march=haswell', '-I' + str(candidate), str(work / 'verify.cpp'), '-o', str(binary)], check=True)
		subprocess.run([str(binary)], check=True)
	if args.measure:
		subprocess.run(['python3', str(work / 'bench_local.py')], check=True)


if __name__ == '__main__':
	main()
