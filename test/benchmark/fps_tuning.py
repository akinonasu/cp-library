import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'fd41ae5df33960b93288ae98f9edac4e9fdb166f'
DRIVERS = {
    'multiply': ('convolution_mod', 'convolution.test.cpp'),
    'inv': ('inv_of_formal_power_series', 'fps_inv.test.cpp'),
    'log': ('log_of_formal_power_series', 'fps_log.test.cpp'),
    'exp': ('exp_of_formal_power_series', 'fps_exp.test.cpp'),
    'pow': ('pow_of_formal_power_series', 'fps_pow.test.cpp'),
    'sqrt': ('sqrt_of_formal_power_series', 'fps_sqrt.test.cpp'),
    'divmod': ('division_of_polynomials', 'fps_divmod.test.cpp'),
    'compose': ('composition_of_formal_power_series', 'fps_compose.test.cpp'),
    'compose_large': ('composition_of_formal_power_series_large', 'fps_compose_large.test.cpp'),
    'compositional_inverse': ('compositional_inverse_of_formal_power_series', 'fps_compositional_inverse.test.cpp'),
    'compositional_inverse_large': ('compositional_inverse_of_formal_power_series_large', 'fps_compositional_inverse_large.test.cpp'),
    'power_projection': ('compositional_inverse_of_formal_power_series_large', 'power_projection.test.cpp'),
    'bostan_mori': ('kth_term_of_linearly_recurrent_sequence', 'fps_bostan_mori.test.cpp'),
    'linear_recurrence': ('kth_term_of_linearly_recurrent_sequence', 'fps_linear_recurrence.test.cpp'),
    'multipoint_eval': ('multipoint_evaluation', 'multipoint_eval.test.cpp'),
    'interpolate': ('polynomial_interpolation', 'fps_interpolate.test.cpp'),
    'taylor_shift': ('polynomial_taylor_shift', 'fps_taylor_shift.test.cpp'),
    'inv_mod': ('inv_of_polynomials', 'polynomial_inv_mod.test.cpp'),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--references', type=Path, default=Path('.verify-helper/cache/fps-tuning-reference'))
    parser.add_argument('--reference-metadata', type=Path, default=Path('test/benchmark/results/fps_tuning_reference.json'))
    parser.add_argument('--official-root', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems')
    parser.add_argument('--only', nargs='*')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--kernel-only', action='store_true')
    parser.add_argument('--judge-only', action='store_true')
    parser.add_argument('--cases', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    lock = open('/tmp/cp-fps-tuning.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero']
    if not args.generic:
        flags.append('-march=native')
    paths = sorted(p for folder in ['fps', 'components', 'math', 'convolution'] for p in (ROOT / folder).glob('*.hpp'))
    metadata = json.loads(args.reference_metadata.read_text())
    environment = dict(date=datetime.now(timezone.utc).isoformat(), baseline=BASELINE, flags=flags,
                       compiler=subprocess.check_output(['g++', '--version'], text=True),
                       cpu=subprocess.check_output(['lscpu'], text=True), uname=subprocess.check_output(['uname', '-a'], text=True),
                       affinity=[args.cpu], trials=args.trials, warmup=1, seed=20261018,
                       current_sha256={p.relative_to(ROOT).as_posix(): sha(p) for p in paths},
                       benchmark_sha256={p: sha(ROOT / p) for p in ['test/benchmark/fps_tuning.cpp', 'test/benchmark/fps_tuning.py', 'test/fps_io.hpp'] + ['test/' + driver for driver in sorted(set(row[1] for row in DRIVERS.values()))]},
                       method='CPU affinity and advisory exclusive lock; kernel excludes input generation/checksum; judge includes process start, regular-file I/O and exit; checker/SHA outside timed region')
    rng = random.Random(20261018)
    results = dict(kernel={}, judge={})
    with tempfile.TemporaryDirectory(prefix='cp-fps-tuning-', dir='/tmp') as directory:
        work = Path(directory)
        trees = {name: work / name for name in ['before', 'current']}
        with exclusive():
            archive = subprocess.check_output(['git', 'archive', BASELINE, 'fps', 'components', 'math', 'convolution', 'test'], cwd=ROOT)
            with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
                for member in tar:
                    if not member.isfile():
                        continue
                    target = trees['before'] / member.name
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(tar.extractfile(member).read())
            for path in paths:
                target = trees['current'] / path.relative_to(ROOT)
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(path.read_bytes())
            for name, tree in trees.items():
                (tree / 'test').mkdir(exist_ok=True)
                for driver in set(row[1] for row in DRIVERS.values()) | {'fps_io.hpp'}:
                    target = tree / 'test' / driver
                    if name == 'before' and target.exists() and driver != 'fps_io.hpp':
                        (tree / 'test' / ('stdio_' + driver)).write_bytes(target.read_bytes())
                    target.write_bytes((ROOT / 'test' / driver).read_bytes())
            environment['before_sha256'] = {p.relative_to(trees['before']).as_posix(): sha(p) for folder in ['fps', 'components', 'math', 'convolution'] for p in (trees['before'] / folder).glob('*.hpp')}
        if not args.judge_only:
            binaries = {}
            for name, tree in trees.items():
                with exclusive():
                    binary = work / ('kernel_' + name)
                    subprocess.run(['g++', *flags, '-I', str(tree), str(ROOT / 'test/benchmark/fps_tuning.cpp'), '-o', str(binary)], check=True)
                    binaries[name] = binary
            operations = ['multiply', 'square', 'inv', 'log', 'exp', 'pow', 'sqrt', 'div_series', 'divmod',
                          'compose', 'compositional_inverse', 'power_projection', 'bostan_mori', 'linear_recurrence',
                          'multipoint_eval', 'interpolate', 'taylor_shift', 'gcd', 'inv_mod']
            for operation in operations:
                if args.only and operation not in args.only:
                    continue
                sizes = [1024, 32768, 131072] if operation in ['compose', 'compositional_inverse', 'power_projection', 'multipoint_eval', 'interpolate', 'bostan_mori', 'linear_recurrence'] else [1024, 32768, 524288]
                if operation in ['gcd', 'inv_mod']:
                    sizes = [1024, 16384, 50000]
                for n in sizes:
                    with exclusive():
                        samples = {name: [] for name in binaries}
                        expected = None
                        for trial in range(-1, args.trials):
                            order = list(binaries)
                            rng.shuffle(order)
                            for name in order:
                                row = json.loads(subprocess.check_output([binaries[name], operation, str(n)], text=True))
                                if expected is None:
                                    expected = row['checksum']
                                if expected != row['checksum']:
                                    raise RuntimeError((operation, n, name, expected, row))
                                if trial >= 0:
                                    samples[name].append(row['ms'])
                        key = operation + '/' + str(n)
                        results['kernel'][key] = dict(samples_ms=samples, checksum=expected, median_ms={name: statistics.median(values) for name, values in samples.items()})
                        print('kernel', key, results['kernel'][key]['median_ms'], flush=True)
        if not args.kernel_only:
            official = {path.name: path for path in args.official_root.glob('*/*') if (path / 'info.toml').exists()}
            for operation, (problem, driver) in DRIVERS.items():
                if args.only and operation not in args.only:
                    continue
                folder = official[problem]
                references = metadata['problems'][problem]['references']
                reference = next(row for row in references if row['lang'].startswith('cpp'))
                identity = str(reference['id'])
                binaries = {}
                for name, tree in trees.items():
                    with exclusive():
                        binary = work / (operation + '_' + name)
                        subprocess.run(['g++', *flags, '-I', str(tree), str(tree / 'test' / driver), '-o', str(binary)], check=True)
                        binaries[name] = binary
                original = trees['before'] / 'test' / ('stdio_' + driver)
                if original.exists():
                    with exclusive():
                        binary = work / (operation + '_before_stdio')
                        subprocess.run(['g++', *flags, '-I', str(trees['before']), str(original), '-o', str(binary)], check=True)
                        binaries['before_stdio'] = binary
                with exclusive():
                    source = args.references / problem / (identity + '.cpp')
                    if sha(source) != reference['source_sha256']:
                        raise RuntimeError('Reference SHA differs: ' + str(source))
                    binary = work / (operation + '_reference')
                    subprocess.run(['g++', *flags, str(source), '-o', str(binary)], check=True)
                    binaries['reference_' + identity] = binary
                cases = sorted((folder / 'in').glob('*.in'), key=lambda path: (-path.stat().st_size, path.name))
                if operation == 'sqrt':
                    cases = [p for p in cases if not (folder / 'out' / (p.stem + '.out')).read_bytes().startswith(b'-1')]
                if operation == 'pow':
                    cases = [p for p in cases if (folder / 'out' / (p.stem + '.out')).read_bytes().split(b' ', 1)[0].strip() != b'0']
                cases = cases[:args.cases]
                if not cases:
                    raise RuntimeError('Official inputs not generated: ' + problem)
                for source in cases:
                    with exclusive():
                        inp, out = work / 'input', work / 'output'
                        shutil.copyfile(source, inp)
                        answer = folder / 'out' / (source.stem + '.out')
                        samples, hashes = {name: [] for name in binaries}, {}
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
                                    subprocess.run([folder / 'checker', inp, out, answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                                    hashes[name] = digest
                                if trial >= 0:
                                    samples[name].append(ms)
                        key = operation + '/' + source.stem
                        results['judge'][key] = dict(problem=problem, reference=identity, input_sha256=sha(inp), answer_sha256=sha(answer),
                            variants={name: dict(samples_ms=values, median_ms=statistics.median(values), output_sha256=hashes[name]) for name, values in samples.items()})
                        print('judge', key, {name: row['median_ms'] for name, row in results['judge'][key]['variants'].items()}, flush=True)
                (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
        for path, digest in environment['current_sha256'].items():
            if sha(ROOT / path) != digest:
                raise RuntimeError('Source changed during benchmark: ' + path)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
