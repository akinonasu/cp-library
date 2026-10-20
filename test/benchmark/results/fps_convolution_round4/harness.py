import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
import difflib
import fcntl
import hashlib
import io
import json
import lzma
import os
from pathlib import Path
import random
import shlex
import shutil
import statistics
import subprocess
import tarfile
import time


ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'bd29f70c70e24e27770b9e9a3777b6cf85664b63'
REFERENCE_SHA = '6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d'
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE',
         '-ftrivial-auto-var-init=zero', '-march=haswell']
OFFICIAL_CASES = ['max_random_00', 'max_random_01', 'all_same_00', 'all_same_01',
                  *['small_and_large_' + str(i).zfill(2) for i in range(4)],
                  'medium_00', 'medium_01']
CUSTOM_CASES = [(65536, 65536), (262144, 262144), (300000, 220000)]
LOCK = '/tmp/cp-fps-tuning.lock'


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as src:
        for data in iter(lambda: src.read(1024 * 1024), b''):
            h.update(data)
    return h.hexdigest()


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    temporary.replace(path)


@contextmanager
def locked(mode):
    with open(LOCK, 'a') as handle:
        fcntl.flock(handle, mode)
        try:
            yield
        finally:
            fcntl.flock(handle, fcntl.LOCK_UN)


def command(args, **kwargs):
    return subprocess.check_output(args, text=True, **kwargs).strip()


def tree_hashes(tree):
    return {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}


def source_paths(tree):
    return sorted([p for p in tree.rglob('*.hpp') if '.git' not in p.parts and
                   ('test' not in p.relative_to(tree).parts or p == tree / 'test/fps_io.hpp')] +
                  [tree / 'test/convolution.test.cpp'])


def snapshot(args):
    target = args.work / 'source'
    if target.exists():
        raise RuntimeError('source snapshot already exists; use a new --work or --prepared')
    target.mkdir(parents=True)
    with locked(fcntl.LOCK_SH):
        baseline = command(['git', '-C', str(ROOT), 'rev-parse', args.baseline])
        tracked = command(['git', '-C', str(ROOT), 'ls-tree', '-r', '--name-only', baseline]).splitlines()
        paths = [p for p in tracked if p.endswith('.hpp') and
                 ('test' not in Path(p).parts or p == 'test/fps_io.hpp')]
        paths.append('test/convolution.test.cpp')
        archive = subprocess.check_output(['git', '-C', str(ROOT), 'archive', baseline, *paths])
        (target / 'before').mkdir()
        with tarfile.open(fileobj=io.BytesIO(archive)) as src:
            src.extractall(target / 'before', filter='data')
        for src in source_paths(ROOT):
            dst = target / 'current' / src.relative_to(ROOT)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, dst)
        shutil.copyfile(args.reference, target / 'reference.cpp')
        assert sha(target / 'reference.cpp') == REFERENCE_SHA
        before = tree_hashes(target / 'before')
        current = tree_hashes(target / 'current')
        assert current == {str(p.relative_to(ROOT)): sha(p) for p in source_paths(ROOT)}
        metadata = dict(baseline=baseline, live_head=command(['git', '-C', str(ROOT), 'rev-parse', 'HEAD']),
                        before=before, current=current, reference_sha256=REFERENCE_SHA)
        write_json(target / 'sources.json', metadata)
    return target


def dependencies(path, cwd):
    contents = path.read_text().replace('\\\n', '')
    return sorted({str((cwd / p).resolve()) for p in shlex.split(contents.split(':', 1)[1])})


def build(variant, source_root, args):
    source = source_root / ('reference.cpp' if variant == 'reference' else variant + '/test/convolution.test.cpp')
    binary = args.work / variant
    depfile = args.work / (variant + '.d')
    cmd = [args.cxx, *FLAGS, '-MD', '-MF', str(depfile)]
    if variant != 'reference':
        cmd += ['-I' + str(source_root / variant)]
    cmd += [str(source), '-o', str(binary)]
    log = args.work / (variant + '.build.log')
    with locked(fcntl.LOCK_SH), log.open('w') as out:
        subprocess.run(cmd, cwd=args.work, stdout=out, stderr=subprocess.STDOUT, check=True)
    paths = dependencies(depfile, args.work)
    result = dict(command=cmd, cwd=str(args.work), source=str(source), source_sha256=sha(source),
                  binary=str(binary), binary_sha256=sha(binary), depfile_sha256=sha(depfile),
                  dependencies_before={p: sha(p) for p in paths}, log=log.read_text())
    print('built', variant, len(paths), 'dependencies', flush=True)
    return result


