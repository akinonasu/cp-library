import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request

PROBLEMS = ['convolution_mod', 'inv_of_formal_power_series', 'log_of_formal_power_series',
            'exp_of_formal_power_series', 'pow_of_formal_power_series', 'sqrt_of_formal_power_series',
            'division_of_polynomials', 'kth_term_of_linearly_recurrent_sequence',
            'composition_of_formal_power_series', 'composition_of_formal_power_series_large',
            'compositional_inverse_of_formal_power_series', 'compositional_inverse_of_formal_power_series_large',
            'multipoint_evaluation', 'polynomial_interpolation', 'polynomial_taylor_shift',
            'inv_of_polynomials']


def get(url):
    return json.load(urllib.request.urlopen(url, timeout=90))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--limit', type=int, default=3)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'

    def fetch(problem):
        try:
            listing = get(api + '/submissions?' + urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=args.limit)))
            refs = []
            target = args.cache / problem
            target.mkdir(exist_ok=True)
            for row in [row for row in listing['submissions'] if row['lang'] == 'cpp'][:args.limit]:
                detail = get(api + '/submissions/' + str(row['id']))
                source = detail['source'].encode()
                (target / (str(row['id']) + '.cpp')).write_bytes(source)
                refs.append(dict(**detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(), url='https://judge.yosupo.jp/submission/' + str(row['id'])))
            print(problem, [(r['id'], r['lang'], r['time']) for r in refs], flush=True)
            return problem, dict(listing=listing, references=refs)
        except Exception as error:
            print(problem, str(error), flush=True)
            return problem, dict(error=str(error))

    data = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), problems=dict(ThreadPoolExecutor(max_workers=4).map(fetch, PROBLEMS)))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(data, indent=2) + '\n')


if __name__ == '__main__':
    main()
