import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time
import urllib.parse
import urllib.request


ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/offline-dynamic-connectivity'
OUTPUT = ROOT / 'test/benchmark/results/offline_dynamic_connectivity'
HEADER = 'graph/connectivity/offline_dynamic_connectivity.hpp'


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def references():
    def get(url):
        with urllib.request.urlopen(url, timeout=60) as response:
            return json.load(response)
    url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(
        dict(problem='dynamic_graph_vertex_add_component_sum', status='AC', order='+time', limit=20))
    listing = get(url)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing_url=url, listing=listing, references=[])
    ids = list(dict.fromkeys([r['id'] for r in listing['submissions'] if r['lang'].startswith('cpp')][:3] + [402761]))
    for identity in ids:
        detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(identity))
        code = detail['source'].encode()
        (CACHE / f'{identity}.cpp').write_bytes(code)
        result['references'].append(dict(**detail['overview'], url=f'https://judge.yosupo.jp/submission/{identity}',
                                         source_sha256=hashlib.sha256(code).hexdigest()))
    (OUTPUT / 'references.json').write_text(json.dumps(result, indent=2) + '\n')


def custom_cases():
    n = 100000
    ops = [(0, v - 1, v) for v in range(1, n)] + [(3, n - 1)]
    ops += [(1, v - 1, v) for v in range(1, n)] + [(3, n - 1)]
    yield 'few_queries', n, ops
    n = 20000
    ops = []
    rng = random.Random(20261020)
    for _ in range(60000):
        u = rng.randrange(n - 1)
        ops.extend([(0, u, u + 1), (3, u), (1, u, u + 1)])
    yield 'churn', n, ops
    n = 1000
    ops = []
    for batch in range(200):
        u = batch % (n - 1)
        ops += [(0, u, u + 1)] * 100
        ops += [(3, u)] + [(1, u + 1, u)] * 99
        ops += [(3, u), (1, u, u + 1), (3, u)]
    yield 'parallel_edges', n, ops
    yield 'query_only', 100000, [(3, rng.randrange(100000)) for _ in range(100000)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', action='store_true')
    parser.add_argument('--problems', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--skip-reference-validation', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    CACHE.mkdir(parents=True, exist_ok=True)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    if args.references:
        references()
        return
    assert args.trials > 0
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-DONLINE_JUDGE', '-I', str(ROOT)]
    sources = [HEADER, 'components/offline_time_segments.hpp', 'components/radix_sort.hpp', 'data_structure/rollback_union_find.hpp',
               'test/offline_dynamic_connectivity.test.cpp', 'test/unit/offline_dynamic_connectivity.cpp',
               'test/unit/offline_time_segments.cpp', 'test/unit/radix_sort.cpp', 'test/benchmark/offline_dynamic_connectivity.cpp',
               'test/benchmark/offline_dynamic_connectivity.py', 'test/benchmark/offline_time_segments_baseline.hpp']
    binaries = {}
    for name, compiler, source in [('library', 'g++', ROOT / 'test/offline_dynamic_connectivity.test.cpp'),
                                    ('clang', 'clang++', ROOT / 'test/offline_dynamic_connectivity.test.cpp'),
                                    ('402761', 'g++', CACHE / '402761.cpp')]:
        binary = CACHE / name
        subprocess.run([compiler, *flags, str(source), '-o', str(binary)], check=True)
        binaries[name] = binary
    problem = args.problems / 'graph/dynamic_graph_vertex_add_component_sum'
    cases = sorted((problem / 'in').glob('*.in'))
    assert len(cases) == 19
    report = dict(compilers={c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']},
                  flags=flags, cpu=subprocess.check_output(['lscpu'], text=True), kernel=subprocess.check_output(['uname', '-a'], text=True),
                  affinity=[args.cpu], trials=args.trials, warmup=1, input_seed=20261020, order_seed=7461,
                  problem_commit=subprocess.check_output(['git', '-C', str(args.problems), 'rev-parse', 'HEAD'], text=True).strip(),
                  source_sha256={p: digest(ROOT / p) for p in sources}, reference_source_sha256=digest(CACHE / '402761.cpp'),
                  judge=[], end_to_end={}, api={}, complete=False)

    def save():
        (OUTPUT / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')

    for path in cases:
        expected = (problem / 'out' / (path.stem + '.out')).read_bytes().split()
        passed = []
        for name, binary in binaries.items():
            if name == '402761' and args.skip_reference_validation:
                continue
            with path.open('rb') as stream:
                actual = subprocess.check_output([str(binary)], stdin=stream, timeout=30)
            assert actual.split() == expected, (path, name)
            passed.append(name)
        report['judge'].append(dict(case=path.stem, input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), passed=passed))
    print('All 19 official cases passed for', report['judge'][0]['passed'], flush=True)
    save()
    rng = random.Random(7461)
    selected = ['link_and_cut_00', 'max_random_00', 'dense_00', 'uv_swapped_00', 'random_00']
    for name in selected:
        path = problem / 'in' / (name + '.in')
        samples = {mode: [] for mode in ['library', '402761']}
        for trial in range(args.trials + 1):
            order = list(samples)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    start = time.perf_counter_ns()
                    subprocess.run([str(binaries[mode])], stdin=stream, stdout=subprocess.DEVNULL, check=True)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                if trial:
                    samples[mode].append(elapsed)
        report['end_to_end'][name] = dict(milliseconds=samples, median_ms={k: statistics.median(v) for k, v in samples.items()})
        print(name, report['end_to_end'][name]['median_ms'], flush=True)
        save()
    vector_header = (ROOT / HEADER).read_text().replace('"../../components/offline_time_segments.hpp"',
        '"' + str(ROOT / 'test/benchmark/offline_time_segments_baseline.hpp') + '"').replace(
        '"../../data_structure/rollback_union_find.hpp"', '"' + str(ROOT / 'data_structure/rollback_union_find.hpp') + '"').replace(
        '"../../components/radix_sort.hpp"', '"' + str(ROOT / 'components/radix_sort.hpp') + '"')
    (CACHE / 'vector.hpp').write_text(vector_header)
    apis = {}
    for name, options in [('csr', []), ('vectors', ['-DOFFLINE_CONNECTIVITY_HEADER="' + str(CACHE / 'vector.hpp') + '"']),
                          ('all_times', ['-DOFFLINE_UNCOMPRESSED=1'])]:
        binary = CACHE / ('api_' + name)
        subprocess.run(['g++', *flags, *options, 'test/benchmark/offline_dynamic_connectivity.cpp', '-o', str(binary)], check=True)
        apis[name] = binary
    workload_paths = [(name, problem / 'in' / (name + '.in')) for name in selected]
    for name, n, ops in custom_cases():
        path = CACHE / (name + '.in')
        path.write_text(f'{n} {len(ops)}\n' + ' '.join(['1'] * n) + '\n' +
                        ''.join(' '.join(map(str, op)) + '\n' for op in ops))
        workload_paths.append((name, path))
    for name, path in workload_paths:
        samples = {mode: [] for mode in apis}
        expected = None
        for trial in range(args.trials + 1):
            order = list(apis)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    result = subprocess.check_output([str(apis[mode])], stdin=stream, text=True, timeout=30).split()
                if expected is None:
                    expected = result[1:]
                assert result[1:] == expected, (name, mode)
                if trial:
                    samples[mode].append(float(result[0]))
        memory = {}
        for mode, binary in apis.items():
            log = CACHE / (mode + '.rss')
            with path.open('rb') as stream:
                subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(log), str(binary)], stdin=stream,
                               stdout=subprocess.DEVNULL, check=True)
            memory[mode] = int(log.read_text())
        report['api'][name] = dict(input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), result=expected,
                                  milliseconds=samples, median_ms={k: statistics.median(v) for k, v in samples.items()}, peak_rss_kib=memory)
        print(name, report['api'][name]['median_ms'], memory, flush=True)
        save()
    for path, expected in report['source_sha256'].items():
        assert digest(ROOT / path) == expected, path
    report['complete'] = True
    save()


if __name__ == '__main__':
    main()
