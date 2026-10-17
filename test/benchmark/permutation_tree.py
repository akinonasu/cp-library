import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(cache, reference):
    source = (reference / '252918.cpp').read_text().split('#include <cstdio>')[0]
    (cache / 'reference.hpp').write_text(source)
    header = (ROOT / 'data_structure/permutation_tree.hpp').read_text().replace('../components/', 'components/')
    aggregate = header.replace('PERMUTATION_TREE_HPP', 'PERMUTATION_TREE_AGGREGATE_HPP').replace('PermutationTree', 'PermutationTreeAggregate')
    begin = aggregate.index('#ifdef __cpp_aggregate_paren_init')
    end = aggregate.index('#endif', begin) + len('#endif')
    aggregate = aggregate[:begin] + '\t\tnodes_.push_back({-1, l, r, lo, hi, type});' + aggregate[end:]
    (cache / 'aggregate.hpp').write_text(aggregate)
    begin = header.index('\t\t\t\t\tif (top.hi - top.lo == r - top.l)')
    end = header.index('\n\t\t\t\t}', begin)
    late = '''\t\t\t\t\tif (minimum.prod(top.lo, top.hi) < top.l) {
\t\t\t\t\t\tFrame removed = top;
\t\t\t\t\t\tstack.pop_back();
\t\t\t\t\t\tassert(!stack.empty());
\t\t\t\t\t\tstack.back().lo = std::min(stack.back().lo, removed.lo);
\t\t\t\t\t\tstack.back().hi = std::max(stack.back().hi, removed.hi);
\t\t\t\t\t} else if (top.hi - top.lo == r - top.l) {
\t\t\t\t\t\tcurrent = top;
\t\t\t\t\t\tintervals.push_back({top.l, r, top.lo, top.hi});
\t\t\t\t\t\tstack.pop_back();
\t\t\t\t\t} else break;'''
    header = header[:begin] + late + header[end:]
    for name in ['packed', 'baseline']:
        variant = header
        if name == 'baseline':
            begin = variant.index('\t\tstd::vector<int> cursor = offsets_;')
            end = variant.index('\t\tcount_ = n;', begin)
            variant = variant[:begin] + '''\t\tstd::vector<int> head(n, -1), cursor = offsets_;
\t\torder_.resize(m);
\t\tfor (int i = 0; i < m; ++i) if (nodes_[i].parent >= 0) {
\t\t\torder_[i] = head[nodes_[i].l];
\t\t\thead[nodes_[i].l] = i;
\t\t}
\t\tfor (int i = 0; i < n; ++i)
\t\t\tfor (int v = head[i]; v >= 0; v = order_[v]) children_[cursor[nodes_[v].parent]++] = v;
''' + variant[end:]
            variant = variant.replace('\t\torder_.reserve(m);\n', '')
        variant = variant.replace('PERMUTATION_TREE_HPP', 'PERMUTATION_TREE_' + name.upper() + '_HPP')
        variant = variant.replace('PermutationTree', 'PermutationTree' + name.title())
        (cache / (name + '.hpp')).write_text(variant)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--n', type=int, default=500000)
    parser.add_argument('--repetitions', type=int, default=3)
    parser.add_argument('--core-only', action='store_true')
    parser.add_argument('--zero-init', action='store_true')
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
    args.cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    prepare(args.cache, args.references)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native']
    if args.zero_init:
        flags.append('-ftrivial-auto-var-init=zero')
    paths = ['data_structure/permutation_tree.hpp', 'components/static_range_min.hpp', 'components/default_init_allocator.hpp',
             'data_structure/sparse_table.hpp', 'components/fast_io.hpp', 'test/permutation_tree.test.cpp',
             'test/benchmark/permutation_tree.py', 'test/benchmark/permutation_tree.cpp']
    environment = dict(date=datetime.datetime.now(datetime.timezone.utc).isoformat(), flags=flags,
                       compiler=subprocess.check_output(['g++', '--version'], text=True),
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu],
                       seed=279529, order_seed=37192, n=args.n, repetitions=args.repetitions, trials=args.trials, warmup=1,
                       sha256={p: sha(ROOT / p) for p in paths},
                       references={str(i): sha(args.references / (str(i) + '.cpp')) for i in [279529, 252918, 405146]},
                       variants={name: sha(args.cache / (name + '.hpp')) for name in ['baseline', 'packed', 'aggregate']},
                       method='Kernel: build and canonical checksum, reference input copy included; judge: fresh process, regular files on Linux /tmp, I/O and exit included; checker and hashing outside timing',
                       official_commit=subprocess.check_output(['git', '-C', str(args.official.parents[1]), 'rev-parse', 'HEAD'], text=True).strip())
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rng = random.Random(37192)
    results = dict(kernel=[], judge={})
    with tempfile.TemporaryDirectory(prefix='cp-permutation-tree-', dir='/tmp') as temp:
        work = Path(temp)
        binary = work / 'kernel'
        subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(args.cache.resolve()),
                        str(ROOT / 'test/benchmark/permutation_tree.cpp'), '-o', str(binary)], check=True)
        binaries = {}
        if not args.core_only:
            sources = dict(current=ROOT / 'test/permutation_tree.test.cpp')
            sources.update({str(i): args.references / (str(i) + '.cpp') for i in [279529, 252918, 405146]})
            for name, source in sources.items():
                binaries[name] = work / name
                subprocess.run(['g++', *flags, str(source), '-o', str(binaries[name])], check=True)
        for kind in ['random', 'ascending', 'descending', 'pairs', 'zigzag', 'blocks', 'recursive']:
            samples = {name: [] for name in ['baseline', 'packed', 'aggregate', 'current', 'reference']}
            expected = None
            for trial in range(-1, args.trials):
                order = list(samples)
                rng.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([str(binary), name, kind, str(args.n), str(args.repetitions)], text=True))
                    digest = row['nodes'], row['checksum']
                    if expected is None:
                        expected = digest
                    if digest != expected:
                        raise RuntimeError((kind, name, digest, expected))
                    if trial >= 0:
                        samples[name].append(row['ms'])
            row = dict(kind=kind, nodes=expected[0], checksum=expected[1], samples=samples,
                       medians={k: statistics.median(v) for k, v in samples.items()})
            results['kernel'].append(row)
            print('kernel', kind, row['medians'], flush=True)
        if not args.core_only:
            inputs = sorted((args.official / 'in').glob('*.in'))
            if not inputs:
                raise RuntimeError('No official cases')
            for source in inputs:
                inp = work / 'input'
                shutil.copyfile(source, inp)
                answer = args.official / 'out' / (source.stem + '.out')
                row = dict(input_sha256=sha(inp), answer_sha256=sha(answer), variants={})
                samples = {name: [] for name in binaries}
                hashes = {}
                for trial in range(-1, args.trials):
                    order = list(binaries)
                    rng.shuffle(order)
                    for name in order:
                        output = work / 'output'
                        with inp.open('rb') as a, output.open('wb') as b:
                            begin = time.perf_counter()
                            subprocess.run([str(binaries[name])], stdin=a, stdout=b, check=True)
                            ms = (time.perf_counter() - begin) * 1000
                        digest = sha(output)
                        if name not in hashes or digest != hashes[name]:
                            subprocess.run([str(args.official / 'checker'), str(inp), str(output), str(answer)],
                                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                            hashes[name] = digest
                        if trial >= 0:
                            samples[name].append(ms)
                for name, values in samples.items():
                    row['variants'][name] = dict(output_sha256=hashes[name], samples=values, median_ms=statistics.median(values))
                results['judge'][source.stem] = row
                print('judge', source.stem, {k: v['median_ms'] for k, v in row['variants'].items()}, flush=True)
        (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
