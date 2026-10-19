import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

ROOT = Path('/workspace/cp-library')
OUT = Path('/workspace/cp-fps-scratch-round3/upstream_f4aa')
WORK = Path('/workspace/cp-fps-scratch-round3/fps-round3-all-speed')
RAW = ROOT / 'test/benchmark/results/fps_round3_all_speed.json'
OFFICIAL = Path('/tmp/library-checker-problems')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inventory(tree):
    return {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}


def save(result):
    (OUT / 'timing.json').write_text(json.dumps(result, indent=2) + '\n')


def main():
    raw = json.loads(RAW.read_text())
    integration = json.loads((OUT / 'integration.json').read_text())
    manifest_path = ROOT / 'test/benchmark/results/fps_round3_validation/judge_official_before.json'
    manifest = json.loads(manifest_path.read_text())
    folders = {p.parent.name: p.parent for p in OFFICIAL.glob('*/*/info.toml')}
    assert integration['all_compiled'] and integration['original_artifacts_preserved']
    assert sha(RAW) == integration['original_raw_sha256']
    assert inventory(WORK / 'current') == integration['original_tree']
    assert inventory(WORK / 'current_upstream_f4aa') == integration['candidate_tree']
    artifacts = integration['original_artifacts'].copy()
    sources = {}
    official = {}
    for row in raw['judge']:
        problem = row['problem']
        if problem not in integration['affected']:
            continue
        build = integration['builds'][problem]
        binary = Path(build['command'][-1])
        artifacts[str(binary)] = build['binary_sha256']
        sources[problem] = {variant: dict(binary=str(path), binary_sha256=sha(path)) for variant, path in {
            'old_f3ac': WORK / (problem + '_current'),
            'new_f4aa': binary,
            'reference': WORK / (problem + '_reference'),
        }.items()}
        folder = folders[problem]
        inp = folder / 'in' / row['input']
        answer = folder / 'out' / (inp.stem + '.out')
        checker = folder / 'checker'
        assert sha(inp) == row['input_sha256'] and sha(answer) == row['answer_sha256']
        for path in [inp, answer, checker]:
            digest = sha(path)
            assert digest == manifest['files'][str(path.relative_to(OFFICIAL))]
            official[str(path)] = digest
    assert all(sha(Path(path)) == digest for path, digest in artifacts.items())
    result = dict(status='prepared', started=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  harness_sha256=sha(Path(__file__)), integration_sha256=sha(OUT / 'integration.json'),
                  original_raw_sha256=sha(RAW), official_manifest_sha256=sha(manifest_path),
                  flags=raw['flags'], compiler=raw['compiler'], cpu=2, trials=21, warmups=1,
                  seed=20261019, official_commit=raw['official_commit'], sources=sources,
                  official_files=official, artifacts_before=artifacts, judge=[])
    save(result)
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        print('waiting EX', flush=True)
        fcntl.flock(lock, fcntl.LOCK_EX)
        print('acquired EX', flush=True)
        os.sched_setaffinity(0, {2})
        rng = random.Random(result['seed'])
        result['status'] = 'timing'
        for row in raw['judge']:
            problem = row['problem']
            if problem not in sources:
                continue
            folder = folders[problem]
            inp = folder / 'in' / row['input']
            answer = folder / 'out' / (inp.stem + '.out')
            samples = {v: [] for v in sources[problem]}
            orders = []
            checker_runs = 0
            for rep in range(22):
                order = list(samples)
                rng.shuffle(order)
                orders.append(order)
                for variant in order:
                    output = OUT / (problem + '_' + variant + '.out')
                    with inp.open('rb') as stdin, output.open('wb') as stdout:
                        begin = time.perf_counter_ns()
                        subprocess.run([sources[problem][variant]['binary']], stdin=stdin, stdout=stdout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    checked = subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], capture_output=True, text=True)
                    assert checked.returncode == 0, (problem, variant, rep, checked.stdout, checked.stderr)
                    checker_runs += 1
                    if rep:
                        samples[variant].append(elapsed)
            variants = {v: dict(median_ms=statistics.median(s), samples_ms=s,
                               output_sha256=sha(OUT / (problem + '_' + v + '.out'))) for v, s in samples.items()}
            paired = [new - old for old, new in zip(samples['old_f3ac'], samples['new_f4aa'])]
            record = dict(problem=problem, input=str(inp), answer=str(answer), checker=str(folder / 'checker'),
                          reference=row['reference'], order=orders, checker_runs=checker_runs, variants=variants,
                          paired_new_minus_old_median_ms=statistics.median(paired),
                          new_wins=sum(x < 0 for x in paired), old_wins=sum(x > 0 for x in paired),
                          new_old_ratio=variants['new_f4aa']['median_ms'] / variants['old_f3ac']['median_ms'])
            assert variants['new_f4aa']['output_sha256'] == variants['old_f3ac']['output_sha256']
            result['judge'].append(record)
            save(result)
            print(problem, {v: round(r['median_ms'], 5) for v, r in variants.items()},
                  'paired', round(record['paired_new_minus_old_median_ms'], 5),
                  'wins', record['new_wins'], flush=True)
    result['artifacts_preserved'] = all(sha(Path(path)) == digest for path, digest in artifacts.items())
    result['official_files_preserved'] = all(sha(Path(path)) == digest for path, digest in official.items())
    result['original_tree_preserved'] = inventory(WORK / 'current') == integration['original_tree']
    result['candidate_tree_preserved'] = inventory(WORK / 'current_upstream_f4aa') == integration['candidate_tree']
    result['live_overlay_preserved'] = all(sha(ROOT / path) == digest for path, digest in integration['overlay'].items())
    result['checker_runs'] = sum(r['checker_runs'] for r in result['judge'])
    assert result['artifacts_preserved'] and result['official_files_preserved']
    assert result['original_tree_preserved'] and result['candidate_tree_preserved'] and result['live_overlay_preserved']
    assert len(result['judge']) == len(integration['affected']) and result['checker_runs'] == len(integration['affected']) * 66
    result['status'] = 'passed'
    result['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save(result)
    print('passed', len(result['judge']), 'problems', result['checker_runs'], 'checker runs', flush=True)


if __name__ == '__main__':
    main()
