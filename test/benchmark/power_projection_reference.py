import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url, json_data=True):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-power-projection'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response) if json_data else response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problems': {}}
    for problem in ('compositional_inverse_of_formal_power_series', 'compositional_inverse_of_formal_power_series_large'):
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 5})
        listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
        references = []
        for item in listing['submissions'][:3]:
            identity = item['id']
            detail = get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}')
            source = detail['source'].encode()
            (args.cache / f'{identity}.cpp').write_bytes(source)
            references.append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                               'url': f'https://judge.yosupo.jp/submission/{identity}'})
        result['problems'][problem] = {'listing': listing, 'references': references}
        print(problem, [(r['id'], r['time'], r['lang']) for r in references], flush=True)
    url = 'https://raw.githubusercontent.com/maspypy/library/main/poly/power_projection.hpp'
    source = get(url, False)
    (args.cache / 'maspypy_power_projection.hpp').write_bytes(source)
    result['library_reference'] = {'url': url, 'source_sha256': hashlib.sha256(source).hexdigest()}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
