from concurrent.futures import ThreadPoolExecutor
import datetime
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


ROOT = Path(__file__).resolve().parents[2]


def main():
    previous = json.loads((ROOT / 'test/benchmark/results/fps_speed_reference.json').read_text())

    def fetch(item):
        problem, old = item
        ids = [row['id'] for row in old.get('references', []) if row['lang'] == 'cpp']
        if not ids:
            return problem, dict(skipped=True)
        url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=30))
        with urllib.request.urlopen(url, timeout=30) as response:
            raw = response.read()
        data = json.loads(raw)
        top = [row for row in data['submissions'] if row['lang'] == 'cpp'][:3]
        return problem, dict(url=url, raw_sha256=hashlib.sha256(raw).hexdigest(), listing=data,
                             top_cpp=top, recorded_reference_ids=ids,
                             fastest_unchanged=bool(top and top[0]['id'] == ids[0]))

    result = dict(retrieved_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  harness_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  problems=dict(ThreadPoolExecutor(max_workers=4).map(fetch, previous['problems'].items())))
    output = ROOT / 'test/benchmark/results/fps_round3_reference_heads.json'
    output.write_text(json.dumps(result, indent=2) + '\n')
    print({name: row.get('fastest_unchanged') for name, row in result['problems'].items()})


if __name__ == '__main__':
    main()
