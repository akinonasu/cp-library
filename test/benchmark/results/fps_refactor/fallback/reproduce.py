from pathlib import Path
import argparse, hashlib, json, re, shutil, subprocess, tempfile
parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='g++')
parser.add_argument('--directory', type=Path)
args = parser.parse_args()
artifacts = Path(__file__).resolve().parent
root = next(p for p in artifacts.parents if (p / 'fps/fps.hpp').exists())
stage = args.directory or Path(tempfile.mkdtemp(prefix='fps-fallback-'))
stage.mkdir(parents=True, exist_ok=True)
expected = json.loads((artifacts / 'snapshot.json').read_text())['input_headers']
final_fps = json.loads((artifacts / 'review.json').read_text())['after_fps_sha256']
seen = set()
def copy(path):
    path = path.resolve()
    if path in seen:
        return
    seen.add(path)
    rel = path.relative_to(root)
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    known = final_fps if str(rel) == 'fps/fps.hpp' else expected.get(str(rel))
    if known is not None and digest != known:
        print('Header differs from recorded snapshot:', rel)
    for variant in ['log_after', 'inverse_generic']:
        target = stage / variant / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)
    for name in re.findall(r'^\s*#include\s+"([^"]+)"', path.read_text(), re.M):
        dep = path.parent / name
        if dep.exists():
            copy(dep)
copy(root / 'fps/fps.hpp')
shutil.copyfile(artifacts / 'driver.cpp', stage / 'check.cpp')
subprocess.run(['patch', '-p1', '-i', str(artifacts / 'inverse_candidate.patch')], cwd=stage / 'inverse_generic', check=True)
for variant in ['log_after', 'inverse_generic']:
    subprocess.run(['flock', '-s', '/tmp/cp-fps-tuning.lock', args.compiler, '-std=c++20', '-O2', '-DNDEBUG', '-I', str(stage / variant), str(stage / 'check.cpp'), '-o', str(stage / variant / 'gcc')], check=True)
    subprocess.run(['flock', '-s', '/tmp/cp-fps-tuning.lock', str(stage / variant / 'gcc')], check=True)
subprocess.run(['python3', str(artifacts / 'benchmark.py'), str(stage), args.compiler], check=True)
print('Results:', stage / 'inverse_benchmark.json')
