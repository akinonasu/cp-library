import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import io

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.compact_fps import lex
from tools.update_fps_headers import read_source, shorten_locals


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def local_map():
    import tools.update_fps_headers as generator
    saved = generator.shorten_locals
    mapping = {}

    def capture(source):
        result, names = saved(source)
        mapping.update(names)
        return result, names

    generator.shorten_locals = capture
    try:
        generator.generate()
    finally:
        generator.shorten_locals = saved
    return mapping


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default='c6bee6c')
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bundle', default='oj-bundle')
    parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    before = work / 'baseline'
    archive = subprocess.check_output(['git', 'archive', args.baseline], cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
        stream.extractall(before, filter='data')
    normal = sorted(path.relative_to(ROOT).as_posix() for path in (ROOT / 'fps').glob('*.hpp') if 'light' not in path.name)
    light = sorted(path.relative_to(ROOT).as_posix() for path in (ROOT / 'fps').glob('*.hpp') if 'light' in path.name)
    body = (ROOT / 'test/unit/fps_public_headers.cpp').read_text()
    body = body[body.index('void require('):]
    prefix = '#include <bits/stdc++.h>\n#if defined(__x86_64__) || defined(__i386__)\n#include <immintrin.h>\n#endif\n#include "math/mint.hpp"\n'
    start, end = 'cp_fps_plain_start', 'cp_fps_plain_end'
    normal_source = prefix + 'namespace ' + start + ' {}\n'
    normal_source += ''.join('#include "' + name + '"\n' for name in normal)
    normal_source += 'namespace ' + end + ' {}\n'
    normal_source += '#include "components/fast_io.hpp"\n#include "test/fps_io.hpp"\n'
    profiles = {'normal_usage': normal_source + body,
                'all_usage': normal_source + ''.join('#include "' + name + '"\n' for name in light)
                             + body.replace('check<mint>();', 'light_fps lf{1,2,1}; require(lf.inv(8).size()==8); check<mint>();'),
                'light_usage': prefix + ''.join('#include "' + name + '"\n' for name in light)
                               + '#include "components/fast_io.hpp"\n'
                               + (ROOT / 'test/unit/fps_light.cpp').read_text()}
    rename = local_map()
    inverse = {value: name for name, value in rename.items()}
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'completed': False,
              'baseline': subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
              'harness_sha256': sha(Path(__file__)), 'local_names': rename,
              'profiles': {}, 'pairs': [],
              'scope': 'Ordinary oj-bundle without postprocessing. Compare full preprocessed tokens after restoring internal local names, ignoring diagnostic pragmas only; NDEBUG removes location-dependent assert strings. Run all-API checks and compare entire ELF hashes.'}
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')

    save()
    for name, source in profiles.items():
        row = report['profiles'][name] = {}
        for variant, tree in [('before', before), ('after', ROOT)]:
            folder = work / name / variant
            folder.mkdir(parents=True)
            path = folder / 'a.cpp'
            path.write_text(source)
            bundled = subprocess.run([args.bundle, '-I', str(tree), str(path)], cwd=tree, capture_output=True, check=True)
            reference = b'//https://qandori.github.io/cp-library/fps/fps.hpp.html\n'
            bundle = folder / 'bundle.cpp'
            bundle.write_bytes(reference + bundled.stdout)
            row[variant] = {'bytes': bundle.stat().st_size, 'sha256': sha(bundle), 'driver_sha256': sha(path)}
            if variant == 'after':
                assert row[variant]['bytes'] < (65536 if name == 'light_usage' else 262144), (name, row[variant])
        save()
        compilers = [(cxx, []) for cxx in args.cxx]
        if name == 'normal_usage':
            compilers.append((args.cxx[0], ['-U__linux__']))
        for index, (cxx, extra) in enumerate(compilers):
            streams, binaries = [], []
            pair = {'profile': name, 'compiler': cxx, 'extra': extra, 'variants': {}}
            for variant, tree in [('before', before), ('after', ROOT)]:
                folder = work / name / variant
                command = [cxx, '-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror',
                           '-ftrivial-auto-var-init=zero', '-march=haswell',
                           '-ffile-prefix-map=' + str(tree) + '=/library',
                           '-ffile-prefix-map=' + str(folder) + '=/source', *extra, '-I', str(tree), str(folder / 'a.cpp')]
                log = folder / f'{index}.log'
                pp = folder / f'{index}.ii'
                binary = folder / f'{index}.bin'
                with log.open('wb') as err, pp.open('wb') as out:
                    subprocess.run([*command, '-E', '-P'], cwd=tree, stdout=out, stderr=err, check=True)
                lines = [line for line in pp.read_text().splitlines() if not line.lstrip().startswith('#pragma GCC diagnostic')]
                tokens = lex('\n'.join(lines))
                if variant == 'after' and name != 'light_usage':
                    begin, finish = tokens.index(start), tokens.index(end)
                    tokens[begin:finish] = [inverse.get(token, token) for token in tokens[begin:finish]]
                streams.append(tokens)
                with log.open('ab') as err:
                    subprocess.run([*command, '-o', str(binary)], cwd=tree, stdout=err, stderr=err, check=True)
                output = subprocess.check_output([str(binary)], cwd=tree)
                binaries.append(sha(binary))
                pair['variants'][variant] = {'command': command, 'binary_sha256': sha(binary),
                                            'stdout_sha256': hashlib.sha256(output).hexdigest(),
                                            'log_sha256': sha(log), 'preprocessed_sha256': sha(pp), 'tokens': len(tokens)}
            pair.update(tokens_equal=streams[0] == streams[1], elf_equal=binaries[0] == binaries[1])
            report['pairs'].append(pair)
            save()
            print(name, cxx, extra, 'tokens', pair['tokens_equal'], 'ELF', pair['elf_equal'], flush=True)
            if not pair['tokens_equal']:
                first = next(i for i, (a, b) in enumerate(zip(*streams)) if a != b)
                report['first_difference'] = {'index': first, 'before': streams[0][first-12:first+18], 'after': streams[1][first-12:first+18]}
                save()
                raise AssertionError(report['first_difference'])
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    save()


if __name__ == '__main__':
    main()
