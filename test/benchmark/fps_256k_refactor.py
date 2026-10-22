import argparse
import csv
from datetime import datetime, timezone
import fcntl
import hashlib
import importlib.util
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
OPERATIONS = {
    'core': ['multiply', 'square', 'inv', 'log', 'exp', 'sqrt', 'pow', 'div_series', 'divmod'],
    'algorithms': ['taylor_shift', 'multipoint_eval', 'interpolate', 'linear_recurrence'],
    'advanced': ['compose', 'compose_stride', 'compose_valuation', 'reversion', 'projection',
                 'projection_weighted', 'projection_short', 'gcd', 'extended_gcd', 'inv_mod'],
}
SIZES = {'core': [1024, 32768, 131072, 131073, 524288],
         'algorithms': [1024, 32768], 'advanced': [4096, 32768]}
DRIVERS = {'core': 'test/benchmark/fps_light.cpp',
           'algorithms': 'test/benchmark/fps_light_algorithms.cpp',
           'advanced': 'test/benchmark/fps_256k_advanced.cpp'}
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE',
         '-ftrivial-auto-var-init=zero', '-march=haswell', '-Wall', '-Wextra']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dependencies(root, driver):
    files = {}

    def visit(path):
        path = path.resolve()
        name = path.relative_to(root).as_posix() if path.is_relative_to(root) else '@driver/' + path.name
        if name in files:
            assert files[name]['path'] == str(path)
            return
        files[name] = {'path': str(path), 'sha256': sha(path)}
        for match in QUOTE.finditer(path.read_text()):
            dependency = path.parent / match[1]
            if not dependency.is_file():
                dependency = root / match[1]
            visit(dependency)

    visit(driver)
    return files


def verify(files):
    for name, entry in files.items():
        assert sha(Path(entry['path'])) == entry['sha256'], ('source changed during measurement', name)


def validate_baseline(files, root, revision, cache):
    for entry in files.values():
        path = Path(entry['path']).resolve()
        if not path.is_relative_to(root):
            continue
        name = path.relative_to(root).as_posix()
        if name not in cache:
            cache[name] = hashlib.sha256(subprocess.check_output(['git', 'show', revision + ':' + name])).hexdigest()
        assert entry['sha256'] == cache[name], ('modified baseline snapshot', name)


