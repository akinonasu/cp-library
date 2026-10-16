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
    parser.add_argument('--n', type=int, default=200000)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--repetitions', type=int, default=5)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--kinds', nargs='+', default=['random', 'binary', 'same', 'periodic', 'fibonacci', 'thue', 'blocks', 'near_periodic'])
    parser.add_argument('--variants', nargs='+', default=['current', 'word', 'vectorized', 'no_index', 'zero_initialized', 'o3', 'scalar', 'budget8', 'budget32', 'comparison_sort', 'divide', 'reference_avx2', 'reference_scalar', 'hash'])
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['components/sequence_match.hpp', 'components/default_init_allocator.hpp', 'string/run_enumerate.hpp', 'test/benchmark/run_enumerate.cpp', 'test/benchmark/run_enumerate.py']
    environment = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'flags': flags,
                   'affinity': [args.cpu], 'n': args.n, 'trials': args.trials, 'repetitions': args.repetitions,
                   'seed': 713, 'variants': args.variants, 'kinds': args.kinds,
                   'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    for key, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
        environment[key] = subprocess.check_output(command, text=True)
    for identity in [406234, 313634]:
        environment.setdefault('reference_sha256', {})[str(identity)] = hashlib.sha256((args.reference_cache / f'{identity}.cpp').read_bytes()).hexdigest()
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    rows, summary = [], []
    with tempfile.TemporaryDirectory(prefix='cp-run-enumerate-') as temporary:
        directory = Path(temporary)
        fastest = (args.reference_cache / '406234.cpp').read_text().split('int main() {')[0]
        fastest = ('#pragma GCC push_options\n#ifndef __AVX2__\n#define CP_BENCH_UNDEF_AVX2\n#endif\n' + fastest +
                   '\n#pragma GCC pop_options\n#ifdef CP_BENCH_UNDEF_AVX2\n#undef __AVX2__\n#undef CP_BENCH_UNDEF_AVX2\n#endif\n')
        hashed = (args.reference_cache / '313634.cpp').read_text()
        hashed = hashed[hashed.index('template <class Seq>'):hashed.index('int main()')]
        reference = fastest + '\nnamespace reference_hash {\nusing namespace std;\n' + hashed + '\n}\n'
        (directory / 'reference.hpp').write_text(reference)
        header = (ROOT / 'string/run_enumerate.hpp').read_text().replace('../components/', 'components/')
        component = (ROOT / 'components/sequence_match.hpp').read_text()
        component = component.replace('SEQUENCE_MATCH_HPP', 'SEQUENCE_MATCH_WORD_HPP').replace('sequence_match_detail', 'sequence_match_word_detail')
        component = component.replace('bool Vectorize = true', 'bool Vectorize = false')
        (directory / 'word_component.hpp').write_text(component)
        for name in ['o3', 'zero_initialized', 'word', 'no_index', 'vectorized']:
            variant = header.replace('RUN_ENUMERATE_HPP', f'RUN_ENUMERATE_{name.upper()}_HPP')
            variant = variant.replace('Run', f'Run_{name}').replace('run_enumerate', f'run_enumerate_{name}')
            if name == 'o3':
                variant = '#pragma GCC push_options\n#pragma GCC optimize("O3")\n' + variant + '\n#pragma GCC pop_options\n'
            elif name == 'word':
                variant = variant.replace('components/sequence_match.hpp', 'word_component.hpp').replace('sequence_match_detail', 'sequence_match_word_detail')
            elif name == 'zero_initialized':
                variant = variant.replace('std::vector<int, DefaultInitAllocator<int>>', 'std::vector<int>')
            elif name == 'vectorized':
                variant = variant.replace('Matcher<T, false>', 'Matcher<T>')
            else:
                variant = variant.replace('if (equal >= 56)', 'if (false)')
            (directory / f'{name}.hpp').write_text(variant)
        binary = directory / 'benchmark'
        subprocess.run([args.cxx, *flags, '-I', str(ROOT), '-I', str(directory), str(ROOT / 'test/benchmark/run_enumerate.cpp'), '-o', str(binary)], check=True)
        order_random = random.Random(32947)
        for kind in args.kinds:
            samples = {name: [] for name in args.variants}
            correct = json.loads(subprocess.check_output([str(binary), 'current', kind, str(args.n), str(args.repetitions)], text=True))
            expected = correct['digest'], correct['runs']
            for trial in range(args.trials):
                order = args.variants.copy()
                order_random.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([str(binary), name, kind, str(args.n), str(args.repetitions)], text=True))
                    answer = row['digest'], row['runs']
                    row['correct'] = answer == expected
                    if not row['correct'] and name != 'hash':
                        raise RuntimeError(f'Wrong answer: {kind} {name} {answer} != {expected}')
                    row['trial'] = trial
                    rows.append(row)
                    samples[name].append(row['seconds'])
            for name, values in samples.items():
                row = {'kind': kind, 'variant': name, 'median_ms': statistics.median(values) * 1000,
                       'min_ms': min(values) * 1000, 'max_ms': max(values) * 1000,
                       'correct': all(row['correct'] for row in rows if row['kind'] == kind and row['variant'] == name)}
                summary.append(row)
                print(json.dumps(row), flush=True)
    for name, values in [('samples', rows), ('summary', summary)]:
        with (args.output / f'{name}.csv').open('w', newline='') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(values[0]))
            writer.writeheader()
            writer.writerows(values)


if __name__ == '__main__':
    main()
