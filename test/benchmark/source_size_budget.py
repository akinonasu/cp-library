import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
import fcntl
import hashlib
import itertools
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.bundle import LIMIT, fit_source
from tools.compact_fps import TOKEN


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bundle', default='oj-bundle')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--clang')
    parser.add_argument('--jobs', type=int, default=3)
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    headers = sorted(path for directory in ('components', 'convolution', 'fps', 'data_structure', 'graph',
                                            'tree', 'math', 'sequence', 'string', 'geometry')
                     for path in (ROOT / directory).rglob('*.hpp'))
    judges = sorted((ROOT / 'test').glob('*.test.cpp'))
    tracked = headers + judges + [ROOT / name for name in ('tools/bundle.py', 'tools/compact_fps.py',
                                                          'test/benchmark/source_size_budget.py')]
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'limit_bytes': LIMIT,
              'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
              'source_sha256': {str(path.relative_to(ROOT)): sha(path) for path in tracked},
              'method': 'ordinary oj-bundle followed by automatic token compression only if >=256 KiB; no function or platform branch deletion',
              'header_count': len(headers), 'judge_count': len(judges), 'sizes': [], 'checks': [],
              'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'completed': False}

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')

    jobs = []
    for index, header in enumerate(headers):
        for io in (False, True):
            source = work / f'header-{index}-{int(io)}.cpp'
            text = '#include "' + str(header.relative_to(ROOT)) + '"\n'
            if io:
                text += '#include "components/fast_io.hpp"\n'
            source.write_text(text + 'int main() {}\n')
            jobs.append((str(header.relative_to(ROOT)), io, source))
    jobs += [(str(path.relative_to(ROOT)), None, path) for path in judges]
    save()

    def measure(job):
        name, io, source = job
        result = subprocess.run([args.bundle, '-I', str(ROOT), str(source)], cwd=ROOT,
                                capture_output=True, check=True)
        original = result.stdout.decode('utf-8')
        generated = fit_source(original)
        label = name.replace('/', '__') + ('-io' if io else '')
        directory = work / label
        directory.mkdir()
        for variant, text in [('before', original), ('after', generated)]:
            folder = directory / variant
            folder.mkdir()
            (folder / 'source.cpp').write_text(text, encoding='utf-8')
        row = {'source': name, 'with_fast_io': io, 'before_bytes': len(result.stdout),
               'after_bytes': len(generated.encode('utf-8')), 'compressed': generated != original,
               'before_sha256': sha(directory / 'before/source.cpp'),
               'after_sha256': sha(directory / 'after/source.cpp'), 'directory': str(directory)}
        return row

    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            for future in as_completed([pool.submit(measure, job) for job in jobs]):
                report['sizes'].append(future.result())
                if len(report['sizes']) % 30 == 0:
                    save()
                    print('size', len(report['sizes']), '/', len(jobs), flush=True)
        report['sizes'].sort(key=lambda row: (row['source'], bool(row['with_fast_io'])))
        save()
        changed = [row for row in report['sizes'] if row['compressed']]
        flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
        report['flags'] = flags

        def check(job):
            row, compiler, extra, binary = job
            directory = Path(row['directory'])
            label = Path(compiler).name.replace('+', 'p') + ('-portable' if extra else '')
            tokens, hashes = [], []
            for variant in ('before', 'after'):
                folder = directory / variant
                command = [compiler, *flags, *extra, '-ffile-prefix-map=' + str(folder) + '=/source',
                           str(folder / 'source.cpp')]
                preprocessed = folder / (label + '.ii')
                with preprocessed.open('w') as out, (folder / (label + '.log')).open('w') as err:
                    subprocess.run([*command, '-E', '-P'], cwd=ROOT, stdout=out, stderr=err, check=True)
                    if binary:
                        executable = folder / (label + '.bin')
                        subprocess.run([*command, '-o', str(executable)], cwd=ROOT, stdout=err, stderr=err, check=True)
                        hashes.append(sha(executable))
                tokens.append(preprocessed)
            streams = [(match[0] for match in TOKEN.finditer(path.read_text())) for path in tokens]
            count = 0
            for left, right in itertools.zip_longest(*streams):
                assert left == right, (row['source'], compiler, extra, count, left, right)
                count += 1
            if binary:
                assert hashes[0] == hashes[1], (row['source'], compiler, extra, hashes)
            return {'source': row['source'], 'with_fast_io': row['with_fast_io'], 'compiler': compiler,
                    'extra': extra, 'tokens_identical': True, 'token_count': count,
                    'binary_identical': binary, 'binary_sha256': hashes,
                    'preprocessed_sha256': [sha(path) for path in tokens]}

        checks = [(row, args.cxx, [], row['with_fast_io'] is None) for row in changed]
        subset = {'test/fps_linear_recurrence.test.cpp', 'test/fps_compose.test.cpp',
                  'test/fps_interpolate.test.cpp', 'test/power_projection.test.cpp',
                  'test/binomial_prefix_sum_offline.test.cpp'}
        for row in changed:
            if row['source'] in subset:
                checks.append((row, args.cxx, ['-U__linux__'], True))
                if args.clang:
                    checks.append((row, args.clang, [], True))
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            for future in as_completed([pool.submit(check, job) for job in checks]):
                report['checks'].append(future.result())
                save()
                print('equal', len(report['checks']), '/', len(checks), flush=True)
    for name, expected in report['source_sha256'].items():
        assert sha(ROOT / name) == expected, name
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat(),
                  maximum_bytes=max(row['after_bytes'] for row in report['sizes']))
    save()
    print('PASS', len(jobs), 'bundles;', len(report['checks']), 'equivalence checks;',
          report['maximum_bytes'], 'maximum bytes', flush=True)


if __name__ == '__main__':
    main()
