import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import urllib.parse
import urllib.request


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--revision', default='56a7b9b')
    parser.add_argument('--cache', type=Path, default=Path('.verify-helper/cache/shared_optimization'))
    parser.add_argument('--references', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    args.cache.mkdir(parents=True, exist_ok=True)
    revision = subprocess.check_output(['git', 'rev-parse', args.revision], cwd=root, text=True).strip()
    pending = ['string/lcs.hpp', 'string/edit_distance.hpp', 'string/run_enumerate.hpp',
               'math/number_theory/is_prime.hpp', 'math/number_theory/factorize.hpp']
    sources = {}
    while pending:
        name = pending.pop()
        if name in sources:
            continue
        data = subprocess.check_output(['git', 'show', revision + ':' + name], cwd=root)
        sources[name] = hashlib.sha256(data).hexdigest()
        target = args.cache / 'baseline' / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        for include in re.findall(r'^\s*#include\s*"([^"]+)"', data.decode(), re.M):
            pending.append((root / Path(name).parent / include).resolve().relative_to(root).as_posix())
    (args.cache / 'baseline.json').write_text(json.dumps({'revision': revision, 'sources': sources}, indent=2) + '\n')
    if not args.references:
        return
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problems': {}}
    for problem in ['runenumerate', 'primality_test', 'factorize']:
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 3})
        with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions?' + query, timeout=30) as response:
            listing = json.load(response)
        references = []
        for item in listing['submissions']:
            with urllib.request.urlopen('https://v3.api.judge.yosupo.jp/submissions/' + str(item['id']), timeout=30) as response:
                detail = json.load(response)
            source = detail['source'].encode()
            (args.cache / (str(item['id']) + '.cpp')).write_bytes(source)
            references.append({'overview': detail['overview'], 'sha256': hashlib.sha256(source).hexdigest(),
                               'url': 'https://judge.yosupo.jp/submission/' + str(item['id'])})
        result['problems'][problem] = {'listing': listing, 'references': references}
    output = root / 'test/benchmark/results/shared_optimization/references.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
