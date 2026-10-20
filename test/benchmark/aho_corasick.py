import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time
import urllib.parse
import urllib.request


ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/aho-corasick'
OUTPUT = ROOT / 'test/benchmark/results/aho_corasick'


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def references():
    def get(url):
        with urllib.request.urlopen(url, timeout=60) as response:
            return json.load(response)
    url = 'https://v3.api.judge.yosupo.jp/submissions?' + urllib.parse.urlencode(
        dict(problem='aho_corasick', status='AC', order='+time', limit=20))
    listing = get(url)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing_url=url, listing=listing, references=[])
    for row in [r for r in listing['submissions'] if r['lang'].startswith('cpp')][:3]:
        detail = get('https://v3.api.judge.yosupo.jp/submissions/' + str(row['id']))
        code = detail['source'].encode()
        (CACHE / f"{row['id']}.cpp").write_bytes(code)
        result['references'].append(dict(**detail['overview'], url=f"https://judge.yosupo.jp/submission/{row['id']}",
                                         source_sha256=hashlib.sha256(code).hexdigest()))
    (OUTPUT / 'references.json').write_text(json.dumps(result, indent=2) + '\n')


def workloads():
    rng = random.Random(20261020)
    random_word = lambda length, alphabet: ''.join(chr(97 + rng.randrange(alphabet)) for _ in range(length))
    yield 'random26', [random_word(30, 26) for _ in range(20000)], random_word(2000000, 26)
    yield 'random2', [random_word(30, 2) for _ in range(20000)], random_word(2000000, 2)
    yield 'long_pattern', ['a' * 600000], 'a' * 1999999 + 'b'
    yield 'suffix_chain', ['a' * length for length in range(1, 1001)], 'a' * 2000000
    yield 'shared_prefix', ['a' * 15000 + random_word(30, 26) for _ in range(40)], 'a' * 2000000
    yield 'duplicates', ['abacaba'] * 80000, 'abacaba' * 285715


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', action='store_true')
    parser.add_argument('--problems', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems')
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--skip-reference-validation', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    CACHE.mkdir(parents=True, exist_ok=True)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    if args.references:
        references()
        return
    assert args.trials > 0
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero', '-DONLINE_JUDGE', '-I', str(ROOT)]
    binaries = {}
    for name, compiler, source in [('library', 'g++', ROOT / 'test/aho_corasick.test.cpp'),
                                    ('clang', 'clang++', ROOT / 'test/aho_corasick.test.cpp'),
                                    ('307744', 'g++', CACHE / '307744.cpp')]:
        binary = CACHE / name
        subprocess.run([compiler, *flags, str(source), '-o', str(binary)], check=True)
        binaries[name] = binary
    problem = args.problems / 'string/aho_corasick'
    cases = sorted((problem / 'in').glob('*.in'))
    assert len(cases) == 72
    report = dict(compilers={c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']},
                  flags=flags, cpu=subprocess.check_output(['lscpu'], text=True),
                  kernel=subprocess.check_output(['uname', '-a'], text=True), affinity=[args.cpu], trials=args.trials,
                  problem_commit=subprocess.check_output(['git', '-C', str(args.problems), 'rev-parse', 'HEAD'], text=True).strip(),
                  source_sha256={p: digest(ROOT / p) for p in ['string/aho_corasick.hpp', 'test/aho_corasick.test.cpp',
                                  'test/unit/aho_corasick.cpp', 'test/benchmark/aho_corasick.cpp', 'test/benchmark/aho_corasick.py']},
                  reference_source_sha256=digest(CACHE / '307744.cpp'),
                  judge=[], end_to_end={}, api={}, complete=False)

    def save():
        (OUTPUT / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')

    actual = CACHE / 'actual.out'
    for path in cases:
        passed = []
        for name, binary in binaries.items():
            if name == '307744' and args.skip_reference_validation:
                continue
            with path.open('rb') as stream, actual.open('wb') as output:
                subprocess.run([str(binary)], stdin=stream, stdout=output, stderr=subprocess.DEVNULL, check=True, timeout=30)
            subprocess.run([str(problem / 'checker'), str(path), str(problem / 'out' / (path.stem + '.out')), str(actual)],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            passed.append(name)
        report['judge'].append(dict(case=path.stem, input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), passed=passed))
    print('All 72 official cases passed for', report['judge'][0]['passed'], flush=True)
    save()
    rng = random.Random(20261021)
    for name in ['bfs_03', 'random_03', 'random_11', 'short_period_00', 'fibonacci_00', 'almost_single_00']:
        path = problem / 'in' / (name + '.in')
        samples = {mode: [] for mode in ['library', '307744']}
        for trial in range(args.trials + 1):
            order = list(samples)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    start = time.perf_counter_ns()
                    subprocess.run([str(binaries[mode])], stdin=stream, stdout=subprocess.DEVNULL,
                                   stderr=subprocess.DEVNULL, check=True)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                if trial:
                    samples[mode].append(elapsed)
        memory = {}
        for mode in samples:
            log = CACHE / (mode + '.rss')
            with path.open('rb') as stream:
                subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(log), str(binaries[mode])], stdin=stream,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            memory[mode] = int(log.read_text())
        report['end_to_end'][name] = dict(milliseconds=samples, median_ms={k: statistics.median(v) for k, v in samples.items()},
                                           peak_rss_kib=memory)
        print(name, report['end_to_end'][name]['median_ms'], flush=True)
        save()
    original = (ROOT / 'string/aho_corasick.hpp').read_text()
    sparse = original.replace('node.next[c] = fallback[c];', '(void)fallback;')
    sparse = sparse.replace('int f = v == root ? root : fallback[c];',
                            'int f = root;\n\t\t\t\t\tif (v != root) {\n\t\t\t\t\t\tf = node.link;\n\t\t\t\t\t\twhile (f && !nodes_[f].next[c]) f = nodes_[f].link;\n\t\t\t\t\t\tf = nodes_[f].next[c];\n\t\t\t\t\t}')
    sparse = sparse.replace('return x < Alphabet ? nodes_[v].next[x] : root;',
                            'if (x >= Alphabet) return root;\n\t\twhile (v && !nodes_[v].next[x]) v = nodes_[v].link;\n\t\treturn nodes_[v].next[x];')
    assert sparse != original and '(void)fallback;' in sparse and 'while (v &&' in sparse
    (CACHE / 'sparse.hpp').write_text(sparse)
    apis = {}
    for name, options in [('dense', []), ('sparse', ['-DAHO_HEADER="' + str(CACHE / 'sparse.hpp') + '"'])]:
        binary = CACHE / ('api_' + name)
        subprocess.run(['g++', *flags, *options, 'test/benchmark/aho_corasick.cpp', '-o', str(binary)], check=True)
        apis[name] = binary
    for name, patterns, text in workloads():
        path = CACHE / (name + '.in')
        path.write_text(str(len(patterns)) + '\n' + '\n'.join(patterns) + '\n' + text + '\n')
        samples = {mode: {'build_ms': [], 'scan_ms': []} for mode in apis}
        expected = None
        for trial in range(args.trials + 1):
            order = list(apis)
            rng.shuffle(order)
            for mode in order:
                with path.open('rb') as stream:
                    result = subprocess.check_output([str(apis[mode])], stdin=stream, text=True, timeout=30).split()
                if expected is None:
                    expected = result[2:]
                assert result[2:] == expected, (name, mode)
                if trial:
                    samples[mode]['build_ms'].append(float(result[0]))
                    samples[mode]['scan_ms'].append(float(result[1]))
        report['api'][name] = dict(patterns=len(patterns), total_length=sum(map(len, patterns)), text_length=len(text),
                                  input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), count_and_checksum=expected,
                                  samples=samples, median_ms={mode: {k: statistics.median(v) for k, v in row.items()} for mode, row in samples.items()})
        print(name, report['api'][name]['median_ms'], flush=True)
        save()
    for path, expected in report['source_sha256'].items():
        assert digest(ROOT / path) == expected, path
    report['complete'] = True
    save()


if __name__ == '__main__':
    main()
