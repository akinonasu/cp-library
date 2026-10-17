import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-cache', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--count', type=int, default=300000)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--skip-check', action='store_true')
    parser.add_argument('--kinds', nargs='+', default=[
        'random', 'full', 'short', 'fibonacci', 'wide', 'wide_short', 'fibonacci64', 'descending64', 'degenerate'])
    args = parser.parse_args()
    if hasattr(os, 'sched_setaffinity'):
        os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['components/mod_linear_prefix_min.hpp', 'math/prefix_min_of_mod_of_linear.hpp',
             'test/benchmark/prefix_min_of_mod_of_linear_baseline.hpp',
             'test/benchmark/prefix_min_of_mod_of_linear.cpp', 'test/benchmark/prefix_min_of_mod_of_linear.py']
    environment = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
                   'flags': flags, 'affinity': [args.cpu], 'seed': 713,
                   'count': args.count, 'trials': args.trials,
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    for key, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
        environment[key] = subprocess.check_output(command, text=True)
    for source in ['405749.cpp', 'min_of_linear_segments.hpp']:
        environment.setdefault('references_sha256', {})[source] = hashlib.sha256(
            (args.reference_cache / source).read_bytes()).hexdigest()
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-prefix-min-') as temporary:
        directory = Path(temporary)
        baseline = (ROOT / 'test/benchmark/prefix_min_of_mod_of_linear_baseline.hpp').read_text()
        baseline = baseline.replace('PREFIX_MIN_OF_MOD_OF_LINEAR_HPP', 'PREFIX_MIN_BASELINE_HPP')
        baseline = baseline.replace('ModLinearPrefixMinSegment', 'BaselineSegment')
        baseline = baseline.replace('prefix_min_of_mod_of_linear', 'prefix_min_baseline')
        (directory / 'baseline.hpp').write_text(baseline)
        component = (ROOT / 'components/mod_linear_prefix_min.hpp').read_text()
        component = component.replace('MOD_LINEAR_PREFIX_MIN_HPP', 'MOD_LINEAR_PREFIX_MIN_INLINED_HPP')
        component = component.replace('mod_linear_detail', 'mod_linear_inlined_detail')
        component = component.replace('void prefix_min(', '[[gnu::always_inline]] inline void prefix_min(')
        (directory / 'inlined_component.hpp').write_text(component)
        inlined = (ROOT / 'math/prefix_min_of_mod_of_linear.hpp').read_text()
        inlined = inlined.replace('PREFIX_MIN_OF_MOD_OF_LINEAR_HPP', 'PREFIX_MIN_INLINED_HPP')
        inlined = inlined.replace('../components/mod_linear_prefix_min.hpp', 'inlined_component.hpp')
        inlined = inlined.replace('mod_linear_detail', 'mod_linear_inlined_detail')
        inlined = inlined.replace('ModLinearPrefixMinSegment', 'InlinedSegment')
        inlined = inlined.replace('prefix_min_of_mod_of_linear', 'prefix_min_inlined')
        inlined = inlined.replace('void prefix_min_inlined(', '[[gnu::always_inline]] inline void prefix_min_inlined(')
        inlined = inlined.replace('inline std::vector', '[[gnu::always_inline]] inline std::vector')
        (directory / 'inlined.hpp').write_text(inlined)
        article = (args.reference_cache / 'min_of_linear_segments.hpp').read_text()
        fastest = (args.reference_cache / '405749.cpp').read_text()
        begin = fastest.index('constexpr itype::u32 LinearModMin(')
        end = fastest.index('\n}\n', begin) + 3
        reference = ('#include <algorithm>\n#include <numeric>\n#include <utility>\n#include <vector>\n'
                     'namespace reference_article {\nusing std::gcd; using std::max; using std::pair;\n'
                     'template<class T> using vc = std::vector<T>;\n'
                     'int ceil(int a, int b) { return a > 0 ? (a - 1) / b + 1 : 0; }\n'
                     '#define eb emplace_back\n' + article + '\n#undef eb\n}\n'
                     'namespace reference_fastest { namespace itype { using u32 = unsigned; '
                     'using u64 = unsigned long long; }\n' + fastest[begin:end] + '\n}\n')
        (directory / 'reference.hpp').write_text(reference)
        binary = directory / 'benchmark'
        subprocess.run([args.cxx, *flags, '-I', str(ROOT), '-I', str(directory),
                        str(ROOT / 'test/benchmark/prefix_min_of_mod_of_linear.cpp'), '-o', str(binary)], check=True)
        order_random = random.Random(32947)
        for kind in args.kinds:
            if not args.skip_check:
                subprocess.run([str(binary), 'check', kind, '20000', '713'], check=True)
            names = ['baseline', 'vector', 'reserved', 'reuse', 'callback', 'inlined', 'inlined_callback']
            if kind not in ['wide', 'wide_short', 'fibonacci64', 'descending64']:
                names += ['article', 'min_only']
            samples = {name: [] for name in names}
            expected = None
            for trial in range(args.trials):
                order = names.copy()
                order_random.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([
                        str(binary), name, kind, str(args.count), '713'], text=True))
                    if name not in ['article', 'min_only']:
                        answer = row['digest'], row['blocks']
                        if expected is None:
                            expected = answer
                        elif answer != expected:
                            raise RuntimeError(f'checksum mismatch: {kind} {name}')
                    row['trial'] = trial
                    rows.append(row)
                    samples[name].append(row['seconds'])
            for name in names:
                median = statistics.median(samples[name])
                summary.append({'kind': kind, 'variant': name, 'seconds': median,
                                'baseline_speedup': statistics.median(samples['baseline']) / median})
            for filename, data in [('samples.csv', rows), ('summary.csv', summary)]:
                with (args.output / filename).open('w', newline='') as file:
                    writer = csv.DictWriter(file, fieldnames=list(data[0]))
                    writer.writeheader()
                    writer.writerows(data)
            print(f'{kind}: measurements complete', flush=True)


if __name__ == '__main__':
    main()
