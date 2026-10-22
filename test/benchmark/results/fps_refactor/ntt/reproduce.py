import argparse
from contextlib import ExitStack, contextmanager
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import re
import shutil
import statistics
import subprocess
import tarfile
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
MANIFEST = json.loads((HERE / 'manifest.json').read_text())


def sha(data):
    return hashlib.sha256(data).hexdigest()


@contextmanager
def locked(exclusive=False):
    with ExitStack() as stack:
        names = ['/tmp/cp-fps-tuning.lock']
        if exclusive:
            names.append('/tmp/cp-wm-opt.lock')
        for name in names:
            lock = stack.enter_context(open(name, 'a'))
            fcntl.flock(lock, fcntl.LOCK_EX if exclusive else fcntl.LOCK_SH)
        yield


def execute(command, report, **kwargs):
    result = subprocess.run([str(value) for value in command], capture_output=True, text=True, **kwargs)
    report['commands'].append(dict(command=[str(value) for value in command],
        returncode=result.returncode, stdout=result.stdout, stderr=result.stderr))
    if result.returncode:
        raise RuntimeError((command, result.stdout, result.stderr))
    return result.stdout


def prepare(work, report):
    baseline = MANIFEST['baseline_commit']
    archive = subprocess.check_output(['git', 'archive', baseline, 'components', 'fps', 'math',
        'convolution', 'test/unit', 'test/fps_io.hpp'], cwd=ROOT)
    trees = {name: work / name for name in MANIFEST['stages']}
    trees['baseline'].mkdir()
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        source.extractall(trees['baseline'], filter='data')
    previous = 'baseline'
    for name, patch in zip(['neon', 'measured', 'final'], MANIFEST['patches']):
        shutil.copytree(trees[previous], trees[name])
        execute(['patch', '--batch', '--forward', '-p1', '-i', HERE / patch], report, cwd=trees[name])
        previous = name
    for name, tree in trees.items():
        header = (tree / 'components/ntt.hpp').read_bytes()
        expected = MANIFEST['stages'][name]
        assert len(header) == expected['bytes'] and sha(header) == expected['sha256'], name
    report['stage_hashes_verified'] = True
    compare = work / 'compare'
    compare.mkdir()
    for variant, name in [('Before', 'baseline'), ('After', 'final')]:
        source = (trees[name] / 'components/ntt.hpp').read_text()
        source = source[:source.index('inline NTT &ntt_plan')]
        source += '}\n#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))\n'
        source += '#pragma GCC pop_options\n#endif\n#endif\n'
        source = source.replace('NTT_998244353_HPP', 'NTT_COMPARE_' + variant + '_HPP')
        source = re.sub(r'\bNTT\b', 'NTT' + variant, source)
        source = source.replace('"../math/mint.hpp"', '"' + str(trees['baseline'] / 'math/mint.hpp') + '"')
        (compare / ('Compare' + variant + '.hpp')).write_text(source)
    return trees, compare


def checks(args, work, trees, compare, report):
    with locked():
        outputs = []
        for name in ['baseline', 'neon']:
            source = work / (name + '_include.cpp')
            source.write_text('#include "' + str(trees[name] / 'components/ntt.hpp') + '"\n')
            outputs.append(execute([args.cxx, '-std=c++20', '-E', '-P', source], report))
            report['commands'][-1]['stdout'] = '<preprocessed source omitted>'
        assert outputs[0] == outputs[1]
        report['neon_preprocessor_equal'] = True
        executable = work / 'primitive_check'
        execute([args.cxx, '-std=c++20', '-O2', '-I', compare, HERE / 'primitive_check.cpp',
                 '-o', executable], report)
        execute([executable], report)
        for compiler, extra in [(args.cxx, ['-U__linux__']), (args.clang, [])]:
            for name in ['fps_algorithms', 'power_projection', 'fps_tuning_advanced']:
                executable = work / (Path(compiler).name + '_' + name)
                execute([compiler, '-std=c++20', '-O2', '-I', trees['measured'], *extra,
                    trees['measured'] / 'test/unit' / (name + '.cpp'), '-o', executable], report)
                execute([executable], report)
        portable = work / 'portable'
        shutil.copytree(trees['measured'], portable)
        header = portable / 'components/ntt.hpp'
        source, count = re.subn(r'\tstatic bool vectorized\(\) \{.*?\n\t\}',
            '\tstatic bool vectorized() { return false; }', header.read_text(), count=1, flags=re.S)
        assert count == 1
        header.write_text(source)
        executable = work / 'portable_check'
        execute([args.cxx, '-std=c++20', '-O2', '-U__linux__', '-I', portable,
            portable / 'test/unit/fps_tuning_advanced.cpp', '-o', executable], report)
        execute([executable], report)
    report['checks_passed'] = True


def benchmark(args, work, trees, report):
    driver = ROOT / MANIFEST['benchmark']['driver']
    assert sha(driver.read_bytes()) == MANIFEST['benchmark']['driver_sha256']
    binary = {}
    with locked(exclusive=True):
        for variant, name in [('before', 'baseline'), ('after', 'measured')]:
            binary[variant] = work / (variant + '_advanced')
            execute([args.cxx, *MANIFEST['benchmark']['flags'], '-I', trees[name], driver,
                '-o', binary[variant]], report)
        affinity = os.sched_getaffinity(0)
        os.sched_setaffinity(0, {args.cpu})
        try:
            rng = random.Random(20261021)
            rows = []
            for n in args.sizes:
                samples = dict(before=[], after=[])
                expected = None
                for trial in range(args.trials):
                    order = ['before', 'after']
                    rng.shuffle(order)
                    for variant in order:
                        result = execute([binary[variant], 'compose', str(n)], report).split()
                        if expected is None:
                            expected = result[1:]
                        assert result[1:] == expected
                        samples[variant].append(float(result[0]))
                median = {name: statistics.median(values) for name, values in samples.items()}
                rows.append(dict(n=n, samples_ms=samples, median_ms=median,
                    after_over_before=median['after'] / median['before'], answer=expected))
            report['benchmark'] = rows
        finally:
            os.sched_setaffinity(0, affinity)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--clang', default='clang++')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--benchmark', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=3)
    parser.add_argument('--sizes', type=int, nargs='+', default=[131072, 2097153])
    args = parser.parse_args()
    if args.work:
        work = args.work.resolve()
        work.mkdir(parents=True, exist_ok=False)
    else:
        work = Path(tempfile.mkdtemp(prefix='fps-ntt-reproduce-'))
    report = dict(work=str(work), completed=False, commands=[])
    print(work, flush=True)
    try:
        trees, compare = prepare(work, report)
        if args.check:
            checks(args, work, trees, compare, report)
        if args.benchmark:
            benchmark(args, work, trees, report)
        report['completed'] = True
    finally:
        (work / 'reproduction.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
