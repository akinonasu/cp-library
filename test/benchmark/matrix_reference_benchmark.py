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
import time

ROOT = Path(__file__).resolve().parents[2]
DRIVERS = {'matrix_product': 'product', 'matrix_det': 'det', 'inverse_matrix': 'inverse', 'matrix_rank': 'rank',
           'system_of_linear_equations': 'solve', 'pow_of_matrix': 'pow',
           'characteristic_polynomial': 'characteristic_polynomial', 'adjugate_matrix': 'adjugate'}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--references', type=Path, default=ROOT / 'test/benchmark/results/matrix_reference.json')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--size', type=int, default=384)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    references = json.loads(args.references.read_text())
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero']
    rows = []
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags, 'trials': args.trials,
           'size': args.size, 'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'affinity': [args.cpu], 'seed': 713, 'cases': []}
    paths = ['components/matrix_arithmetic.hpp', 'math/matrix.hpp', 'math/mint.hpp', 'components/fast_io.hpp', 'test/matrix_common.hpp', 'test/benchmark/matrix_reference_benchmark.py']
    paths += [f'test/matrix_{driver}.test.cpp' for driver in DRIVERS.values()]
    env['sha256'] = {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in paths}
    with tempfile.TemporaryDirectory(prefix='cp-matrix-reference-') as temporary:
        tmp = Path(temporary)
        binaries = {}
        for problem, driver in DRIVERS.items():
            reference = next(r for r in references['problems'][problem]['references'] if r['lang'] == 'cpp')
            source = args.cache / (str(reference['id']) + '.cpp')
            assert hashlib.sha256(source.read_bytes()).hexdigest() == reference['source_sha256']
            binaries[problem] = {}
            for name, path in [('library', ROOT / f'test/matrix_{driver}.test.cpp'), ('reference', source)]:
                binary = tmp / (problem + '_' + name)
                build = subprocess.run(['g++', *flags, '-I', str(ROOT), str(path), '-o', str(binary)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                if build.returncode:
                    (ROOT / '.verify-helper/cache/matrix-reference-build.log').write_bytes(build.stderr)
                    build.check_returncode()
                binaries[problem][name] = binary
            print('compiled', problem, reference['id'], flush=True)
        order_rng = random.Random(7919)
        for problem in DRIVERS:
            n = min(args.size, 128) if problem == 'pow_of_matrix' else args.size
            m = n + 37 if problem == 'matrix_rank' else n
            reference = next(r for r in references['problems'][problem]['references'] if r['lang'] == 'cpp')
            rng = random.Random(713)
            a = [[rng.randrange(998244353) for _ in range(m)] for _ in range(n)]
            lines = [f'{n} {m} {n}' if problem == 'matrix_product' else f'{n} {m}' if problem in ('matrix_rank', 'system_of_linear_equations') else f'{n} 1000000000000000000' if problem == 'pow_of_matrix' else str(n)]
            lines += [' '.join(map(str, row)) for row in a]
            if problem == 'matrix_product':
                lines += [' '.join(str(rng.randrange(998244353)) for _ in range(n)) for _ in range(m)]
            if problem == 'system_of_linear_equations':
                x = [rng.randrange(998244353) for _ in range(m)]
                lines.append(' '.join(str(sum(v * y for v, y in zip(row, x)) % 998244353) for row in a))
            input_path = tmp / (problem + '.in')
            input_path.write_text('\n'.join(lines) + '\n')
            env['cases'].append({'problem': problem, 'reference': reference, 'n': n, 'm': m,
                                 'input_sha256': hashlib.sha256(input_path.read_bytes()).hexdigest()})
            expected = None
            for trial in range(args.trials):
                order = ['library', 'reference']; order_rng.shuffle(order)
                for name in order:
                    with input_path.open('rb') as file:
                        begin = time.perf_counter()
                        output = subprocess.check_output([str(binaries[problem][name])], stdin=file, stderr=subprocess.DEVNULL)
                        elapsed = time.perf_counter() - begin
                    tokens = output.split()
                    if expected is None:
                        expected = tokens
                    if tokens != expected:
                        raise RuntimeError(f'Output mismatch: {problem} {name}')
                    rows.append({'problem': problem, 'n': n, 'm': m, 'variant': name, 'trial': trial, 'seconds': elapsed,
                                 'output_sha256': hashlib.sha256(b' '.join(tokens)).hexdigest()})
            print('measured', problem, 'full outputs match', flush=True)
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    summary = [{'problem': problem, 'variant': name, 'seconds': statistics.median(r['seconds'] for r in rows if r['problem'] == problem and r['variant'] == name)}
               for problem in DRIVERS for name in ('library', 'reference')]
    for filename, values in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / filename).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=values[0].keys()); writer.writeheader(); writer.writerows(values)


if __name__ == '__main__':
    main()
