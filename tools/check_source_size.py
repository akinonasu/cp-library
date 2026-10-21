import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description='Check the byte size of a bundled submission')
    parser.add_argument('source', type=Path)
    parser.add_argument('--limit', type=int, choices=(64, 256), default=64, help='limit in KiB')
    args = parser.parse_args()
    if not args.source.is_file():
        parser.error('source must be a file')
    size = args.source.stat().st_size
    limit = args.limit * 1024
    print(f'{size} / {limit} bytes ({args.limit} KiB)')
    if size > limit:
        parser.exit(1, f'{size - limit} bytes over the limit\n')
    print(f'{limit - size} bytes remaining')


if __name__ == '__main__':
    main()
