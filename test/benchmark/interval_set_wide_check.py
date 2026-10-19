import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

import yaml

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--cases-dir', type=Path, required=True)
args = parser.parse_args()
output = root / 'test/benchmark/results'
cases = args.cases_dir.resolve()
flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-I', str(root)]


def run(command, **kwargs):
    return subprocess.run(command, cwd=root, check=True, **kwargs)


sources = ['data_structure/interval_set.hpp', 'docs/interval_set.md', 'test/unit/interval_set.cpp',
           'test/unit/interval_set_wide.cpp', 'test/unit/standalone.cpp', 'test/interval_set_predecessor.test.cpp',
           'components/fast_io.hpp']
result = {'sources': {p: hashlib.sha256((root / p).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for p in sources},
          'system': subprocess.check_output(['uname', '-a'], text=True).strip(),
          'compilers': {}, 'checks': []}
with tempfile.TemporaryDirectory(prefix='interval-set-wide-') as temporary:
    temporary = Path(temporary)
    for compiler in ['g++', 'clang++']:
        result['compilers'][compiler] = subprocess.check_output([compiler, '--version'], text=True)
        executable = temporary / compiler
        print(compiler, 'updated wide integer tests', flush=True)
        run([compiler, *flags, '-Werror', 'test/unit/interval_set_wide.cpp', '-o', str(executable)])
        run([str(executable)], timeout=120)
        result['checks'].append({'compiler': compiler, 'check': 'wide unit', 'flags': flags + ['-Werror']})
        if compiler == 'clang++':
            sanitize = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
            print(compiler, 'updated wide ASan/UBSan tests', flush=True)
            run([compiler, *flags, *sanitize, 'test/unit/interval_set_wide.cpp', '-o', str(executable)])
            run([str(executable)], timeout=120)
            result['checks'].append({'compiler': compiler, 'check': 'wide ASan/UBSan', 'flags': flags + sanitize})
        print(compiler, 'Predecessor Problem', flush=True)
        run([compiler, *flags, '-DNDEBUG', 'test/interval_set_predecessor.test.cpp', '-o', str(executable)])
        count = 0
        for path in sorted(cases.glob('*.in')):
            with path.open('rb') as stream:
                actual = subprocess.check_output([str(executable)], stdin=stream, timeout=30).split()
            assert actual == path.with_suffix('.out').read_bytes().split(), path
            count += 1
        assert count, 'No official cases found'
        result['checks'].append({'compiler': compiler, 'check': 'Predecessor Problem', 'cases': count,
                                'flags': flags + ['-DNDEBUG']})
        print(compiler, count, 'official cases passed', flush=True)
    documentation = (root / 'docs/interval_set.md').read_text()
    assert yaml.safe_load(documentation.split('---', 2)[1])['documentation_of'] == '//data_structure/interval_set.hpp'
    rows = [row for row in documentation.splitlines() if row.startswith('|')]
    assert rows and all(len(row.split('|')) == 5 for row in rows)
    code = subprocess.check_output(['oj-bundle', 'data_structure/interval_set.hpp'], cwd=root, text=True, stderr=subprocess.DEVNULL)
    assert '#include "' not in code
    code += '''
#include <boost/multiprecision/cpp_int.hpp>
int main() {
    IntervalSet<__int128_t> a;
    __int128_t x = __int128_t(1) << 100;
    a.insert(-x, x);
    assert(a.size() == __uint128_t(2) * x);
    using boost::multiprecision::cpp_int;
    IntervalSet<cpp_int> b;
    cpp_int y = cpp_int(1) << 1000;
    b.insert(-y, y);
    assert(b.size() == 2 * y);
}
'''
    source = temporary / 'bundle.cpp'
    source.write_text(code)
    for compiler in ['g++', 'clang++']:
        run([compiler, *flags, '-Werror', str(source), '-o', str(executable)])
        run([str(executable)], timeout=120)
        result['checks'].append({'compiler': compiler, 'check': 'bundle, Boost included after header'})
    result['documentation'] = 'API table, source mapping and bundled code passed'
result['result'] = 'passed'
(output / 'interval_set_wide_validation.json').write_text(json.dumps(result, indent=2) + '\n')
print('All additional checks passed', flush=True)
