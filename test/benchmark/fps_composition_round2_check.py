import fcntl
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('/tmp/fps-composition-round2-final-check')
WORK.mkdir(parents=True, exist_ok=True)
BIN = WORK / 'bin'
BIN.mkdir(exist_ok=True)
wrapper = BIN / 'g++'
wrapper.write_text('#!/bin/sh\nexec /usr/bin/g++ -Werror "$@"\n')
wrapper.chmod(0o755)
OUT = ROOT / 'test/benchmark/results/fps_composition_round2_check.json'
LOG = OUT.with_suffix('.log')
sources = ['test/unit/fps_composition_blocked.cpp', 'test/unit/fps_composition_tables.cpp']
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
def hashes():
    paths = [p for folder in ('fps', 'components', 'math', 'convolution') for p in (ROOT / folder).rglob('*.hpp')]
    paths += [ROOT / name for name in sources]
    return {str(p.relative_to(ROOT)): sha(p) for p in sorted(paths)}
env = os.environ.copy()
env['PATH'] = str(BIN) + os.pathsep + env['PATH']
env['ASAN_OPTIONS'], env['UBSAN_OPTIONS'] = 'detect_leaks=0', 'halt_on_error=1'
result = dict(compiler=subprocess.check_output(['/usr/bin/g++', '--version'], text=True).splitlines()[0],
              harness_sha256=sha(Path(__file__)), unit_extra_flags=['-Werror'],
              environment={key: env[key] for key in ('ASAN_OPTIONS', 'UBSAN_OPTIONS')}, checks=[])
with LOG.open('w') as log, open('/tmp/cp-fps-tuning.lock', 'w') as lock:
    for sanitize in (False, True):
        for source in sources:
            selection = WORK / 'selection.json'
            selection.write_text(json.dumps(dict(headers=[], unit=[source], sanitize=[source])))
            cmd = [sys.executable, str(ROOT / 'test/verify.py'), '--unit-only', '--cxx', 'g++', '--selection', str(selection)]
            if sanitize: cmd.append('--sanitize')
            fcntl.flock(lock, fcntl.LOCK_SH)
            try:
                before = hashes()
                log.write('COMMAND ' + json.dumps(cmd) + '\n'); log.flush()
                started = time.monotonic()
                p = subprocess.run(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, cwd=ROOT)
                after = hashes()
            finally: fcntl.flock(lock, fcntl.LOCK_UN)
            entry = dict(source=source, sanitize=sanitize, command=cmd, seconds=time.monotonic() - started,
                         exit_code=p.returncode, hashes_before=before, hashes_after=after, unchanged=before == after)
            result['checks'].append(entry)
            result['success'] = all(x['exit_code'] == 0 and x['unchanged'] for x in result['checks'])
            OUT.write_text(json.dumps(result, indent=2) + '\n')
            print(source, 'sanitize' if sanitize else 'normal', p.returncode, entry['unchanged'], flush=True)
            if not result['success']: raise RuntimeError(entry)
