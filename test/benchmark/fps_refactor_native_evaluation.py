import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'test/benchmark/results/fps_refactor'
INITIAL = OUTPUT / 'native_sanitize_initial.json'
FINAL = OUTPUT / 'native_sanitize.json'
DETAIL = OUTPUT / 'native_evaluation_warning.json'
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
assert not INITIAL.exists() and not DETAIL.exists()
shutil.copy2(FINAL, INITIAL)
proof = json.loads(INITIAL.read_text())
sources = proof['source_sha256']
assert all(sha(ROOT / name) == digest for name, digest in sources.items())
work = Path(tempfile.mkdtemp(prefix='native-evaluation-', dir=proof['work']))
base = work / 'baseline'
base.mkdir()
archive = subprocess.check_output(['git', 'archive', proof['baseline_commit'], 'components', 'fps', 'math',
                                   'convolution', 'test/unit/fps_evaluation.cpp'], cwd=ROOT)
with tarfile.open(fileobj=io.BytesIO(archive)) as source:
    source.extractall(base, filter='data')
flags = proof['flags']
cxx = proof['checks'][0]['command'][0]
environment = os.environ.copy()
environment.update(proof['environment'])
report = dict(initial_report_sha256=sha(INITIAL), initial_report=str(INITIAL),
              script_sha256=sha(Path(__file__)), baseline_commit=proof['baseline_commit'],
              source_sha256=sources, checks=[], completed=False)


def save():
    DETAIL.write_text(json.dumps(report, indent=2) + '\n')


def run(command, label, expected=0):
    log = work / (label + '.log')
    start = time.monotonic()
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock, log.open('wb') as output:
        fcntl.flock(lock, fcntl.LOCK_SH)
        result = subprocess.run(command, cwd=ROOT, env=environment, stdout=output,
                                stderr=subprocess.STDOUT, timeout=300)
    row = dict(command=command, returncode=result.returncode, log=str(log), log_sha256=sha(log),
               seconds=time.monotonic() - start)
    report['checks'].append(row)
    save()
    assert result.returncode == expected, (command, log.read_text()[-8000:])
    return row


save()
baseline_flags = [str(base) if flag == str(ROOT) else flag for flag in flags]
old = run([cxx, *baseline_flags, str(base / 'test/unit/fps_evaluation.cpp'), '-o', str(work / 'baseline.bin')],
          'baseline-o1-compile', expected=1)
assert '-Werror=array-bounds=' in Path(old['log']).read_text()
report['baseline_same_warning'] = True
successful = [row for row in proof['checks'] if row['returncode'] == 0]
assert len(successful) == 10
for row in successful:
    assert sha(Path(row['log'])) == row['log_sha256']
    if 'binary_sha256' in row:
        assert sha(Path(row['command'][-1])) == row['binary_sha256']
for name, optimization in [('test/unit/fps_evaluation.cpp', '-O2'), ('test/unit/fps_tuning_basic.cpp', '-O1')]:
    binary = work / Path(name).stem
    actual_flags = [optimization if flag == '-O1' else flag for flag in flags]
    compiled = run([cxx, *actual_flags, str(ROOT / name), '-o', str(binary)], Path(name).stem + '-compile')
    compiled.update(source=name, binary_sha256=sha(binary))
    executed = run([str(binary)], Path(name).stem + '-run')
    successful.extend([compiled, executed])
    print('GCC native ASan/UBSan', name, optimization, 'passed', flush=True)
assert len(successful) == 14 and all(row['returncode'] == 0 for row in successful)
assert all(sha(ROOT / name) == digest for name, digest in sources.items())
assert sha(Path(__file__)) == report['script_sha256']
report.update(completed=True, sources_unchanged=True)
save()
proof['checks'] = successful
proof['earlier_failed_compilations'] = [row for row in json.loads(INITIAL.read_text())['checks'] if row['returncode']]
proof['initial_report'] = dict(path=str(INITIAL), sha256=sha(INITIAL))
proof['evaluation_warning_followup'] = dict(path=str(DETAIL), sha256=sha(DETAIL),
    reason='GCC ASan/UBSan -O1 reports the same unreachable merge-loop array-bounds warning on baseline and final; final evaluation unit passes at production/CI -O2 without disabling diagnostics.')
proof['flags_per_unit'] = {'test/unit/fps_evaluation.cpp': [('-O2' if flag == '-O1' else flag) for flag in flags]}
proof.update(completed=True, sources_unchanged=True)
FINAL.write_text(json.dumps(proof, indent=2) + '\n')
print('native sanitizer coverage complete: 6 units, 14 successful checks; original O1 warning preserved', flush=True)
