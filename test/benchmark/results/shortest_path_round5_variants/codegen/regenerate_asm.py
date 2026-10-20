import argparse
import fcntl
import json
import os
from pathlib import Path
import subprocess

ARCHIVE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('--work', type=Path, required=True)
args = parser.parse_args()
work = args.work.resolve()
os.sched_setaffinity(0, {2})
flags = json.loads((ARCHIVE / 'screen_native.json').read_text())['flags']
lock = open('/tmp/cp-wm-opt.lock', 'w')
for name in ['old', 'current', 'skip', 'split', 'split_skip', 'destination', 'destination_local']:
    tree = work / name
    fcntl.flock(lock, fcntl.LOCK_EX)
    try:
        subprocess.run(['g++', *flags, '-I', str(tree), '-S', str(tree / 'actual.cpp'), '-o', str(tree / 'actual.s')], check=True)
    finally:
        fcntl.flock(lock, fcntl.LOCK_UN)
fcntl.flock(lock, fcntl.LOCK_EX)
try:
    subprocess.run(['g++', *flags, '-S', str(ARCHIVE / 'copy_probe.cpp'), '-o', str(work / 'copy_probe.s')], check=True)
finally:
    fcntl.flock(lock, fcntl.LOCK_UN)
print('Assembly regenerated in', work)
