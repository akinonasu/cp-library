import argparse
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description='Restore the convolution phase audit without changing repository headers.')
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-conv-phases-reproduction'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--build', action='store_true')
    parser.add_argument('--measure', action='store_true')
    args = parser.parse_args()
    if args.prepare_only and (args.build or args.measure):
        parser.error('--prepare-only excludes --build and --measure')
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or repo in work.parents:
        parser.error('--work must be outside the repository')
    recorded = json.loads((HERE / 'results.json').read_text())
    revision = recorded['metadata']['revision']
    if sha(args.reference) != recorded['metadata']['reference_original_sha256']:
        raise RuntimeError('Reference 409465 SHA-256 differs from the recorded source')
    work.mkdir(parents=True, exist_ok=True)
    source = work / 'source'
    check = dict(baseline=revision, source_hashes={}, analysis_driver_hashes={})
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        archive = subprocess.check_output(['git', 'archive', revision, 'fps', 'components', 'math', 'convolution',
                                           'test/convolution.test.cpp', 'test/fps_io.hpp'], cwd=repo)
        if source.exists():
            shutil.rmtree(source)
        with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
            stream.extractall(source, filter='data')
        actual = {str(p.relative_to(source)): sha(p) for p in sorted(source.rglob('*')) if p.is_file()}
        if actual != recorded['metadata']['snapshot_sha256']:
            raise RuntimeError('Restored source snapshot differs from the measurement')
        for name in ('profile.hpp', 'profile_current.cpp', 'clock_calibration.cpp', 'compile.py', 'measure.py'):
            shutil.copyfile(HERE / name, work / name)
        shutil.copyfile(args.reference, work / 'reference_original.cpp')
        shutil.copyfile(args.reference, work / 'profile_reference.cpp')
        subprocess.run(['patch', '-p1', '-i', str(HERE / 'reference.patch')], cwd=work, check=True)
        for name in ('profile_current', 'profile_reference', 'clock_calibration'):
            actual_hash = sha(work / (name + '.cpp'))
            if actual_hash != recorded['build'][name]['source_sha256']:
                raise RuntimeError('Restored analysis driver differs: ' + name)
            check['analysis_driver_hashes'][name] = actual_hash
        check['source_hashes'] = actual
        check['source_files_match_recorded'] = len(actual)
        metadata = dict(revision=revision, compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                        reference_original_sha256=sha(work / 'reference_original.cpp'), snapshot_sha256=actual)
        (work / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
        shutil.copyfile(HERE / 'results.json', work / 'recorded.json')
        (work / 'reconstruction.json').write_text(json.dumps(check, indent=2) + '\n')
    print('Restored', len(actual), 'source files and both analysis drivers with matching SHA-256 values.', flush=True)
    if args.build or args.measure:
        subprocess.run(['python3', str(work / 'compile.py')], check=True)
    if args.measure:
        environment = os.environ.copy()
        environment['FPS_OFFICIAL_ROOT'] = str(args.official_root.resolve())
        subprocess.run(['python3', str(work / 'measure.py')], env=environment, check=True)


if __name__ == '__main__':
    main()
