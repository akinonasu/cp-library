import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'}), timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'
    query = urllib.parse.urlencode({'problem': 'rational_approximation', 'status': 'AC', 'order': '+time', 'limit': 4})
    listing = get(api + '/submissions?' + query)
    metadata = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': [],
                'boost_documentation': 'https://www.boost.org/doc/libs/1_85_0/libs/rational/rational.html'}
    for item in listing['submissions']:
        identity = item['id']
        path = args.cache / f'{identity}.json'
        detail = json.loads(path.read_text()) if path.exists() else get(api + f'/submissions/{identity}')
        path.write_text(json.dumps(detail, indent=2) + '\n')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        metadata['references'].append({**detail['overview'],
            'source_sha256': hashlib.sha256(source).hexdigest(),
            'url': f'https://judge.yosupo.jp/submission/{identity}'})
    url = 'https://raw.githubusercontent.com/boostorg/rational/boost-1.85.0/include/boost/rational.hpp'
    with urllib.request.urlopen(url, timeout=60) as response:
        source = response.read()
    (args.cache / 'boost_rational.hpp').write_bytes(source)
    metadata['boost_source'] = {'url': url, 'sha256': hashlib.sha256(source).hexdigest()}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(metadata, indent=2) + '\n')
    print(json.dumps(metadata, indent=2))


if __name__ == '__main__':
    main()
