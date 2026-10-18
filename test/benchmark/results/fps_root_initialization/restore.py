import argparse
import hashlib
import io
import json
from pathlib import Path
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
    args = parser.parse_args()
    manifest = json.loads((HERE / 'manifest.json').read_text())
    for name, expected in manifest['files'].items():
        actual = sha(HERE / name)
        if actual != expected:
            raise RuntimeError(('Archive hash differs', name, actual, expected))
    raw = json.loads((HERE / 'raw.json').read_text())
    if sha(HERE / 'original_harness.py') != raw['harness_sha256']:
        raise RuntimeError('Original harness differs')
    for name, info in raw['builds'].items():
        if name.startswith('safety_') and sha(HERE / 'safety.cpp') != info['source_sha256']:
            raise RuntimeError('Safety source differs')
    if args.work.exists():
        raise RuntimeError('Use a new directory for --work')
    args.work.mkdir(parents=True)
    baseline, candidate = args.work / 'baseline', args.work / 'candidate'
    paths = ['fps', 'components', 'math', 'convolution', 'test/fps_io.hpp', 'test/benchmark/fps_tuning_basic.cpp']
    paths += [f'test/fps_{op}.test.cpp' for op in ['log', 'exp', 'pow', 'inv', 'sqrt']]
    data = subprocess.check_output(['git', '-C', str(args.repo.resolve()), 'archive', raw['baseline'], *paths])
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        archive.extractall(baseline, filter='data')
    shutil.copytree(baseline, candidate)
    subprocess.run(['git', 'apply', '--no-index', str(HERE / 'candidate.patch')], cwd=candidate, check=True)
    for name, tree in [('baseline', baseline), ('candidate', candidate)]:
        for path, expected in raw['sources'][name].items():
            if sha(tree / path) != expected:
                raise RuntimeError(('Restored header differs', name, path))
        if sha(tree / 'test/benchmark/fps_tuning_basic.cpp') != raw['builds']['kernel_' + name]['source_sha256']:
            raise RuntimeError('Kernel driver differs')
        for op in ['log', 'exp', 'pow', 'inv', 'sqrt']:
            if sha(tree / f'test/fps_{op}.test.cpp') != raw['builds']['whole_' + op + '_' + name]['source_sha256']:
                raise RuntimeError(('Whole driver differs', op))
    report = dict(baseline=raw['baseline'], repository=str(args.repo.resolve()),
                  archive_manifest='passed', original_harness='passed', safety_source='passed',
                  headers={name: len(raw['sources'][name]) for name in ['baseline', 'candidate']},
                  kernel_drivers=2, whole_drivers=10, restoration='passed')
    (args.work / 'restoration.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
