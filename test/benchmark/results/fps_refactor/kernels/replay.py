import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('--work', type=Path, required=True)
parser.add_argument('--cxx', default='g++')
parser.add_argument('--cpu', type=int, default=2)
parser.add_argument('--trials', type=int, default=5)
parser.add_argument('--variants', nargs='+', default=['baseline', 'all_top', 'shared_mont'])
parser.add_argument('--prepare-only', action='store_true')
args = parser.parse_args()
args.work.mkdir(parents=True, exist_ok=False)
metadata = json.loads((HERE / 'metadata.json').read_text())
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
for name in ['baseline.tar.xz', 'candidates.tar.xz', 'driver.cpp']:
    assert sha(HERE / name) == metadata['artifacts'][name]
for name in ['baseline', 'candidates']:
    with tarfile.open(HERE / (name + '.tar.xz')) as archive:
        archive.extractall(args.work / name, filter='data')
for name in args.variants:
    target = args.work / name
    if name != 'baseline':
        shutil.copytree(args.work / 'baseline', target)
    for source, expected in metadata['variants'][name]['source_hashes'].items():
        shutil.copyfile(args.work / 'candidates' / name / source, target / source)
        assert sha(target / source) == expected
    for source, expected in metadata['baseline_sources'].items():
        if source not in metadata['variants'][name]['source_hashes']:
            assert sha(target / source) == expected
if args.prepare_only:
    print('Restored candidate sources and verified all source hashes')
    raise SystemExit()
os.sched_setaffinity(0, {args.cpu})
for name in args.variants:
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        subprocess.run([args.cxx, *metadata['flags'], '-I' + str((args.work / name).resolve()), str(HERE / 'driver.cpp'), '-o', str(args.work / (name + '.bin'))], check=True)
rng = random.Random(20261021)
report = {'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'trials': args.trials, 'rows': []}
with open('/tmp/cp-fps-tuning.lock', 'a') as lock, open('/tmp/cp-wm-opt.lock', 'a') as legacy:
    fcntl.flock(lock, fcntl.LOCK_EX)
    fcntl.flock(legacy, fcntl.LOCK_EX)
    for case in json.loads((HERE / 'screen_mont.json').read_text())['rows']:
        values = {name: [] for name in args.variants}
        checks = set()
        for trial in range(args.trials):
            order = args.variants.copy()
            rng.shuffle(order)
            for name in order:
                row = json.loads(subprocess.check_output([str((args.work / (name + '.bin')).resolve()), case['op'], str(case['n']), str(case['repeat'])], text=True, timeout=120))
                values[name].append(row['ms'])
                checks.add((row['checksum'], row['size']))
        assert len(checks) == 1
        report['rows'].append({'op': case['op'], 'n': case['n'], 'samples': values, 'median_ms': {name: statistics.median(v) for name, v in values.items()}})
        (args.work / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
        print(case['op'], case['n'], report['rows'][-1]['median_ms'], flush=True)
