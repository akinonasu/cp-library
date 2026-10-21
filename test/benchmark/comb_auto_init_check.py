import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=root / 'test/benchmark/results/comb_auto_init')
    output = parser.parse_args().output
    checks = []

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    common = '#define COMB_MAX 20\n#include "math/combinatorics/comb.hpp"\n#include <cstdlib>\n'
    primary = common + '''
mint* other_fact();
mint* other_finv();
const unsigned before_main = comb(10, 3).val();
int main() {
    if (before_main != 120 || fact[0] != mint(7)) std::abort();
    if (other_fact() != comb_detail::factorial || other_finv() != comb_detail::inverse_factorial) std::abort();
    if (finv[0] != mint(1) || comb(20, 10).val() != 184756) std::abort();
}
'''
    secondary = common + '''
const bool marked = [] {
    if (fact[0] != mint(1) || finv[0] != mint(1) || comb(10, 3).val() != 120) std::abort();
    comb_detail::factorial[0] = 7;
    return true;
}();
mint* other_fact() { return comb_detail::factorial; }
mint* other_finv() { return comb_detail::inverse_factorial; }
'''
    default = '''
#include "math/combinatorics/comb.hpp"
#include <cstdlib>
#include <type_traits>
static_assert(std::is_same_v<decltype(fact[0]), mint>);
int main() {
    unsigned long long expected = 1;
    for (int i = 0; i <= COMB_INF; ++i) {
        if (i) expected = expected * i % mint::mod();
        if (fact[i].val() != expected || fact[i] * finv[i] != mint(1)) std::abort();
    }
    if (comb(10, 3).val() != 120) std::abort();
}
'''
    with tempfile.TemporaryDirectory(prefix='comb-auto-check-') as temporary:
        temporary = Path(temporary)
        (temporary / 'primary.cpp').write_text(primary)
        (temporary / 'secondary.cpp').write_text(secondary)
        (temporary / 'default.cpp').write_text(default)
        example = re.search(r'```cpp\n(.*?)\n```', (root / 'docs/comb.md').read_text(), re.S)[1].replace('"lib/', '"')
        cases = sorted((root / '.verify-helper/cache/comb-factorials/samples/3172').glob('*.in'))
        assert cases, 'Download yukicoder 3172 samples first'
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            objects = []
            for name in ['primary', 'secondary']:
                obj = str(temporary / (name + '.o'))
                run([*flags, '-c', str(temporary / (name + '.cpp')), '-o', obj])
                objects.append(obj)
            executable = str(temporary / 'check')
            for order in [objects, objects[::-1]]:
                run([compiler, *order, '-o', executable]); run([executable])
            checks.append({'compiler': compiler, 'multiple_translation_units': 'passed in both link orders'})
            print(compiler, 'multiple translation units passed', flush=True)
            options = ['-DNDEBUG']
            if compiler == 'g++':
                options += ['-march=native', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE',
                            '-fconstexpr-depth=1024', '-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152']
            run([*flags, *options, str(temporary / 'default.cpp'), '-o', executable]); run([executable])
            checks[-1]['default_limit'] = 10000000
            checks[-1]['release_flags'] = options
            print(compiler, 'default limit and accessors passed', flush=True)
            for target in ['math/combinatorics/comb.hpp', 'test/comb.test.cpp']:
                bundle = subprocess.check_output(['oj-bundle', target], cwd=root, text=True, stderr=subprocess.DEVNULL)
                assert '#include "' not in bundle
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True)
            run([*flags, '-x', 'c++', '-', '-o', executable], input=example, text=True); run([executable])
            run([*flags, 'test/comb.test.cpp', '-o', executable])
            for path in cases:
                with path.open('rb') as stream:
                    actual = subprocess.check_output([executable], stdin=stream)
                assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            checks[-1].update(bundles='passed', example='passed', yukicoder_3172_samples=len(cases))
            print(compiler, 'bundles, example and', len(cases), 'samples passed', flush=True)
    names = ['math/combinatorics/comb.hpp', 'docs/comb.md', 'test/unit/comb.cpp', 'test/unit/comb_zero.cpp',
             'test/unit/comb_factorials.cpp', 'test/unit/standalone.cpp', 'test/comb.test.cpp']
    output.mkdir(parents=True, exist_ok=True)
    report = {'result': 'passed', 'checks': checks,
              'sources': {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names},
              'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']}}
    (output / 'validation.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
