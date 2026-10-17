import fcntl
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
from fps_tuning_baseline import prepare

root = Path.cwd()
work = Path('/tmp/cp-fps-tuning-leaf')
work.mkdir(exist_ok=True)
lock = open('/tmp/cp-fps-tuning.lock', 'w')
fcntl.flock(lock, fcntl.LOCK_EX)
baseline = prepare(root)
source = (baseline / 'components/ntt.hpp').read_text()
fcntl.flock(lock, fcntl.LOCK_UN)
variants = ['before', 'group4', 'group2', 'group1', 'fused4']
for variant in variants:
    fcntl.flock(lock, fcntl.LOCK_EX)
    path = work / variant
    shutil.copytree(baseline, path, dirs_exist_ok=True)
    if variant != 'before':
        count = int(variant[-1])
        before = '\t\t\tinverse_leaf_products8<8>(a + i, b + i, ws, qs);\n\t\t\tinverse_leaf_products8<8, true>(a + i, b + i, ws, qs);'
        if variant == 'fused4':
            after = f'\t\t\tfor (int j = 0; j < 8; j += {count}) {{\n\t\t\t\tinverse_leaf_products8<{count}>(a + i + 8 * j, b + i + 8 * j, ws + j, qs + j);\n\t\t\t\tinverse_leaf_products8<{count}, true>(a + i + 8 * j, b + i + 8 * j, ws + j, qs + j);\n\t\t\t}}'
        else:
            after = f'\t\t\tfor (int j = 0; j < 8; j += {count}) inverse_leaf_products8<{count}>(a + i + 8 * j, b + i + 8 * j, ws + j, qs + j);\n\t\t\tfor (int j = 0; j < 8; j += {count}) inverse_leaf_products8<{count}, true>(a + i + 8 * j, b + i + 8 * j, ws + j, qs + j);'
        assert before in source
        (path / 'components/ntt.hpp').write_text(source.replace(before, after))
    subprocess.run(['g++', '-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-I', str(path), str(root / 'test/benchmark/fps_tuning.cpp'), '-o', str(work / (variant + '.exe'))], check=True)
    fcntl.flock(lock, fcntl.LOCK_UN)
os.sched_setaffinity(0, {2})
rng = random.Random(20261018)
rows = []
for operation in ['inv', 'sqrt', 'log', 'exp']:
    for n in [32768, 131072, 500000, 524288]:
        fcntl.flock(lock, fcntl.LOCK_EX)
        samples = {name: [] for name in variants}
        expected = None
        for trial in range(7):
            order = list(samples)
            rng.shuffle(order)
            for variant in order:
                result = json.loads(subprocess.check_output([work / (variant + '.exe'), operation, str(n)], text=True))
                if expected is None:
                    expected = result['checksum']
                assert expected == result['checksum'], (operation, n, variant)
                samples[variant].append(result['ms'])
        fcntl.flock(lock, fcntl.LOCK_UN)
        row = dict(operation=operation, n=n, samples=samples, medians={key: statistics.median(values) for key, values in samples.items()}, checksum=expected)
        rows.append(row)
        print(json.dumps(row), flush=True)
(root / '.verify-helper/cache/fps-tuning-ntt-leaf-probe.json').write_text(json.dumps(rows, indent=2) + '\n')
