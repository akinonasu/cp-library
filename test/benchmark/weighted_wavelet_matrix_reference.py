import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
    with urllib.request.urlopen(request, timeout=90) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, default=Path('.verify-helper/cache/wm-weight/reference'))
    parser.add_argument('--output', type=Path, default=Path('test/benchmark/results/weighted_wavelet_matrix_reference.json'))
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    problem = 'point_add_rectangle_sum'
    url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=100))
    listing = get(url)
    references = []
    for row in listing['submissions'][:5]:
        detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(row['id']))
        source = detail['source'].encode()
        (args.cache / (str(row['id']) + '.cpp')).write_bytes(source)
        references.append(dict(**detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(),
                               url='https://judge.yosupo.jp/submission/' + str(row['id'])))
        print(row['id'], row.get('user_name'), row['time'], flush=True)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), problem=problem, listing_url=url,
                  listing=listing, references=references)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
