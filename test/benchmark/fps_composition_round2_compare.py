import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'test/benchmark/results/fps_composition_round2_compare.json'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
names = {'table_only': 'tableonly', 'compact': 'tight', 'padding': 'padding'}
records = {name: json.loads((ROOT / ('test/benchmark/results/fps_composition_round2_' + stem + '.json')).read_text()) for name, stem in names.items()}
result = dict(cpu=2, trials=21, seed=20261023, metric='whole process wall milliseconds; file input/output; warmup 1',
              harness_sha256=sha(Path(__file__)), manifests={name: sha(ROOT / ('test/benchmark/results/fps_composition_round2_' + stem + '.json')) for name, stem in names.items()},
              builds={}, judge=[])
with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
    fcntl.flock(lock, fcntl.LOCK_EX)
    os.sched_setaffinity(0, {2})
    rng = random.Random(result['seed'])
    for op, problem in [('compose', 'composition_of_formal_power_series'), ('compose_large', 'composition_of_formal_power_series_large')]:
        entries = {name: record['sources'][op + '_candidate'] for name, record in records.items()}
        for name in ('baseline', 'reference'): entries[name] = records['padding']['sources'][op + '_' + name]
        result['builds'][op] = entries
        binaries = {name: Path(entry['command'][-1]) for name, entry in entries.items()}
        for name, binary in binaries.items(): assert sha(binary) == entries[name]['binary_sha256']
        folder = Path('/tmp/library-checker-problems/polynomial') / problem
        inp, answer = folder / 'in/max_random_00.in', folder / 'out/max_random_00.out'
        row = dict(problem=problem, input_sha256=sha(inp), answer_sha256=sha(answer), variants={}, orders=[])
        samples = {name: [] for name in entries}
        for rep in range(result['trials'] + 1):
            order = list(entries); rng.shuffle(order); row['orders'].append(order)
            for name in order:
                output = binaries[name].with_suffix('.compare.out')
                with inp.open('rb') as source, output.open('wb') as out:
                    begin = time.perf_counter_ns()
                    subprocess.run([str(binaries[name])], stdin=source, stdout=out, check=True)
                    elapsed = (time.perf_counter_ns() - begin) / 1e6
                checked = subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], capture_output=True, text=True)
                if checked.returncode: raise RuntimeError(checked.stderr)
                if rep: samples[name].append(elapsed)
        for name, values in samples.items(): row['variants'][name] = dict(median_ms=statistics.median(values), samples_ms=values)
        result['judge'].append(row)
        OUT.write_text(json.dumps(result, indent=2) + '\n')
        print(problem, {name: value['median_ms'] for name, value in row['variants'].items()}, flush=True)
