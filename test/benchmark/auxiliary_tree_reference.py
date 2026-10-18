import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(url, timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode({'problem': 'lca', 'status': 'AC', 'order': '+time', 'limit': 20})
    listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
    selected = listing['submissions'][:1] + [r for r in listing['submissions'] if r['lang'] == 'cpp'][:3]
    references = []
    for identity in dict.fromkeys(r['id'] for r in selected):
        detail = get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        references.append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                           'url': f'https://judge.yosupo.jp/submission/{identity}'})
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problem': 'lca',
              'listing': listing, 'references': references,
              'auxiliary_tree_references': ['https://atcoder.jp/contests/abc340/editorial/9249',
                                           'https://atcoder.jp/contests/abc359/editorial/10259?lang=ja']}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print([(r['id'], r['time'], r['lang']) for r in references], flush=True)


if __name__ == '__main__':
    main()
