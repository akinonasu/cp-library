import argparse
from pathlib import Path
import fcntl
import hashlib
import json
import os
import random
import shutil
import statistics
import subprocess
import tempfile
import time

HERE = Path(os.environ.get('CP_SP_ROUND5_CODEGEN_WORK', Path(__file__).resolve().parent))
parser = argparse.ArgumentParser()
parser.add_argument('--trials', type=int, default=9)
parser.add_argument('--generic', action='store_true')
parser.add_argument('--variants', nargs='+', default=['old', 'current', 'skip', 'split', 'split_skip'])
parser.add_argument('--kinds', nargs='+', default=['phase', 'actual'])
parser.add_argument('--cases', nargs='+', default=['max_dense_random_00', 'max_dense_random_01', 'max_dense_zero_00', 'max_sparse_random_01', 'max_star_00', 'line_00', 'grid_swirl_00', 'max_dense_long_00'])
parser.add_argument('--output', default='screen_native.json')
args = parser.parse_args()
os.sched_setaffinity(0, {2})
problem = Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
flags = ['-std=gnu++23', '-O2', '-DNDEBUG', '-DATCODER', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero'] + ([] if args.generic else ['-march=native'])
output = HERE / args.output
lock = open('/tmp/cp-wm-opt.lock', 'w')
temporary = tempfile.TemporaryDirectory(prefix='cp-round5-codegen-')
work = Path(temporary.name)
binaries = {}
manifest = {}
for name in args.variants:
    tree = HERE / name
    manifest[name] = {str(p.relative_to(tree)): hashlib.sha256(p.read_bytes()).hexdigest() for p in tree.rglob('*') if p.is_file()}
    for kind in args.kinds:
        key = name + '_' + kind
        binaries[key] = work / key
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            subprocess.run(['g++', *flags, '-I', str(tree), str(tree / (kind + '.cpp')), '-o', str(binaries[key])], check=True)
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)
        print('compiled', key, flush=True)
result = dict(flags=flags, cpu=2, trials=args.trials, compiler=subprocess.check_output(['g++', '--version'], text=True), source_hashes=manifest, samples={})
rng = random.Random(20261019)
for case in args.cases:
    source = problem / 'in' / (case + '.in')
    if not source.exists():
        print('missing', case, flush=True)
        continue
    inp = work / 'input.in'
    shutil.copyfile(source, inp)
    case_result = dict(input_sha256=hashlib.sha256(inp.read_bytes()).hexdigest(), raw={}, median={})
    expected = None
    fcntl.flock(lock, fcntl.LOCK_EX)
    try:
        for trial in range(-1, args.trials):
            order = list(binaries)
            rng.shuffle(order)
            for key in order:
                out = work / 'answer.out'
                with inp.open('rb') as fin, out.open('wb') as fout:
                    started = time.perf_counter()
                    p = subprocess.run([binaries[key]], stdin=fin, stdout=fout, stderr=subprocess.PIPE, check=True)
                    whole = (time.perf_counter() - started) * 1000
                distance = int(out.read_bytes().split(maxsplit=1)[0])
                if expected is None:
                    expected = distance
                assert distance == expected
                if trial == -1:
                    subprocess.run([problem / 'checker', inp, out, problem / 'out' / (case + '.out')], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                else:
                    row = json.loads(p.stderr) if key.endswith('_phase') else {}
                    row['whole_ms'] = whole
                    case_result['raw'].setdefault(key, []).append(row)
    finally:
        fcntl.flock(lock, fcntl.LOCK_UN)
    for key, rows in case_result['raw'].items():
        case_result['median'][key] = {field: statistics.median(row[field] for row in rows) for field in rows[0]}
    result['samples'][case] = case_result
    output.write_text(json.dumps(result, indent=2) + '\n')
    print(case, {key: round(row['whole_ms'], 3) for key, row in case_result['median'].items()}, flush=True)
result['validation'] = 'all distances and warmup paths passed'
output.write_text(json.dumps(result, indent=2) + '\n')
