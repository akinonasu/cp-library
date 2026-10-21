import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import pathlib
import re
import subprocess

import yaml


root = pathlib.Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--selection', type=pathlib.Path)
parser.add_argument('--jobs', type=int, default=1)
args = parser.parse_args()
if args.jobs < 1:
    parser.error('--jobs must be positive')
selection = json.loads(args.selection.read_text()) if args.selection else None
selected = set(selection.get('bundles', selection['headers'])) if selection is not None else None
headers = sorted(path for folder in ('components', 'convolution', 'fps', 'data_structure', 'graph', 'tree', 'math', 'sequence', 'string', 'geometry') for path in (root / folder).rglob('*.hpp'))
bundles = []
for header in headers:
    path = header.relative_to(root)
    doc = root / '.verify-helper/markdown' / (str(path) + '.md')
    data = yaml.safe_load(doc.read_text().split('---', 2)[1])
    assert data['title'] != str(path), f'{path}: documentation is missing'
    code = data['data']['bundledCode']
    assert 'Traceback (most recent call last)' not in code, f'{path}: bundle failed'
    assert len(code.encode('utf-8')) < 256 * 1024, f'{path}: bundle exceeds 256 KiB'
    copied = '//https://qandori.github.io/cp-library/' + path.as_posix() + '.html\n' + code
    assert len(copied.encode('utf-8')) < 256 * 1024, f'{path}: copied bundle exceeds 256 KiB'
    assert not re.search(r'^\s*#\s*include\s*"', code, re.MULTILINE), f'{path}: local include remains'
    if selected is None or path.as_posix() in selected:
        bundles.append(code)


def check(code):
    subprocess.run(['g++', '-std=c++23', '-fsyntax-only', '-x', 'c++', '-'], input=code, text=True, check=True, cwd=root)


with ThreadPoolExecutor(max_workers=args.jobs) as pool:
    futures = [pool.submit(check, code) for code in bundles]
    try:
        for future in as_completed(futures):
            future.result()
    except BaseException:
        for future in futures:
            future.cancel()
        raise
print(f'Checked documentation for {len(headers)} libraries; compiled {len(bundles)} affected bundles')
