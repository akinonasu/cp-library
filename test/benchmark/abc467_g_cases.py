import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
from html.parser import HTMLParser
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile
import time
import urllib.request

from weighted_wavelet_matrix import FILES, DRIVER


REFERENCE = 'https://atcoder.jp/contests/abc467/editorial/23445'


class CodeBlocks(HTMLParser):
    def __init__(self, text):
        super().__init__()
        self.blocks, self.current = [], None
        self.feed(text)

    def handle_starttag(self, tag, attrs):
        if tag == 'pre':
            self.current = []

    def handle_endtag(self, tag):
        if tag == 'pre' and self.current is not None:
            self.blocks.append(''.join(self.current))
            self.current = None

    def handle_data(self, data):
        if self.current is not None:
            self.current.append(data)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=Path('test/benchmark/results/weighted_wavelet_matrix/abc467_g_cases.json'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--reference-trials', type=int, default=3)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    assert args.trials > 0 and args.reference_trials > 0
    root = Path(__file__).resolve().parents[2]
    os.chdir(root)
    cases = sorted(args.input_dir.glob('*.txt'))
    assert cases, args.input_dir
    os.sched_setaffinity(0, {args.cpu})
    cache = root / '.verify-helper/cache/wm-weight/reference'
    cache.mkdir(parents=True, exist_ok=True)
    original = cache / 'abc467_g_editorial.cpp'
    if not original.exists():
        with urllib.request.urlopen(REFERENCE, timeout=60) as response:
            blocks = CodeBlocks(response.read().decode()).blocks
        source = next(code for code in blocks if '#include<bits/stdc++.h>' in code)
        original.write_text(source)
    source = original.read_text()
    assert '\nmain()' in source
    normalized = source.replace('\nmain()', '\nint main()')
    flags = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE', '-I.']
    clangflags = ['-std=gnu++2b', '-O2', '-I.']
    files = FILES + [DRIVER, 'components/fast_io.hpp', 'test/benchmark/abc467_g_cases.py', 'test/benchmark/weighted_wavelet_matrix.py']
    frozen = {path: sha(root / path) for path in files}
    report = dict(started_at=datetime.now(timezone.utc).isoformat(), reference_url=REFERENCE,
                  original_reference_sha256=sha(original), normalization='Only main() -> int main().',
                  reference_sha256=hashlib.sha256(normalized.encode()).hexdigest(),
                  source_sha256=frozen, flags=flags, clang_flags=clangflags,
                  gcc=subprocess.check_output(['g++', '--version'], text=True),
                  clang=subprocess.check_output(['clang++', '--version'], text=True),
                  cpu=subprocess.check_output(['lscpu'], text=True),
                  kernel=subprocess.check_output(['uname', '-a'], text=True), affinity=[args.cpu],
                  trials=args.trials, reference_trials=args.reference_trials, warmup=1, order_seed=20261019,
                  timer='Fresh process, input parsing, registration, build, updates, queries and output to Linux temporary file; copying inputs excluded.',
                  correctness='Exact byte equality with independent editorial square-root decomposition; no official answer files provided. Clang generic checked separately.',
                  results=[], completed=False)
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def save():
        temporary = args.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(report, indent=2) + '\n')
        temporary.replace(args.output)

    save()
    rng = random.Random(20261019)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock, tempfile.TemporaryDirectory(prefix='abc467-g-') as directory:
        work = Path(directory)
        ref = work / 'reference.cpp'
        ref.write_text(normalized)
        programs = {'current': work / 'current', 'editorial': work / 'editorial'}
        generic = work / 'clang_generic'
        fcntl.flock(lock, fcntl.LOCK_EX)
        subprocess.run(['g++', *flags, DRIVER, '-o', programs['current']], check=True)
        subprocess.run(['g++', *flags, ref, '-o', programs['editorial']], check=True)
        subprocess.run(['clang++', *clangflags, DRIVER, '-o', generic], check=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        for case in cases:
            inp = work / 'input'
            inp.write_bytes(case.read_bytes())
            with inp.open() as stream:
                n, q = map(int, stream.readline().split())
            samples = {name: [] for name in programs}
            outputs, rss = set(), {}
            counts = dict(current=args.trials, editorial=args.reference_trials)
            fcntl.flock(lock, fcntl.LOCK_EX)
            for trial in range(-1, max(counts.values())):
                order = [name for name in programs if trial < counts[name]]
                rng.shuffle(order)
                for name in order:
                    out = work / 'output'
                    with inp.open('rb') as stream, out.open('wb') as destination:
                        start = time.perf_counter()
                        subprocess.run([programs[name]], stdin=stream, stdout=destination, check=True)
                        elapsed = (time.perf_counter() - start) * 1000
                    outputs.add(sha(out))
                    assert len(out.read_bytes().splitlines()) == q
                    if trial >= 0:
                        samples[name].append(elapsed)
            for name, binary in {**programs, 'clang_generic': generic}.items():
                memory, out = work / 'rss', work / 'output'
                with inp.open('rb') as stream, out.open('wb') as destination:
                    subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(memory), binary], stdin=stream, stdout=destination, check=True)
                rss[name] = int(memory.read_text())
                outputs.add(sha(out))
            fcntl.flock(lock, fcntl.LOCK_UN)
            assert len(outputs) == 1, (case.name, outputs)
            row = dict(case=case.name, n=n, q=q, input_sha256=sha(inp), output_sha256=outputs.pop(),
                       passed=True, samples_ms=samples,
                       median_ms={name: statistics.median(xs) for name, xs in samples.items()}, peak_rss_kib=rss)
            report['results'].append(row)
            save()
            print(case.name, row['median_ms'], rss, flush=True)
    for path, expected in frozen.items():
        assert sha(root / path) == expected, path
    report['completed'] = True
    report['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()
    print('All cases match the independent reference and Clang build', flush=True)


if __name__ == '__main__':
    main()
