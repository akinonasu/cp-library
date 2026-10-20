import argparse
import fcntl
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import tarfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    metadata = json.loads((HERE / 'metadata.json').read_text())
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-io-round4-reproduction'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--reference-cache', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--experiment', choices=[n for n in metadata['raw_experiments'] if n != 'extra_memory9'], default='construct_extra_confirm21')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--verify', action='store_true')
    parser.add_argument('--measure', action='store_true')
    parser.add_argument('--memory', action='store_true')
    args = parser.parse_args()
    if args.prepare_only and (args.verify or args.measure or args.memory):
        parser.error('--prepare-only excludes execution')
    if args.memory and not args.measure:
        parser.error('--memory requires --measure to build its target binaries')
    if args.memory and 'input_extra_capacity' not in metadata['raw_experiments'][args.experiment]['variants']:
        parser.error('--memory requires an experiment containing input_extra_capacity')
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or repo in work.parents:
        parser.error('--work must be outside the repository')
    work.mkdir(parents=True, exist_ok=True)
    result = dict(baseline=metadata['baseline'], headers={}, checks=[], actions=[])

    def save():
        (work / 'reproduction.json').write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        archive = subprocess.check_output(['git', 'archive', metadata['baseline'], 'components', 'convolution', 'fps', 'math',
                                          'test/fps_io.hpp', 'test/convolution.test.cpp', 'test/fps_log.test.cpp',
                                          'test/unit/fps_io.cpp', 'test/unit/fast_io_bulk.cpp'], cwd=repo)
        for variant in metadata['headers']:
            tree = work / variant
            if tree.exists():
                shutil.rmtree(tree)
            if variant == 'baseline':
                with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
                    stream.extractall(tree, filter='data')
            else:
                shutil.copytree(work / 'baseline', tree)
                patch = HERE / (variant + '.patch')
                if sha(patch) != metadata['patches'][variant]:
                    raise RuntimeError('Patch differs: ' + variant)
                subprocess.run(['patch', '-p1', '-i', str(patch)], cwd=tree, check=True, stdout=subprocess.DEVNULL)
            result['headers'][variant] = {name: sha(tree / name) for name in metadata['headers'][variant]}
            if result['headers'][variant] != metadata['headers'][variant]:
                raise RuntimeError('Reconstructed headers differ: ' + variant)
        save()
        print('All 30 header SHA-256 comparisons passed.', flush=True)
        if args.verify:
            environment = dict(__import__('os').environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
            tests = [('parser_tile4', 'test/unit/fps_io.cpp', False), ('parser_tile4', 'test/unit/fast_io_bulk.cpp', False),
                     ('tile4_shift_ni', 'test/unit/fps_io.cpp', False), ('tile4_shift_ni', 'check_io_values.cpp', True),
                     ('input_resize', 'test/unit/fps_io.cpp', False), ('input_extra_capacity', 'extended_fps_io.cpp', True),
                     ('input_extra_capacity', 'check_extra_capacity.cpp', True)]
            for variant, source, bundled in tests:
                for sanitize in [False, True]:
                    binary = work / (variant + '_' + Path(source).stem + ('_san' if sanitize else ''))
                    command = ['g++', *metadata['flags'], '-I', str(work / variant)]
                    if sanitize:
                        command += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
                    command += [str((HERE if bundled else work / variant) / source), '-o', str(binary)]
                    subprocess.run(command, check=True)
                    checked = subprocess.run([str(binary)], capture_output=True, text=True, check=True, env=environment)
                    result['checks'].append(dict(command=command, returncode=checked.returncode, stdout=checked.stdout, stderr=checked.stderr))
                    save()
    if not args.measure:
        return
    raw = json.loads((HERE / (args.experiment + '.json')).read_text())
    for problem, reference in metadata['references'].items():
        if sha(args.reference_cache / problem / (str(reference['id']) + '.cpp')) != reference['sha256']:
            raise RuntimeError('Reference differs: ' + problem)
    for row in raw['results']:
        folder = args.official_root / ('convolution' if row['problem'] == 'convolution_mod' else 'polynomial') / row['problem']
        for path, key in [(folder / 'in' / (row['case'] + '.in'), 'input_sha256'),
                          (folder / 'out' / (row['case'] + '.out'), 'answer_sha256'), (folder / 'checker', 'checker_sha256')]:
            if sha(path) != row[key]:
                raise RuntimeError('Official fixture/checker differs: ' + str(path))
    original = HERE / ('measure.py' if raw.get('rusage_enabled') else 'measure_v1.py')
    if sha(original) != raw['harness_sha256']:
        raise RuntimeError('Recorded timing harness differs')
    text = original.read_text().replace("'/tmp/library-checker-problems'", repr(str(args.official_root.resolve()))).replace("'/tmp/fps-reference'", repr(str(args.reference_cache.resolve())))
    runner = work / 'measure.py'
    runner.write_text(text)
    variants = [v for v in raw['results'][0]['variants'] if v != 'reference']
    command = ['python3', str(runner), '--variants', *variants, '--trials', str(raw['trials']), '--seed', str(raw['seed']),
               '--output', str(work / ('replayed_' + args.experiment + '.json')), '--cases', *metadata['raw_experiments'][args.experiment]['cases']]
    if raw.get('rusage_enabled'):
        command += ['--rusage']
    result['actions'].append(command)
    save()
    subprocess.run(command, check=True)
    if args.memory:
        for problem in metadata['references']:
            for variant in ['baseline', 'input_extra_capacity', 'reference']:
                if not (work / (problem + '_' + variant)).exists():
                    raise RuntimeError('--memory requires an experiment containing baseline, input_extra_capacity and reference')
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            subprocess.run(['gcc', '-O2', str(HERE / 'memory_launcher.c'), '-o', str(work / 'memory_launcher')], check=True)
        shutil.copy2(HERE / 'memory_launcher.c', work / 'memory_launcher.c')
        (work / 'memory.py').write_text((HERE / 'memory.py').read_text().replace("'/tmp/library-checker-problems'", repr(str(args.official_root.resolve()))))
        command = ['python3', str(work / 'memory.py'), '--output', str(work / 'replayed_memory9.json')]
        result['actions'].append(command)
        save()
        subprocess.run(command, check=True)


if __name__ == '__main__':
    main()
