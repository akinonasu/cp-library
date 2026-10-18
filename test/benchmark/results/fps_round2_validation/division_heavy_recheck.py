from pathlib import Path
from datetime import datetime, timezone
import argparse, fcntl, hashlib, json, os, platform, random, statistics, subprocess, time

ROOT = Path(__file__).resolve().parents[4]
parser = argparse.ArgumentParser()
parser.add_argument('--parent-json', type=Path, default=ROOT / 'test/benchmark/results/fps_round2_all_speed.json')
parser.add_argument('--work', type=Path, default=Path('/tmp/fps-round2-all-speed'))
parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
parser.add_argument('--output', type=Path, default=Path(__file__).with_suffix('.json'))
parser.add_argument('--cpu', type=int, default=2)
parser.add_argument('--trials', type=int, default=21)
parser.add_argument('--seed', type=int, default=20261029)
args = parser.parse_args()
args.parent_json = args.parent_json.resolve()
args.work = args.work.resolve()
args.official_root = args.official_root.resolve()
args.output = args.output.resolve()
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
parent = json.loads(args.parent_json.read_text())
parent_sha = sha(args.parent_json)
variants = ['before', 'current', 'reference']
records = {name: parent['sources']['division_of_polynomials_' + name] for name in variants}
binaries = {name: args.work / ('division_of_polynomials_' + name) for name in variants}
folder = args.official_root / 'polynomial/division_of_polynomials'
cases = ['v_random_01', 'v_random_02', 'v_random_03', 'n_max_00']

result = {
    'started_at': datetime.now(timezone.utc).isoformat(),
    'parent_measurement': {'path': str(args.parent_json), 'sha256': parent_sha,
                           'baseline': parent['baseline'], 'harness_sha256': parent['harness_sha256']},
    'purpose': 'Heavy Division of Polynomials cases using exactly the completed integrated benchmark binaries; no compilation or source edits',
    'compiler': parent['compiler'], 'flags': parent['flags'], 'hardware': parent['hardware'],
    'platform': platform.platform(), 'cpu_affinity': [args.cpu], 'seed': args.seed,
    'trials': args.trials, 'warmups_per_case_variant': 1,
    'metric': 'Whole-process wall milliseconds including startup, regular-file input/output, computation, and exit; checker executes after the timer stops',
    'lock': {'path': '/tmp/cp-fps-tuning.lock', 'mode': 'exclusive'},
    'checker_order': ['input', 'actual', 'answer'],
    'official_commit': subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip(),
    'parent_binary_source_records': records,
    'parent_source_snapshots': {name: parent['sources'][name] for name in ['before', 'current']},
    'harness_sha256': sha(Path(__file__)), 'workloads': [],
}
assert result['official_commit'] == parent['official_commit']

def verify_parent():
    assert sha(args.parent_json) == parent_sha, 'Parent measurement changed'
    binary_hashes = {}
    driver_hashes = {}
    snapshots = {}
    for name in variants:
        record = records[name]
        assert Path(record['command'][-1]).resolve() == binaries[name]
        assert sha(binaries[name]) == record['binary_sha256'], ('binary', name)
        source = Path(record['command'][-3])
        assert sha(source) == record['source_sha256'], ('driver/source', name)
        binary_hashes[name] = sha(binaries[name])
        driver_hashes[name] = {'path': str(source), 'sha256': sha(source)}
    for name in ['before', 'current']:
        hashes = {rel: sha(args.work / name / rel) for rel in parent['sources'][name]}
        assert hashes == parent['sources'][name], ('source snapshot', name)
        snapshots[name] = hashes
    return {'binaries': binary_hashes, 'drivers': driver_hashes, 'source_snapshots': snapshots}

def save():
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')

print('Waiting for exclusive timing lock', flush=True)
with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
    fcntl.flock(lock, fcntl.LOCK_EX)
    os.sched_setaffinity(0, {args.cpu})
    result['lock_acquired_at'] = datetime.now(timezone.utc).isoformat()
    result['verified_before'] = verify_parent()
    rng = random.Random(args.seed)
    checker_hash = sha(folder / 'checker')
    print('Lock acquired; parent binaries, drivers, and snapshots verified', flush=True)
    for case in cases:
        inp = folder / 'in' / (case + '.in')
        answer = folder / 'out' / (case + '.out')
        input_hash, answer_hash = sha(inp), sha(answer)
        samples = {name: [] for name in variants}
        checks = {name: [] for name in variants}
        orders = []
        for repetition in range(args.trials + 1):
            order = variants.copy()
            rng.shuffle(order)
            orders.append(order)
            for name in order:
                out = args.work / ('division_heavy_' + case + '_' + name + '.out')
                with inp.open('rb') as stdin, out.open('wb') as stdout:
                    started = time.perf_counter_ns()
                    run = subprocess.run([str(binaries[name])], stdin=stdin, stdout=stdout,
                                         stderr=subprocess.PIPE, timeout=120)
                    elapsed = (time.perf_counter_ns() - started) / 1e6
                assert run.returncode == 0, (case, name, run.stderr)
                command = [str(folder / 'checker'), str(inp), str(out), str(answer)]
                checked = subprocess.run(command, capture_output=True, text=True, timeout=120)
                assert checked.returncode == 0, (command, checked.stdout, checked.stderr)
                checks[name].append({'repetition': repetition, 'returncode': checked.returncode,
                                     'actual_sha256': sha(out), 'stdout': checked.stdout, 'stderr': checked.stderr})
                if repetition:
                    samples[name].append(elapsed)
        medians = {name: statistics.median(values) for name, values in samples.items()}
        paired = [b - c for b, c in zip(samples['before'], samples['current'])]
        row = {
            'case': case, 'input': str(inp), 'input_header': inp.open().readline().strip(),
            'input_sha256': input_hash, 'answer_sha256': answer_hash, 'checker_sha256': checker_hash,
            'orders_including_warmup': orders, 'samples_ms': samples, 'median_ms': medians,
            'median_saving_ms': medians['before'] - medians['current'],
            'paired_saving_ms': paired, 'paired_median_saving_ms': statistics.median(paired),
            'current_wins': sum(value > 0 for value in paired), 'checks_including_warmup': checks,
        }
        assert sha(inp) == input_hash and sha(answer) == answer_hash and sha(folder / 'checker') == checker_hash
        result['workloads'].append(row)
        save()
        print(case, medians, 'paired', row['paired_median_saving_ms'], 'wins', row['current_wins'], flush=True)
    result['verified_after'] = verify_parent()
    result['dependencies_unchanged'] = result['verified_before'] == result['verified_after']
    assert result['dependencies_unchanged']
    result['finished_at'] = datetime.now(timezone.utc).isoformat()
    save()
    print('All 264 official checker calls passed; source/binary snapshots unchanged', flush=True)
