import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import re
import statistics
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[2]
QUOTE = re.compile(r'^\s*#\s*include\s*"([^"\r\n]+)"', re.MULTILINE)
CORE = ['multiply', 'square', 'inv', 'log', 'exp', 'sqrt', 'pow', 'div_series', 'divmod']
ALGORITHMS = ['taylor_shift', 'multipoint_eval', 'interpolate', 'linear_recurrence']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dependency_hashes(root, driver):
    hashes = {}

    def visit(path):
        path = path.resolve()
        name = path.relative_to(root).as_posix()
        if name in hashes:
            return
        hashes[name] = sha(path)
        for match in QUOTE.finditer(path.read_text()):
            dependency = path.parent / match[1]
            if not dependency.is_file():
                dependency = root / match[1]
            visit(dependency)

    visit(root / driver)
    return hashes


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='5e7f6ec')
    parser.add_argument('--baseline-tree', type=Path)
    parser.add_argument('--candidate-tree', type=Path, default=ROOT)
    parser.add_argument('--suite', choices=['core', 'algorithms'], default='core')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--sizes', nargs='+', type=int)
    parser.add_argument('--operations', nargs='+')
    parser.add_argument('--target', choices=['generic', 'haswell', 'native'], default='haswell')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    operations = CORE if args.suite == 'core' else ALGORITHMS
    args.operations = args.operations or operations
    args.sizes = args.sizes or ([1024, 32768, 131072, 131073, 524288] if args.suite == 'core' else [1024, 32768])
    if args.trials < 1 or min(args.sizes) < 2 or any(op not in operations for op in args.operations):
        parser.error('positive trials, sizes >= 2 and operations from the selected suite are required')
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('--cpu is unavailable')
    os.chdir(ROOT)
    os.sched_setaffinity(0, {args.cpu})
    args.candidate_tree = args.candidate_tree.resolve()
    filename = 'benchmark.json' if args.suite == 'core' else 'algorithms.json'
    args.output = args.output or ROOT / 'test/benchmark/results/fps_light_speed' / filename
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-Wall', '-Wextra']
    if args.target != 'generic':
        flags += ['-march=' + args.target]
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], text=True).strip()
    driver = 'test/benchmark/fps_light.cpp' if args.suite == 'core' else 'test/benchmark/fps_light_algorithms.cpp'
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'baseline_revision': baseline, 'candidate_revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(), 'suite': args.suite, 'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu], 'flags': flags, 'trials': args.trials, 'sizes': args.sizes, 'seed': 731, 'execution_order_seed': 20261021, 'harness_sha256': sha(Path(__file__)), 'initialization': 'fresh process for every sample; no transform prewarming; input construction and canonical coefficient checksumming outside timing; allocation included; sqrt constant17^2; distinct evaluation points0..n-1; recurrence k=10^18', 'results': [], 'binaries': {}, 'sources': {}, 'completed': False}
    samples = []

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        with args.output.with_suffix('.csv').open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=['operation', 'n', 'variant', 'trial', 'ms', 'result_size', 'remainder_size', 'checksum'], lineterminator='\n')
            writer.writeheader()
            writer.writerows(samples)

    save()
    order = random.Random(20261021)
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy, tempfile.TemporaryDirectory(prefix='fps-light-speed-') as temporary:
        fcntl.flock(lock, fcntl.LOCK_EX)
        fcntl.flock(legacy, fcntl.LOCK_EX)
        work = Path(temporary)
        before = args.baseline_tree.resolve() if args.baseline_tree else work / 'baseline'
        if not args.baseline_tree:
            archived = subprocess.check_output(['git', 'archive', baseline, 'components', 'fps', 'math', 'convolution', 'test/benchmark/fps_light.cpp', 'test/benchmark/fps_light_algorithms.cpp'])
            with tarfile.open(fileobj=io.BytesIO(archived)) as archive:
                archive.extractall(before, filter='data')
        baseline_hashes = dependency_hashes(before, driver)
        for name, digest in baseline_hashes.items():
            expected = hashlib.sha256(subprocess.check_output(['git', 'show', baseline + ':' + name])).hexdigest()
            assert digest == expected, ('modified baseline snapshot', name)
        candidate_hashes = dependency_hashes(args.candidate_tree, driver)
        assert sha(before / driver) == sha(args.candidate_tree / driver), 'benchmark input generators differ'
        report['sources'] = {'baseline': {'root': str(before), 'sha256': baseline_hashes}, 'candidate': {'root': str(args.candidate_tree), 'sha256': candidate_hashes}}
        binaries = {}
        for variant, tree in [('before', before), ('after', args.candidate_tree), ('heavy', before)]:
            exe = work / variant
            extra = [] if variant == 'heavy' else ['-DLIGHT_FPS']
            command = [args.cxx, *flags, *extra, '-I', str(tree), str(before / driver), '-o', str(exe)]
            built = subprocess.run(command, capture_output=True, text=True, check=True)
            report['binaries'][variant] = {'command': command, 'sha256': sha(exe), 'bytes': exe.stat().st_size, 'compiler_stdout': built.stdout, 'compiler_stderr': built.stderr}
            binaries[variant] = exe
        for n in args.sizes:
            for operation in args.operations:
                values = {variant: [] for variant in binaries}
                answers = set()
                for trial in range(args.trials):
                    variants = list(binaries)
                    order.shuffle(variants)
                    for variant in variants:
                        raw = subprocess.check_output([str(binaries[variant]), operation, str(n)], text=True, timeout=180).split()
                        ms, result_size = float(raw[0]), int(raw[1])
                        remainder_size = int(raw[2]) if args.suite == 'core' else -1
                        checksum = raw[-1]
                        values[variant].append(ms)
                        answers.add((result_size, remainder_size, checksum))
                        samples.append({'operation': operation, 'n': n, 'variant': variant, 'trial': trial, 'ms': ms, 'result_size': result_size, 'remainder_size': remainder_size, 'checksum': checksum})
                assert len(answers) == 1, (operation, n, answers)
                medians = {variant: statistics.median(ms) for variant, ms in values.items()}
                row = {'operation': operation, 'n': n, 'median_ms': medians, 'before_over_after': medians['before'] / medians['after'], 'after_over_heavy': medians['after'] / medians['heavy'], 'answer': list(answers.pop())}
                report['results'].append(row)
                save()
                print(operation, n, medians, 'before/after', row['before_over_after'], 'after/heavy', row['after_over_heavy'], flush=True)
        for tree, hashes in [(before, baseline_hashes), (args.candidate_tree, candidate_hashes)]:
            for name, digest in hashes.items():
                assert sha(tree / name) == digest, name
    assert report['harness_sha256'] == sha(Path(__file__))
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()


if __name__ == '__main__':
    main()
