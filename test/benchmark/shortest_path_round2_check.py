from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
from pathlib import Path
import platform
import re
import subprocess


ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / '.verify-helper/cache/shortest-path/round2/check'
TREE = WORK / 'source'
RESULT = ROOT / 'test/benchmark/results/shortest_path_round2_validation.json'
PROBLEM = Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
UNITS = ['dijkstra', 'dijkstra_csr', 'dijkstra_topological', 'dijkstra_forced_path', 'standalone']
DRIVERS = ['dijkstra', 'dijkstra_path', 'dijkstra_topological']
FLAGS = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror']


def digest(data):
    return hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest()


def snapshot():
    pending = [f'test/unit/{name}.cpp' for name in UNITS]
    pending += [f'test/{name}.test.cpp' for name in [*DRIVERS, 'dijkstra_bigint_yuki']]
    pending += ['test/benchmark/flow_check.py']
    hashes = {}
    while pending:
        name = pending.pop()
        if name in hashes:
            continue
        source = ROOT / name
        data = source.read_bytes().replace(b'\r\n', b'\n')
        hashes[name] = digest(data)
        destination = TREE / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
        for include in re.findall(r'^\s*#\s*include\s+"([^"]+)"', data.decode(), re.M):
            for candidate in [source.parent / include, ROOT / include]:
                if candidate.is_file():
                    pending.append(candidate.resolve().relative_to(ROOT).as_posix())
                    break
    return dict(sorted(hashes.items()))


@contextmanager
def exclusive():
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)


def recorded(command):
    return [str(arg).replace(str(ROOT), '<root>') for arg in command]


def main():
    copied = snapshot()
    WORK.mkdir(parents=True, exist_ok=True)
    cases = sorted((PROBLEM / 'in').glob('*.in'))
    assert len(cases) == 29
    result = dict(recorded_at=datetime.now(timezone.utc).isoformat(), platform=platform.platform(),
                  compilers={cxx: subprocess.check_output([cxx, '--version'], text=True).splitlines()[0]
                             for cxx in ['g++', 'clang++']},
                  source_sha256=copied, commands=[], checks={}, cases=[path.stem for path in cases],
                  checker_sha256=digest((PROBLEM / 'checker').read_bytes()),
                  problem_revision=subprocess.check_output(['git', '-C', str(PROBLEM.parents[1]), 'rev-parse', 'HEAD'], text=True).strip())

    def save():
        RESULT.write_text(json.dumps(result, indent=2) + '\n')

    for cxx in ['g++', 'clang++']:
        for unit in UNITS:
            binary = WORK / (cxx + '-' + unit)
            command = [cxx, *FLAGS, '-I', str(TREE), str(TREE / f'test/unit/{unit}.cpp'), '-o', str(binary)]
            with exclusive():
                subprocess.run(command, check=True)
                subprocess.run([binary], check=True, timeout=120)
            result['commands'].append(dict(compile=recorded(command), run=recorded([binary])))
            result['checks'][cxx + '-' + unit] = 'passed'
            save()
            print(cxx, unit, 'passed', flush=True)
    for unit in UNITS[:-1]:
        binary = WORK / ('sanitize-' + unit)
        command = ['g++', '-std=c++2b', '-O1', '-g', '-fsanitize=address,undefined',
                   '-fno-omit-frame-pointer', '-I', str(TREE), str(TREE / f'test/unit/{unit}.cpp'), '-o', str(binary)]
        with exclusive():
            subprocess.run(command, check=True)
            subprocess.run([binary], check=True, timeout=120)
        result['commands'].append(dict(compile=recorded(command), run=recorded([binary])))
        result['checks']['sanitize-' + unit] = 'passed'
        save()
        print('sanitize', unit, 'passed', flush=True)
    for cxx in ['g++', 'clang++']:
        for driver in DRIVERS:
            binary = WORK / (cxx + '-driver-' + driver)
            command = [cxx, *FLAGS, '-I', str(TREE), str(TREE / f'test/{driver}.test.cpp'), '-o', str(binary)]
            output = WORK / 'output.txt'
            with exclusive():
                subprocess.run(command, check=True)
                for inp in cases:
                    with inp.open('rb') as src, output.open('wb') as dst:
                        subprocess.run([binary], stdin=src, stdout=dst, check=True, timeout=120)
                    subprocess.run([PROBLEM / 'checker', inp, output, PROBLEM / 'out' / (inp.stem + '.out')],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=True)
            result['commands'].append(dict(compile=recorded(command), run=recorded([binary]),
                                            checker=recorded([PROBLEM / 'checker', '<input>', output, '<answer>'])))
            result['checks'][cxx + '-judge-' + driver] = dict(result='passed', cases=len(cases))
            save()
            print(cxx, driver, len(cases), 'checker passed', flush=True)
    command = ['python3', str(TREE / 'test/benchmark/flow_check.py'), '--samples',
               str(ROOT / '.verify-helper/cache/flow-samples'), '--only', 'dijkstra_bigint_yuki']
    with exclusive():
        completed = subprocess.run(command, capture_output=True, text=True, check=True)
    for cxx in ['g++', 'clang++']:
        assert f'{cxx} dijkstra_bigint_yuki: 2 samples passed; 60 Fraction oracle cases passed' in completed.stdout
    print(completed.stdout, end='', flush=True)
    result['commands'].append(dict(run=recorded(command)))
    result['checks']['boost'] = dict(result='passed', samples=2, random=60, compilers=2)
    result['sources_unchanged_during_validation'] = copied == {name: digest((ROOT / name).read_bytes()) for name in copied}
    assert result['sources_unchanged_during_validation']
    result['result'] = 'passed'
    result['completed_at'] = datetime.now(timezone.utc).isoformat()
    save()


if __name__ == '__main__':
    main()
