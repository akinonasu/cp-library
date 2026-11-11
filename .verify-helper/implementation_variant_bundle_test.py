from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from atcoder_flags import constexpr_flags
from bundle_test import bundle


ROOT = Path(__file__).resolve().parents[1]
STEMS = ('convolution', 'cyclic_convolution', 'multiplicative_convolution',
         'multivariate_convolution', 'ordered_convolution', 'online_convolution',
         'max_min_convolution')


@unittest.skipUnless(shutil.which('oj-bundle') and shutil.which('g++'), 'C++ bundle tools are unavailable')
class ImplementationVariantBundleTest(unittest.TestCase):
    def test_mixed_implementations_keep_public_selection(self):
        usage = (ROOT / 'test/unit/convolution_light_coexist.cpp').read_text()
        usage = usage[usage.index('#include <cassert>'):]
        programs = {}
        for first_light in (False, True):
            source = ''
            for light in (first_light, not first_light):
                for stem in STEMS:
                    source += '#include "convolution/' + stem + ('_light' if light else '') + '.hpp"\n'
            programs['light_first' if first_light else 'fast_first'] = source + usage
        cross_usage = '''#include <cassert>
int main() {
    fps a{1, 2, 3};
    OnlineConvolution queue(3);
    assert(queue.push(a[0], 1) == mint(1));
    assert(queue.push(a[1], 2) == mint(4));
    assert(queue.push(a[2], 3) == mint(10));
}
'''
        programs['fast_fps_light_online'] = ('#include "fps/fps.hpp"\n'
                                             '#include "convolution/online_convolution_light.hpp"\n' + cross_usage)
        programs['light_fps_fast_online'] = ('#include "fps/fps_light.hpp"\n'
                                             '#include "convolution/online_convolution.hpp"\n' + cross_usage)
        self.check_programs(programs)

    def check_programs(self, programs):
        with tempfile.TemporaryDirectory(prefix='convolution-bundle-') as directory:
            source = Path(directory) / 'test.cpp'
            executable = Path(directory) / 'test'
            for name, program in programs.items():
                with self.subTest(order=name):
                    source.write_text(program)
                    generated = bundle(source)
                    source.write_bytes(generated)
                    subprocess.run(['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror',
                                    *constexpr_flags('g++'), str(source), '-o', str(executable)], check=True)
                    subprocess.run([str(executable)], check=True, timeout=30)


    def test_fps_public_headers(self):
        fast = ('fps', 'taylor_shift', 'bostan_mori', 'linear_recurrence', 'multipoint_eval',
                'interpolate', 'compose', 'power_projection', 'compositional_inverse',
                'polynomial_gcd', 'rational_fps_coefficients')
        light = ('fps_light', 'fps_light_algorithms', 'compose_light', 'power_projection_light',
                 'compositional_inverse_light', 'polynomial_gcd_light', 'rational_fps_coefficients_light')
        programs = {}
        for first_light in (False, True):
            source = ''
            for stems in ((light, fast) if first_light else (fast, light)):
                source += ''.join('#include "fps/' + stem + '.hpp"\n' for stem in stems)
            source += '#include "test/fixtures/fps_public_api.hpp"\n'
            source += 'static_assert(std::is_same_v<fps, light_fps> == ' + str(first_light).lower() + ');\n'
            programs[str(first_light)] = source + 'int main() { fps_public_api_test::check(); }\n'
        self.check_programs(programs)

    def test_combinatorics_public_headers(self):
        stems = ('comb', 'binomial_prefix_sum', 'binomial_prefix_sum_incremental')
        usage = '''#include <cassert>
#include <type_traits>
int main() {
    Comb c;
    assert(c.comb(10, 3) == mint(120) && comb(10, 3) == mint(120));
    assert(fact[5] == mint(120) && fact[20] * finv[20] == mint(1));
    BinomialPrefixSum sum(12);
    IncrementalBinomialPrefixSum incremental(12);
    assert(sum.query(5, 3) == mint(16));
    assert(incremental.query(6, 3) == mint(22));
    assert(incremental.query(5, 3) == mint(16));
    std::vector<std::pair<int, int>> queries{{5, 3}, {6, 3}, {0, 1}};
    assert((binomial_prefix_sum(queries) == std::vector<mint>{16, 22, 1}));
}
'''
        programs = {}
        for first_light in (False, True):
            source = '#define COMB_MAX 200\n'
            for light in (first_light, not first_light):
                source += ''.join('#include "math/' + stem + ('_light' if light else '') + '.hpp"\n'
                                  for stem in stems)
            source += '#include <type_traits>\n'
            for name in ('Comb', 'BinomialPrefixSum', 'IncrementalBinomialPrefixSum'):
                source += ('static_assert(std::is_same_v<' + name + '<>, light_combinatorics::' + name
                           + '<>> == ' + str(first_light).lower() + ');\n')
            programs[str(first_light)] = source + usage
        self.check_programs(programs)


if __name__ == '__main__':
    unittest.main()
