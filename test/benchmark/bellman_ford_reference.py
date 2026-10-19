from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': 'cp-library'}), timeout=60) as response:
        return response.read()


def main():
    root = Path(__file__).resolve().parents[2]
    cache = root / '.verify-helper/cache/bellman_ford_references'
    cache.mkdir(parents=True, exist_ok=True)
    output = root / 'test/benchmark/results/bellman_ford'
    query = urllib.parse.urlencode({'problem': 'shortest_path', 'status': 'AC', 'order': '+time', 'limit': 3})
    listing = json.loads(get('https://v3.api.judge.yosupo.jp/submissions?' + query))
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'nonnegative_listing': listing,
              'nonnegative_references': [], 'sources': [], 'paper': 'https://doi.org/10.4230/LIPIcs.SEA.2025.10'}
    for item in listing['submissions']:
        detail = json.loads(get('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id'])))
        source = detail['source'].encode()
        (cache / (str(item['id']) + '.cpp')).write_bytes(source)
        result['nonnegative_references'].append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                                                 'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
    repositories = [('networkx/networkx', 'networkx/algorithms/shortest_paths/weighted.py'),
                    ('PaoloLRinaldi/negative_weight_SSSP', None)]
    for repository, path in repositories:
        head = json.loads(get('https://api.github.com/repos/' + repository + '/commits?per_page=1'))[0]['sha']
        if path is None:
            tree = json.loads(get('https://api.github.com/repos/' + repository + '/git/trees/' + head + '?recursive=1'))
            candidates = [item['path'] for item in tree['tree'] if item['type'] == 'blob' and
                          any(word in item['path'].lower() for word in ['goldberg', 'gor.', 'bellman'])]
            print(repository, candidates, flush=True)
        else:
            candidates = [path]
        for name in candidates:
            url = 'https://raw.githubusercontent.com/' + repository + '/' + head + '/' + name
            source = get(url)
            (cache / (repository.split('/')[0] + '_' + name.replace('/', '_'))).write_bytes(source)
            result['sources'].append({'url': url, 'commit': head, 'sha256': hashlib.sha256(source).hexdigest()})
    (output / 'reference.json').write_text(json.dumps(result, indent=2) + '\n')
    print([(r['overview']['id'], r['overview']['time']) for r in result['nonnegative_references']], flush=True)


if __name__ == '__main__':
    main()
