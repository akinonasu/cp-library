import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def fetch(url):
    with urllib.request.urlopen(url, timeout=30) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    repo = 'brunomaletta/DynamicPatternMatching'
    commit = json.loads(fetch('https://api.github.com/repos/' + repo + '/commits?per_page=1'))[0]['sha']
    tree = json.loads(fetch('https://api.github.com/repos/' + repo + '/git/trees/' + commit + '?recursive=1'))
    sources = []
    for entry in tree['tree']:
        path = entry['path']
        if entry['type'] != 'blob' or not (path.startswith('src/') or path in ['README.md', 'sample.cpp']):
            continue
        url = 'https://raw.githubusercontent.com/' + repo + '/' + commit + '/' + path
        source = fetch(url)
        target = args.cache / 'author' / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(source)
        sources.append({'path': path, 'url': url, 'sha256': hashlib.sha256(source).hexdigest()})
    references = []
    query = urllib.parse.urlencode({'problem': 'suffixarray', 'status': 'AC', 'order': '+time', 'limit': 3})
    listing = json.loads(fetch('https://v3.api.judge.yosupo.jp/submissions?' + query))
    for item in listing['submissions']:
        detail = json.loads(fetch('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id'])))
        source = detail['source'].encode()
        (args.cache / (str(item['id']) + '.cpp')).write_bytes(source)
        references.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                           'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({'retrieved_at': datetime.now(timezone.utc).isoformat(),
                                      'paper': 'https://arxiv.org/html/2506.11318v1',
                                      'author_commit': commit, 'author_sources': sources,
                                      'listing': listing, 'references': references}, indent=2) + '\n')
    print('author', commit)
    print([s['path'] for s in sources])
    print([(r['overview']['id'], r['overview']['time']) for r in references])


if __name__ == '__main__':
    main()
