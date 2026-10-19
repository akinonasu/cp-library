import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
BEFORE = '2a4d721b9a8fc2ed25a7cf05ff7312782a162653'
SAMPLES = [
    ('3 2\n1 2 3\n3 6 3\n3 1 4\n2 3 9\n', '10\n13\n'),
    ('5 1\n1 2 3 4 5\n1000000000 1000000000 1000000000 1000000000 1000000000\n1 1 1000000000\n', '5000000000\n'),
    ('10 10\n6 2 4 1 5 1 6 6 5 3\n45 65 71 52 86 52 48 60 40 98\n5 6 5\n8 4 34\n6 7 83\n1 3 21\n7 5 85\n7 4 51\n8 2 81\n2 7 54\n6 1 5\n8 6 30\n', '394\n379\n462\n457\n459\n414\n443\n479\n401\n396\n')]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native', '-ftrivial-auto-var-init=zero']
    sources = ['data_structure/advanced/partially_retroactive_priority_queue.hpp', 'components/splitmix64.hpp',
               'components/fast_io.hpp', 'test/fixtures/retroactive_scheduling.hpp',
               'test/benchmark/retroactive_priority_queue_scheduling.cpp',
               'test/benchmark/retroactive_priority_queue_changes.cpp', 'test/benchmark/retroactive_priority_queue_changes.py']
    environment = dict(compiler=subprocess.check_output(['g++', '--version'], text=True), flags=flags,
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu], before=BEFORE,
                       trials=args.trials, warmup=1, seed=5834509, scheduling_seed=36310496, order_seed=10496,
                       source_sha256={p: sha(ROOT / p) for p in sources})
    references = []
    for url in ['https://atcoder.jp/contests/abc363/submissions?f.Task=abc363_g&f.Status=AC&orderBy=time_consumption',
                'https://atcoder.jp/contests/abc363/submissions/55828915']:
        try:
            with urllib.request.urlopen(url, timeout=20) as response:
                data = response.read()
                references.append(dict(url=url, status=response.status, final_url=response.url,
                                       sha256=hashlib.sha256(data).hexdigest(), source_available=b'id="submission-code"' in data))
        except (urllib.error.URLError, TimeoutError) as error:
            references.append(dict(url=url, error=str(error)))
    environment['references'] = references
    rng = random.Random(10496)
    results = dict(kernel={}, scheduling={}, samples={})
    with tempfile.TemporaryDirectory(prefix='cp-retroactive-changes-', dir='/tmp') as directory:
        work = Path(directory)
        before = subprocess.check_output(['git', 'show', BEFORE + ':data_structure/partially_retroactive_priority_queue.hpp'], cwd=ROOT)
        environment['before_sha256'] = hashlib.sha256(before).hexdigest()
        (work / 'before.hpp').write_text(before.decode().replace('PARTIALLY_RETROACTIVE_PRIORITY_QUEUE_HPP', 'BEFORE_PRPQ_HPP')
                                         .replace('PartiallyRetroactivePriorityQueue', 'QueueBefore')
                                         .replace('"../components/splitmix64.hpp"', '"components/splitmix64.hpp"'))
        subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(work), str(ROOT / 'test/benchmark/retroactive_priority_queue_changes.cpp'), '-o', str(work / 'kernel')], check=True)
        subprocess.run(['g++', *flags, '-Wall', '-Wextra', '-Werror', str(ROOT / 'test/benchmark/retroactive_priority_queue_scheduling.cpp'), '-o', str(work / 'scheduling')], check=True)
        bundle = subprocess.check_output(['oj-bundle', str(ROOT / 'test/benchmark/retroactive_priority_queue_scheduling.cpp')], cwd=ROOT)
        (work / 'scheduling.cpp').write_bytes(bundle)
        environment['bundle_sha256'] = sha(work / 'scheduling.cpp')
        subprocess.run(['clang++', '-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', str(work / 'scheduling.cpp'), '-o', str(work / 'clang')], check=True)
        for name in ['scheduling', 'clang']:
            for data, expected in SAMPLES:
                actual = subprocess.check_output([work / name], input=data, text=True)
                if actual != expected:
                    raise RuntimeError((name, actual, expected))
            results['samples'][name] = len(SAMPLES)
        for pattern, kind in enumerate(['growing', 'balanced', 'steady']):
            samples = {name: [] for name in ['before', 'current', 'notified']}
            expected = None
            for trial in range(-1, args.trials):
                order = list(samples)
                rng.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([work / 'kernel', str(list(samples).index(name)), str(pattern)], text=True))
                    digest = row['checksum'], row['accepted'], row['bytes']
                    if expected is None:
                        expected = digest
                    if digest != expected:
                        raise RuntimeError((kind, name, digest, expected))
                    if trial >= 0:
                        samples[name].append(row)
            results['kernel'][kind] = dict(samples=samples, medians={name: {key: statistics.median(row[key] for row in rows) for key in ['build_ms', 'mixed_ms']} for name, rows in samples.items()})
            print(kind, results['kernel'][kind]['medians'], flush=True)
        for kind in ['random', 'one', 'all', 'alternating']:
            generator = random.Random(36310496)
            n = q = 100000
            def deadline(i):
                return 1 if kind == 'one' else n if kind == 'all' else (1 if i & 1 else n) if kind == 'alternating' else generator.randrange(1, n + 1)
            d = [deadline(i) for i in range(n)]
            p = [generator.randrange(1, 1000000001) for _ in range(n)]
            lines = [f'{n} {q}', ' '.join(map(str, d)), ' '.join(map(str, p))]
            for i in range(q):
                lines.append(f'{generator.randrange(1, n + 1)} {deadline(i)} {generator.randrange(1, 1000000001)}')
            inp = work / 'input'
            inp.write_text('\n'.join(lines) + '\n')
            output = work / 'output'
            samples, expected = [], None
            for trial in range(-1, args.trials):
                with inp.open('rb') as a, output.open('wb') as b:
                    start = time.perf_counter()
                    subprocess.run([work / 'scheduling'], stdin=a, stdout=b, check=True)
                    ms = (time.perf_counter() - start) * 1000
                digest = sha(output)
                if expected is None:
                    expected = digest
                    other = work / 'clang-output'
                    with inp.open('rb') as a, other.open('wb') as b:
                        subprocess.run([work / 'clang'], stdin=a, stdout=b, check=True)
                    if sha(other) != digest:
                        raise RuntimeError((kind, 'Clang output differs'))
                if expected != digest:
                    raise RuntimeError((kind, 'output differs'))
                if trial >= 0:
                    samples.append(ms)
            results['scheduling'][kind] = dict(input_sha256=sha(inp), output_sha256=expected, samples_ms=samples, median_ms=statistics.median(samples))
            print('scheduling', kind, statistics.median(samples), flush=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
