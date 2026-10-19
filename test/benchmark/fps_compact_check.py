import argparse
import concurrent.futures
import fcntl
import hashlib
import importlib.util
import itertools
import json
import shutil
import subprocess
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bundle', default=shutil.which('oj-bundle'))
    parser.add_argument('--gcc', default=shutil.which('g++'))
    parser.add_argument('--gcc13')
    parser.add_argument('--clang')
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    work, output = args.work.resolve(), args.output.resolve()
    work.mkdir(parents=True, exist_ok=False)
    output.mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location('compact_fps', ROOT / 'tools/compact_fps.py')
    pack = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pack)
    drivers = json.loads((ROOT / 'test/benchmark/results/fps_source_size/drivers.json').read_text())
    sources = [p for folder in ('components', 'fps', 'math') for p in (ROOT / folder).rglob('*.hpp')]
    sources += [ROOT / name for name in drivers] + [ROOT / 'test/fps_io.hpp', ROOT / 'tools/compact_fps.py', Path(__file__).resolve()]
    source_hashes = {str(p.relative_to(ROOT)): sha(p) for p in sources}
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = {
        'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'flags': flags, 'source_hashes': source_hashes,
        'bundle_tool_sha256': sha(Path(args.bundle).resolve()), 'sizes': [], 'checks': [], 'status': 'running',
    }

    def save():
        (output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')

    fixture = '#line 1 "compact_fixture.cpp"\n#include <cstdio>\n#include <cassert>\n#include <cstdint>\n'
    fixture += '#define TEXT(x) #x\n#define ADD(x,y) ((x)+(y))\n'
    fixture += ''.join(
        f'inline std::uint32_t function_{i}(std::uint32_t x) {{ const std::uint32_t y=x+123; return y^(y>>3); }}\n'
        for i in range(24)
    )
    fixture += '''int main(int argc, char**) {
    const char* first = TEXT(std::uint32_t);
    const char* second = TEXT
    // Keep the following macro argument spelling.
    (
        std::uint32_t
    );
    assert(
        argc > 0
    );
    const char* raw = R"mark(raw text with /* comments */ and ")
    \" and parentheses ( ) )mark";
    /* A multiline comment
       with code-looking tokens const std::uint32_t */
    std::printf("%s %s %s %d %d\\n", first, second, raw, __LINE__, ADD(argc, 2));
}
'''
    for variant, text in [('before', fixture), ('after', pack.compact(fixture))]:
        directory = work / variant / 'compact_fixture'
        directory.mkdir(parents=True)
        (directory / 'source.cpp').write_text(text, encoding='utf-8')
    try:
        pack.compact('int main() {}\n')
    except ValueError:
        result['unbundled_input_rejected'] = True
    else:
        raise AssertionError('unbundled input was accepted')

    for name in drivers:
        bundle = subprocess.run([args.bundle, '-I', '.', name], cwd=ROOT, capture_output=True, check=True).stdout.decode()
        packed = pack.compact(bundle)
        row = {'source': name}
        for variant, text in [('before', bundle), ('after', packed)]:
            directory = work / variant / Path(name).stem
            directory.mkdir(parents=True)
            path = directory / 'source.cpp'
            path.write_text(text, encoding='utf-8')
            row[variant] = {'bytes': path.stat().st_size, 'sha256': sha(path)}
        result['sizes'].append(row)
        print('size', name, row['before']['bytes'], row['after']['bytes'], flush=True)
        save()
    kth = next(row for row in result['sizes'] if row['source'] == 'test/fps_linear_recurrence.test.cpp')
    assert kth['after']['bytes'] < 256 * 1024
    if args.prepare_only:
        result['status'] = 'sizes-passed'
        save()
        return

    jobs = [(args.gcc, name, []) for name in drivers]
    subset = ['test/fps_linear_recurrence.test.cpp', 'test/fps_compose.test.cpp']
    if args.gcc13:
        jobs += [(args.gcc13, name, []) for name in subset]
    if args.clang:
        jobs += [(args.clang, name, []) for name in subset]
    jobs += [(args.gcc, name, ['-U__linux__']) for name in subset]
    jobs += [(args.gcc, 'compact_fixture.cpp', [])]
    result['compilers'] = {
        compiler: {'sha256': sha(Path(compiler).resolve()), 'version': subprocess.check_output([compiler, '--version'], text=True)}
        for compiler in sorted({job[0] for job in jobs})
    }

    def check(job):
        compiler, name, extra = job
        label = Path(compiler).name + ('-portable' if extra else '')
        row = {'compiler': compiler, 'source': name, 'extra_flags': extra, 'variants': {}}
        begin = time.monotonic()
        for variant in ('before', 'after'):
            directory = work / variant / Path(name).stem
            binary, log, preprocessed = (directory / (label + suffix) for suffix in ('.bin', '.log', '.ii'))
            command = [compiler, *flags, *extra, '-ffile-prefix-map=' + str(directory) + '=/source', str(directory / 'source.cpp')]
            with open('/tmp/cp-fps-tuning.lock', 'a') as lock, log.open('w') as out:
                fcntl.flock(lock, fcntl.LOCK_SH)
                subprocess.run([*command, '-o', str(binary)], stdout=out, stderr=subprocess.STDOUT, check=True)
                with preprocessed.open('w') as pp:
                    subprocess.run([*command, '-E', '-P'], stdout=pp, stderr=out, check=True)
            row['variants'][variant] = {
                'command': command, 'binary_sha256': sha(binary), 'binary_bytes': binary.stat().st_size,
                'log_sha256': sha(log), 'preprocessed_sha256': sha(preprocessed),
            }
        assert row['variants']['before']['binary_sha256'] == row['variants']['after']['binary_sha256'], row
        if name == 'compact_fixture.cpp':
            runs = [subprocess.check_output([str(work / variant / 'compact_fixture' / (label + '.bin'))])
                    for variant in ('before', 'after')]
            assert runs[0] == runs[1]
            row['runtime_stdout_sha256'] = hashlib.sha256(runs[0]).hexdigest()
        paths = [work / variant / Path(name).stem / (label + '.ii') for variant in ('before', 'after')]
        texts = [path.read_text() for path in paths]
        streams = ((match[0] for match in pack.TOKEN.finditer(text)) for text in texts)
        count = 0
        for left, right in itertools.zip_longest(*streams):
            assert left == right, (row, count, left, right)
            count += 1
        row.update(preprocessed_tokens_identical=True, token_count=count, seconds=time.monotonic() - begin)
        return row

    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        for row in pool.map(check, jobs):
            result['checks'].append(row)
            save()
            print('identical executable and preprocessed tokens', row['compiler'], row['source'], row['extra_flags'], flush=True)
    assert all(sha(ROOT / name) == digest for name, digest in source_hashes.items())
    assert all(sha(Path(name).resolve()) == info['sha256'] for name, info in result['compilers'].items())
    result.update(status='passed', sources_unchanged=True, compilers_unchanged=True)
    save()
    print('PASS', len(result['checks']), 'pairs; byte-identical executables and equal preprocessing tokens', flush=True)


if __name__ == '__main__':
    main()
