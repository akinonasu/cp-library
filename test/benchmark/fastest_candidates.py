import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
PROBLEMS = {
    'range_chmin_chmax_add_range_sum': ('data_structure', 'segtree_beats'),
    'general_matching': ('graph', 'general_matching'),
    'stern_brocot_tree': ('number_theory', 'stern_brocot_tree'),
}
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def fetch(url):
    return urllib.request.urlopen(url, timeout=90).read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--refresh', action='store_true')
    parser.add_argument('--check-only', action='store_true')
    parser.add_argument('--problems', nargs='+', choices=PROBLEMS, default=list(PROBLEMS))
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    destination = ROOT / 'submissions'
    destination.mkdir(exist_ok=True)
    references = {}
    for problem in args.problems:
        path = args.cache / (problem + '.json')
        if args.refresh or not path.exists():
            query = urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=10))
            url = 'https://v3.api.judge.yosupo.jp/submissions?' + query
            listing = json.loads(fetch(url))
            sources = []
            for entry in listing['submissions'][:3]:
                submission = json.loads(fetch('https://v3.api.judge.yosupo.jp/submissions/' + str(entry['id'])))
                lang = entry['lang']
                extension = {'cpp': '.cpp', 'rust': '.rs'}[lang]
                source = args.cache / (str(entry['id']) + extension)
                source.write_bytes(submission['source'].encode())
                sources.append(dict(id=entry['id'], lang=lang, sha256=sha(source)))
            data = dict(retrieved_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                        url=url, problem=problem, listing=listing, sources=sources)
            path.write_text(json.dumps(data, indent=2) + '\n')
        references[problem] = json.loads(path.read_text())
        for source in references[problem]['sources']:
            extension = '.rs' if source.get('lang') == 'rust' else '.cpp'
            if sha(args.cache / (str(source['id']) + extension)) != source['sha256']:
                raise RuntimeError('Reference source hash mismatch')
    if args.refresh or not (args.cache / 'langs.toml').exists():
        (args.cache / 'langs.toml').write_bytes(fetch(
            'https://raw.githubusercontent.com/yosupo06/library-checker-judge/master/langs/langs.toml'))
        (args.cache / 'langs.json').write_bytes(fetch('https://v3.api.judge.yosupo.jp/langs'))
    flags = FLAGS if not args.check_only else ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror']
    environment = dict(date=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                       compiler=subprocess.check_output([args.cxx, '--version'], text=True), flags=flags,
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu],
                       mode='validation' if args.check_only else 'benchmark',
                       trials=1 if args.check_only else args.trials, warmup=0 if args.check_only else 1, seed=20261018,
                       official_commit=subprocess.check_output(['git', '-C', str(args.official), 'rev-parse', 'HEAD'], text=True).strip(),
                       method='Fresh process, regular files on Linux /tmp, launch/build/I/O/exit included; compilation and checker excluded',
                       references=references, langs=json.loads((args.cache / 'langs.json').read_text()),
                       langs_toml=(args.cache / 'langs.toml').read_text(),
                       script_sha256=sha(Path(__file__)), sources={})
    result = {}
    rng = random.Random(20261018)
    with tempfile.TemporaryDirectory(prefix='cp-fastest-', dir='/tmp') as temp:
        work = Path(temp)
        for problem in args.problems:
            category, driver = PROBLEMS[problem]
            bundled = subprocess.check_output(['oj-bundle', f'test/{driver}.test.cpp'], cwd=ROOT, stderr=subprocess.DEVNULL, text=True)
            path = destination / (problem + '.cpp')
            if path.exists() and path.read_text() != bundled and args.check_only:
                raise RuntimeError('Stale submission bundle: ' + problem)
            if not args.check_only:
                path.write_text(bundled)
            sources = {'library': path}
            if not args.check_only:
                sources.update({str(s['id']): args.cache / (str(s['id']) + ('.rs' if s.get('lang') == 'rust' else '.cpp'))
                                for s in references[problem]['sources']})
            binaries = {}
            for variant, source in sources.items():
                binary = work / (problem + '-' + variant)
                if source.suffix == '.rs':
                    compile_command = ['rustc', '--edition', '2024', '-C', 'opt-level=3', '-C', 'target-cpu=native', str(source), '-o', str(binary)]
                    environment['rustc'] = subprocess.check_output(['rustc', '--version'], text=True)
                else:
                    compile_command = [args.cxx, *flags, str(source), '-o', str(binary)]
                subprocess.run(compile_command, check=True)
                environment.setdefault('commands', {})[problem + '/' + variant] = compile_command
                binaries[variant] = binary
                environment['sources'][str(source.relative_to(ROOT)) if source.is_relative_to(ROOT) else str(source)] = sha(source)
            official = args.official / category / problem
            inputs = sorted((official / 'in').glob('*.in'))
            if not inputs:
                raise RuntimeError('Generate official cases first: ' + str(official))
            cases = {}
            for original in inputs:
                input_path = work / 'input'
                shutil.copyfile(original, input_path)
                answer = official / 'out' / (original.stem + '.out')
                entry = dict(input_sha256=sha(input_path), answer_sha256=sha(answer), variants={})
                samples = {variant: dict(wall_ms=[], cpu_ms=[], output_sha256=[]) for variant in sources}
                for trial in range(0 if args.check_only else -1, 1 if args.check_only else args.trials):
                    order = list(sources)
                    rng.shuffle(order)
                    for variant in order:
                        output = work / 'output'
                        with input_path.open('rb') as inp, output.open('wb') as out:
                            before = resource.getrusage(resource.RUSAGE_CHILDREN)
                            begin = time.perf_counter()
                            subprocess.run([str(binaries[variant])], stdin=inp, stdout=out, check=True)
                            wall = (time.perf_counter() - begin) * 1000
                            after = resource.getrusage(resource.RUSAGE_CHILDREN)
                            cpu = (after.ru_utime + after.ru_stime - before.ru_utime - before.ru_stime) * 1000
                        subprocess.run([str(official / 'checker'), str(input_path), str(output), str(answer)],
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                        if trial >= 0:
                            samples[variant]['wall_ms'].append(wall)
                            samples[variant]['cpu_ms'].append(cpu)
                            samples[variant]['output_sha256'].append(sha(output))
                for variant, values in samples.items():
                    entry['variants'][variant] = dict(**values, median_wall_ms=statistics.median(values['wall_ms']),
                                                       median_cpu_ms=statistics.median(values['cpu_ms']))
                cases[original.stem] = entry
                print(problem, original.stem, {k: round(v['median_wall_ms'], 3) for k, v in entry['variants'].items()}, flush=True)
            result[problem] = cases
            (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')


if __name__ == '__main__':
    main()
