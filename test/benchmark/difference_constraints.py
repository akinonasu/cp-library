import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cases', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    args = parser.parse_args()
    args.build_dir.mkdir(parents=True, exist_ok=True)
    inputs = sorted(args.cases.glob('*.in'))
    if not inputs:
        raise RuntimeError('AOJ test cases are missing')
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT),
             '-fconstexpr-depth=1024']
    limits = {'g++': ['-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152'],
              'clang++': ['-fconstexpr-steps=524288']}
    compilers, cases = {}, []
    for cxx in ['g++', 'clang++']:
        executable = args.build_dir / ('driver-' + cxx)
        subprocess.run([cxx, *flags, *limits[cxx], str(ROOT / 'test/difference_constraints.test.cpp'),
                        '-o', str(executable)], check=True)
        compilers[cxx] = subprocess.check_output([cxx, '--version'], text=True).splitlines()[0]
        for path in inputs:
            with path.open('rb') as source:
                output = subprocess.check_output([str(executable)], stdin=source, timeout=15)
            expected = path.with_suffix('.out').read_bytes()
            if output.split() != expected.split():
                raise RuntimeError('Wrong answer: ' + path.name)
            cases.append(dict(compiler=cxx, case=path.name, input_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                              output_sha256=hashlib.sha256(expected).hexdigest()))
    executable = args.build_dir / 'benchmark'
    subprocess.run(['g++', *flags, *limits['g++'], str(ROOT / 'test/benchmark/difference_constraints.cpp'),
                    '-o', str(executable)], check=True)
    samples = {}
    output = subprocess.check_output(['taskset', '-c', '0', str(executable)], text=True)
    for row in output.splitlines():
        name, n, m, mode, operation, run, milliseconds = row.split('\t')
        samples.setdefault((name, int(n), int(m), mode, operation), []).append(float(milliseconds))
    paths = ['graph/difference_constraints.hpp', 'graph/bellman_ford.hpp', 'graph/dijkstra.hpp',
             'test/difference_constraints.test.cpp', 'test/unit/difference_constraints.cpp',
             'test/benchmark/difference_constraints.cpp', 'test/benchmark/difference_constraints.py']
    report = dict(platform=platform.platform(), compilers=compilers, flags=flags, constexpr_limits=limits, cpu_affinity=0,
                  random_seed=20261022,
                  cpu_model=next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                 if line.startswith('model name')),
                  sources={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
                  cases=cases, measurements=[dict(name=s, n=n, m=m, mode=mode, operation=op, samples_ms=values,
                                                median_ms=statistics.median(values))
                                            for (s, n, m, mode, op), values in samples.items()])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print('Public cases passed:', len(cases))
    for row in report['measurements']:
        print(row['name'], row['mode'], row['operation'], f"{row['median_ms']:.3f} ms")


if __name__ == '__main__':
    main()
