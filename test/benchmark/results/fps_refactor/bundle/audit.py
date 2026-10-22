import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
from itertools import zip_longest
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE',
         '-ftrivial-auto-var-init=zero', '-march=haswell']
PROFILES = ['normal', 'normal_and_light', 'public_api_usage']
TOKEN = re.compile(
    r'(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"(?:[^\W\d]\w*)?'
    r'|(?:u8|u|U|L)?"(?:[^"\\\n]|\\[\s\S])*"(?:[^\W\d]\w*)?'
    r"|(?:u8|u|U|L)?'(?:[^'\\\n]|\\[\s\S])*'(?:[^\W\d]\w*)?"
    r'|[^\W\d]\w*'
    r"|(?:\d|\.\d)(?:[eEpP][+-]|[\w.'])*"
    r'|%:%:|->\*|\.\.\.|<<=|>>=|<=>|::|->|\.\*|\+\+|--|&&|\|\||<<|>>|<=|>=|==|!='
    r'|\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|<:|:>|<%|%>|%:|[^\s]', re.DOTALL)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def token_compare(left, right):
    hashes = [hashlib.sha256(), hashlib.sha256()]
    counts = [0, 0]
    first_difference = None
    previous = []
    for index, pair in enumerate(zip_longest(TOKEN.finditer(left), TOKEN.finditer(right))):
        spellings = [match[0] if match is not None else None for match in pair]
        for side, spelling in enumerate(spellings):
            if spelling is not None:
                data = spelling.encode('utf-8')
                hashes[side].update(len(data).to_bytes(4, 'little'))
                hashes[side].update(data)
                counts[side] += 1
        if spellings[0] != spellings[1] and first_difference is None:
            first_difference = dict(index=index, raw=spellings[0], compact=spellings[1],
                                    previous=previous[-8:])
        previous.append(spellings[0])
        if len(previous) > 8:
            del previous[0]
    return dict(equal=first_difference is None, counts=counts,
                sha256=[value.hexdigest() for value in hashes], first_difference=first_difference)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--artifacts', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--gxx', default='g++')
    parser.add_argument('--clang', default='clang++')
    args = parser.parse_args()
    args.output = args.output.resolve()
    if args.output.exists():
        raise FileExistsError(args.output)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.work:
        work = args.work.resolve()
        work.mkdir(parents=True, exist_ok=False)
    else:
        work = Path(tempfile.mkdtemp(prefix='fps-bundle-audit-'))
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), completed=False,
        flags=FLAGS, work=str(work), artifacts=str(args.artifacts.resolve()),
        purpose='Current raw bundle versus its current minified bundle; no comparison with old library code.',
        token_method='Compiler -E -P output is lexed independently of tools.compact_fps; whitespace is excluded, token spellings and boundaries are compared directly. Pragmas remain in this comparison.',
        token_hash_format='For each UTF-8 token: unsigned 4-byte little-endian byte length, then token bytes.',
        locking='Shared /tmp/cp-fps-tuning.lock held across all compilation and runs; exclusive performance measurements cannot overlap.',
        script_sha256=sha(Path(__file__).read_bytes()), commands=[], profiles={}, comparisons=[])

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')

    def execute(command, label):
        result = subprocess.run([str(value) for value in command], capture_output=True, timeout=180)
        row = dict(label=label, command=[str(value) for value in command], returncode=result.returncode,
                   stdout_sha256=sha(result.stdout), stderr_sha256=sha(result.stderr),
                   stderr=result.stderr.decode('utf-8', errors='replace')[-4000:])
        report['commands'].append(row)
        save()
        if result.returncode:
            raise RuntimeError((command, result.stderr.decode('utf-8', errors='replace')))
        return result.stdout

    try:
        for profile in PROFILES:
            sources = {}
            for variant, filename in [('raw', 'after_raw.cpp'), ('compact', 'after_compact.cpp')]:
                source = args.artifacts / profile / filename
                data = source.read_bytes()
                destination = work / profile / ('before' if variant == 'raw' else 'after') / 'source.cpp'
                destination.parent.mkdir(parents=True)
                destination.write_bytes(data)
                sources[variant] = dict(original=str(source.resolve()), bytes=len(data),
                    sha256=sha(data), copied_to=str(destination))
            report['profiles'][profile] = sources
        compilers = [('gcc', args.gxx), ('clang', args.clang)]
        report['compiler_versions'] = {name: subprocess.check_output([compiler, '--version'], text=True)
                                       for name, compiler in compilers}
        report['compiler_sha256'] = {name: sha(Path(shutil.which(compiler) or compiler).resolve().read_bytes())
                                     for name, compiler in compilers}
        report['cpu'] = subprocess.check_output(['lscpu'], text=True)
        report['tools_sha256'] = {name: sha((ROOT / name).read_bytes())
            for name in ['tools/compact_fps.py', 'tools/bundle.py', 'test/benchmark/fps_256k_refactor.py']}
        save()
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            for compiler_name, compiler in compilers:
                for profile in PROFILES:
                    configurations = [([], 'native')]
                    if compiler_name == 'gcc' and profile == 'public_api_usage':
                        configurations.append((['-U__linux__'], 'nonlinux_dispatch'))
                    for extra, configuration in configurations:
                        outputs, binaries, runs = [], [], []
                        for variant in ['raw', 'compact']:
                            directory = Path(report['profiles'][profile][variant]['copied_to']).parent
                            source = directory / 'source.cpp'
                            flags = [*FLAGS, *extra,
                                '-ffile-prefix-map=' + str(directory) + '=' + str(work / profile / 'normalized')]
                            output = execute([compiler, *flags, '-E', '-P', source],
                                             compiler_name + '-' + profile + '-' + configuration + '-' + variant + '-preprocess')
                            pp_path = directory / (compiler_name + '-' + configuration + '.ii')
                            pp_path.write_bytes(output)
                            outputs.append(output.decode('utf-8'))
                            executable = directory / (compiler_name + '-' + configuration + '.elf')
                            execute([compiler, *flags, source, '-o', executable],
                                    compiler_name + '-' + profile + '-' + configuration + '-' + variant + '-compile')
                            binaries.append(sha(executable.read_bytes()))
                            runs.append(execute([executable],
                                compiler_name + '-' + profile + '-' + configuration + '-' + variant + '-run'))
                        compared = token_compare(*outputs)
                        row = dict(compiler=compiler_name, profile=profile, configuration=configuration,
                            preprocessing=compared, elf_sha256=binaries, elf_equal=binaries[0] == binaries[1],
                            stdout_sha256=[sha(value) for value in runs], run_equal=runs[0] == runs[1])
                        report['comparisons'].append(row)
                        save()
                        assert compared['equal'], row
                        assert row['run_equal'], row
                        print(compiler_name, profile, configuration, 'tokens equal; ELF equal:', row['elf_equal'], flush=True)
        for sources in report['profiles'].values():
            for source in sources.values():
                assert sha(Path(source['original']).read_bytes()) == source['sha256']
        for name, digest in report['tools_sha256'].items():
            assert sha((ROOT / name).read_bytes()) == digest
        report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    finally:
        save()


if __name__ == '__main__':
    main()
