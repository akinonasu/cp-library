import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
EXPERIMENTS = {
    'wide': ('whole.json', 'exp_of_formal_power_series', 'exp'),
    'ifma': ('whole_ifma.json', 'exp_of_formal_power_series', 'exp'),
    'pow': ('whole_pow_scaled.json', 'pow_of_formal_power_series', 'pow'),
    'pow-confirm': ('whole_pow_scaled_confirm.json', 'pow_of_formal_power_series', 'pow'),
    'pow-template': ('whole_pow_scaled_template.json', 'pow_of_formal_power_series', 'pow'),
    'tail': ('whole_exp_tail.json', 'exp_of_formal_power_series', 'exp'),
    'tail-outlined': ('whole_exp_tail_outlined.json', 'exp_of_formal_power_series', 'exp'),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-exp-round3-reproduction'))
    parser.add_argument('--reference-cache', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--experiment', choices=EXPERIMENTS, default='tail-outlined')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--verify', action='store_true')
    parser.add_argument('--measure', action='store_true')
    args = parser.parse_args()
    if args.prepare_only and (args.verify or args.measure):
        parser.error('--prepare-only excludes --verify and --measure')
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or repo in work.parents:
        parser.error('--work must be outside the repository')
    work.mkdir(parents=True, exist_ok=True)
    metadata = json.loads((HERE / 'metadata.json').read_text())
    result = dict(baseline=metadata['baseline'], headers={}, builds={}, checks=[], measurements=[])
    trees = {v: work / v for v in metadata['headers']}
    raw_name, problem, prefix = EXPERIMENTS[args.experiment]
    raw = json.loads((HERE / raw_name).read_text())
    variants = list(raw['binaries'])
    driver = 'test/fps_' + prefix + '.test.cpp'

    def save():
        (work / ('reproduction_' + args.experiment + '.json')).write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        archive = subprocess.check_output(['git', 'archive', metadata['baseline'], 'components', 'fps', 'math', 'convolution',
                                          'test/fps_exp.test.cpp', 'test/fps_pow.test.cpp', 'test/fps_log.test.cpp',
                                          'test/fps_io.hpp', 'test/unit/fps_exp_blocked.cpp'], cwd=repo)
        for tree in trees.values():
            if tree.exists():
                shutil.rmtree(tree)
        with tarfile.open(fileobj=io.BytesIO(archive)) as stream:
            stream.extractall(trees['baseline'], filter='data')
        for variant in trees:
            if variant != 'baseline':
                shutil.copytree(trees['baseline'], trees[variant])
                patch = HERE / (variant + '.patch')
                if sha(patch) != metadata['patches'][variant]:
                    raise RuntimeError('Patch hash differs: ' + variant)
                subprocess.run(['patch', '-p1', '-i', str(patch)], cwd=trees[variant], check=True,
                               stdout=subprocess.DEVNULL)
            result['headers'][variant] = sha(trees[variant] / 'components/fps_exp_blocked.hpp')
            if result['headers'][variant] != metadata['headers'][variant]:
                raise RuntimeError('Header hash differs: ' + variant)
        save()
        print('All eight reconstructed headers match their recorded SHA-256 values.', flush=True)
        if not args.verify and not args.measure:
            return
        result['compiler'] = subprocess.check_output(['g++', '--version'], text=True).splitlines()[0]
        if args.verify:
            environment = os.environ.copy()
            environment.update(ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
            for name, source, extra in [
                ('existing', trees['tail_outlined'] / 'test/unit/fps_exp_blocked.cpp', []),
                ('tail', HERE / 'unit.cpp', []),
                ('sanitize', HERE / 'unit.cpp', ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g'])]:
                binary = work / ('unit_' + name)
                command = ['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1',
                           '-I', str(trees['tail_outlined']), *extra, str(source), '-o', str(binary)]
                subprocess.run(command, check=True, env=environment)
                checked = subprocess.run([str(binary)], check=True, capture_output=True, text=True,
                                         env=environment, timeout=180)
                result['checks'].append(dict(name=name, command=command, returncode=checked.returncode,
                                             stdout=checked.stdout, stderr=checked.stderr))
                save()
        if not args.measure:
            return
        reference = metadata['references'][problem]
        source = args.reference_cache.resolve() / problem / (str(reference['id']) + '.cpp')
        if sha(source) != reference['sha256']:
            raise RuntimeError('Reference source hash differs')
        flags = metadata['flags']
        jobs = []
        for variant in variants:
            binary = work / (prefix + '_' + variant)
            command = (['g++', *flags, str(source)] if variant == 'reference' else
                       ['g++', *flags, '-I', str(trees[variant]), str(trees[variant] / driver)])
            jobs.append((variant, binary, command + ['-o', str(binary)]))

        def build(job):
            variant, binary, command = job
            subprocess.run(command, check=True)
            return variant, dict(command=command, binary_sha256=sha(binary))

        with ThreadPoolExecutor(max_workers=2) as pool:
            for variant, value in pool.map(build, jobs):
                result['builds'][variant] = value
        save()
        folder = args.official_root.resolve() / 'polynomial' / problem
        for row in raw['measurements']:
            if sha(folder / 'in' / (row['case'] + '.in')) != row['input_sha256']:
                raise RuntimeError('Official input hash differs: ' + row['case'])
            if sha(folder / 'out' / (row['case'] + '.out')) != row['answer_sha256']:
                raise RuntimeError('Official answer hash differs: ' + row['case'])
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        rng = random.Random(raw['shuffle_seed'])
        for expected in raw['measurements']:
            case = expected['case']
            inp, answer = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
            sample = {v: [] for v in variants}
            orders = []
            for rep in range(raw['trials'] + 1):
                order = variants.copy()
                rng.shuffle(order)
                orders.append(order)
                for variant in order:
                    output = work / (case + '_' + prefix + '_' + variant + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(work / (prefix + '_' + variant))], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if rep:
                        sample[variant].append(elapsed)
            if orders != expected['orders']:
                raise RuntimeError('Restored shuffle order differs')
            row = dict(case=case, orders=orders, samples_ms=sample,
                       median_ms={v: statistics.median(s) for v, s in sample.items()})
            result['measurements'].append(row)
            save()
            print(case, row['median_ms'], flush=True)


if __name__ == '__main__':
    main()
