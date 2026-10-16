import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('header')
parser.add_argument('unit', nargs='+')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='cp-benchmark-verify-') as directory:
    selection = Path(directory) / 'selection.json'
    selection.write_text(json.dumps({'headers': [args.header], 'unit': args.unit}))
    command = [sys.executable, str(root / 'test/verify.py'), '--unit-only', '--selection', str(selection)]
    subprocess.run([*command, '--cxx', 'g++', 'clang++'], cwd=root, check=True)
    environment = os.environ.copy()
    environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
    subprocess.run([*command, '--sanitize', '--cxx', 'clang++'], cwd=root, env=environment, check=True)
