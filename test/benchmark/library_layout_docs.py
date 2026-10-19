import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import yaml


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.verify-helper'))
from documentation import generate


if __name__ == '__main__':
    os.chdir(ROOT)
    output = ROOT / 'test/benchmark/results/library_layout'
    manifest = json.loads((output / 'manifest.json').read_text())
    selection_path = output / 'selection.json'
    selection = json.loads(selection_path.read_text())
    previous = json.loads((output / 'docs.json').read_text()) if (output / 'docs.json').exists() else {}
    headers = set(selection['state']['targets']['headers'])
    documentation = {}
    for path in (ROOT / 'docs').glob('*.md'):
        text = path.read_text()
        if text.startswith('---\n'):
            data = yaml.safe_load(text.split('---', 2)[1])
            source = data.get('documentation_of', '').removeprefix('//')
            if source:
                assert source not in documentation, source
                documentation[source] = data
    assert headers == set(documentation)
    for before, after in manifest['moves'].items():
        redirects = documentation[after]['redirect_from']
        if isinstance(redirects, str):
            redirects = [redirects]
        assert '/' + before + '.html' in redirects
    readme_entries = set(re.findall(r'`((?:components|convolution|fps|data_structure|graph|tree|math|string)/[^`]+\.hpp)`',
                                    (ROOT / 'README.md').read_text()))
    assert headers == readme_entries, (headers - readme_entries, readme_entries - headers)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        markdown = ROOT / '.verify-helper/markdown'
        for path in markdown.rglob('*.hpp.md'):
            source = path.relative_to(markdown).as_posix().removesuffix('.md')
            if source.split('/')[0] in {'components', 'convolution', 'fps', 'data_structure', 'graph', 'tree', 'math', 'string'} and source not in headers:
                assert path.resolve().is_relative_to(markdown.resolve())
                path.unlink()
        generate(selection)
        hashes = {}
        for name in selection['bundles']:
            text = (markdown / (name + '.md')).read_text()
            data = yaml.safe_load(text.split('---', 2)[1])
            code = data['data']['bundledCode']
            hashes[name] = hashlib.sha256(code.encode()).hexdigest()
        dirty = [name for name in selection['bundles'] if previous.get('result') != 'passed'
                 or previous.get('bundle_source_sha256', {}).get(name) != hashes[name]]
        checking = dict(selection, bundles=dirty)
        checking_path = ROOT / '.verify-helper/cache/library_layout_docs_selection.json'
        checking_path.write_text(json.dumps(checking, indent=2) + '\n')
        subprocess.run([sys.executable, '.verify-helper/check-docs.py', '--selection', str(checking_path)], check=True)
    index = yaml.safe_load((ROOT / '.verify-helper/markdown/index.md').read_text().split('---', 2)[1])
    categories = index['data']['libraryCategories']
    category_paths = {category['name']: [page['path'] for page in category['pages']] for category in categories}
    for path in headers:
        assert path in category_paths[str(Path(path).parent)], path
    result = dict(result='passed', libraries=len(headers), old_urls_redirected=len(manifest['moves']),
                  bundles_compiled=len(dirty), bundles_reused=len(hashes) - len(dirty),
                  bundles_verified=len(hashes), bundle_source_sha256=hashes, readme_complete=True,
                  categories={name: len(paths) for name, paths in category_paths.items()})
    (output / 'docs.json').write_text(json.dumps(result, indent=2) + '\n')
    print('All categories and old URL redirects passed', flush=True)
