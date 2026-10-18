import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
FILES = ['math/comb.hpp', 'components/binomial_prefix_core.hpp', 'components/fps_factorials.hpp', 'math/mint.hpp', 'components/ntt.hpp']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def references(cache, output):
    def get(url):
        with urllib.request.urlopen(url, timeout=90) as stream:
            return json.load(stream)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), problems={})
    for problem in ['binomial_coefficient_prime_mod', 'polynomial_taylor_shift']:
        url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=3))
        listing = get(url)
        rows = []
        for row in listing['submissions']:
            identity = row['id']
            detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(identity))
            code = detail['source'].encode()
            dest = cache / f'{identity}.txt'
            dest.write_bytes(code)
            rows.append(dict(**detail['overview'], source_sha256=sha(dest), url=f'https://judge.yosupo.jp/submission/{identity}'))
        result['problems'][problem] = dict(listing_url=url, listing=listing, references=rows)
    output.write_text(json.dumps(result, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--output', type=Path, default=Path('test/benchmark/results/comb_factorials.json'))
    parser.add_argument('--references', action='store_true')
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    args = parser.parse_args()
    os.chdir(ROOT)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/comb-factorials/reference'
    cache.mkdir(parents=True, exist_ok=True)
    if args.references:
        references(cache, args.output)
        return
    if not args.baseline:
        parser.error('--baseline is required')
    baseline = json.loads(args.baseline.read_text())
    if 'baseline_sources' in baseline:
        baseline = baseline['baseline_sources']
    frozen = {p: sha(ROOT / p) for p in FILES + ['test/benchmark/comb_factorials.cpp', 'test/benchmark/comb_factorials.py']}
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=gnu++23', '-O2', '-DNDEBUG', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE',
             '-fconstexpr-depth=1024', '-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152', '-I.']
    if not args.generic:
        flags.append('-march=native')
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), flags=flags, trials=args.trials,
                  compiler=subprocess.check_output(['g++', '--version'], text=True),
                  cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu],
                  kernel=subprocess.check_output(['uname', '-a'], text=True), source_sha256=frozen,
                  baseline_sources=baseline, current_sources={p: (ROOT / p).read_text() for p in FILES},
                  order_seed=20261019, results=[], completed=False,
                  timer='Each fresh process warms the table once, then measures repeated complete generation; per-call time excludes checksums, allocation after warmup, I/O and process startup. Repeats=max(1,min(20000,10000000/(N+1))).')

    def save():
        temporary = args.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(report, indent=2) + '\n')
        temporary.replace(args.output)

    save()
    rng = random.Random(20261019)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock, tempfile.TemporaryDirectory(prefix='comb-factorials-') as directory:
        work = Path(directory)
        before = work / 'baseline'
        for path, code in baseline.items():
            dest = before / path
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_text(code)
        for n in [0, 254, 255, 256, 4096, 502022, 1048576, 4194304]:
            programs = {}
            for name in ['baseline', 'current']:
                binary = work / (name + str(n))
                options = []
                if name == 'baseline':
                    options = ['-DCOMB_HEADER="' + str(before / FILES[0]) + '"', '-DPREFIX_HEADER="' + str(before / FILES[1]) + '"']
                fcntl.flock(lock, fcntl.LOCK_EX)
                subprocess.run(['g++', *flags, *options, f'-DCOMB_MAX={n}', 'test/benchmark/comb_factorials.cpp', '-o', binary], check=True)
                fcntl.flock(lock, fcntl.LOCK_UN)
                programs[name] = binary
            for mode in ['comb', 'prefix']:
                samples = {name: [] for name in programs}
                checksums, repeats = set(), set()
                fcntl.flock(lock, fcntl.LOCK_EX)
                for trial in range(args.trials):
                    order = list(programs)
                    rng.shuffle(order)
                    for name in order:
                        ms, checksum, count = subprocess.check_output([programs[name], mode], text=True).split()
                        samples[name].append(float(ms))
                        checksums.add(checksum)
                        repeats.add(int(count))
                fcntl.flock(lock, fcntl.LOCK_UN)
                assert len(checksums) == len(repeats) == 1, (n, mode)
                row = dict(max_n=n, entries=n + 1, mode=mode, samples_ms=samples,
                           median_ms={name: statistics.median(xs) for name, xs in samples.items()},
                           checksum=checksums.pop(), repeats=repeats.pop())
                report['results'].append(row)
                save()
                print(n, mode, row['median_ms'], flush=True)
    for path, expected in frozen.items():
        assert sha(ROOT / path) == expected, path
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()
    print('All checksums and source hashes match', flush=True)


if __name__ == '__main__':
    main()
