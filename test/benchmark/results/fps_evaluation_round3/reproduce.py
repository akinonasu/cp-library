import argparse
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def main():
    parser = argparse.ArgumentParser()
    variants = ['taylor_evex', 'taylor_wide32', 'taylor_wide64', 'taylor_split', 'taylor_split_noodd', 'evaluation_evex']
    parser.add_argument('--variant', choices=variants, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--reference-root', type=Path, default=Path('/tmp/fps-reference'))
    args = parser.parse_args()
    if args.work.exists():
        raise ValueError('work directory already exists')
    args.work.mkdir(parents=True)
    metadata = json.loads((HERE / 'metadata.json').read_text())
    baseline = args.work / 'before'
    data = subprocess.check_output(['git', 'archive', metadata['baseline'], 'fps', 'components', 'math', 'convolution', 'test'], cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        archive.extractall(baseline, filter='data')
    for name, expected in metadata['baseline_headers'].items():
        if hashlib.sha256((baseline / name).read_bytes()).hexdigest() != expected:
            raise ValueError('baseline header hash mismatch: ' + name)
    candidate = args.work / 'candidate'
    shutil.copytree(baseline, candidate)
    patch = HERE / (args.variant + '.patch')
    info = metadata['candidates'][args.variant]
    if hashlib.sha256(patch.read_bytes()).hexdigest() != info['patch_sha256']:
        raise ValueError('candidate patch hash mismatch')
    subprocess.run(['patch', '-s', '-p1', '-d', str(candidate), '-i', str(patch)], check=True)
    if hashlib.sha256((candidate / info['header']).read_bytes()).hexdigest() != info['candidate_sha256']:
        raise ValueError('candidate header hash mismatch')
    print('Prepared', args.variant, flush=True)
    if args.measure:
        operations = ['eval', 'interpolate'] if args.variant == 'evaluation_evex' else ['taylor']
        subprocess.run([sys.executable, str(ROOT / 'test/benchmark/fps_evaluation.py'),
                        '--baseline', metadata['baseline'], '--baseline-tree', str(baseline),
                        '--candidate-root', str(candidate), '--output', str(args.work / 'measurement'),
                        '--operations', *operations, '--trials', str(args.trials),
                        '--official-root', str(args.official_root), '--reference-root', str(args.reference_root)], check=True)


if __name__ == '__main__':
    main()
