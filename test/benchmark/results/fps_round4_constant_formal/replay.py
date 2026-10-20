import argparse
import fcntl
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-round4-formal-replay'))
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve()
    if work == ROOT or ROOT in work.parents:
        parser.error('--work must be outside the repository')
    work.mkdir(parents=True, exist_ok=True)
    metadata = json.loads((HERE / 'metadata.json').read_text())
    core = (HERE / metadata['core_source_pack']['relative_directory']).resolve()
    for name, expected in metadata['core_source_pack']['files'].items():
        if sha(core / name) != expected:
            raise RuntimeError('Core archive differs: ' + name)
    for path, value in [(HERE / 'drivers.tar.xz', metadata['drivers_tar_sha256']), (HERE / 'harness.py', metadata['harness_sha256'])]:
        if sha(path) != value:
            raise RuntimeError('Pack file differs: ' + str(path))
    for key in ('raw_result', 'initial_result'):
        record = metadata[key]
        if sha((HERE / record['relative_path']).resolve()) != record['sha256']:
            raise RuntimeError('Recorded result differs: ' + key)
    source_metadata = json.loads((core / 'sources.json').read_text())
    expected_before = dict(source_metadata['before'])
    expected_before.update(metadata['additional_drivers_sha256'])
    expected = dict(before=expected_before, current=metadata['source_snapshot_sha256'])
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        actual = {}
        for variant in ('before', 'current'):
            tree = work / variant
            if tree.exists():
                shutil.rmtree(tree)
            tree.mkdir()
            with tarfile.open(core / 'baseline.tar.xz') as archive:
                archive.extractall(tree, filter='data')
            if variant == 'current':
                subprocess.run(['patch', '-p1', '-i', str(core / 'current.patch')], cwd=tree, check=True, stdout=subprocess.DEVNULL)
            with tarfile.open(HERE / 'drivers.tar.xz') as archive:
                archive.extractall(tree, filter='data')
            actual[variant] = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}
        if actual != expected:
            raise RuntimeError('Reconstructed source bytes differ')
        result = dict(files=actual, current_file_count=len(actual['current']), baseline_file_count=len(actual['before']), success=True,
                      scope='Source-only reconstruction. No compile or timing is performed; historical binary identity is not claimed.')
        (work / 'reconstruction.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Reconstructed 169 current and 167 baseline source files exactly.', flush=True)


if __name__ == '__main__':
    main()
