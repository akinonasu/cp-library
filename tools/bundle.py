import argparse
from pathlib import Path
import shutil
import subprocess
import sys

try:
    from .compact_fps import compact
except ImportError:
    from compact_fps import compact


LIMIT = 256 * 1024


def fit_source(source, limit=LIMIT):
    if limit <= 0:
        raise ValueError('the source limit must be positive')
    if len(source.encode('utf-8')) >= limit:
        source = compact(source)
    size = len(source.encode('utf-8'))
    if size >= limit:
        raise ValueError(f'{size} bytes is not below {limit} bytes')
    return source


def bundle(source, *, include_paths=('.',), executable='oj-bundle', limit=LIMIT):
    command = [executable]
    for path in include_paths:
        command += ['-I', str(path)]
    command.append(str(source))
    result = subprocess.run(command, capture_output=True, check=True)
    original = result.stdout.decode('utf-8')
    return original, fit_source(original, limit)


def main():
    parser = argparse.ArgumentParser(description='Bundle C++ source below a UTF-8 byte limit')
    parser.add_argument('source', type=Path)
    parser.add_argument('-I', '--include', action='append', type=Path)
    parser.add_argument('-o', '--output', type=Path)
    parser.add_argument('--limit', type=int, default=LIMIT)
    parser.add_argument('--bundle', default=shutil.which('oj-bundle') or 'oj-bundle')
    args = parser.parse_args()
    try:
        original, result = bundle(args.source, include_paths=args.include or ['.'],
                                  executable=args.bundle, limit=args.limit)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr.decode('utf-8', errors='replace'), file=sys.stderr)
        parser.error(str(error))
    if args.output:
        args.output.write_bytes(result.encode('utf-8'))
    else:
        sys.stdout.buffer.write(result.encode('utf-8'))
    print(f'{len(original.encode("utf-8"))} -> {len(result.encode("utf-8"))} bytes', file=sys.stderr)


if __name__ == '__main__':
    main()
