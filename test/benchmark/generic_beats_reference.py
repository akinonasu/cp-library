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
    problem = 'range_chmin_chmax_add_range_sum'
    query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 20})
    listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
    references = []
    for row in [r for r in listing['submissions'] if r['lang'] == 'cpp'][:3]:
        detail = get(f"https://v3.api.judge.yosupo.jp/submissions/{row['id']}")
        source = detail['source'].encode()
        (args.cache / f"{row['id']}.cpp").write_bytes(source)
        references.append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                           'url': f"https://judge.yosupo.jp/submission/{row['id']}"})
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problem': problem,
              'listing': listing, 'references': references,
              'other_references': ['https://atcoder.jp/contests/abc256/editorial/4113',
                  'https://hitonanode.github.io/cplib-cpp/segmenttree/acl_beats.hpp.html']}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print([(r['id'], r['time']) for r in references], flush=True)


if __name__ == '__main__':
    main()
