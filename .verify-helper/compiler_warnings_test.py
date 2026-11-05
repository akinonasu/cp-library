from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile
import unittest

from atcoder_flags import constexpr_flags


ROOT = Path(__file__).resolve().parent.parent
PREFIX = '''#include "math/mint.hpp"
#include "math/modint.hpp"
#include "math/fraction.hpp"
#include "math/bigint.hpp"
#include "math/matrix.hpp"
#include "math/matrix_light.hpp"
#include "fps/fps.hpp"
#include "fps/fps_light.hpp"
#include "data_structure/dynamic_bitset.hpp"
#include "data_structure/segtree.hpp"
#include "data_structure/lazy_segtree.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "data_structure/union_find.hpp"
#include "tree/tree_center.hpp"
#include "tree/tree_diameter.hpp"
#include "components/fast_io.hpp"
#include <string_view>
long long combine(long long a, long long b) { return a + b; }
long long zero() { return 0; }
long long scale(long long a, long long b) { return a * b; }
long long one() { return 1; }
using Segment = Segtree<long long, combine, zero>;
using Lazy = LazySegtree<long long, combine, zero, long long, scale, scale, one>;
'''

CASES = [
    ('mint', 'mint a = 2;', 'a.pow(3)'),
    ('mint_inverse', 'mint a = 2;', 'a.inv()'),
    ('mint_value', 'mint a = 2;', 'a.val()'),
    ('mint_add', 'mint a = 2;', 'a + 1'),
    ('mint_left', 'mint a = 2;', '1 + a'),
    ('static_modint', 'StaticModint<17> a = 2;', 'a.pow(3)'),
    ('dynamic_modint', 'DynamicModint<> a = 2;', 'a.inv()'),
    ('fraction', 'fraction a(2, 3);', 'a.inv()'),
    ('fraction_floor', 'Fraction<int> a(2, 3);', 'floor(a)'),
    ('fraction_sum', 'fraction a(2, 3);', 'a + 1'),
    ('bigint', 'BigInt a = 2;', 'a + 1'),
    ('bigint_divmod', 'BigInt a = 2;', 'divmod(a, BigInt(1))'),
    ('bigint_sign', 'BigInt a = 2;', 'a.sign()'),
    ('fps_inverse', 'fps a{1, 1};', 'a.inv(3)'),
    ('fps_log', 'fps a{1, 1};', 'a.log(3)'),
    ('fps_exp', 'fps a{0, 1};', 'a.exp(3)'),
    ('fps_eval', 'fps a{1, 1};', 'a.eval(2)'),
    ('fps_product', 'fps a{1, 1};', 'a * a'),
    ('fps_divmod', 'fps a{1, 1};', 'a.divmod(a)'),
    ('light_fps_inverse', 'LightFormalPowerSeries<> a{1, 1};', 'a.inv(3)'),
    ('light_fps_eval', 'LightFormalPowerSeries<> a{1, 1};', 'a.eval(2)'),
    ('light_fps_product', 'LightFormalPowerSeries<> a{1, 1};', 'a * a'),
    ('matrix_transpose', 'Matrix<mint> a(2);', 'a.transpose()'),
    ('matrix_inverse', 'Matrix<mint> a(2);', 'a.inverse()'),
    ('matrix_det', 'Matrix<mint> a(2);', 'a.det()'),
    ('matrix_data', 'Matrix<mint> a(2);', 'a(0, 0)'),
    ('light_matrix', 'LightMatrix<mint> a(2);', 'a.transpose()'),
    ('bits_count', 'DynamicBitset a(2);', 'a.count()'),
    ('bits_any', 'DynamicBitset a(2);', 'a.any()'),
    ('bits_proxy', 'DynamicBitset a(2);', 'a[0]'),
    ('bits_slice', 'DynamicBitset a(2);', 'a.slice(0, 1)'),
    ('bits_shift', 'DynamicBitset a(2);', 'a << 1'),
    ('segment_product', 'Segment a(2);', 'a.prod(0, 2)'),
    ('segment_get', 'Segment a(2);', 'a.get(0)'),
    ('segment_search', 'Segment a(2);', 'a.max_right(0, [](auto x) { return x < 9; })'),
    ('lazy_product', 'Lazy a(2);', 'a.prod(0, 2)'),
    ('lazy_search', 'Lazy a(2);', 'a.min_left(2, [](auto x) { return x < 9; })'),
    ('fenwick_sum', 'FenwickTree<int> a(2);', 'a.sum(0, 2)'),
    ('union_same', 'UnionFind a(2);', 'a.same(0, 1)'),
    ('union_leader', 'UnionFind a(2);', 'a.leader(0)'),
    ('union_groups', 'UnionFind a(2);', 'a.groups()'),
    ('center', 'std::vector<std::vector<int>> a(1);', 'tree_center(a)'),
    ('diameter', 'TreeDiameter<> a(1);', 'a.solve()'),
]
VALUE_TYPES = ('mint', 'fps_mint', 'StaticModint<17>', 'DynamicModint<>',
               'fraction', 'Fraction<int>', 'BigInt', 'fps', 'LightFormalPowerSeries<>',
               'Matrix<mint>', 'LightMatrix<mint>', 'DynamicBitset')


