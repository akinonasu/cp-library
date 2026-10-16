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
    query = urllib.parse.urlencode({'problem': 'dynamic_graph_vertex_add_component_sum', 'status': 'AC', 'order': '+time', 'limit': 8})
    listing = get(api + '/submissions?' + query)
    metadata = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': []}
    identities = [entry['id'] for entry in listing['submissions']]
    for identity in [100559, 235063, 214503, 283414, 243102, 147932, 120555, 212940, 28024, 93624]:
        if identity not in identities:
            identities.append(identity)
    for identity in identities:
        path = args.cache / f'{identity}.json'
        detail = json.loads(path.read_text()) if path.exists() else get(api + f'/submissions/{identity}')
        path.write_text(json.dumps(detail, indent=2) + '\n')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        metadata['references'].append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                                       'url': f'https://judge.yosupo.jp/submission/{identity}'})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(metadata, indent=2) + '\n')
    print(json.dumps(metadata, indent=2))


if __name__ == '__main__':
    main()
