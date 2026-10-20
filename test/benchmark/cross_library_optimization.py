import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess


ROOT = Path(__file__).resolve().parents[2]
TARGETS = ['components/mo_order.hpp', 'components/undirected_csr.hpp', 'tree/auxiliary_tree.hpp']
BASE = '8f30994'


def source(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode()


def candidates(threshold):
    mo = source(TARGETS[0]).replace('#include <algorithm>', '#include "radix_sort.hpp"\n#include <algorithm>')
    mo = mo.replace('std::sort(keys.begin(), keys.end());',
                    f'if (keys.size() < {threshold}) std::sort(keys.begin(), keys.end());\n'
                    '\telse stable_radix_sort(keys, [](const auto &x) { return x.first; });')
    csr = source(TARGETS[1]).replace('++start[u + 1], ++start[v + 1];', '++start[u], ++start[v];')
    csr = csr.replace('for (int v = 0; v < n; ++v) start[v + 1] += start[v];',
                      'for (int v = 1; v <= n; ++v) start[v] += start[v - 1];')
    csr = csr.replace('\t\tstd::vector<int> cursor(start.begin(), start.end() - 1);\n', '')
    csr = csr.replace('for (int i = 0; i < static_cast<int>(edges.size()); ++i)',
                      'for (int i = static_cast<int>(edges.size()); i-- > 0;)')
    csr = csr.replace('adj[cursor[u]++] = {v, i}, adj[cursor[v]++] = {u, i};',
                      'adj[--start[v]] = {u, i}, adj[--start[u]] = {v, i};')
    aux = source(TARGETS[2]).replace('#include <algorithm>', '#include "../components/radix_sort.hpp"\n#include <algorithm>')
    aux = aux.replace('if (!sorted) std::sort(keys_.begin(), keys_.end());',
                      f'if (!sorted) {{\n\t\t\tif (keys_.size() < {threshold}) std::sort(keys_.begin(), keys_.end());\n'
                      '\t\t\telse stable_radix_sort(keys_, [](const auto &x) { return static_cast<unsigned>(x.first); });\n\t\t}')
    return dict(zip(TARGETS, [mo, csr, aux]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--screen', action='store_true')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    cache = ROOT / '.verify-helper/cache/cross_library_optimization'
    cache.mkdir(parents=True, exist_ok=True)
    lock = open('/tmp/cp-wm-opt.lock', 'w')
    fcntl.flock(lock, fcntl.LOCK_EX)
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-ftrivial-auto-var-init=zero']
    if not args.generic:
        flags.append('-march=native')
    baseline = {p: source(p) for p in TARGETS}
    variants = {'before': baseline}
    if args.screen:
        for threshold in [256, 512, 1024, 2048]:
            variants[f'candidate_{threshold}'] = candidates(threshold)
    else:
        variants['after'] = {p: (ROOT / p).read_text() for p in TARGETS}
    manifest = {}
    for name, headers in variants.items():
        directory = cache / name
        for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'tree', 'math', 'sequence', 'string']:
            for path in (ROOT / folder).rglob('*.hpp'):
                dest = directory / path.relative_to(ROOT)
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, dest)
        for path, contents in headers.items():
            (directory / path).write_text(contents)
        for program in ['cross_library_optimization', 'auxiliary_tree']:
            subprocess.run(['g++', *flags, '-I' + str(directory), '-I' + str(ROOT),
                            str(ROOT / 'test/benchmark' / (program + '.cpp')), '-o', str(directory / program)], check=True)
        manifest[name] = {p: hashlib.sha256(text.encode()).hexdigest() for p, text in headers.items()}
    cases = []
    for size in [16, 128, 255, 256, 511, 512, 1023, 1024, 2048, 8192, 200000, 1000000]:
        cases.append(('mo', ['mo', 'random', '1000000', str(size), str(max(1, 1000000 // size))]))
    for shape in ['duplicates', 'sorted']:
        cases.append(('mo', ['mo', shape, '1000000', '200000', '3']))
    if args.screen:
        for shape in ['random', 'path', 'star', 'sorted']:
            for mode in ['csr', 'lowlink', 'twoedge']:
                cases.append((mode, [mode, shape, '200000', '600000', '3']))
        cases.append(('csr', ['csr', 'random', '2000000', '4000000', '2']))
    for backend in ['lca', 'hld']:
        for size in [2, 16, 128, 255, 256, 511, 512, 1023, 1024, 2048, 8192, 200000]:
            cases.append(('aux', ['random', 'library', '200000', str(size), str(max(3, 400000 // size)), backend]))
        cases.append(('aux', ['random', 'library_sorted', '200000', '4096', '100', backend]))
    result = dict(created_at=datetime.now(timezone.utc).isoformat(), baseline_commit=BASE,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  system=subprocess.check_output(['uname', '-a'], text=True).strip(),
                  cpu=next(x.strip() for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),
                  cpu_affinity=[2], flags=flags, trials=args.trials, sources=manifest,
                  dependency_sha256={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in [
                      'components/radix_sort.hpp', 'components/default_init_allocator.hpp',
                      'graph/connectivity/low_link.hpp', 'graph/connectivity/two_edge_connected_components.hpp',
                      'tree/lca.hpp', 'data_structure/advanced/hld.hpp', 'data_structure/range_query/sparse_table.hpp']},
                  driver_sha256={p: hashlib.sha256((ROOT / 'test/benchmark' / p).read_bytes()).hexdigest()
                                 for p in ['cross_library_optimization.cpp', 'auxiliary_tree.cpp']}, cases=[])
    order_rng = random.Random(20261020)
    for kind, command in cases:
        names = list(variants)
        if args.screen and kind in ['csr', 'lowlink', 'twoedge']:
            names = ['before', 'candidate_256']
        runs = {name: [] for name in names}
        checksum = None
        for trial in range(args.trials + 1):
            order_rng.shuffle(names)
            for name in names:
                binary = 'auxiliary_tree' if kind == 'aux' else 'cross_library_optimization'
                raw = subprocess.check_output([str(cache / name / binary), *command], text=True).split()
                if checksum is None:
                    checksum = raw[1]
                if raw[1] != checksum:
                    raise RuntimeError((kind, command, name, checksum, raw))
                if trial:
                    runs[name].append(float(raw[0]))
        row = dict(kind=kind, command=command, checksum=checksum, milliseconds=runs,
                   median_ms={name: statistics.median(values) for name, values in runs.items()})
        result['cases'].append(row)
        print(kind, ' '.join(command), row['median_ms'], flush=True)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
