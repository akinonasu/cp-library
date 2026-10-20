import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
FILES = ['data_structure/wavelet_matrix/wavelet_matrix.hpp', 'data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp',
         'data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp', 'components/fenwick_sum.hpp',
         'components/quad_bit_vector.hpp', 'components/oct_bit_vector.hpp', 'components/splitmix64.hpp']
DRIVER = 'test/benchmark/abc467_g.cpp'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def generate(path, mode, n, q):
    rng = random.Random(829971)
    limit = 2 if mode == 'binary' else 256 if mode == 'dense' else 1000000000
    with path.open('w') as stream:
        stream.write(f'{n} {q}\n')
        stream.write(' '.join(str(rng.randrange(1, limit + 1)) for _ in range(n)) + '\n')
        for _ in range(q):
            p, x = rng.randrange(n), rng.randrange(1, limit + 1)
            l, r = sorted([rng.randrange(n), rng.randrange(n)])
            if mode == 'full':
                l, r = 0, n - 1
            elif mode == 'short':
                r = min(n - 1, l + rng.randrange(16))
            target = rng.randrange(1, max(2, limit * (r - l + 1) // 4 + 1))
            stream.write(f'{p+1} {x} {l+1} {r+1} {target}\n')


def single(source):
    source = source.replace('template <class Choose>\n', 'template <bool Single = false, class Choose>\n')
    source = source.replace('int base = 3 * d + 1;\n\t\t\tint a',
        'if constexpr (Single) if (total.count == 1) return {x, total};\n\t\t\tint base = 3 * d + 1;\n\t\t\tint a', 1)
    source = source.replace('auto [x, leaf] = walk(', 'auto [x, leaf] = walk<true>(')
    source = source.replace('answer + Sum(index.values[x]) * k', 'answer + (leaf.count == 1 ? leaf.sum : Sum(index.values[x])) * k')
    source = source.replace('Sum value = Sum(index.values[x]);', 'Sum value = leaf.count == 1 ? leaf.sum : Sum(index.values[x]);')
    source = source.replace('return index.values[x];', 'return leaf.count == 1 ? T(leaf.sum) : index.values[x];')
    return source


def common(source):
    begin = source.index('\t\twhile (l != r) {')
    end = source.index('\n\t\treturn result;', begin)
    return source[:begin] + '''
		if (l == r) return result;
		unsigned shared = unsigned(l) & (std::numeric_limits<unsigned>::max() << std::bit_width(unsigned(l ^ r)));
		for (; unsigned(l) != shared; l &= l - 1) result -= data[l - 1];
		for (; unsigned(r) != shared; r &= r - 1) result += data[r - 1];''' + source[end:]


def separated(source):
    begin = source.index('\t\twhile (l != r) {')
    end = source.index('\n\t\treturn result;', begin)
    return source[:begin] + '''
		for (; l; l &= l - 1) result -= data[l - 1];
		for (; r; r &= r - 1) result += data[r - 1];''' + source[end:]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--explore', action='store_true')
    parser.add_argument('--snapshot', type=Path)
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--n', type=int, default=100000)
    parser.add_argument('--q', type=int, default=100000)
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {args.cpu})
    lock = open('/tmp/cp-wm-opt.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    source = {path: (ROOT / path).read_text() for path in FILES}
    frozen = {path: digest(ROOT / path) for path in FILES + [DRIVER, 'components/fast_io.hpp', 'test/benchmark/weighted_wavelet_matrix.py']}
    if args.snapshot:
        source = json.loads(args.snapshot.read_text())['base_sources']
    variants = {'current': source.copy()}
    if args.explore:
        variants = {'initial': source.copy()}
        for name in ['single', 'common', 'separated', 'common_single', 'two_updates']:
            v = source.copy()
            if 'single' in name:
                v[FILES[1]] = single(v[FILES[1]])
            if 'common' in name:
                v['components/fenwick_sum.hpp'] = common(v['components/fenwick_sum.hpp']).replace('#include <cassert>', '#include <cassert>\n#include <bit>')
            if name == 'separated':
                v['components/fenwick_sum.hpp'] = separated(v['components/fenwick_sum.hpp'])
            if name == 'two_updates':
                v[FILES[2]] = v[FILES[2]].replace('wm.move_weight(current[p], q, 1);', 'if (current[p] != q) wm.add_weight(current[p], -1), wm.add_weight(q, 1);')
            variants[name] = v
    flags = ['-std=gnu++23', '-O2', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE']
    if not args.generic:
        flags += ['-march=native']
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), flags=flags, affinity=[args.cpu],
                  compiler=subprocess.check_output(['g++', '--version'], text=True),
                  cpu=subprocess.check_output(['lscpu'], text=True), kernel=subprocess.check_output(['uname', '-a'], text=True),
                  trials=args.trials, warmup=1, order_seed=20261019, input_seed=829971, source_sha256=frozen,
                  base_sources=source, variants=variants, results=[], completed=False,
                  timer='Fresh process including stdin parsing, candidate registration, build, updates, queries and output to a Linux temporary file.')
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def save():
        temporary = args.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(report, indent=2) + '\n')
        temporary.replace(args.output)

    save()
    rng = random.Random(20261019)
    with tempfile.TemporaryDirectory(prefix='wm-weight-') as temp:
        work = Path(temp)
        binaries = {}
        driver = (ROOT / DRIVER).read_text()
        for name, files in variants.items():
            for path, code in files.items():
                dest = work / name / path
                dest.parent.mkdir(parents=True, exist_ok=True)
                dest.write_text(code)
            cpp = work / (name + '.cpp')
            cpp.write_text(driver)
            binary = work / name / 'a.out'
            with exclusive():
                subprocess.run(['g++', *flags, '-I.', '-DPOINT_WM_HEADER="' + str(work / name / FILES[2]) + '"', str(cpp), '-o', str(binary)], check=True)
            binaries[name] = binary
        if not args.explore:
            code = driver.replace('output.println(wm.min_count_largest(z.l, z.r, z.target));', '''
		int answer = -1;
		if (wm.range_sum(z.l, z.r) >= z.target) {
			int lo = 0, hi = z.r - z.l;
			while (hi - lo > 1) {
				int mid = lo + (hi - lo) / 2;
				if (wm.sum_largest(z.l, z.r, mid) >= z.target) hi = mid;
				else lo = mid;
			}
			answer = hi;
		}
		output.println(answer);''')
            assert code != driver
            cpp = work / 'binary_search.cpp'
            cpp.write_text(code)
            binary = work / 'binary_search'
            with exclusive():
                subprocess.run(['g++', *flags, '-I.', str(cpp), '-o', str(binary)], check=True)
            binaries['binary_search'] = binary
            report['binary_search_source'] = code
        for mode in ['random', 'full', 'short', 'dense', 'binary']:
            inp = work / 'input'
            generate(inp, mode, args.n, args.q)
            programs = binaries if args.explore or mode in ['random', 'short'] else {'current': binaries['current']}
            samples = {name: [] for name in programs}
            outputs = set()
            with exclusive():
                for trial in range(-1, args.trials):
                    order = list(programs)
                    rng.shuffle(order)
                    for name in order:
                        out = work / 'output'
                        with inp.open('rb') as stream, out.open('wb') as destination:
                            start = time.perf_counter()
                            subprocess.run([programs[name]], stdin=stream, stdout=destination, check=True)
                            ms = (time.perf_counter() - start) * 1000
                        outputs.add(digest(out))
                        if trial >= 0:
                            samples[name].append(ms)
                assert len(outputs) == 1, (mode, outputs)
                rss = {}
                for name in programs:
                    memory = work / 'rss'
                    with inp.open('rb') as stream, (work / 'output').open('wb') as destination:
                        subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(memory), programs[name]], stdin=stream, stdout=destination, check=True)
                    rss[name] = int(memory.read_text())
            row = dict(n=args.n, q=args.q, mode=mode, input_sha256=digest(inp), output_sha256=outputs.pop(),
                       samples_ms=samples, median_ms={name: statistics.median(xs) for name, xs in samples.items()}, peak_rss_kib=rss)
            report['results'].append(row)
            print(mode, row['median_ms'], flush=True)
            save()
    for path, expected in frozen.items():
        assert digest(ROOT / path) == expected, path
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()
    print('All outputs and source hashes match', flush=True)


if __name__ == '__main__':
    main()
