import os
from pathlib import Path
import fcntl
import hashlib
import json
import random
import shutil

HERE = Path(os.environ.get('CP_SP_ROUND5_STORAGE_WORK', Path(__file__).resolve().parent))
source = Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
output = HERE / 'shuffled'
(output / 'in').mkdir(parents=True, exist_ok=True)
(output / 'out').mkdir(parents=True, exist_ok=True)
checker = output / 'checker'
if not checker.exists():
    checker.symlink_to(source / 'checker')
lock = open('/tmp/cp-wm-opt.lock', 'w')
manifest = {}
fcntl.flock(lock, fcntl.LOCK_EX)
try:
    for name in ['max_dense_random_00', 'max_dense_random_01', 'max_sparse_random_01', 'max_star_00', 'line_00', 'grid_random_00']:
        path = source / 'in' / (name + '.in')
        raw = path.read_bytes()
        lines = raw.splitlines(keepends=True)
        header, edges = lines[0], lines[1:]
        seed = 20261019
        random.Random(seed).shuffle(edges)
        data = header + b''.join(edges)
        target = output / 'in' / (name + '_shuffled.in')
        target.write_bytes(data)
        shutil.copyfile(source / 'out' / (name + '.out'), output / 'out' / (name + '_shuffled.out'))
        manifest[target.name] = dict(source_name=path.name, source_sha256=hashlib.sha256(raw).hexdigest(), sha256=hashlib.sha256(data).hexdigest(), shuffle_seed=seed)
finally:
    fcntl.flock(lock, fcntl.LOCK_UN)
(HERE / 'shuffled_inputs.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(output)
