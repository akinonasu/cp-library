import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(url, timeout=60) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode({'problem': 'line_add_get_min', 'status': 'AC', 'order': '+time', 'limit': 3})
    listing = json.loads(get('https://v3.api.judge.yosupo.jp/submissions?' + query))
    references = []
    for item in listing['submissions']:
        detail = json.loads(get('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id'])))
        source = detail['source'].encode()
        (args.cache / (str(item['id']) + '.cpp')).write_bytes(source)
        references.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                           'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
    commit = json.loads(get('https://api.github.com/repos/ekzhang/library/commits?path=kinetic_tournament.cpp&per_page=1'))[0]['sha']
    url = 'https://raw.githubusercontent.com/ekzhang/library/' + commit + '/kinetic_tournament.cpp'
    source = get(url)
    (args.cache / 'ekzhang.cpp').write_bytes(source)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': references,
              'kinetic': {'url': url, 'sha256': hashlib.sha256(source).hexdigest()}}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print([(r['overview']['id'], r['overview']['time']) for r in references])


if __name__ == '__main__':
    main()
