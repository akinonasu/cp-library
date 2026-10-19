import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(url, timeout=60) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    problems = {}
    for problem in ['range_linear_add_range_min', 'range_add_range_min']:
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 3})
        listing = json.loads(get('https://v3.api.judge.yosupo.jp/submissions?' + query))
        references = []
        for item in listing['submissions']:
            detail = json.loads(get('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id'])))
            source = detail['source'].encode()
            (args.cache / (str(item['id']) + '.cpp')).write_bytes(source)
            references.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                               'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
        problems[problem] = {'listing': listing, 'references': references}
        print(problem, [(r['overview']['id'], r['overview']['time']) for r in references], flush=True)
    official = {'editorial': 'https://atcoder.jp/contests/jsc2024-final/editorial/10766',
                'sample_submission': 'https://atcoder.jp/contests/jsc2024-final/submissions/56754335'}
    try:
        source = get(official['sample_submission'])
        (args.cache / 'official_submission.html').write_bytes(source)
        official['retrieved_html_sha256'] = hashlib.sha256(source).hexdigest()
    except Exception as error:
        official['source_fetch_error'] = str(error)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({'retrieved_at': datetime.now(timezone.utc).isoformat(),
                                      'official': official, 'problems': problems}, indent=2) + '\n')


if __name__ == '__main__':
    main()
