import csv
import hashlib
from pathlib import Path
import random
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parents[2]
variants, out = map(Path, sys.argv[1:3])
trials = int(sys.argv[3]) if len(sys.argv) > 3 else 5
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/dynamic_segtree_abc403g.cpp').read_text()
with tempfile.TemporaryDirectory(prefix='dynamic-segtree-process-') as directory:
    work = Path(directory)
    trees = {
        'base': ('base.hpp', 'DynamicSegtree_base'),
        'current': ('data_structure/segment_tree/dynamic_segtree.hpp', 'DynamicSegtree'),
        'combo_branch': ('combo_branch.hpp', 'DynamicSegtree_combo_branch'),
        'combo_leaf': ('combo_leaf.hpp', 'DynamicSegtree_combo_leaf'),
        'bucket8': ('test/benchmark/dynamic_segtree_bucket.hpp', 'DynamicSegtreeBucket'),
    }
    paths = {}
    for name, (header, tree) in trees.items():
        text = source.replace('"data_structure/segment_tree/dynamic_segtree.hpp"', '"' + header + '"').replace('DynamicSegtree<Value', tree + '<Value')
        path = work / (name + '.cpp')
        path.write_text(text)
        for packed in [False, True]:
            label = name + ('_packed' if packed else '')
            executable = work / label
            subprocess.run(['g++', '-std=c++23', '-O2', '-DNDEBUG', '-I', str(root), '-I', str(variants), *(['-DDYNAMIC_SEGTREE_PACKED'] if packed else []), str(path), '-o', str(executable)], check=True)
            paths[label] = executable
    samples = [('5\n1\n3\n1\n999999994\n999999993\n', '2\n2\n8\n6\n1000000006\n'), ('8\n105282053\n695234822\n468007124\n120710491\n568831200\n700753895\n765188109\n262666319\n', '105282054\n105282054\n905798931\n599798602\n995656103\n891549225\n1652393438\n1652393438\n')]
    for executable in paths.values():
        for data, expected in samples:
            assert subprocess.check_output([str(executable)], input=data.encode()).decode() == expected
    rng = random.Random(8652963)
    data = work / 'input.txt'
    data.write_text('300000\n' + '\n'.join(str(rng.randrange(1000000000)) for _ in range(300000)) + '\n')
    with (out / 'process.csv').open('w') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['variant', 'trial', 'ms', 'max_rss_kib', 'checksum'])
        expected = None
        for trial in range(trials):
            order = list(paths)
            rng.shuffle(order)
            for name in order:
                output, report = work / 'output.txt', work / 'time.txt'
                start = time.perf_counter()
                with data.open('rb') as fin, output.open('wb') as fout:
                    subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(report), str(paths[name])], stdin=fin, stdout=fout, check=True)
                ms = (time.perf_counter() - start) * 1000
                checksum = hashlib.sha256(output.read_bytes()).hexdigest()
                if expected is None:
                    expected = checksum
                assert checksum == expected, name
                writer.writerow([name, trial, ms, int(report.read_text()), checksum])
