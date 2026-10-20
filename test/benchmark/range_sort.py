import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--explore', action='store_true')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['data_structure/segment_tree/sortable_segtree.hpp', 'components/predecessor_set.hpp',
             'components/fast_io.hpp', 'test/sortable_segtree.test.cpp', 'test/benchmark/range_sort.py']
    environment = dict(date=datetime.now(timezone.utc).isoformat(), flags=flags,
                       compiler=subprocess.check_output(['g++', '--version'], text=True),
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu],
                       trials=args.trials, warmup=1, order_seed=393919,
                       sha256={p: sha(ROOT / p) for p in paths},
                       references={str(i): sha(args.references / (str(i) + '.cpp')) for i in [393919, 401387]},
                       official_commit=subprocess.check_output(['git', '-C', str(args.official.parents[1]), 'rev-parse', 'HEAD'], text=True).strip(),
                       method='Fresh process, regular files on Linux /tmp, I/O and exit included; checker and SHA-256 outside timing')
    rng = random.Random(393919)
    results = {}
    with tempfile.TemporaryDirectory(prefix='cp-range-sort-', dir='/tmp') as directory:
        work = Path(directory)
        sources = dict(current=ROOT / 'test/sortable_segtree.test.cpp')
        sources.update({str(i): args.references / (str(i) + '.cpp') for i in [393919, 401387]})
        reference = (args.references / '393919.cpp').read_text()
        (work / 'treap.cpp').write_text(reference.replace('toy::PatriciaSortableSegmentTree tree(', 'toy::TreapSortableSegmentTree tree(')
                                      .replace('std::vector<toy::u32> keys(n);', 'std::vector<int> keys(n);'))
        sources['treap393919'] = work / 'treap.cpp'
        if args.explore:
            header = (ROOT / 'data_structure/segment_tree/sortable_segtree.hpp').read_text().replace('"../components/predecessor_set.hpp"', '"components/predecessor_set.hpp"')
            inline = header.replace('bit = -1, leaf;', 'bit = -1, leaf;\n\t\tU key = 0;')
            inline = inline.replace('keys[pool[a].leaf]', 'pool[a].key').replace('keys[pool[b].leaf]', 'pool[b].key').replace('keys[pool[x].leaf]', 'pool[x].key')
            inline = inline.replace('node.leaf = a.leaf;', 'node.leaf = a.leaf; node.key = a.key;')
            inline = inline.replace('pool.emplace_back(a[i].second, i); roots[i] = i + 1;', 'pool.emplace_back(a[i].second, i); pool.back().key = keys.back(); roots[i] = i + 1;')
            start = header.index('\tS prod(int l, int r) const {')
            end = header.index('\tS all_prod()', start)
            split = header[:start] + '''\tS prod(int l, int r) {
\t\tassert(0 <= l && l <= r && r <= n);
\t\tif (l == r) return e();
\t\tcut(l); cut(r); return middle_prod(l, r);
\t}
''' + header[end:]
            for name, body in [('inline', inline), ('split', split)]:
                (work / (name + '.hpp')).write_text(body)
                driver = (ROOT / 'test/sortable_segtree.test.cpp').read_text()
                driver = driver.replace('"../components/fast_io.hpp"', '"components/fast_io.hpp"').replace('"../data_structure/segment_tree/sortable_segtree.hpp"', '"' + name + '.hpp"')
                (work / (name + '.cpp')).write_text(driver)
                sources[name] = work / (name + '.cpp')
        environment['variants'] = {name: sha(source) for name, source in sources.items()}
        binaries = {}
        for name, source in sources.items():
            binaries[name] = work / name
            subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(work), str(source), '-o', str(binaries[name])], check=True)
        inputs = sorted((args.official / 'in').glob('*.in'))
        if args.explore:
            inputs = [p for p in inputs if p.stem in ['max_random_00', 'many_1_query_00', 'many_2_query_00', 'sort_only_short_00']]
        if not inputs:
            raise RuntimeError('No official inputs')
        for source in inputs:
            inp = work / 'input'
            shutil.copyfile(source, inp)
            answer = args.official / 'out' / (source.stem + '.out')
            samples = {name: [] for name in binaries}
            hashes = {}
            for trial in range(-1, args.trials):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    out = work / 'output'
                    with inp.open('rb') as a, out.open('wb') as b:
                        start = time.perf_counter()
                        subprocess.run([binaries[name]], stdin=a, stdout=b, check=True)
                        ms = (time.perf_counter() - start) * 1000
                    digest = sha(out)
                    if name not in hashes or digest != hashes[name]:
                        subprocess.run([str(args.official / 'checker'), str(inp), str(out), str(answer)], check=True,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        hashes[name] = digest
                    if trial >= 0:
                        samples[name].append(ms)
            results[source.stem] = dict(input_sha256=sha(inp), answer_sha256=sha(answer),
                                       variants={name: dict(samples_ms=values, median_ms=statistics.median(values), output_sha256=hashes[name]) for name, values in samples.items()})
            print(source.stem, {name: row['median_ms'] for name, row in results[source.stem]['variants'].items()}, flush=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
