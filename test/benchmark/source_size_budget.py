import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
LIMIT = 256 * 1024


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description='Audit ordinary oj-bundle output sizes')
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bundle', default='oj-bundle')
    parser.add_argument('--jobs', type=int, default=3)
    args = parser.parse_args()
    args.bundle = shutil.which(args.bundle) or args.bundle
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    headers = sorted(path for directory in ('components', 'convolution', 'fps', 'data_structure', 'graph',
                                            'tree', 'math', 'sequence', 'string', 'geometry')
                     for path in (ROOT / directory).rglob('*.hpp'))
    judges = sorted((ROOT / 'test').glob('*.test.cpp'))
    tracked = headers + judges + sorted((ROOT / 'components').glob('*.inc')) + sorted((ROOT / 'fps').glob('*.inc'))
    tracked += [ROOT / name for name in ('tools/update_fps_headers.py', 'tools/compact_fps.py',
                                         'test/benchmark/source_size_budget.py')]
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'limit_bytes': LIMIT,
              'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
              'source_sha256': {str(path.relative_to(ROOT)): sha(path) for path in tracked},
              'method': 'ordinary oj-bundle without postprocessing; UTF-8 bytes including the reference URL; size checks only',
              'bundler': str(Path(args.bundle).resolve()), 'bundler_sha256': sha(Path(args.bundle)),
              'header_count': len(headers), 'judge_count': len(judges), 'sizes': [], 'completed': False}

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
        command = [args.bundle, '-I', str(ROOT), str(source)]
        result = subprocess.run(command, cwd=ROOT, capture_output=True, check=True)
        reference = ('//https://qandori.github.io/cp-library/' + name + '.html\n').encode()
        copied = reference + result.stdout
        assert len(copied) < LIMIT, (name, io, len(copied))
        target = work / (name.replace('/', '__') + ('-io' if io else '') + '.cpp')
        target.write_bytes(copied)
        return {'source': name, 'with_fast_io': io, 'raw_bytes': len(result.stdout), 'copy_bytes': len(copied),
                'sha256': sha(target), 'command': command, 'stderr_sha256': hashlib.sha256(result.stderr).hexdigest()}

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for future in as_completed([pool.submit(measure, job) for job in jobs]):
            report['sizes'].append(future.result())
            if len(report['sizes']) % 30 == 0:
                save()
                print('size', len(report['sizes']), '/', len(jobs), flush=True)
    report['sizes'].sort(key=lambda row: (row['source'], bool(row['with_fast_io'])))
    assert all(sha(ROOT / name) == value for name, value in report['source_sha256'].items())
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat(),
                  maximum_copy_bytes=max(row['copy_bytes'] for row in report['sizes']))
    save()


if __name__ == '__main__':
    main()