def generate_custom(path, n, m):
    rng = random.Random(617 + n)
    with path.open('w') as out:
        out.write(f'{n} {m}\n')
        out.write(' '.join(str(rng.randrange(998244353)) for _ in range(n)) + '\n')
        out.write(' '.join(str(rng.randrange(998244353)) for _ in range(m)) + '\n')


def prepare_cases(args, builds):
    cases = [dict(name=name, kind='official', input=str(args.official / 'in' / (name + '.in')),
                  answer=str(args.official / 'out' / (name + '.out'))) for name in OFFICIAL_CASES]
    args.custom.mkdir(parents=True, exist_ok=True)
    with locked(fcntl.LOCK_SH):
        for n, m in CUSTOM_CASES:
            path = args.custom / f'{n}_{m}.in'
            answer = path.with_suffix('.out')
            generated = args.work / (path.stem + '.generated.in')
            generate_custom(generated, n, m)
            if path.exists():
                assert sha(path) == sha(generated), 'custom generator mismatch'
                generated.unlink()
            else:
                generated.replace(path)
            if not answer.exists():
                with path.open('rb') as src, answer.open('wb') as out:
                    subprocess.run([builds['reference']['binary']], stdin=src, stdout=out, check=True)
            cases.append(dict(name=path.stem, kind='custom', input=str(path), answer=str(answer),
                              generator=dict(random='Python random.Random', seed=617 + n,
                                             n=n, m=m, modulus=998244353)))
        for row in cases:
            path = Path(row['input'])
            row.update(input_sha256=sha(path), answer_sha256=sha(row['answer']),
                       dimensions=[int(v) for v in path.open().readline().split()])
    assert len(cases) == 13
    return cases


def check_case(checker, row, binary, output):
    with Path(row['input']).open('rb') as src, output.open('wb') as dst:
        begin = time.perf_counter_ns()
        subprocess.run([binary], stdin=src, stdout=dst, check=True)
        elapsed = time.perf_counter_ns() - begin
    checked = subprocess.run([str(checker), row['input'], str(output), row['answer']],
                             capture_output=True, text=True)
    if checked.returncode:
        raise RuntimeError('checker rejected output: ' + checked.stdout + checked.stderr)
    return elapsed, sha(output)


def guard(args, source_root, metadata, builds, cases, checker, official_head):
    result = dict(snapshot_before=tree_hashes(source_root / 'before'),
                  snapshot_current=tree_hashes(source_root / 'current'),
                  reference_sha256=sha(source_root / 'reference.cpp'),
                  compiler_sha256=sha(shutil.which(args.cxx)), checker_sha256=sha(checker),
                  official_head=command(['git', '-C', str(args.official), 'rev-parse', 'HEAD']),
                  dependencies={v: {p: sha(p) for p in b['dependencies_before']} for v, b in builds.items()},
                  binaries={v: sha(b['binary']) for v, b in builds.items()},
                  data={r['name']: dict(input_sha256=sha(r['input']), answer_sha256=sha(r['answer'])) for r in cases})
    assert result['snapshot_before'] == metadata['before']
    assert result['snapshot_current'] == metadata['current']
    assert result['reference_sha256'] == REFERENCE_SHA
    assert result['official_head'] == official_head
    for variant, build_info in builds.items():
        assert result['dependencies'][variant] == build_info['dependencies_before']
        assert result['binaries'][variant] == build_info['binary_sha256']
    for row in cases:
        assert result['data'][row['name']] == {k: row[k] for k in ['input_sha256', 'answer_sha256']}
    if not args.prepared:
        live = {str(p.relative_to(ROOT)): sha(p) for p in source_paths(ROOT)}
        assert live == metadata['current'], 'live production source changed'
        result['live_source_sha256'] = live
        assert sha(args.reference) == REFERENCE_SHA
    return result


