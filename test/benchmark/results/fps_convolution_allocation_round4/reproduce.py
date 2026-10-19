import argparse
import fcntl
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

HERE = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    metadata = json.loads((HERE / 'metadata.json').read_text())
    parser = argparse.ArgumentParser()
    parser.add_argument('--repository', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--variant', choices=list(metadata['candidates']), default='capacity_1m')
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--trials', type=int, default=41)
    parser.add_argument('--seed', type=int, default=20261044)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    args = parser.parse_args()
    args.work = args.work.resolve()
    if args.work.exists():
        raise ValueError('Choose a new work directory')
    for name, expected in metadata['files'].items():
        if sha(HERE / name) != expected:
            raise ValueError('Artifact hash mismatch: ' + name)
    args.work.mkdir(parents=True)
    baseline = args.work / 'before'
    data = subprocess.check_output(['git', 'archive', metadata['baseline'], 'fps', 'components',
                                    'math', 'convolution', 'test'], cwd=args.repository)
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        archive.extractall(baseline, filter='data')
    for name, expected in metadata['baseline_headers'].items():
        if sha(baseline / name) != expected:
            raise ValueError('Baseline header hash mismatch: ' + name)
    driver = metadata['recorded_driver']
    if (baseline / driver['restore_path']).read_bytes() != (HERE / driver['artifact']).read_bytes():
        raise ValueError('Recorded driver differs from reconstructed baseline')
    candidate = args.work / 'candidate'
    shutil.copytree(baseline, candidate)
    info = metadata['candidates'][args.variant]
    patch = HERE / (args.variant + '.patch')
    subprocess.run(['patch', '-s', '-p1', '-d', str(candidate), '-i', str(patch)], check=True)
    if sha(patch) != info['patch_sha256'] or sha(candidate / info['header']) != info['header_sha256']:
        raise ValueError('Candidate patch/header hash mismatch')
    if gzip.decompress((HERE / info['actual_source']).read_bytes()) != (candidate / info['header']).read_bytes():
        raise ValueError('Reconstructed source differs from measured source')
    reference = args.work / '409465.cpp'
    reference.write_bytes(gzip.decompress((HERE / metadata['reference']['source']).read_bytes()))
    if sha(reference) != metadata['reference']['sha256']:
        raise ValueError('Reference hash mismatch')
    print('Prepared', args.variant, flush=True)
    if args.measure:
        subprocess.run([sys.executable, str(HERE / 'whole.py'), '--baseline-root', str(baseline),
                        '--variant', args.variant + '=' + str(candidate), '--output', str(args.work / 'measurement'),
                        '--trials', str(args.trials), '--seed', str(args.seed), '--official', str(args.official),
                        '--reference', str(reference), '--cases', 'max_random_00', 'max_random_01',
                        'small_and_large_03', 'medium_02'], check=True)


if __name__ == '__main__':
    main()
