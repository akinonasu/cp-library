import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--manifest', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--trials', type=int, default=21)
    p.add_argument('--seed', type=int, default=20261019)
    a = p.parse_args()
    result = json.loads(a.manifest.read_text())
    result['parent_manifest_sha256'] = sha(a.manifest)
    result['repeat_harness_sha256'] = sha(Path(__file__))
    result['trials'], result['seed'] = a.trials, a.seed
    builds = {}
    for name, entry in result['sources'].items():
        if name.endswith('_headers'): continue
        binary = Path(entry['command'][-1])
        assert sha(binary) == entry['binary_sha256'], name
        builds[name] = binary
    result['judge'] = []
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {result['cpu']})
        rng = random.Random(a.seed)
        for op, problem, _ in [
            ('compose', 'composition_of_formal_power_series', 409336),
            ('compose_large', 'composition_of_formal_power_series_large', 409535),
            ('inverse', 'compositional_inverse_of_formal_power_series', 409399),
            ('inverse_large', 'compositional_inverse_of_formal_power_series_large', 409549),
        ]:
            if op + '_candidate' not in builds: continue
            folder = Path('/tmp/library-checker-problems/polynomial') / problem
            inp, answer = folder / 'in/max_random_00.in', folder / 'out/max_random_00.out'
            row = dict(problem=problem, input_sha256=sha(inp), answer_sha256=sha(answer), variants={})
            samples = {v: [] for v in ('baseline', 'candidate', 'reference')}
            orders = []
            for rep in range(a.trials + 1):
                order = list(samples); rng.shuffle(order); orders.append(order)
                for v in order:
                    binary = builds[op + '_' + v]
                    output = binary.with_suffix('.repeat.out')
                    with inp.open('rb') as source, output.open('wb') as out:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(binary)], stdin=source, stdout=out, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    checked = subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], capture_output=True, text=True)
                    if checked.returncode: raise RuntimeError(checked.stderr)
                    if rep: samples[v].append(elapsed)
            for v, values in samples.items(): row['variants'][v] = dict(median_ms=statistics.median(values), samples_ms=values, output_sha256=sha(builds[op + '_' + v].with_suffix('.repeat.out')))
            row['orders'] = orders
            result['judge'].append(row)
            a.output.parent.mkdir(parents=True, exist_ok=True)
            a.output.write_text(json.dumps(result, indent=2) + '\n')
            print(problem, {v: x['median_ms'] for v, x in row['variants'].items()}, flush=True)


if __name__ == '__main__': main()
