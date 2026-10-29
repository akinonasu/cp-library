"""Avoid allocating a macOS runner for changes without C++ verification inputs."""

import json
import os
from pathlib import Path
import re
import subprocess


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


def documentation_only(path):
    return path.startswith(('docs/', 'submissions/')) or path in ('README.md', 'AGENTS.md')


def needed(root, event, context):
    kind = context.get('GITHUB_EVENT_NAME')
    if kind not in ('push', 'pull_request'):
        return True
    head = context.get('GITHUB_SHA', '')
    pull = event.get('pull_request', {})
    base = event.get('before', '') if kind == 'push' else pull.get('base', {}).get('sha', '')
    source = head if kind == 'push' else pull.get('head', {}).get('sha', '')
    if not all(isinstance(value, str) and re.fullmatch('[0-9a-f]{40}', value)
               and value != '0' * 40 for value in (base, head, source)):
        return True
    try:
        message = git(root, 'show', '-s', '--format=%B', source).decode()
        if 'Verify-Mode: no-cache' in message:
            return True
        diff = base + ('..' if kind == 'push' else '...') + head
        paths = git(root, 'diff', '--no-renames', '--name-only', '-z', diff).decode().split('\0')
    except (subprocess.CalledProcessError, UnicodeError):
        return True
    return any(not documentation_only(path) for path in paths if path)


def main():
    root = Path(__file__).resolve().parent.parent
    try:
        event = json.loads(Path(os.environ['GITHUB_EVENT_PATH']).read_text())
        run = needed(root, event, os.environ)
    except (KeyError, OSError, ValueError, TypeError, AttributeError):
        run = True
    value = 'true' if run else 'false'
    with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
        output.write('needed=' + value + '\n')
    print('macOS verification required' if run else 'Skipping macOS: only documentation/submission changes')


if __name__ == '__main__':
    main()
