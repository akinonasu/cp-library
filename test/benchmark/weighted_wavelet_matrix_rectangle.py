import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=Path('test/benchmark/results/weighted_wavelet_matrix_rectangle.json'))
    parser.add_argument('--problem-root', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/data_structure/point_add_rectangle_sum')
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    os.chdir(root)
    os.sched_setaffinity(0, {args.cpu})
    lock = open('/tmp/cp-wm-opt.lock', 'w')
    cache = root / '.verify-helper/cache/wm-weight/reference'
    metadata = json.loads((root / 'test/benchmark/results/weighted_wavelet_matrix_reference.json').read_text())
    flags = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE', '-I.']
    rust = '/home/qand/.cargo/bin/rustc'
    rustflags = ['--edition=2021', '-C', 'opt-level=3', '-C', 'target-cpu=native']
    files = ['data_structure/weighted_wavelet_matrix.hpp', 'data_structure/wavelet_matrix.hpp', 'components/fenwick_sum.hpp',
             'components/quad_bit_vector.hpp', 'components/oct_bit_vector.hpp', 'components/splitmix64.hpp', 'components/fast_io.hpp',
             'test/weighted_wavelet_matrix_rectangle.test.cpp', 'test/benchmark/weighted_wavelet_matrix_rectangle.py']
    frozen = {p: sha(root / p) for p in files}
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), flags=flags, rustflags=rustflags,
                  gcc=subprocess.check_output(['g++', '--version'], text=True), rust=subprocess.check_output([rust, '--version'], text=True),
                  cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu], trials=args.trials, warmup=1,
                  order_seed=20261019, source_sha256=frozen, reference_sha256={}, correctness=[], results=[], completed=False,
                  note='Whole programs with their own I/O. Current retains both weight and key*weight sums using int128; references maintain weight sums only.')
    with tempfile.TemporaryDirectory(prefix='wm-rectangle-') as directory:
        work = Path(directory)
        binaries = {'current': work / 'current'}
        fcntl.flock(lock, fcntl.LOCK_EX)
        subprocess.run(['g++', *flags, 'test/weighted_wavelet_matrix_rectangle.test.cpp', '-o', binaries['current']], check=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        for identity in [401818, 206958]:
            row = next(x for x in metadata['references'] if x['id'] == identity)
            source = cache / (str(identity) + '.cpp')
            assert sha(source) == row['source_sha256']
            report['reference_sha256'][str(identity)] = sha(source)
            binary = work / str(identity)
            fcntl.flock(lock, fcntl.LOCK_EX)
            if identity == 401818:
                copied = work / 'reference.rs'
                copied.write_bytes(source.read_bytes())
                subprocess.run([rust, *rustflags, copied, '-o', binary], check=True)
            else:
                subprocess.run(['g++', *flags, source, '-o', binary], check=True)
            fcntl.flock(lock, fcntl.LOCK_UN)
            binaries[str(identity)] = binary
        for case in sorted((args.problem_root / 'in').glob('*.in')):
            inp, out = work / 'input', work / 'output'
            inp.write_bytes(case.read_bytes())
            answer = args.problem_root / 'out' / case.with_suffix('.out').name
            for name, binary in binaries.items():
                fcntl.flock(lock, fcntl.LOCK_EX)
                with inp.open('rb') as stream, out.open('wb') as dest:
                    subprocess.run([binary], stdin=stream, stdout=dest, check=True)
                fcntl.flock(lock, fcntl.LOCK_UN)
                subprocess.run([args.problem_root / 'checker', inp, out, answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            report['correctness'].append(dict(case=case.stem, input_sha256=sha(inp), answer_sha256=sha(answer), passed=True))
        rng = random.Random(20261019)
        for case in ['max_random_00', 'many_points_00', 'many_queries_00', 'xy_concentrate_00']:
            inp = work / 'input'
            inp.write_bytes((args.problem_root / 'in' / (case + '.in')).read_bytes())
            samples = {name: [] for name in binaries}
            outputs = set()
            fcntl.flock(lock, fcntl.LOCK_EX)
            for trial in range(-1, args.trials):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    out = work / 'output'
                    with inp.open('rb') as stream, out.open('wb') as dest:
                        import time
                        start = time.perf_counter()
                        subprocess.run([binaries[name]], stdin=stream, stdout=dest, check=True)
                        ms = (time.perf_counter() - start) * 1000
                    outputs.add(sha(out))
                    if trial >= 0:
                        samples[name].append(ms)
            fcntl.flock(lock, fcntl.LOCK_UN)
            assert len(outputs) == 1
            row = dict(case=case, input_sha256=sha(inp), output_sha256=outputs.pop(), samples_ms=samples,
                       median_ms={name: statistics.median(xs) for name, xs in samples.items()})
            report['results'].append(row)
            print(case, row['median_ms'], flush=True)
    for path, expected in frozen.items():
        assert sha(root / path) == expected, path
    report['completed'] = True
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print('All reference outputs and frozen source hashes match', flush=True)


if __name__ == '__main__':
    main()
