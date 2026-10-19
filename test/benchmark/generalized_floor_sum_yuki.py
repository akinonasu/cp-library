import hashlib
import json
from pathlib import Path
import random
import re
import subprocess
import tempfile

import yaml


MOD = 998244353


def direct(case):
    p, q, n, m, a, b = case
    return sum(pow(i, p, MOD) * pow((a * i + b) // m, q, MOD) for i in range(n + 1)) % MOD


def polynomial(case):
    p, q, n, m, a, b = case
    assert a % m == 0
    u, v = a // m, b // m
    values, total = [], 0
    degree = p + q + 1
    for i in range(degree + 1):
        total = (total + pow(i, p, MOD) * pow(u * i + v, q, MOD)) % MOD
        values.append(total)
    x = n % MOD
    if x <= degree:
        return values[x]
    result = 0
    for i, value in enumerate(values):
        numerator = denominator = 1
        for j in range(degree + 1):
            if i != j:
                numerator = numerator * (x - j) % MOD
                denominator = denominator * (i - j) % MOD
        result += value * numerator * pow(denominator, -1, MOD)
    return result % MOD


def main():
    root = Path(__file__).resolve().parents[2]
    output = root / 'test/benchmark/results/generalized_floor_sum_yuki'
    output.mkdir(parents=True, exist_ok=True)
    samples = root / '.verify-helper/cache/generalized-floor-sum-yuki2996/samples'
    if not list(samples.glob('*.in')):
        with (output / 'download.log').open('w') as stream:
            subprocess.run(['oj', 'download', '--directory', str(samples), 'https://yukicoder.me/problems/no/2996'],
                           cwd=root, stdout=stream, stderr=subprocess.STDOUT, check=True)
    fixtures = []
    for path in sorted(samples.glob('*.in')):
        fixtures.append((path.read_bytes(), list(map(int, path.with_suffix('.out').read_text().split()))))
    assert len(fixtures) == 2
    sample_count = len(fixtures)
    rng = random.Random(20261020)
    generated = []
    for p in range(3):
        for q in range(3):
            for n in range(6):
                for m in range(1, 6):
                    for a in [-5, -1, 0, 1, 5]:
                        for b in [-5, -1, 0, 1, 5]:
                            case = (p, q, n, m, a, b)
                            generated.append((case, direct(case)))
    exhaustive_count = len(generated)
    for p in range(11):
        for q in range(11):
            for trial in range(2):
                case = (p, q, rng.randrange(21), rng.randrange(1, 31), rng.randrange(-101, 102), rng.randrange(-101, 102))
                generated.append((case, direct(case)))
    for trial in range(500):
        case = (rng.randrange(3), rng.randrange(3), rng.randrange(51), rng.randrange(1, 10**9 + 1),
                rng.randrange(-10**9, 10**9 + 1), rng.randrange(-10**9, 10**9 + 1))
        generated.append((case, direct(case)))
    direct_count = len(generated)
    for p in range(11):
        for q in range(11):
            for n in [0, 1, MOD - 1, MOD, MOD + 1, 10**9]:
                m = rng.choice([1, 2, 7, 10**9])
                case = (p, q, n, m, rng.choice([-1, 0, 1]) * m, rng.choice([-10**9, -1, 0, 1, 10**9]))
                generated.append((case, polynomial(case)))
    for small in [True, False]:
        group = [(c, answer) for c, answer in generated if (c[0] <= 2 and c[1] <= 2) == small]
        limit = 5000 if small else 5
        for start in range(0, len(group), limit):
            batch = group[start:start + limit]
            data = (str(len(batch)) + '\n' + ''.join(' '.join(map(str, c)) + '\n' for c, _ in batch)).encode()
            fixtures.append((data, [answer for _, answer in batch]))
    source = 'test/generalized_floor_sum_yuki.test.cpp'
    document = (root / 'docs/generalized_floor_sum.md').read_text()
    assert yaml.safe_load(document.split('---', 2)[1])['documentation_of'] == '//math/floor_sum/generalized_floor_sum.hpp'
    workflow = yaml.safe_load((root / '.github/workflows/generalized-floor-sum-benchmark.yml').read_text())
    assert any(source in step.get('run', '') for step in workflow['jobs']['benchmark']['steps'])
    example = re.search(r'```cpp\n(.*?)\n```', document, re.S)[1].replace('"lib/', '"')
    example += '\nint main() { assert(value == 5 && weighted == 5 && signed_sum == -4); }\n'
    checks = []
    with tempfile.TemporaryDirectory(prefix='floor-yuki-check-') as temporary:
        for compiler, sanitize in [('g++', False), ('clang++', False), ('clang++', True)]:
            executable = str(Path(temporary) / 'driver')
            flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            if sanitize:
                flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
            subprocess.run([compiler, *flags, source, '-o', executable], cwd=root, check=True)
            for i, (data, expected) in enumerate(fixtures):
                actual = subprocess.check_output([executable], input=data, timeout=30)
                assert list(map(int, actual.split())) == expected, (compiler, sanitize, i)
            if not sanitize:
                bundle = subprocess.check_output(['oj-bundle', source], cwd=root, text=True, stderr=subprocess.DEVNULL)
                subprocess.run([compiler, *flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True, cwd=root, check=True)
                subprocess.run([compiler, *flags, '-x', 'c++', '-', '-o', executable], input=example, text=True, cwd=root, check=True)
                subprocess.run([executable], check=True)
            checks.append({'compiler': compiler, 'sanitize': sanitize, 'fixtures': len(fixtures), 'passed': True})
            print(compiler, 'ASan/UBSan' if sanitize else 'O2 and bundle', 'passed', flush=True)
    names = [source, 'math/floor_sum/generalized_floor_sum.hpp', 'math/mint.hpp', 'components/fast_io.hpp',
             'test/unit/generalized_floor_sum.cpp', 'docs/generalized_floor_sum.md', '.github/workflows/generalized-floor-sum-benchmark.yml']
    result = {'checks': checks, 'samples': sample_count, 'exhaustive': exhaustive_count,
              'direct': direct_count, 'interpolated': len(generated) - direct_count, 'generated': len(generated),
              'seed': 20261020, 'official_system_tests': 'not run locally; registered for CI', 'docs_and_workflow': 'passed',
              'inputs_sha256': hashlib.sha256(b''.join(data for data, _ in fixtures)).hexdigest(),
              'sources': {n: hashlib.sha256((root / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for n in names},
              'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']}}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
