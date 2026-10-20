import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import re
import statistics
import subprocess
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/bipartite_matching'
OUT = ROOT / 'test/benchmark/results/bipartite_matching'


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def custom_cases():
    rng = random.Random(20261020)
    result = []
    def save(name, l, r, edges):
        path = CACHE / (name + '.in')
        with path.open('w') as stream:
            stream.write(f'{l} {r} {len(edges)}\n')
            for u, v in edges:
                stream.write(f'{u} {v}\n')
        result.append(path)
    n = 100000
    save('custom_long_chain', n, n, [(i, i + 1) for i in range(n - 1)] + [(i, i) for i in range(n)])
    edges = [divmod(i, 1000) for i in rng.sample(range(n * 1000), 200000)]
    save('custom_unbalanced', n, 1000, edges)
    save('custom_unbalanced_transpose', 1000, n, [(v, u) for u, v in edges])
    save('custom_dense', 700, 700, [divmod(i, 700) for i in rng.sample(range(700 * 700), 200000)])
    save('custom_empty', n, n, [])
    save('custom_hub', n, n, [(i, 0) for i in range(n)])
    edges = [(rng.randrange(50000), rng.randrange(50000)) for _ in range(1000)] * 200
    rng.shuffle(edges)
    save('custom_duplicates', 50000, 50000, edges)
    edges = [(u, v) for base in range(0, n, 2) for u in range(base, base + 2) for v in range(base, base + 2)]
    rng.shuffle(edges)
    save('custom_disconnected_squares', n, n, edges)
    return result


