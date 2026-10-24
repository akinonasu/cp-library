import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import unittest

from ci import environment_key, no_cache_context


ROOT = Path(__file__).resolve().parent.parent


def fingerprint(root, environment):
    names = subprocess.check_output(['git', '-C', str(root), 'ls-files', '--cached', '--others', '--exclude-standard', '-z'])
    inputs = []
    for name in sorted(set(names.decode().split('\0')) - {''}):
        if name.startswith('docs/') or name in ('README.md', 'AGENTS.md'):
            continue
        path = root / name
        inputs.append([name, hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None])
    return hashlib.sha256(json.dumps([1, environment, inputs], sort_keys=True).encode()).hexdigest()


def run(root, path, key, *, enabled=True, force=False):
    try:
        previous = json.loads(path.read_text())
    except (OSError, ValueError):
        previous = {}
    if enabled and not force and previous == {'version': 1, 'fingerprint': key}:
        print('Reusing successful CI support tests with matching sources and tools', flush=True)
        return
    subprocess.run([sys.executable, '.verify-helper/support_tests.py', 'suite'], cwd=root, check=True)
    if enabled:
        path.write_text(json.dumps({'version': 1, 'fingerprint': key}) + '\n')


def suite(root):
    loader = unittest.TestLoader()
    tests = unittest.TestSuite()
    for path in sorted((root / '.verify-helper').glob('*_test.py')):
        if path.stem != 'code_highlight_test':
            tests.addTests(loader.loadTestsFromName(path.stem))
    return tests


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices=('key', 'run', 'suite'))
    parser.add_argument('--state', type=Path, default=ROOT / '.verify-helper/support-state.json')
    args = parser.parse_args()
    if args.command == 'suite':
        sys.exit(not unittest.TextTestRunner().run(suite(ROOT)).wasSuccessful())
    key = fingerprint(ROOT, environment_key())
    if args.command == 'key':
        if os.environ.get('GITHUB_OUTPUT'):
            with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
                output.write('key=' + key + '\n')
        print('CI support test key: ' + key, flush=True)
    else:
        run(ROOT, args.state, key, enabled=not no_cache_context(), force=os.environ.get('VERIFY_FULL') == 'true')


if __name__ == '__main__':
    main()
