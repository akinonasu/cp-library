import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
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
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
p = argparse.ArgumentParser()
p.add_argument('--work', type=Path, required=True)
p.add_argument('--reference', type=Path, required=True)
p.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
p.add_argument('--prepare-only', action='store_true')
p.add_argument('--build', action='store_true')
p.add_argument('--measure', action='store_true')
a = p.parse_args()
a.work.mkdir(parents=True, exist_ok=True)
metadata = json.loads((HERE / 'metadata.json').read_text())
expected = json.loads((HERE / 'build.json').read_text())
assert sha(a.reference) == metadata['reference_original_sha256']
data = subprocess.check_output(['git', 'archive', metadata['revision'], 'fps', 'components', 'math', 'convolution', 'test/convolution.test.cpp', 'test/fps_io.hpp'], cwd=ROOT)
with tarfile.open(fileobj=io.BytesIO(data)) as archive:
    archive.extractall(a.work / 'source', filter='data')
for name, digest in metadata['snapshot_sha256'].items():
    assert sha(a.work / 'source' / name) == digest, name
for name in ['profile.hpp', 'profile_current.cpp', 'clock_calibration.cpp', 'measure.py']:
    shutil.copy2(HERE / name, a.work / name)
shutil.copy2(a.reference, a.work / 'reference_original.cpp')
shutil.copy2(a.reference, a.work / 'profile_reference.cpp')
subprocess.run(['patch', '--batch', '--silent', str(a.work / 'profile_reference.cpp'), str(HERE / 'reference.patch')], check=True)
sources = {'current': a.work / 'source/test/convolution.test.cpp', 'reference': a.work / 'reference_original.cpp', 'profile_current': a.work / 'profile_current.cpp', 'profile_reference': a.work / 'profile_reference.cpp', 'clock_calibration': a.work / 'clock_calibration.cpp'}
for variant, source in sources.items():
    assert sha(source) == expected[variant]['source_sha256'], variant
(a.work / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
print('All snapshot and profile sources reproduce measured SHA-256 values.')
if not (a.build or a.measure):
    raise SystemExit(0)
flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
def build(pair):
    variant, source = pair
    binary = a.work / variant
    command = ['g++', *flags, '-I', str(a.work / 'source'), '-I', str(a.work), str(source), '-o', str(binary)]
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        subprocess.run(command, check=True)
    return variant, {'command': command, 'source_sha256': sha(source), 'binary_sha256': sha(binary)}
with ThreadPoolExecutor(max_workers=3) as pool:
    builds = dict(pool.map(build, sources.items()))
(a.work / 'build.json').write_text(json.dumps(builds, indent=2) + '\n')
if a.measure:
    path = a.work / 'measure.py'
    text = path.read_text().replace("Path('/tmp/library-checker-problems/convolution/convolution_mod')", 'Path(' + repr(str(a.official_root / 'convolution/convolution_mod')) + ')')
    path.write_text(text)
    subprocess.run([sys.executable, str(path), '--trials', '9', '--cases', 'max_random_00', 'max_random_01', '--output', 'replay.json'], check=True)
