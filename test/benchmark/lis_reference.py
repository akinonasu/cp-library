import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode({'problem': 'longest_increasing_subsequence', 'status': 'AC', 'order': '+time', 'limit': 3})
    with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions?' + query, timeout=30) as response:
        listing = json.load(response)
    references = []
    for item in listing['submissions']:
        with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id']), timeout=30) as response:
            detail = json.load(response)
        source = detail['source'].encode()
        (args.cache / (str(item['id']) + '.cpp')).write_bytes(source)
        references.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                           'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({'retrieved_at': datetime.now(timezone.utc).isoformat(),
                                      'listing': listing, 'references': references}, indent=2) + '\n')
    print([(r['overview']['id'], r['overview']['time']) for r in references])


if __name__ == '__main__':
    main()
