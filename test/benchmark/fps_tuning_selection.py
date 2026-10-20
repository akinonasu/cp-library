import argparse
import json
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--ignore-access-declarations', action='store_true')
    parser.add_argument('--baseline', default='fd41ae5df33960b93288ae98f9edac4e9fdb166f')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    changed = set(subprocess.check_output(['git', '-c', 'core.autocrlf=true', '-c', 'core.safecrlf=false', 'diff', '--name-only', args.baseline], cwd=root, text=True).splitlines())
    changed.update(subprocess.check_output(['git', 'ls-files', '--others', '--exclude-standard'], cwd=root, text=True).splitlines())
    if args.ignore_access_declarations:
        name = 'components/fast_io.hpp'
        before = subprocess.check_output(['git', 'show', args.baseline + ':' + name], cwd=root, text=True)
        after = (root / name).read_text()
        restored = after.replace('struct BulkInput; struct BulkOutput;', 'struct BulkInput;').replace('\tfriend struct fast_io_detail::BulkOutput;\n', '')
        if restored != before:
            raise RuntimeError('fast_io.hpp has changes beyond the BulkOutput friendship declarations')
        changed.discard(name)
    cache = {}

    def dependencies(name):
        if name in cache:
            return cache[name]
        cache[name] = found = {name}
        path = root / name
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', path.read_text(), re.M):
            for candidate in [path.parent / include, root / include]:
                candidate = candidate.resolve()
                if candidate.is_file() and candidate.is_relative_to(root):
                    found.update(dependencies(candidate.relative_to(root).as_posix()))
                    break
        return found

    def affected(paths):
        return [p.relative_to(root).as_posix() for p in paths if dependencies(p.relative_to(root).as_posix()) & changed]

    headers = sorted(p for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'tree', 'math', 'string'] for p in (root / folder).glob('*.hpp'))
    selection = dict(headers=affected(headers), unit=affected(sorted((root / 'test/unit').glob('*.cpp'))), verification=affected(sorted((root / 'test').glob('*.test.cpp'))))
    selection['sanitize'] = selection['unit'].copy()
    selection['bundles'] = selection['headers'].copy()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(selection, indent=2) + '\n')
    print({key: len(value) for key, value in selection.items()})


if __name__ == '__main__':
    main()
