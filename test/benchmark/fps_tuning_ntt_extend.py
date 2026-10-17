import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
from fps_tuning_baseline import prepare

root = Path.cwd()
work = Path('/tmp/cp-fps-tuning-ntt')
work.mkdir(exist_ok=True)
lock = open('/tmp/cp-fps-tuning.lock', 'w')
fcntl.flock(lock, fcntl.LOCK_EX)
baseline = prepare(root)
fcntl.flock(lock, fcntl.LOCK_UN)
extension = '''\tvoid extend(int size) {
\t\tint first = roots.size();
\t\tif (first >= size) return;
\t\tassert(twists.empty());
\t\troots.resize(size); inverse_roots.resize(size);
\t\troot_quotients.resize(size); inverse_root_quotients.resize(size);
\t\tfor (int h = first; h < size; h <<= 1) {
\t\t\tU r = in(T(3).pow((mod - 1) / (4 * h)).v);
\t\t\tU ir = in(T(3).pow(mod - 1 - (mod - 1) / (4 * h)).v);
\t\t\tfor (int i = 0; i < h; ++i) {
\t\t\t\tU a = mul(roots[i], r), b = mul(inverse_roots[i], ir);
\t\t\t\troot_quotients[h + i] = (a < mod ? a : a - mod) * 998244351u;
\t\t\t\tinverse_root_quotients[h + i] = (b < mod ? b : b - mod) * 998244351u;
\t\t\t\troots[h + i] = out(a); inverse_roots[h + i] = out(b);
\t\t\t}
\t\t}
\t}
'''
source = (baseline / 'components/ntt.hpp').read_text()
for variant in ['before', 'extend']:
    fcntl.flock(lock, fcntl.LOCK_EX)
    path = work / variant
    shutil.copytree(baseline, path, dirs_exist_ok=True)
    if variant == 'extend':
        changed = source.replace('\tvoid prepare_cubes(int n) const {', extension + '\tvoid prepare_cubes(int n) const {')
        changed = changed.replace('t = NTT(size, false);', 't.extend(size);')
        (path / 'components/ntt.hpp').write_text(changed)
    subprocess.run(['g++', '-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-I', str(path), str(root / 'test/benchmark/fps_tuning.cpp'), '-o', str(work / (variant + '.exe'))], check=True)
    fcntl.flock(lock, fcntl.LOCK_UN)
os.sched_setaffinity(0, {2})
rng = random.Random(20261018)
rows = []
for operation in ['multiply', 'square', 'inv', 'log', 'exp', 'sqrt', 'div_series', 'taylor_shift']:
    for n in [32768, 524288]:
        for repeat in [1, 5]:
            fcntl.flock(lock, fcntl.LOCK_EX)
            samples = {name: [] for name in ['before', 'extend']}
            expected = None
            for trial in range(5):
                order = list(samples)
                rng.shuffle(order)
                for variant in order:
                    result = json.loads(subprocess.check_output([work / (variant + '.exe'), operation, str(n), str(repeat)], text=True))
                    if expected is None:
                        expected = result['checksum']
                    assert expected == result['checksum'], (operation, n, repeat, variant)
                    samples[variant].append(result['ms'])
            fcntl.flock(lock, fcntl.LOCK_UN)
            row = dict(operation=operation, n=n, repeat=repeat, samples=samples, medians={key: statistics.median(values) for key, values in samples.items()}, checksum=expected)
            rows.append(row)
            print(json.dumps(row), flush=True)
(root / '.verify-helper/cache/fps-tuning-ntt-probe.json').write_text(json.dumps(rows, indent=2) + '\n')
