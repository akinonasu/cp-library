import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    return json.load(urllib.request.urlopen(url, timeout=90))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'
    query = urllib.parse.urlencode(dict(problem='common_interval_decomposition_tree', status='AC', order='+time', limit=8))
    listing = get(api + '/submissions?' + query)
    data = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing=listing, references=[])
    for item in listing['submissions']:
        identity = item['id']
        detail = get(api + '/submissions/' + str(identity))
        source = detail['source'].encode()
        (args.cache / (str(identity) + '.cpp')).write_bytes(source)
        data['references'].append(dict(**detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(),
                                       url='https://judge.yosupo.jp/submission/' + str(identity)))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(data, indent=2) + '\n')
    print([(r['id'], r['lang'], r['time']) for r in data['references']])


if __name__ == '__main__':
    main()
