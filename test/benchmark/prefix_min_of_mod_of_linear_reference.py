import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--maspypy-ref', default='9aaac53044867f5a22ca09e58d98a5db065ebb33')
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'
    query = urllib.parse.urlencode({'problem': 'min_of_mod_of_linear', 'status': 'AC',
                                    'order': '+time', 'limit': 4})
    listing = get(api + '/submissions?' + query)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(),
              'listing': listing, 'references': []}
    ids = [item['id'] for item in listing['submissions']]
    for submission in dict.fromkeys([*ids, 405749, 389955, 217811, 335228, 100952]):
        cached = args.cache / f'{submission}.json'
        detail = json.loads(cached.read_text()) if cached.exists() else get(api + f'/submissions/{submission}')
        source = detail['source'].encode()
        (args.cache / f'{submission}.cpp').write_bytes(source)
        (args.cache / f'{submission}.json').write_text(json.dumps(detail, indent=2) + '\n')
        result['references'].append({**detail['overview'],
            'source_sha256': hashlib.sha256(source).hexdigest(),
            'url': f'https://judge.yosupo.jp/submission/{submission}'})
    tree = get(f'https://api.github.com/repos/maspypy/library/git/trees/{args.maspypy_ref}?recursive=1')
    paths = [item['path'] for item in tree['tree'] if 'min_of_mod_of_linear' in item['path'] or 'min_of_linear' in item['path']]
    result['maspypy_tree_sha'] = tree['sha']
    result['maspypy_paths'] = paths
    result['maspypy_sources'] = []
    for path in paths:
        if path.endswith('.hpp'):
            url = f'https://raw.githubusercontent.com/maspypy/library/{tree["sha"]}/{path}'
            with urllib.request.urlopen(url, timeout=60) as response:
                source = response.read()
            (args.cache / Path(path).name).write_bytes(source)
            result['maspypy_sources'].append({'path': path, 'url': url,
                'source_sha256': hashlib.sha256(source).hexdigest()})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
