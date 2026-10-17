import argparse
import datetime
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
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
BASE = '829efa17b452f904eef5558930d91dd3792feffa'
KINDS = ['random', 'binary', 'same', 'periodic', 'fibonacci', 'thue', 'blocks',
         'near_periodic', 'long_period', 'period64', 'period4096']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def variants(cache):
    header = subprocess.check_output(['git', 'show', BASE + ':string/run_enumerate.hpp'], cwd=ROOT, text=True)
    header = header.replace('../components/', 'components/')
    for name in ['baseline', 'filtered', 'cached', 'guarded', 'unindexed']:
        begin = header.index('struct Run {')
        end = header.index('namespace run_enumerate_detail', begin)
        source = (header[:begin] + header[end:]).replace('run_enumerate', 'run_enumerate_' + name)
        source = source.replace('RUN_ENUMERATE_HPP', 'RUN_ENUMERATE_' + name.upper() + '_HPP')
        if name != 'baseline':
            replacement = ('if (j == n || s[i] != s[j]) continue;' if name == 'filtered' else
                           'if (j == n) continue;\n\t\t\tif constexpr (!Indexed) if (!common && s[i] != s[j]) continue;' if name == 'guarded' else
                           'if (j == n) continue;\n\t\t\tif constexpr (!Indexed) if (s[i] != s[j]) continue;' if name == 'unindexed' else
                           'if (j == n) { backward_lce[i] = 0; continue; }\n\t\t\tif (s[i] != s[j]) { backward_lce[i] = 0; continue; }')
            source = source.replace('if (j == n) continue;', replacement)
        if name == 'cached':
            source = source.replace('int top = 0;', 'int top = 0, boundary = n, distance = 0;\n\t\tstd::vector<int, DefaultInitAllocator<int>> backward_lce(n);')
            begin = source.index('\t\t\tint back = i &&')
            end = source.index('\t\t\tint ahead = common;', begin)
            source = source[:begin] + '''\t\t\tint back;
\t\t\tif (i > boundary && backward_lce[i + distance] < i - boundary) {
\t\t\t\tback = backward_lce[i + distance];
\t\t\t} else {
\t\t\t\tint known = std::max(0, i - boundary);
\t\t\t\tint extra = extend(true, i + 1 - known, j + 1 - known, i + 1 - known);
\t\t\t\tif (extra < 0) return false;
\t\t\t\tback = known + extra;
\t\t\t\tboundary = i - back;
\t\t\t\tdistance = j - i;
\t\t\t}
\t\t\tbackward_lce[i] = back;
\t\t\t--back;
\t\t\tif (back >= period) continue;
''' + source[end:]
        (cache / (name + '.hpp')).write_text(source)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--sizes', nargs='+', type=int, default=[200000])
    parser.add_argument('--repetitions', type=int, default=3)
    parser.add_argument('--variants', nargs='+', default=['baseline', 'filtered', 'cached', 'guarded', 'unindexed', 'linear', 'current', 'fastest'])
    parser.add_argument('--core-only', action='store_true')
    parser.add_argument('--judge-only', action='store_true')
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    args.cache.mkdir(parents=True, exist_ok=True)
    source_url = 'https://raw.githubusercontent.com/jonas-ellert/linear-time-runs/065dfae01cccc8fa7fa8b116cfa91f272c0b22ba/runs.hpp'
    if not (args.cache / 'reference.hpp').exists():
        (args.cache / 'reference.hpp').write_bytes(urllib.request.urlopen(source_url, timeout=90).read())
    variants(args.cache)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native']
    paths = ['string/run_enumerate.hpp', 'components/sequence_match.hpp', 'components/default_init_allocator.hpp',
             'components/fast_io.hpp', 'test/run_enumerate.test.cpp',
             'test/benchmark/run_enumerate_linear.py', 'test/benchmark/run_enumerate_linear.cpp']
    environment = dict(date=datetime.datetime.now(datetime.timezone.utc).isoformat(), baseline=BASE,
                       compiler=subprocess.check_output(['g++', '--version'], text=True), flags=flags,
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu], seed=20261018,
                       trials=args.trials, warmup=1, repetitions=args.repetitions, sizes=args.sizes,
                       source_url=source_url, author_sha256=sha(args.cache / 'reference.hpp'),
                       fastest_sha256=sha(args.references / '406234.cpp'),
                       sha256={p: sha(ROOT / p) for p in paths},
                       variants={name: sha(args.cache / (name + '.hpp')) for name in ['baseline', 'filtered', 'cached', 'guarded', 'unindexed']},
                       timing='Kernel includes output conversion/sort/checksum; judge includes process launch, I/O, build, sort, exit; regular files on Linux /tmp',
                       official_commit=subprocess.check_output(['git', '-C', str(args.official.parents[1]), 'rev-parse', 'HEAD'], text=True).strip())
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rng = random.Random(20261018)
    results = dict(kernel=[], judge={})
    with tempfile.TemporaryDirectory(prefix='cp-run-linear-', dir='/tmp') as temp:
        work = Path(temp)
        binaries = {}
        judge_binaries = {}
        for name in args.variants:
            binary = work / name
            if name == 'fastest':
                command = ['g++', *flags, str(args.references / '406234.cpp'), '-o', str(binary)]
            else:
                variant = dict(baseline=0, filtered=1, cached=2, linear=3, current=4, guarded=5, unindexed=6)[name]
                command = ['g++', *flags, '-DCP_RUN_VARIANT=' + str(variant), '-I', str(ROOT), '-I', str(args.cache.resolve()),
                           str(ROOT / 'test/benchmark/run_enumerate_linear.cpp'), '-o', str(binary)]
            subprocess.run(command, check=True)
            binaries[name] = binary
            judge_binaries[name] = binary
            if not args.core_only and name in ['baseline', 'current']:
                driver = (ROOT / 'test/run_enumerate.test.cpp').read_text().replace('../string/', 'string/').replace('../components/', 'components/')
                if name == 'baseline':
                    driver = driver.replace('#include "string/run_enumerate.hpp"', '#include "string/run_enumerate.hpp"\n#include "baseline.hpp"')
                    driver = driver.replace('run_enumerate(s)', 'run_enumerate_baseline(s)')
                driver_path = work / (name + '.cpp')
                driver_path.write_text(driver)
                judge_binary = work / (name + '-judge')
                subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(args.cache.resolve()),
                                str(driver_path), '-o', str(judge_binary)], check=True)
                judge_binaries[name] = judge_binary
                environment.setdefault('judge_driver_sha256', {})[name] = sha(driver_path)
        (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
        if not args.judge_only:
            for n in args.sizes:
                for kind in KINDS:
                    samples = {name: [] for name in args.variants if name != 'fastest'}
                    expected = None
                    for trial in range(-1, args.trials):
                        order = list(samples)
                        rng.shuffle(order)
                        for name in order:
                            row = json.loads(subprocess.check_output([str(binaries[name]), kind, str(n), str(args.repetitions)], text=True))
                            digest = row['count'], row['digest']
                            if expected is None:
                                expected = digest
                            if expected != digest:
                                raise RuntimeError((kind, name, expected, digest))
                            if trial >= 0:
                                samples[name].append(row['ms'])
                    row = dict(n=n, kind=kind, count=expected[0], digest=expected[1], samples=samples,
                               medians={k: statistics.median(v) for k, v in samples.items()})
                    results['kernel'].append(row)
                    print('kernel', n, kind, row['medians'], flush=True)
        if not args.core_only:
            inputs = sorted((args.official / 'in').glob('*.in'))
            if not inputs:
                raise RuntimeError('No official inputs')
            for original in inputs:
                inp = work / 'input'
                shutil.copyfile(original, inp)
                answer = args.official / 'out' / (original.stem + '.out')
                row = dict(input_sha256=sha(inp), answer_sha256=sha(answer), variants={})
                samples = {name: [] for name in judge_binaries}
                for trial in range(-1, args.trials):
                    order = list(judge_binaries)
                    rng.shuffle(order)
                    for name in order:
                        output = work / 'output'
                        with inp.open('rb') as a, output.open('wb') as b:
                            begin = time.perf_counter()
                            subprocess.run([str(judge_binaries[name])], stdin=a, stdout=b, check=True)
                            ms = (time.perf_counter() - begin) * 1000
                        subprocess.run([str(args.official / 'checker'), str(inp), str(output), str(answer)],
                                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        if trial >= 0:
                            samples[name].append(ms)
                            row['variants'][name] = dict(output_sha256=sha(output))
                for name, values in samples.items():
                    row['variants'][name].update(samples=values, median_ms=statistics.median(values))
                results['judge'][original.stem] = row
                print('judge', original.stem, {k: v['median_ms'] for k, v in row['variants'].items()}, flush=True)
        (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
