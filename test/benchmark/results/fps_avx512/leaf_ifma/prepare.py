from pathlib import Path
import argparse, hashlib, json, shutil, subprocess

BASELINE = 'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0'
parser = argparse.ArgumentParser()
parser.add_argument('repo', type=Path)
parser.add_argument('work', type=Path)
args = parser.parse_args()
pack = Path(__file__).resolve().parent
work = args.work.resolve()
work.mkdir(parents=True, exist_ok=True)
if any(work.iterdir()):
    raise SystemExit('work must be empty')
base = work / 'base'
base.mkdir()
archive = subprocess.Popen(['git', '-C', str(args.repo), 'archive', BASELINE], stdout=subprocess.PIPE)
subprocess.run(['tar', '-x', '-C', str(base)], stdin=archive.stdout, check=True)
archive.stdout.close()
if archive.wait():
    raise SystemExit('git archive failed')
expected = json.loads((pack / 'baseline-headers.json').read_text())['headers']
actual = {str(p.relative_to(base)): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in sorted(base.rglob('*.hpp'))}
assert actual == expected, 'baseline header hash mismatch'
for tree, patch in [('instrumented', 'range-instrumentation.patch'), ('base32', 'window-align32.patch')]:
    shutil.copytree(base, work / tree)
    subprocess.run(['git', 'apply', '--no-index', str(pack / patch)], cwd=work / tree, check=True)
for name in ['leaf_ifma.hpp', 'check.cpp', 'micro.cpp', 'range.cpp', 'alignment_check.cpp']:
    shutil.copy2(pack / name, work / name)
for name in ['build.py', 'bench.py']:
    source = (pack / name).read_text()
    assert source.count("Path('/tmp/fps-ifma-leaf-probe')") == 1
    (work / name.replace('.py', '_local.py')).write_text(
        source.replace("Path('/tmp/fps-ifma-leaf-probe')", repr(work).replace('PosixPath', 'Path')))
shutil.copy2(pack / 'check_extra.py', work / 'check_extra.py')
proof = json.loads((pack / 'range-proof.json').read_text())
assert hashlib.sha256((work / 'instrumented/components/fps_blocked.hpp').read_bytes()).hexdigest() == proof['range_trace_instrumented_header_sha256']
print(json.dumps({'baseline': BASELINE, 'baseline_headers_matched': len(actual), 'work': str(work)}))