def archive_sources(source_root, output):
    metadata = json.loads((source_root / 'sources.json').read_text())
    with tarfile.open(output / 'baseline.tar.xz', 'w:xz') as dst:
        for path in sorted((source_root / 'before').rglob('*')):
            if path.is_file():
                data = path.read_bytes()
                info = tarfile.TarInfo(str(path.relative_to(source_root / 'before')))
                info.size = len(data)
                info.mode = 0o644
                dst.addfile(info, io.BytesIO(data))
    lines = []
    for path in sorted(set(metadata['before']) | set(metadata['current'])):
        old = source_root / 'before' / path
        new = source_root / 'current' / path
        if metadata['before'].get(path) == metadata['current'].get(path):
            continue
        lines.extend(difflib.unified_diff(old.read_text().splitlines(True) if old.exists() else [],
                                         new.read_text().splitlines(True) if new.exists() else [],
                                         fromfile='a/' + path if old.exists() else '/dev/null',
                                         tofile='b/' + path if new.exists() else '/dev/null'))
    (output / 'current.patch').write_text(''.join(lines))
    (output / 'reference.cpp.xz').write_bytes(lzma.compress((source_root / 'reference.cpp').read_bytes()))
    shutil.copyfile(source_root / 'sources.json', output / 'sources.json')
    shutil.copyfile(Path(__file__), output / 'harness.py')
    (output / 'replay.py').write_text(REPLAY)


