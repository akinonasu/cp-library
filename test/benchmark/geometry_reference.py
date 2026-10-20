import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-geometry-audit'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problems': {}}
    for problem in ['static_convex_hull', 'closest_pair', 'furthest_pair', 'sort_points_by_argument', 'minimum_enclosing_circle']:
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 20})
        listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
        first = listing['submissions'][0]
        references = []
        for cpp in [item for item in listing['submissions'] if item['lang'] == 'cpp'][:2]:
            detail = get(f"https://v3.api.judge.yosupo.jp/submissions/{cpp['id']}")
            source = detail['source'].encode()
            (args.cache / f"{cpp['id']}.cpp").write_bytes(source)
            references.append({'overview': detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                               'url': f"https://judge.yosupo.jp/submission/{cpp['id']}"})
        if not references:
            raise RuntimeError('No C++ AC submission in the first 20 entries: ' + problem)
        result['problems'][problem] = {'fastest_all_languages': first, 'cpp': references}
        print(problem, [(item['overview']['id'], item['overview']['time']) for item in references], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