class CompilerWarnings(unittest.TestCase):
    def test_unused_values_and_results(self):
        compilers = [cxx for cxx in ('g++', 'clang++') if shutil.which(cxx)]
        if not compilers:
            self.skipTest('C++ compiler not installed')
        with tempfile.TemporaryDirectory(prefix='compiler-warnings-') as directory:
            path = Path(directory) / 'test.cpp'
            for mode in ('discard', 'explicit_discard', 'updates'):
                lines = PREFIX.splitlines()
                expected = set()
                if mode != 'updates':
                    for name, setup, expression in CASES:
                        lines += [f'void check_{name}() {{', setup]
                        expected.add(len(lines) + 1)
                        lines += [f'{expression};' if mode == 'discard' else f'(void)({expression});', '}']
                    for i, value_type in enumerate(VALUE_TYPES):
                        lines += [f'void variable_{i}() {{']
                        expected.add(len(lines) + 1)
                        lines += [('' if mode == 'discard' else '[[maybe_unused]] ')
                                  + f'{value_type} value;', '}']
                else:
                    lines += ['''void updates() {
mint a = 2; a += 1; a *= 2; a++; ++a;
StaticModint<17> b = 2; b += 1; b++; ++b;
DynamicModint<> c = 2; c += 1; c++; ++c;
fraction d(2, 3); d += 1; d++; ++d;
BigInt e = 2; e += 1; e++; ++e;
fps f{1, 1}; f += f; f.resize(3);
LightFormalPowerSeries<> g{1, 1}; g += g; g.trim();
Matrix<mint> h(2); h += h; h(0, 0) = 1;
DynamicBitset bits(2); bits.set(0); bits[1] = true; bits.flip(0); bits <<= 1;
Segment segment(2); segment.set(0, 1);
Lazy lazy(2); lazy.apply(0, 2, 2);
FenwickTree<int> fenwick(2); fenwick.add(0, 1);
UnionFind uf(2); uf.merge(0, 1);
TreeDiameter<> tree(2); tree.add_edge(0, 1);
FastInput input(std::string_view("5")); int n = 0; input.read(n);
FastOutput output;
}''']
                path.write_text('\n'.join(lines) + '\n')
                for compiler in compilers:
                    with self.subTest(compiler=compiler, mode=mode):
                        flags = ['-std=c++23', '-Wall', '-Wextra', '-Werror', '-fsyntax-only',
                                 '-fdiagnostics-color=never', *constexpr_flags(compiler)]
                        version = subprocess.check_output([compiler, '--version'], text=True).lower()
                        if 'clang' in version:
                            flags.append('-ferror-limit=0')
                        result = subprocess.run([compiler, *flags, '-I', str(ROOT), str(path)],
                                                capture_output=True, text=True,
                                                env={**os.environ, 'LC_ALL': 'C'})
                        if mode == 'discard':
                            self.assertNotEqual(result.returncode, 0)
                            errors = {int(line) for line in re.findall(
                                re.escape(str(path)) + r':(\d+):\d+: error:', result.stderr)}
                            self.assertEqual(errors, expected, result.stderr)
                            self.assertIn('unused-result', result.stderr)
                            self.assertIn('unused-variable', result.stderr)
                        else:
                            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()
