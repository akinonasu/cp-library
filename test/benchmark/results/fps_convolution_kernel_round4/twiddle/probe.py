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

ROOT = Path('/workspace/cp-library')
WORK = Path('/workspace/cp-fps-round4/kernel')
REFERENCE = Path('/tmp/fps-reference/convolution_mod/409465.cpp')
OFFICIAL = Path('/tmp/library-checker-problems/convolution/convolution_mod')
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inventory(tree):
    return {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--variants', nargs='+', default=['base', 'tile64', 'tile1024', 'reference'])
    parser.add_argument('--cases', nargs='+', default=['max_random_00', 'max_random_01', 'all_same_00'])
    parser.add_argument('--output', type=Path, default=WORK / 'tiles9.json')
    parser.add_argument('--prepare', action='store_true')
    args = parser.parse_args()
    WORK.mkdir(parents=True, exist_ok=True)
    result = dict(baseline=subprocess.check_output(['git', 'rev-parse', '7521ea8'], cwd=ROOT, text=True).strip(),
                  flags=FLAGS, compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  cpu=2, trials=args.trials, seed=20261022, harness_sha256=sha(Path(__file__)), sources={}, rows=[])
    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n')
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        if args.prepare:
            base = WORK / 'base'
            assert not base.exists()
            data = subprocess.check_output(['git', 'archive', result['baseline'], 'fps', 'components', 'math', 'convolution', 'test/convolution.test.cpp', 'test/fps_io.hpp'], cwd=ROOT)
            base.mkdir()
            with tarfile.open(fileobj=io.BytesIO(data)) as archive:
                archive.extractall(base, filter='data')
            original = (base / 'components/fps_convolution.hpp').read_text()
            for variant in ['tile64', 'tile1024']:
                tree = WORK / variant
                shutil.copytree(base, tree)
                before, body = original.rsplit('class Subtrees {', 1)
                if variant == 'tile64':
                    body = body.replace('\n\t\tcase 256: return tile<256>(a, b, k);', '')
                else:
                    body = body.replace('\n\t\tcase 256: return tile<256>(a, b, k);', '\n\t\tcase 256: return tile<256>(a, b, k);\n\t\tcase 1024: return tile<1024>(a, b, k);')
                (tree / 'components/fps_convolution.hpp').write_text(before + 'class Subtrees {' + body)
        def build(variant):
            source = REFERENCE if variant == 'reference' else WORK / variant / 'test/convolution.test.cpp'
            binary = WORK / (variant + '.bin')
            tree = WORK / ('base' if variant == 'reference' else variant)
            command = ['g++', *FLAGS, '-I', str(tree), str(source), '-o', str(binary)]
            completed = subprocess.run(command, capture_output=True, text=True)
            assert completed.returncode == 0, completed.stderr
            return variant, dict(command=command, source_sha256=sha(source), binary_sha256=sha(binary),
                                 tree=inventory(tree), stderr=completed.stderr)
        with ThreadPoolExecutor(max_workers=3) as pool:
            for variant, record in pool.map(build, args.variants):
                result['sources'][variant] = record
                print('built', variant, flush=True)
        save()
        fcntl.flock(lock, fcntl.LOCK_UN)
        print('waiting EX', flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        print('acquired EX', flush=True)
        os.sched_setaffinity(0, {2})
        rng = random.Random(result['seed'])
        for case in args.cases:
            inp, answer = OFFICIAL / 'in' / (case + '.in'), OFFICIAL / 'out' / (case + '.out')
            samples = {v: [] for v in args.variants}
            orders = []
            for rep in range(args.trials + 1):
                order = args.variants.copy()
                rng.shuffle(order)
                orders.append(order)
                for variant in order:
                    output = WORK / (case + '_' + variant + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        start = time.perf_counter_ns()
                        subprocess.run([str(WORK / (variant + '.bin'))], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - start) / 1e6
                    checked = subprocess.run([str(OFFICIAL / 'checker'), str(inp), str(output), str(answer)], capture_output=True, text=True)
                    assert checked.returncode == 0, checked.stderr
                    if rep:
                        samples[variant].append(elapsed)
            row = dict(case=case, input_sha256=sha(inp), answer_sha256=sha(answer), checker_sha256=sha(OFFICIAL / 'checker'),
                       order=orders, variants={v: dict(samples_ms=s, median_ms=statistics.median(s), output_sha256=sha(WORK / (case + '_' + v + '.out'))) for v, s in samples.items()})
            result['rows'].append(row)
            save()
            print(case, {v: round(r['median_ms'], 5) for v, r in row['variants'].items()}, flush=True)
    result['source_binary_preserved'] = all(sha(Path(r['command'][-1])) == r['binary_sha256'] and sha(Path(r['command'][-3])) == r['source_sha256'] for r in result['sources'].values())
    result['tree_preserved'] = all(inventory(WORK / ('base' if v == 'reference' else v)) == r['tree'] for v, r in result['sources'].items())
    assert result['source_binary_preserved'] and result['tree_preserved']
    result['status'] = 'passed'
    save()


if __name__ == '__main__':
    main()
