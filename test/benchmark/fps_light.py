import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import re
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
QUOTE = re.compile(r'^\s*#\s*include\s*"([^"\r\n]+)"[^\r\n]*(?:\n|$)', re.MULTILINE)
OPERATIONS = ['multiply', 'square', 'inv', 'log', 'exp', 'sqrt', 'pow', 'div_series', 'divmod']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def expand(source, dependency_hashes):
    seen = set()

    def visit(path):
        path = path.resolve()
        if path in seen:
            return ''
        seen.add(path)
        name = path.relative_to(ROOT).as_posix()
        dependency_hashes[name] = sha(path)

        def include(match):
            target = path.parent / match[1]
            if not target.is_file():
                target = ROOT / match[1]
            if not target.is_file():
                raise FileNotFoundError((path, match[1]))
            return visit(target)

        return '#line 1 "' + name + '"\n' + QUOTE.sub(include, path.read_text())

    body = '#line 1 "source.cpp"\n' + QUOTE.sub(lambda match: visit(ROOT / match[1]), source)
    return body.encode('utf-8'), sorted(path.relative_to(ROOT).as_posix() for path in seen)


def sizes():
    hashes, rows = {}, []
    public = sorted((ROOT / 'fps').glob('fps_light*.hpp'))
    assert public
    for header in public:
        name = header.relative_to(ROOT).as_posix()
        for io in [False, True]:
            source = '#include "' + name + '"\n'
            if io:
                source += '#include "components/fast_io.hpp"\n'
            body, dependencies = expand(source + 'int main() {}\n', hashes)
            rows.append({'source': name, 'with_fast_io': io, 'expanded_bytes': len(body), 'expanded_sha256': hashlib.sha256(body).hexdigest(), 'dependencies': dependencies})
    for driver in sorted((ROOT / 'test').glob('fps_light_*.test.cpp')):
        body, dependencies = expand('#include "' + driver.relative_to(ROOT).as_posix() + '"\n', hashes)
        rows.append({'source': driver.relative_to(ROOT).as_posix(), 'expanded_bytes': len(body), 'expanded_sha256': hashlib.sha256(body).hexdigest(), 'dependencies': dependencies})
    assert all(row['expanded_bytes'] < 65536 for row in rows), rows
    return {'method': 'custom quoted-include expansion, every local file once, all preprocessing branches and standard includes retained; one #line per local file; NOT oj-bundle', 'limit_bytes': 65536, 'rows': rows, 'source_sha256': hashes}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--sizes', nargs='+', type=int, default=[1024, 32768, 131072, 131073])
    parser.add_argument('--operations', nargs='+', choices=OPERATIONS, default=OPERATIONS)
    parser.add_argument('--target', choices=['generic', 'haswell', 'native'], default='haswell')
    parser.add_argument('--size-only', action='store_true')
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_light/benchmark.json')
    args = parser.parse_args()
    if args.trials < 1 or min(args.sizes) < 2:
        parser.error('positive trials and sizes >= 2 are required')
    os.chdir(ROOT)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    size_report = sizes()
    size_path = args.output.with_name('sizes.json')
    size_path.write_text(json.dumps(size_report, indent=2) + '\n')
    if args.size_only:
        print(json.dumps(size_report, indent=2))
        return
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('--cpu is unavailable')
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-Wall', '-Wextra']
    if args.target != 'generic':
        flags += ['-march=' + args.target]
    hashes = size_report['source_sha256'].copy()
    expand('#include "fps/fps.hpp"\n', hashes)
    for name in ['test/benchmark/fps_light.cpp', 'test/benchmark/fps_light.py']:
        hashes[name] = sha(ROOT / name)
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(), 'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu], 'flags': flags, 'trials': args.trials, 'sizes': args.sizes, 'seed': 731, 'execution_order_seed': 20261021, 'source_sha256': hashes, 'initialization': 'random input construction outside timing; fresh process for every run; no transform prewarming; sqrt input has constant coefficient 17^2', 'results': [], 'binaries': {}, 'completed': False}
    samples = []

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        with args.output.with_suffix('.csv').open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=['operation', 'n', 'variant', 'trial', 'ms', 'result_size', 'remainder_size', 'checksum'], lineterminator='\n')
            writer.writeheader()
            writer.writerows(samples)

    save()
    order = random.Random(20261021)
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy, tempfile.TemporaryDirectory(prefix='fps-light-benchmark-') as temporary:
        fcntl.flock(lock, fcntl.LOCK_EX)
        fcntl.flock(legacy, fcntl.LOCK_EX)
        binaries = {}
        for variant in ['existing', 'light']:
            exe = Path(temporary) / variant
            extra = ['-DLIGHT_FPS'] if variant == 'light' else []
            command = [args.cxx, *flags, *extra, '-I', str(ROOT), 'test/benchmark/fps_light.cpp', '-o', str(exe)]
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
                        raw = subprocess.check_output([str(binaries[variant]), operation, str(n)], text=True, timeout=120).split()
                        ms, result_size, remainder_size, checksum = raw
                        values[variant].append(float(ms))
                        answers.add((result_size, remainder_size, checksum))
                        samples.append({'operation': operation, 'n': n, 'variant': variant, 'trial': trial, 'ms': float(ms), 'result_size': int(result_size), 'remainder_size': int(remainder_size), 'checksum': checksum})
                assert len(answers) == 1, (operation, n, answers)
                medians = {variant: statistics.median(ms) for variant, ms in values.items()}
                row = {'operation': operation, 'n': n, 'median_ms': medians, 'light_over_existing': medians['light'] / medians['existing'], 'answer': list(answers.pop())}
                report['results'].append(row)
                save()
                print(operation, n, medians, 'light/existing', row['light_over_existing'], flush=True)
    for name, digest in hashes.items():
        assert sha(ROOT / name) == digest, name
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()


if __name__ == '__main__':
    main()
