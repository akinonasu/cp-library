import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(url, timeout=60) as stream:
        return json.load(stream)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp/submissions'
    report = {'utc': datetime.now(timezone.utc).isoformat(), 'problems': {}}
    for problem in ('sum_of_multiplicative_function', 'sum_of_multiplicative_function_large', 'sum_of_totient_function'):
        query = urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=3))
        listing = get(api + '?' + query)
        references = []
        selected = [row['id'] for row in listing['submissions'][:2]]
        fixed = {'sum_of_multiplicative_function': 400957, 'sum_of_multiplicative_function_large': 405757, 'sum_of_totient_function': 199449}
        for identity in dict.fromkeys([*selected, fixed[problem]]):
            detail = get(api + '/' + str(identity))
            source = detail['source'].encode()
            (args.cache / (str(identity) + '.cpp')).write_bytes(source)
            references.append({**detail['overview'], 'url': f'https://judge.yosupo.jp/submission/{identity}',
                               'source_sha256': hashlib.sha256(source).hexdigest()})
        report['problems'][problem] = {'listing': listing, 'references': references}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
