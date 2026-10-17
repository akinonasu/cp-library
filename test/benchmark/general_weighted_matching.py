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
GREEDY = '''\t\tfor (int u = 1; u <= n; ++u) if (!match[u]) {
\t\t\tfor (std::size_t i = start[u]; i < start[u + 1]; ++i) {
\t\t\t\tint v = adjacent[i];
\t\t\t\tif (!match[v] && edge(u, v).weight == maximum) { match[u] = v; match[v] = u; break; }
\t\t\t}
\t\t}
'''


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native', '-ftrivial-auto-var-init=zero']
    sources = ['components/weighted_blossom.hpp', 'graph/general_weighted_matching.hpp', 'components/fast_io.hpp',
               'test/general_weighted_matching.test.cpp', 'test/benchmark/general_weighted_matching.cpp', 'test/benchmark/general_weighted_matching.py']
    environment = dict(date=datetime.now(timezone.utc).isoformat(), flags=flags,
                       compiler=subprocess.check_output(['g++', '--version'], text=True),
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu],
                       trials=args.trials, warmup=1, order_seed=404902, sha256={p: sha(ROOT / p) for p in sources},
                       references={str(i): sha(args.references / (str(i) + '.cpp')) for i in [404902, 324220]},
                       official_commit=subprocess.check_output(['git', '-C', str(args.official.parents[1]), 'rev-parse', 'HEAD'], text=True).strip(),
                       method='Kernel excludes file parsing/output, includes graph allocation/add/build; judge uses fresh processes with regular files in Linux /tmp including I/O/exit; correctness outside timing')
    rng = random.Random(404902)
    results = dict(kernel={}, judge={})
    with tempfile.TemporaryDirectory(prefix='cp-weighted-matching-', dir='/tmp') as directory:
        work = Path(directory)
        core = (ROOT / 'components/weighted_blossom.hpp').read_text()
        wrapper = (ROOT / 'graph/general_weighted_matching.hpp').read_text()
        has_greedy = GREEDY in core
        variants = ['current', 'greedy', 'int64', 'int128']
        for name in variants:
            folder = work / name
            (folder / 'graph').mkdir(parents=True)
            (folder / 'components').mkdir()
            modified = core
            if name == 'greedy' and not has_greedy:
                modified = modified.replace('\t\tqueue.reserve(n);', GREEDY + '\t\tqueue.reserve(n);')
            (folder / 'components/weighted_blossom.hpp').write_text(modified)
            modified = wrapper
            begin = modified.index('\t\tstd::int64_t bound = ')
            end = modified.index('\n\t\ttotal = 0;', begin)
            if name in ['int64', 'int128']:
                integer = 'std::int64_t' if name == 'int64' else '__int128_t'
                modified = modified[:begin] + f'\t\tweighted_matching_detail::Blossom<{integer}>(n, weights, maximum).solve(mate);' + modified[end:]
            (folder / 'graph/general_weighted_matching.hpp').write_text(modified)
        driver = (ROOT / 'test/general_weighted_matching.test.cpp').read_text().replace('"../components/fast_io.hpp"', '"components/fast_io.hpp"').replace('"../graph/general_weighted_matching.hpp"', '"graph/general_weighted_matching.hpp"')
        (work / 'driver.cpp').write_text(driver)
        binaries, kernels = {}, {}
        for name in variants:
            binaries[name], kernels[name] = work / (name + '.exe'), work / (name + '-kernel')
            for source, binary in [(work / 'driver.cpp', binaries[name]), (ROOT / 'test/benchmark/general_weighted_matching.cpp', kernels[name])]:
                subprocess.run(['g++', *flags, '-I', str(work / name), '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        for identity in [404902, 324220]:
            name = str(identity)
            binaries[name] = work / name
            subprocess.run(['g++', *flags, str(args.references / (name + '.cpp')), '-o', str(binaries[name])], check=True)
        environment['greedy_in_source'] = has_greedy
        environment['generated_sha256'] = {str(p.relative_to(work)): sha(p) for p in work.rglob('*') if p.suffix in ['.hpp', '.cpp']}
        for stem in ['max_random_00', 'sparse_04', 'complete_00', 'complete_plane_01', 'complete_mandist_01', 'cycles_00']:
            inp = work / 'input'
            shutil.copyfile(args.official / 'in' / (stem + '.in'), inp)
            samples = {name: [] for name in kernels}
            expected = None
            for trial in range(-1, args.trials):
                order = list(kernels)
                rng.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([kernels[name], inp], text=True))
                    if expected is None:
                        expected = row['weight']
                    if expected != row['weight']:
                        raise RuntimeError((stem, name, expected, row))
                    if trial >= 0:
                        samples[name].append(row)
            results['kernel'][stem] = dict(input_sha256=sha(inp), samples=samples,
                medians={name: {key: statistics.median(row[key] for row in rows) for key in ['graph_ms', 'solve_ms']} for name, rows in samples.items()})
            print('kernel', stem, results['kernel'][stem]['medians'], flush=True)
        for source in sorted((args.official / 'in').glob('*.in')):
            inp, out = work / 'input', work / 'output'
            shutil.copyfile(source, inp)
            answer = args.official / 'out' / (source.stem + '.out')
            samples = {name: [] for name in binaries}
            hashes = {}
            for trial in range(-1, args.trials):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    with inp.open('rb') as a, out.open('wb') as b:
                        start = time.perf_counter()
                        subprocess.run([binaries[name]], stdin=a, stdout=b, check=True)
                        ms = (time.perf_counter() - start) * 1000
                    digest = sha(out)
                    if name not in hashes or hashes[name] != digest:
                        subprocess.run([args.official / 'checker', inp, out, answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        hashes[name] = digest
                    if trial >= 0:
                        samples[name].append(ms)
            results['judge'][source.stem] = dict(input_sha256=sha(inp), answer_sha256=sha(answer),
                variants={name: dict(samples_ms=values, median_ms=statistics.median(values), output_sha256=hashes[name]) for name, values in samples.items()})
            print('judge', source.stem, {k: v['median_ms'] for k, v in results['judge'][source.stem]['variants'].items()}, flush=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