REPLAY = '''import argparse, hashlib, json, lzma, shutil, subprocess, sys, tarfile
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('--work', type=Path, required=True)
p.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
p.add_argument('--build-and-measure', action='store_true')
a = p.parse_args()
here = Path(__file__).resolve().parent
source = a.work.resolve() / 'source'
source.mkdir(parents=True)
(source / 'before').mkdir()
with tarfile.open(here / 'baseline.tar.xz') as archive:
    archive.extractall(source / 'before', filter='data')
shutil.copytree(source / 'before', source / 'current')
with (here / 'current.patch').open('rb') as patch:
    subprocess.run(['patch', '-s', '-p1'], cwd=source / 'current', stdin=patch, check=True)
(source / 'reference.cpp').write_bytes(lzma.decompress((here / 'reference.cpp.xz').read_bytes()))
shutil.copyfile(here / 'sources.json', source / 'sources.json')
metadata = json.loads((source / 'sources.json').read_text())
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
for variant in ['before', 'current']:
    actual = {str(p.relative_to(source / variant)): sha(p) for p in sorted((source / variant).rglob('*')) if p.is_file()}
    assert actual == metadata[variant]
assert sha(source / 'reference.cpp') == metadata['reference_sha256']
print('source reconstruction verified', len(metadata['before']), len(metadata['current']), flush=True)
if a.build_and_measure:
    subprocess.run([sys.executable, str(here / 'harness.py'), '--prepared', str(source), '--work', str(a.work),
                    '--official', str(a.official), '--custom', str(a.work / 'custom'),
                    '--output', str(a.work / 'results')], check=True)
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-round4/final-convolution'))
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_convolution_round4')
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--reference', type=Path, default=Path('/tmp/fps-reference/convolution_mod/409465.cpp'))
    parser.add_argument('--custom', type=Path, default=Path('/workspace/cp-fps-round4/root/custom'))
    parser.add_argument('--prepared', type=Path)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--seed', type=int, default=20261044)
    args = parser.parse_args()
    for attr in ['work', 'output', 'official', 'reference', 'custom', 'prepared']:
        value = getattr(args, attr)
        if value is not None:
            setattr(args, attr, value.resolve())
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    assert args.trials >= 1
    source_root = args.prepared or snapshot(args)
    metadata = json.loads((source_root / 'sources.json').read_text())
    variants = ['before', 'current', 'reference']
    with ThreadPoolExecutor(max_workers=2) as pool:
        builds = dict(zip(variants, pool.map(lambda v: build(v, source_root, args), variants)))
    cases = prepare_cases(args, builds)
    checker = args.official / 'checker'
    official_head = command(['git', '-C', str(args.official), 'rev-parse', 'HEAD'])
    result = dict(baseline=metadata['baseline'], current_snapshot=metadata['current'],
                  flags=FLAGS, cpu=2, trials=args.trials, warmup=1, seed=args.seed,
                  harness_sha256=sha(Path(__file__)), compiler=dict(command=args.cxx,
                      resolved=str(Path(shutil.which(args.cxx)).resolve()),
                      sha256=sha(shutil.which(args.cxx)), version=command([args.cxx, '--version'])),
                  hardware=command(['lscpu']), uname=command(['uname', '-a']),
                  python=command(['python3', '--version']), builds=builds, cases=cases,
                  checker=dict(path=str(checker), sha256=sha(checker), order='checker input actual answer'),
                  official_head=official_head, dependency_scope='-MD: compiler-reported user and system files',
                  custom_generation='generate_custom in this harness; seed=617+n; Python random.Random.randrange(998244353)',
                  source_manifest=metadata, rows=[], checks=0,
                  measurement='fresh subprocess, normal-file stdin/stdout, spawn through wait; checker and hashes outside timer')
    result['before_guard'] = guard(args, source_root, metadata, builds, cases, checker, official_head)
    write_json(args.output / 'primary.json', result)
    print('waiting for exclusive timing lock', flush=True)
    with locked(fcntl.LOCK_EX):
        os.sched_setaffinity(0, {2})
        rng = random.Random(args.seed)
        for row in cases:
            samples = {v: [] for v in variants}
            runs = []
            for trial in range(args.trials + 1):
                order = variants.copy()
                rng.shuffle(order)
                record = dict(trial=trial, warmup=trial == 0, order=order, variants={})
                for variant in order:
                    output = args.work / (variant + '.actual')
                    ns, output_sha = check_case(checker, row, builds[variant]['binary'], output)
                    record['variants'][variant] = dict(elapsed_ns=ns, output_sha256=output_sha, checker='accepted')
                    if trial:
                        samples[variant].append(ns / 1e6)
                    result['checks'] += 1
                runs.append(record)
            saving = [a - b for a, b in zip(samples['before'], samples['current'])]
            gap = [a - b for a, b in zip(samples['current'], samples['reference'])]
            output_row = dict(name=row['name'], runs=runs, samples_ms=samples,
                              median_ms={v: statistics.median(s) for v, s in samples.items()},
                              paired_before_minus_current_ms=saving,
                              paired_current_minus_reference_ms=gap,
                              paired_saving_median_ms=statistics.median(saving),
                              paired_gap_median_ms=statistics.median(gap),
                              current_wins_before=sum(x > 0 for x in saving),
                              current_wins_reference=sum(x < 0 for x in gap))
            result['rows'].append(output_row)
            write_json(args.output / 'primary.json', result)
            print(row['name'], output_row['median_ms'], 'saving', output_row['paired_saving_median_ms'],
                  'wins', output_row['current_wins_before'], flush=True)
    result['after_guard'] = guard(args, source_root, metadata, builds, cases, checker, official_head)
    assert result['before_guard'] == result['after_guard'], 'guard changed'
    assert result['checks'] == 13 * (args.trials + 1) * 3
    result['complete'] = True
    write_json(args.output / 'primary.json', result)
    archive_sources(source_root, args.output)
    replay_work = args.work / 'reconstruction'
    replay = subprocess.run(['python3', str(args.output / 'replay.py'), '--work', str(replay_work)],
                            capture_output=True, text=True, check=True)
    write_json(args.output / 'reconstruction.json', dict(command=replay.args, stdout=replay.stdout,
                                                        stderr=replay.stderr, success=True))
    write_json(args.output / 'artifacts.json', {p.name: dict(sha256=sha(p), bytes=p.stat().st_size)
                                               for p in sorted(args.output.iterdir())
                                               if p.is_file() and p.name != 'artifacts.json'})
    print('complete', result['checks'], 'checker successes', args.output, flush=True)


if __name__ == '__main__':
    main()
