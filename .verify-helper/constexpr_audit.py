import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / '.verify-helper'))
from affected import LIBRARIES
from atcoder_flags import constexpr_flags


def main():
    parser = argparse.ArgumentParser(description='Compile all library headers and test sources with AtCoder constexpr limits')
    parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
    parser.add_argument('--include', type=Path, action='append', default=[])
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    if args.output.exists():
        parser.error('--output must be a new file')
    paths = {
        'header': sorted(path for folder in LIBRARIES for path in (ROOT / folder).rglob('*.hpp')),
        'unit': sorted((ROOT / 'test/unit').rglob('*.cpp')),
        'judge': sorted((ROOT / 'test').glob('*.test.cpp')),
    }
    versions = {cxx: subprocess.check_output([cxx, '--version'], text=True).splitlines()[0] for cxx in args.cxx}
    limits = {cxx: constexpr_flags(cxx) for cxx in args.cxx}
    flags = ['-std=c++23', '-O2', '-DDEBUG=1', '-fsyntax-only']
    if args.native:
        flags += ['-march=native']
    includes = [ROOT, ROOT / 'test', *args.include]
    for path in includes:
        flags += ['-I', str(path.resolve())]

    def check(task):
        cxx, kind, path = task
        with tempfile.TemporaryDirectory(prefix='constexpr-audit-') as directory:
            source = path
            if kind == 'header':
                source = Path(directory) / 'main.cpp'
                source.write_text(f'#include "{path.relative_to(ROOT).as_posix()}"\nint main() {{}}\n')
            started = time.monotonic()
            try:
                result = subprocess.run([cxx, *flags, *limits[cxx], str(source)], cwd=ROOT,
                                        text=True, capture_output=True, timeout=180)
                status, diagnostic = result.returncode, result.stderr
            except subprocess.TimeoutExpired as error:
                status, diagnostic = 124, str(error)
            record = {'compiler': cxx, 'path': path.relative_to(ROOT).as_posix(), 'kind': kind,
                      'returncode': status, 'seconds': round(time.monotonic() - started, 3)}
            if status:
                record['diagnostic'] = diagnostic
                print(f'FAIL {cxx}: {record["path"]}', flush=True)
            return record

    tasks = [(cxx, kind, path) for cxx in args.cxx for kind, sources in paths.items() for path in sources]
    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for future in as_completed([pool.submit(check, task) for task in tasks]):
            results.append(future.result())
            if len(results) % 25 == 0:
                print(f'{len(results)}/{len(tasks)} compilations', flush=True)
    report = {
        'compilers': versions, 'flags': limits, 'common_flags': flags,
        'counts': {kind: len(sources) for kind, sources in paths.items()},
        'results': sorted(results, key=lambda record: (record['compiler'], record['kind'], record['path'])),
        'source_sha256': {path.relative_to(ROOT).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                          for sources in paths.values() for path in sources},
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    failures = sum(record['returncode'] != 0 for record in results)
    print(f'{len(results)} compilations; {failures} failures', flush=True)
    return int(failures != 0)


if __name__ == '__main__':
    raise SystemExit(main())
