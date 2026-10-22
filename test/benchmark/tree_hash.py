import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--problem', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    args = parser.parse_args()
    args.build_dir.mkdir(parents=True, exist_ok=True)
    problem = args.problem.resolve()
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT),
             '-fconstexpr-depth=1024']
    compilers = {}
    cases = []
    for cxx in ['g++', 'clang++']:
        limits = (['-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152'] if cxx == 'g++'
                  else ['-fconstexpr-steps=524288'])
        executable = args.build_dir / ('driver-' + cxx)
        subprocess.run([cxx, *flags, *limits, str(ROOT / 'test/rooted_tree_hash.test.cpp'), '-o', str(executable)], check=True)
        compilers[cxx] = subprocess.check_output([cxx, '--version'], text=True).splitlines()[0]
        for path in sorted((problem / 'in').glob('*.in')):
            with tempfile.TemporaryDirectory() as directory:
                output = Path(directory) / 'answer.txt'
                with path.open('rb') as source, output.open('wb') as destination:
                    subprocess.run([str(executable)], stdin=source, stdout=destination, check=True, timeout=15)
                check = subprocess.run([str(problem / 'checker'), str(path), str(output),
                                        str(problem / 'out' / (path.stem + '.out'))], capture_output=True)
                if check.returncode:
                    raise RuntimeError(check.stderr.decode())
            cases.append(dict(compiler=cxx, case=path.name, input_sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    executable = args.build_dir / 'benchmark'
    subprocess.run(['g++', *flags, '-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152',
                    str(ROOT / 'test/benchmark/tree_hash.cpp'), '-o', str(executable)], check=True)
    samples = {}
    output = subprocess.check_output(['taskset', '-c', '0', str(executable)], text=True)
    for row in output.splitlines():
        shape, n, operation, run, milliseconds = row.split('\t')
        samples.setdefault((shape, int(n), operation), []).append(float(milliseconds))
    paths = ['components/tree_hash_core.hpp', 'components/splitmix64.hpp', 'tree/rooted_tree_hash.hpp',
             'tree/tree_hash.hpp', 'test/rooted_tree_hash.test.cpp', 'test/unit/tree_hash.cpp',
             'test/benchmark/tree_hash.cpp', 'test/benchmark/tree_hash.py']
    report = dict(platform=platform.platform(), compilers=compilers, flags=flags, cpu_affinity=0,
                  cpu_model=next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                 if line.startswith('model name')),
                  sources={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
                  problem_commit=subprocess.check_output(['git', '-C', str(problem), 'rev-parse', 'HEAD'], text=True).strip(),
                  cases=cases, measurements=[dict(shape=s, n=n, operation=op, samples_ms=values,
                                                median_ms=statistics.median(values))
                                            for (s, n, op), values in samples.items()])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print('Public cases passed:', len(cases))
    for row in report['measurements']:
        print(row['shape'], row['operation'], f"{row['median_ms']:.3f} ms")


if __name__ == '__main__':
    main()
