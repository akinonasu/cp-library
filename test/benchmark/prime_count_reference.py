import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(url, timeout=60) as stream:
        return json.load(stream)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp/submissions'
    query = urllib.parse.urlencode(dict(problem='counting_primes', status='AC', order='+time', limit=5))
    listing = get(api + '?' + query)
    report = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': []}
    for row in listing['submissions']:
        detail = get(api + '/' + str(row['id']))
        source = detail['source'].encode()
        suffix = '.cpp' if row['lang'] == 'cpp' else '.txt'
        (args.cache / (str(row['id']) + suffix)).write_bytes(source)
        report['references'].append({**detail['overview'], 'url': 'https://judge.yosupo.jp/submission/' + str(row['id']),
                                     'source_sha256': hashlib.sha256(source).hexdigest()})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
