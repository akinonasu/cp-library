import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
API = 'https://v3.api.judge.yosupo.jp'
PROBLEMS = {
    'dynamic_tree_vertex_add_path_sum': ('sum', [270626, 405115, 278244, 332878]),
    'dynamic_tree_vertex_set_path_composite': ('composite', [270645, 278247, 405116, 334740]),
}


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def cached_json(path, url):
    if path.exists():
        return json.loads(path.read_text())
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
    with urllib.request.urlopen(request, timeout=60) as response:
        data = json.load(response)
    path.write_text(json.dumps(data, indent=2) + '\n')
    return data


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--source-dir', type=Path, required=True)
    parser.add_argument('--data-root', type=Path)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    args.source_dir.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    if hasattr(os, 'sched_setaffinity'):
        os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra']
    metadata = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'api': API,
                'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
                'flags': flags, 'affinity': [args.cpu], 'trials': args.trials,
                'input_storage': 'Copied into the temporary directory before timing',
                'problems': {}, 'cases': {}, 'implementation_sha256': {
                    str(p): sha256((ROOT / p).read_bytes()) for p in [
                        'data_structure/link_cut_tree.hpp', 'math/mint.hpp',
                        'test/benchmark/link_cut_tree_baseline.hpp',
                        'test/link_cut_tree_sum.test.cpp', 'test/link_cut_tree_composite.test.cpp']}}
    for key, command in [('uname', ['uname', '-a']), ('cpu', ['lscpu'])]:
        metadata[key] = subprocess.check_output(command, text=True)
    for problem, (_, submissions) in PROBLEMS.items():
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 4})
        listing = cached_json(args.source_dir / (problem + '-list.json'), API + '/submissions?' + query)
        records = []
        for submission in submissions:
            detail = cached_json(args.source_dir / f'{submission}.json', API + f'/submissions/{submission}')
            source = detail['source'].encode()
            (args.source_dir / f'{submission}.cpp').write_bytes(source)
            records.append({**detail['overview'], 'source_sha256': sha256(source),
                            'url': f'https://judge.yosupo.jp/submission/{submission}'})
        metadata['problems'][problem] = {'fastest_listing': listing, 'references': records}
    (args.output / 'references.json').write_text(json.dumps(metadata, indent=2) + '\n')
    if args.data_root is None:
        return
    rows = []
    summary = []
    order_random = random.Random(92647)
    with tempfile.TemporaryDirectory(prefix='cp-lct-reference-') as temporary:
        directory = Path(temporary)
        for problem, (kind, submissions) in PROBLEMS.items():
            binaries = {}
            before = directory / kind
            for subdirectory in ['test', 'data_structure', 'math']:
                (before / subdirectory).mkdir(parents=True, exist_ok=True)
            driver = f'test/link_cut_tree_{kind}.test.cpp'
            shutil.copyfile(ROOT / driver, before / driver)
            shutil.copyfile(ROOT / 'math/mint.hpp', before / 'math/mint.hpp')
            shutil.copyfile(ROOT / 'test/benchmark/link_cut_tree_baseline.hpp',
                            before / 'data_structure/link_cut_tree.hpp')
            for name, source in [('baseline', before / driver), ('current', ROOT / driver),
                                 ('fastest', args.source_dir / f'{submissions[0]}.cpp')]:
                binary = directory / (kind + '_' + name)
                subprocess.run([args.cxx, *flags, str(source), '-o', str(binary)], check=True)
                binaries[name] = binary
            data = args.data_root / problem
            cached = ROOT / '.verify-helper/cache' / hashlib.md5(
                ('https://judge.yosupo.jp/problem/' + problem).encode()).hexdigest() / 'test'
            if (cached / 'almost_line_00.in').exists():
                data = cached
            if not (data / 'almost_line_00.in').exists():
                subprocess.run(['oj', 'download', '-s', '-d', str(data),
                                'https://judge.yosupo.jp/problem/' + problem], check=True)
            metadata['cases'][problem] = {}
            for case in ['almost_line_00', 'almost_line_01', 'max_random_00']:
                input_path = directory / (case + '.in')
                shutil.copyfile(data / input_path.name, input_path)
                expected = (data / (case + '.out')).read_bytes().split()
                with input_path.open() as source:
                    dimensions = source.readline().strip()
                metadata['cases'][problem][case] = {'input_sha256': sha256(input_path.read_bytes()),
                                                    'output_sha256': sha256(b'\n'.join(expected)),
                                                    'n_q': dimensions}
                samples = {name: [] for name in binaries}
                for trial in range(args.trials):
                    order = list(binaries)
                    order_random.shuffle(order)
                    for name in order:
                        output = directory / 'answer.txt'
                        with input_path.open('rb') as source, output.open('wb') as sink:
                            begin = time.perf_counter()
                            subprocess.run([str(binaries[name])], stdin=source, stdout=sink, check=True)
                            elapsed = time.perf_counter() - begin
                        if output.read_bytes().split() != expected:
                            raise RuntimeError(f'answer mismatch: {problem} {case} {name}')
                        row = {'problem': problem, 'case': case, 'variant': name,
                               'trial': trial, 'seconds': elapsed}
                        rows.append(row)
                        samples[name].append(elapsed)
                for name in binaries:
                    median = statistics.median(samples[name])
                    summary.append({'problem': problem, 'case': case, 'variant': name,
                                    'seconds': median,
                                    'speedup': statistics.median(samples['baseline']) / median})
                print(f'{problem} {case}: all outputs match official answers', flush=True)
                for filename, values in [('samples.csv', rows), ('summary.csv', summary)]:
                    with (args.output / filename).open('w', newline='') as file:
                        writer = csv.DictWriter(file, fieldnames=list(values[0]))
                        writer.writeheader()
                        writer.writerows(values)
        (args.output / 'references.json').write_text(json.dumps(metadata, indent=2) + '\n')


if __name__ == '__main__':
    main()
