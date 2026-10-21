import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
from pathlib import Path
import resource
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--problems', type=Path, required=True)
parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
parser.add_argument('--resume', action='store_true')
parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/acl_structures/validation.json')
args = parser.parse_args()
args.problems = args.problems.resolve()
soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
problems = [
    ('data_structure/unionfind', 'union_find'),
    ('data_structure/point_add_range_sum', 'fenwick_tree'),
    ('data_structure/range_affine_range_sum', 'lazy_segtree'),
    ('graph/scc', 'scc'),
    ('other/two_sat', 'two_sat'),
]
report = json.loads(args.output.read_text()) if args.resume else {'started_at': datetime.now(timezone.utc).isoformat(), 'generator_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.problems, text=True).strip(), 'compilers': {}, 'cases': {}, 'results': [], 'completed': False}
report['stack_limit'] = list(resource.getrlimit(resource.RLIMIT_STACK))
args.output.parent.mkdir(parents=True, exist_ok=True)

def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save():
    args.output.write_text(json.dumps(report, indent=2) + '\n')

save()
with open('/tmp/cp-fps-tuning.lock', 'w') as lock, tempfile.TemporaryDirectory(prefix='acl-check-') as temporary:
    fcntl.flock(lock, fcntl.LOCK_SH)
    temp = Path(temporary)
    for problem, name in problems:
        directory = args.problems / problem
        source = ROOT / 'test' / (name + '.test.cpp')
        done = {row['compiler'] for row in report['results'] if row['driver'] == str(source.relative_to(ROOT)) and row['source_sha256'] == sha256(source)}
        if all(cxx in done for cxx in args.cxx):
            continue
        subprocess.run([sys.executable, 'generate.py', str(directory / 'info.toml')], cwd=args.problems, check=True)
        inputs = sorted((directory / 'in').glob('*.in'))
        assert inputs
        report['cases'][name] = {case.name: {'input_sha256': sha256(case), 'answer_sha256': sha256(directory / 'out' / (case.stem + '.out'))} for case in inputs}
        for cxx in args.cxx:
            if cxx in done:
                continue
            report['compilers'][cxx] = subprocess.check_output([cxx, '--version'], text=True)
            exe = temp / name
            subprocess.run([cxx, '-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT), str(source), '-o', str(exe)], check=True)
            for case in inputs:
                actual = temp / 'actual.txt'
                expected = directory / 'out' / (case.stem + '.out')
                with case.open('rb') as inp, actual.open('wb') as out:
                    subprocess.run([str(exe)], stdin=inp, stdout=out, timeout=60, check=True)
                if name in ['scc', 'two_sat']:
                    checked = subprocess.run([str(directory / 'checker'), str(case), str(actual), str(expected)], capture_output=True, text=True, timeout=60)
                    assert checked.returncode == 0, (case, checked.stdout, checked.stderr)
                else:
                    assert actual.read_bytes().split() == expected.read_bytes().split(), case
            report['results'].append({'driver': str(source.relative_to(ROOT)), 'source_sha256': sha256(source), 'compiler': cxx, 'cases_passed': len(inputs)})
            save()
            print(name, cxx, len(inputs), 'passed', flush=True)
report['completed'] = True
report['completed_at'] = datetime.now(timezone.utc).isoformat()
save()