def compact_module(tree):
    path = tree / 'tools/compact_fps.py'
    spec = importlib.util.spec_from_file_location('compact_' + sha(path), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def bundle_sizes(before, after, bundler, artifact_directory, revision, baseline_cache):
    result = {'tool': {'path': bundler, 'sha256': sha(Path(bundler).resolve())}, 'profiles': {}}
    headers = sorted(path.relative_to(before).as_posix() for path in (before / 'fps').glob('*.hpp')
                     if not path.name.startswith('fps_light'))
    assert headers == sorted(path.relative_to(after).as_posix() for path in (after / 'fps').glob('*.hpp')
                             if not path.name.startswith('fps_light'))
    profiles = {}
    for name, selected in [('normal', headers), ('normal_and_light', sorted(path.relative_to(before).as_posix()
                                                                          for path in (before / 'fps').glob('*.hpp')))]:
        body = ''.join('#include "' + header + '"\n' for header in selected)
        body += '#include "components/fast_io.hpp"\n#include "test/fps_io.hpp"\nint main() {}\n'
        profiles[name] = (body, selected + ['components/fast_io.hpp', 'test/fps_io.hpp'])
    usage = ROOT / 'test/unit/fps_public_headers.cpp'
    if usage.is_file():
        unit_source = usage.read_text()
        usage_hash = hashlib.sha256(unit_source.encode()).hexdigest()
        prefix = ''.join('#include "' + header + '"\n' for header in headers)
        prefix += '#include "components/fast_io.hpp"\n#include "test/fps_io.hpp"\n'
        prefix += '#include <cassert>\n#include <cstdio>\n#include <cstdlib>\n#include <string_view>\n#include <type_traits>\n'
        prefix += '#if defined(CP_FPS_HEADERS_ACL)\n#include <atcoder/modint>\n#endif\n'
        profiles['public_api_usage'] = (prefix + unit_source[unit_source.index('void require('):],
                                         headers + ['components/fast_io.hpp', 'test/fps_io.hpp'])
        result['public_api_usage_fixture'] = {
            'source': 'test/unit/fps_public_headers.cpp', 'sha256': usage_hash,
            'method': 'directly include each normal public header once, FastIO and test/fps_io.hpp; retain require/check/main from the unit driver; omit conditional repeated-include stress prefixes',
        }
    current_tool_hash = sha(after / 'tools/compact_fps.py')
    current_tool = compact_module(after)
    for profile, (body, included) in profiles.items():
        result['profiles'][profile] = {'headers': included, 'input_sha256': hashlib.sha256(body.encode()).hexdigest(),
                                        'variants': {}}
        for variant, tree in [('before', before), ('after', after)]:
            with tempfile.TemporaryDirectory(prefix='fps-256k-union-') as temporary:
                source = Path(temporary) / 'all_fps.cpp'
                source.write_text(body)
                files = dependencies(tree, source)
                files.pop('@driver/all_fps.cpp')
                files['@compact_script'] = {'path': str(tree / 'tools/compact_fps.py'),
                                            'sha256': sha(tree / 'tools/compact_fps.py')}
                files['@current_compact_script'] = {'path': str(after / 'tools/compact_fps.py'),
                                                    'sha256': current_tool_hash}
                if profile == 'public_api_usage':
                    files['@public_api_usage'] = {'path': str(usage),
                                                 'sha256': usage_hash}
                if variant == 'before':
                    validate_baseline(files, before, revision, baseline_cache)
                command = [bundler, '-I', str(tree), str(source)]
                bundled = subprocess.run(command, capture_output=True, text=True, check=True, cwd=tree)
                raw = bundled.stdout
                compacted = compact_module(tree).compact(raw)
                current_compacted = current_tool.compact(raw) if variant == 'before' else compacted
                if artifact_directory:
                    directory = artifact_directory / profile
                    directory.mkdir(parents=True, exist_ok=True)
                    (directory / (variant + '_raw.cpp')).write_text(raw)
                    (directory / (variant + '_compact.cpp')).write_text(compacted)
                verify(files)
                result['profiles'][profile]['variants'][variant] = {
                    'raw_bytes': len(raw.encode()), 'compact_bytes': len(compacted.encode()),
                    'current_tool_compact_bytes': len(current_compacted.encode()),
                    'raw_sha256': hashlib.sha256(raw.encode()).hexdigest(),
                    'compact_sha256': hashlib.sha256(compacted.encode()).hexdigest(),
                    'compact_script_sha256': sha(tree / 'tools/compact_fps.py'),
                    'under_256k': len(compacted.encode()) < 256 * 1024,
                    'remaining_bytes': 256 * 1024 - len(compacted.encode()),
                    'command': command, 'stderr': bundled.stderr, 'sources': files,
                }
                print('bundle', profile, variant, len(raw.encode()), len(compacted.encode()), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='32f6930')
    parser.add_argument('--baseline-tree', type=Path)
    parser.add_argument('--candidate-tree', type=Path, default=ROOT)
    parser.add_argument('--suite', choices=[*OPERATIONS, 'all'], default='all')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--sizes', nargs='+', type=int)
    parser.add_argument('--operations', nargs='+')
    parser.add_argument('--output', type=Path)
    parser.add_argument('--bundler')
    parser.add_argument('--bundle-directory', type=Path)
    parser.add_argument('--size-only', action='store_true')
    args = parser.parse_args()
    if args.trials < 1 or args.sizes and min(args.sizes) < 2:
        parser.error('positive trials and sizes >= 2 are required')
    if args.cpu not in os.sched_getaffinity(0):
        parser.error('--cpu is unavailable')
    if args.suite == 'all' and args.operations:
        parser.error('--operations requires an individual --suite')
    if args.operations and any(operation not in OPERATIONS[args.suite] for operation in args.operations):
        parser.error('operation does not belong to the selected suite')
    if args.size_only and not args.bundler:
        parser.error('--size-only requires --bundler')
    os.chdir(ROOT)
    os.sched_setaffinity(0, {args.cpu})
    after = args.candidate_tree.resolve()
    suites = list(OPERATIONS) if args.suite == 'all' else [args.suite]
    args.output = args.output or ROOT / 'test/benchmark/results/fps_256k_refactor' / (args.suite + '.json')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], text=True).strip()
    report = {
        'started_at': datetime.now(timezone.utc).isoformat(), 'completed': False,
        'baseline_revision': baseline,
        'candidate_revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
        'suite': args.suite, 'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
        'cpu': subprocess.check_output(['lscpu'], text=True), 'affinity': [args.cpu],
        'flags': FLAGS, 'trials': args.trials, 'seed': 731, 'execution_order_seed': 20261021,
        'harness_sha256': sha(Path(__file__)), 'sources': {}, 'binaries': {}, 'results': [],
        'initialization': 'fresh process per sample; no prewarming; input preparation and full canonical coefficient checksumming excluded; allocation included; sqrt constant17^2; distinct evaluation points0..n-1; recurrence k=10^18',
    }
    samples = []

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')
        with args.output.with_suffix('.csv').open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=['suite', 'operation', 'n', 'variant', 'trial', 'ms',
                                                       'result_size', 'second_size', 'third_size', 'checksum'],
                                    lineterminator='\n')
            writer.writeheader()
            writer.writerows(samples)

    save()
    order = random.Random(20261021)
    with tempfile.TemporaryDirectory(prefix='fps-256k-refactor-') as temporary:
        work = Path(temporary)
        before = args.baseline_tree.resolve() if args.baseline_tree else work / 'baseline'
        if not args.baseline_tree:
            archived = subprocess.check_output(['git', 'archive', baseline, 'components', 'fps', 'math',
                                                'convolution', 'test/fps_io.hpp', 'tools/compact_fps.py',
                                                DRIVERS['core'], DRIVERS['algorithms']])
            with tarfile.open(fileobj=io.BytesIO(archived)) as archive:
                archive.extractall(before, filter='data')
        baseline_cache = {}
        if args.bundler:
            with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
                fcntl.flock(lock, fcntl.LOCK_SH)
                report['bundle'] = bundle_sizes(before, after, args.bundler, args.bundle_directory, baseline, baseline_cache)
            save()
        if not args.size_only:
            binaries = {}
            with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
                fcntl.flock(lock, fcntl.LOCK_SH)
                for suite in suites:
                    driver = ROOT / DRIVERS[suite] if suite == 'advanced' else before / DRIVERS[suite]
                    report['sources'][suite] = {}
                    report['binaries'][suite] = {}
                    binaries[suite] = {}
                    for variant, tree in [('before', before), ('after', after)]:
                        files = dependencies(tree, driver)
                        if variant == 'before':
                            validate_baseline(files, before, baseline, baseline_cache)
                        report['sources'][suite][variant] = {'root': str(tree), 'files': files}
                        executable = work / (suite + '_' + variant)
                        command = [args.cxx, *FLAGS, '-I', str(tree), str(driver), '-o', str(executable)]
                        built = subprocess.run(command, capture_output=True, text=True, check=True)
                        verify(files)
                        report['binaries'][suite][variant] = {
                            'command': command, 'sha256': sha(executable), 'bytes': executable.stat().st_size,
                            'compiler_stdout': built.stdout, 'compiler_stderr': built.stderr,
                        }
                        binaries[suite][variant] = executable
                        save()
                        print('built', suite, variant, executable.stat().st_size, flush=True)
                save()
            with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as legacy:
                fcntl.flock(lock, fcntl.LOCK_EX)
                fcntl.flock(legacy, fcntl.LOCK_EX)
                for suite in suites:
                    for variant in ['before', 'after']:
                        verify(report['sources'][suite][variant]['files'])
                for suite in suites:
                    sizes = args.sizes or SIZES[suite]
                    operations = args.operations or OPERATIONS[suite]
                    for n in sizes:
                        for operation in operations:
                            values = {variant: [] for variant in binaries[suite]}
                            answers = set()
                            for trial in range(args.trials):
                                variants = list(binaries[suite])
                                order.shuffle(variants)
                                for variant in variants:
                                    raw = subprocess.check_output([str(binaries[suite][variant]), operation, str(n)],
                                                                  text=True, timeout=300).split()
                                    ms, lengths, checksum = float(raw[0]), [int(value) for value in raw[1:-1]], raw[-1]
                                    lengths += [-1] * (3 - len(lengths))
                                    answers.add((*lengths, checksum))
                                    values[variant].append(ms)
                                    samples.append({'suite': suite, 'operation': operation, 'n': n, 'variant': variant,
                                                    'trial': trial, 'ms': ms, 'result_size': lengths[0],
                                                    'second_size': lengths[1], 'third_size': lengths[2], 'checksum': checksum})
                            assert len(answers) == 1, (suite, operation, n, answers)
                            medians = {variant: statistics.median(value) for variant, value in values.items()}
                            row = {'suite': suite, 'operation': operation, 'n': n, 'median_ms': medians,
                                   'after_over_before': medians['after'] / medians['before'], 'answer': list(answers.pop())}
                            report['results'].append(row)
                            save()
                            print(suite, operation, n, medians, 'after/before', row['after_over_before'], flush=True)
            for suite in suites:
                for variant in ['before', 'after']:
                    verify(report['sources'][suite][variant]['files'])
        if args.bundler:
            for profile in report['bundle']['profiles'].values():
                for variant in ['before', 'after']:
                    verify(profile['variants'][variant]['sources'])
        assert report['harness_sha256'] == sha(Path(__file__))
        report['completed'] = True
        report['completed_at'] = datetime.now(timezone.utc).isoformat()
        save()


if __name__ == '__main__':
    main()