def main():
    from onlinejudge.service.library_checker import LibraryCheckerService
    parser = argparse.ArgumentParser()
    parser.add_argument('--repeats', type=int, default=7)
    parser.add_argument('--screen', action='store_true')
    parser.add_argument('--io-only', action='store_true')
    args = parser.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    CACHE.mkdir(parents=True, exist_ok=True)
    for identity in [396318, 396316, 396312, 339108]:
        path = CACHE / (str(identity) + '.json')
        if not path.exists():
            request = urllib.request.Request(f'https://v3.api.judge.yosupo.jp/submissions/{identity}', headers={'User-Agent':'cp-library'})
            detail = json.load(urllib.request.urlopen(request))
            path.write_text(json.dumps(detail, indent=2) + '\n')
            (CACHE / (str(identity) + '.cpp')).write_text(detail['source'])
    references = []
    for identity in [396318, 396316, 396312, 339108]:
        detail = json.loads((CACHE / (str(identity) + '.json')).read_text())
        references.append({'overview': detail['overview'], 'url': f'https://judge.yosupo.jp/submission/{identity}',
                           'source_sha256': sha(CACHE / (str(identity) + '.cpp'))})
    (OUT / 'references.json').write_text(json.dumps({'retrieved_at':datetime.now(timezone.utc).isoformat(),
        'listing':json.loads((CACHE/'listing.json').read_text()) if (CACHE/'listing.json').exists() else None,
        'references':references},indent=2)+'\n')
    source = (CACHE / '396318.cpp').read_text()
    core = source[source.index('constexpr uint32_t N'):source.index('std::array<uint32_t, 2> oriEdge')]
    (CACHE / 'reference_core.hpp').write_text('#include <bits/stdc++.h>\n' + core)
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-I', str(ROOT)]
    header = (ROOT / 'graph/matching/bipartite_matching.hpp').read_text()
    if args.io_only:
        report = json.loads((OUT / 'measurements.json').read_text())
        for name in ['graph/matching/bipartite_matching.hpp', 'components/bipartite_matching_core.hpp', 'test/benchmark/bipartite_matching_api.cpp']:
            assert sha(ROOT / name) == report['sources'][name], name
        binary = CACHE / 'judge-bulk'
        subprocess.run(['g++', *flags, 'test/bipartite_matching.test.cpp', '-o', str(binary)], cwd=ROOT, check=True)
        problem = Path(LibraryCheckerService._get_cloned_repository_path()) / 'graph/bipartitematching'
        rng = random.Random(20261020)
        for case in report['cases']:
            path = CACHE / (case['case'] + '.in') if case['custom'] else problem / 'in' / (case['case'] + '.in')
            expected = CACHE / (case['case'] + '.out') if case['custom'] else problem / 'out' / (case['case'] + '.out')
            assert sha(path) == case['input_sha256']
            values = {'ours': [], 'reference': []}
            for trial in range(-1, args.repeats):
                modes = list(values)
                rng.shuffle(modes)
                for mode in modes:
                    command = binary if mode == 'ours' else CACHE / 'reference'
                    output = CACHE / (mode + '-bulk.out')
                    with path.open('rb') as stream, output.open('wb') as destination:
                        start = time.perf_counter()
                        subprocess.run(['taskset','-c','2',str(command)],stdin=stream,stdout=destination,check=True)
                        elapsed = (time.perf_counter()-start)*1000
                    if trial == -1:
                        checked = subprocess.run([str(problem/'checker'),str(path),str(output),str(expected)],capture_output=True)
                        assert checked.returncode == 0, (case['case'],mode,checked.stderr)
                    if trial >= 0:
                        values[mode].append(elapsed)
            case['full_scalar_ms'] = case['full_ms']
            case['full_ms'] = values
            print(case['case'], {mode:round(statistics.median(v),3) for mode,v in values.items()},flush=True)
        report['api_sources'] = report['sources']
        report['sources'] = {name:sha(ROOT/name) for name in report['sources']}
        report['sources'].update({name:sha(ROOT/name) for name in ['components/fast_io.hpp','components/fast_io_bulk.hpp']})
        report['io_update'] = 'Only full-program timings repeated after switching the driver to bulk I/O; API sources checked unchanged'
        (OUT/'measurements.json').write_text(json.dumps(report,indent=2)+'\n')
        return
    baseline_core = re.sub(r'inline int augment\(.*?\n}\n',
        'inline int augment(const CSR& graph, std::vector<int>& left, std::vector<int>& right) {\n'
        '\treturn hopcroft_karp(graph, left, right).added;\n}\n',
        (ROOT/'components/bipartite_matching_core.hpp').read_text(), count=1, flags=re.S)
    baseline = baseline_core.replace('BIPARTITE_MATCHING_CORE_HPP','BASELINE_BIPARTITE_MATCHING_CORE_HPP') + '\n' + header.replace(
        '#include "../../components/bipartite_matching_core.hpp"','').replace('BIPARTITE_MATCHING_HPP','BASELINE_BIPARTITE_MATCHING_HPP').replace(
        'BipartiteMatching','BaselineBipartiteMatching')
    (CACHE/'baseline.hpp').write_text(baseline.replace('bipartite_matching_detail','baseline_bipartite_matching_detail'))
    variants = {'ours': None}
    if args.screen:
        variants.update(no_transpose=('bool transpose = l_ > r_;', 'bool transpose = false;'),
                        no_greedy=('if (left_[u] < 0 && right_[v] < 0)', 'if (false && left_[u] < 0 && right_[v] < 0)'),
                        budget4=('transpose ? left_ : right_);', 'transpose ? left_ : right_, 4);'),
                        budget64=('transpose ? left_ : right_);', 'transpose ? left_ : right_, 64);'),
                        roots0=('transpose ? left_ : right_);', 'transpose ? left_ : right_, 16, 0);'),
                        roots16=('transpose ? left_ : right_);', 'transpose ? left_ : right_, 16, 16);'),
                        roots64=('transpose ? left_ : right_);', 'transpose ? left_ : right_, 16, 64);'))
    binaries = {}
    for mode, replacement in variants.items():
        extra = []
        if replacement:
            modified = header.replace(*replacement).replace('../../components/', 'components/')
            path = CACHE / (mode + '.hpp')
            path.write_text(modified)
            extra = ['-DBIPARTITE_MATCHING_HEADER="' + str(path) + '"']
        binary = CACHE / ('api-' + mode)
        subprocess.run(['g++', *flags, *extra, 'test/benchmark/bipartite_matching_api.cpp', '-o', str(binary)], cwd=ROOT, check=True)
        binaries[mode] = [str(binary)]
    binaries['reference'] = [str(CACHE / 'api-ours'), 'reference']
    binaries['baseline'] = [str(CACHE / 'api-ours'), 'baseline']
    judge = CACHE / 'judge'
    subprocess.run(['g++', *flags, 'test/bipartite_matching.test.cpp', '-o', str(judge)], cwd=ROOT, check=True)
    raw = CACHE / 'reference'
    subprocess.run(['g++', *flags, str(CACHE / '396318.cpp'), '-o', str(raw)], check=True)
    problem = Path(LibraryCheckerService._get_cloned_repository_path()) / 'graph/bipartitematching'
    cases = sorted((problem / 'in').glob('*.in'))
    if args.screen:
        cases = [p for p in cases if any(s in p.stem for s in ['unique_matching_00', 'augmented_cycle_00', 'random_00', 'random_09', 'issue1068_large_00', 'many_paths_00'])]
    else:
        cases += custom_cases()
    assert cases
    report = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
              'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': list(os.uname()),
              'pin': 2, 'seed': 20261020, 'repeats': args.repeats,
              'problem_commit': subprocess.check_output(['git', '-C', str(problem), 'rev-parse', 'HEAD'], text=True).strip(),
              'cases': []}
    rng = random.Random(20261020)
    for path in cases:
        custom = path.stem.startswith('custom_')
        expected_path = CACHE / (path.stem + '.out') if custom else problem / 'out' / (path.stem + '.out')
        if custom:
            with path.open('rb') as stream, expected_path.open('wb') as output:
                subprocess.run([str(raw)], stdin=stream, stdout=output, check=True)
        expected = int(expected_path.read_text().split()[0])
        values = {mode: [] for mode in binaries}
        full = {mode: [] for mode in ['ours', 'reference']}
        for trial in range(-1, args.repeats):
            modes = list(binaries)
            rng.shuffle(modes)
            for mode in modes:
                with path.open('rb') as stream:
                    parts = subprocess.check_output(['taskset', '-c', '2', *binaries[mode]], stdin=stream).split()
                assert int(parts[0]) == expected, (path, mode, parts)
                if trial >= 0:
                    values[mode].append(float(parts[1]))
            if not args.screen:
                for mode, binary in [('ours', judge), ('reference', raw)]:
                    output = CACHE / (mode + '.out')
                    with path.open('rb') as stream, output.open('wb') as destination:
                        start = time.perf_counter()
                        subprocess.run(['taskset', '-c', '2', str(binary)], stdin=stream, stdout=destination, check=True)
                        elapsed = (time.perf_counter() - start) * 1000
                    if trial == -1:
                        result = subprocess.run([str(problem / 'checker'), str(path), str(output), str(expected_path)], capture_output=True)
                        assert result.returncode == 0, (path, mode, result.stderr)
                    if trial >= 0:
                        full[mode].append(elapsed)
        report['cases'].append({'case': path.stem, 'custom': custom, 'input_sha256': sha(path), 'expected': expected, 'api_ms': values, 'full_ms': full})
        print(path.stem, {mode: round(statistics.median(v), 3) for mode, v in values.items()}, flush=True)
    names = ['graph/matching/bipartite_matching.hpp', 'components/bipartite_matching_core.hpp',
             'test/bipartite_matching.test.cpp', 'test/benchmark/bipartite_matching.py', 'test/benchmark/bipartite_matching_api.cpp']
    report['sources'] = {name: sha(ROOT / name) for name in names}
    report['reference_source_sha256'] = sha(CACHE / '396318.cpp')
    (OUT / ('screen.json' if args.screen else 'measurements.json')).write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
