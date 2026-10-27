from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

from size_profiles import LIGHTWEIGHT, LIMITS

ROOT = Path(__file__).resolve().parents[1]
LIMIT = LIMITS['full']
LIGHT_LIMIT = LIMITS[LIGHTWEIGHT]


def bundle(path):
    result = subprocess.run(['oj-bundle', '-I', str(ROOT), str(path)], cwd=ROOT,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode:
        raise AssertionError(result.stderr.decode(errors='replace'))
    return result.stdout


def light_headers():
    headers = set(ROOT.rglob('*_light.hpp'))
    for directory in ('fps', 'components'):
        headers.update((ROOT / directory).glob('fps_light*.hpp'))
    return sorted(path for path in headers if path.is_file())


def dependencies(path, seen=None):
    seen = set() if seen is None else seen
    path = path.resolve()
    if path in seen:
        return seen
    seen.add(path)
    for include in re.findall(r'^\s*#\s*include\s*"([^"\n]+)"', path.read_text(), re.MULTILINE):
        target = path.parent / include
        if not target.is_file():
            target = ROOT / include
        if not target.is_file():
            raise FileNotFoundError((path, include))
        dependencies(target, seen)
    return seen


class BundleTest(unittest.TestCase):
    def test_full_fps_uses_readable_cpp(self):
        closure = set()
        for header in sorted((ROOT / 'fps').glob('*.hpp')):
            dependencies(header, closure)
        self.assertTrue(closure)
        for path in sorted(closure):
            name = path.relative_to(ROOT).as_posix()
            with self.subTest(header=name):
                self.assertEqual(path.suffix, '.hpp')
                source = path.read_text()
                self.assertTrue(source.isascii())
                guard = re.search(r'^\s*#\s*ifndef\s+([A-Z][A-Z0-9_]*)\s*$', source, re.MULTILINE)
                self.assertIsNotNone(guard)
                for definition in re.findall(r'^\s*#\s*define[^\n]*', source, re.MULTILINE):
                    if definition.strip() == '#define ' + guard[1]:
                        continue
                    self.assertRegex(definition, r'^\s*#\s*define\s+[A-Z][A-Z0-9_]*\s+[0-9]+[uUlL]*\s*$')

    def test_fps_is_edited_as_headers(self):
        for directory in ('fps', 'components'):
            self.assertEqual(list((ROOT / directory).rglob('*.inc')), [])
        for tool in ('compact_fps.py', 'update_fps_headers.py'):
            self.assertFalse((ROOT / 'tools' / tool).exists())

    def test_light_fps_uses_readable_cpp(self):
        headers = ['fps/fps_light.hpp', 'fps/fps_light_algorithms.hpp',
                   'components/fps_light_ntt.hpp', 'components/fps_light_product_tree.hpp',
                   'components/fps_light_backend.hpp', 'components/fps_light_core.hpp', 'math/mint.hpp']
        for name in headers:
            source = (ROOT / name).read_text()
            self.assertTrue(source.isascii(), name)
            definitions = [line for line in source.splitlines() if line.startswith('#define ')]
            self.assertEqual(len(definitions), 1, name)
            self.assertEqual(definitions[0].split()[1], source.splitlines()[0].split()[1])
        lines = sum((ROOT / ('components/' + name)).read_text().count('\n')
                    for name in ('fps_light_core.hpp', 'fps_light_backend.hpp'))
        self.assertGreater(lines, 500)

    def test_light_dependency_closure_uses_readable_cpp(self):
        headers = light_headers()
        self.assertTrue(headers)
        closure = set()
        for header in headers:
            dependencies(header, closure)
        for path in sorted(closure):
            name = path.relative_to(ROOT).as_posix()
            with self.subTest(header=name):
                self.assertNotEqual(name, 'components/fps_backend.hpp')
                self.assertEqual(path.suffix, '.hpp')
                source = path.read_text()
                self.assertTrue(source.isascii())
                guard = re.search(r'^\s*#\s*ifndef\s+([A-Z][A-Z0-9_]*)\s*$', source, re.MULTILINE)
                self.assertIsNotNone(guard)
                for definition in re.findall(r'^\s*#\s*define[^\n]*', source, re.MULTILINE):
                    if definition.strip() == '#define ' + guard[1]:
                        continue
                    self.assertRegex(definition, r'^\s*#\s*define\s+[A-Z][A-Z0-9_]*\s+[0-9]+[uUlL]*\s*$')

    @unittest.skipUnless(shutil.which('oj-bundle'), 'oj-bundle is unavailable')
    def test_light_dependency_budgets(self):
        headers = light_headers()
        self.assertTrue(headers)
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'light.cpp'
            for header in headers:
                name = header.relative_to(ROOT).as_posix()
                with self.subTest(header=name):
                    path.write_text('#include "' + name + '"\n#include "components/fast_io.hpp"\nint main() {}\n')
                    generated = bundle(path)
                    reference = ('//https://qandori.github.io/cp-library/lightweight/' + name + '.html\n').encode()
                    self.assertLess(len(reference + generated), LIGHT_LIMIT)

    @unittest.skipUnless(shutil.which('oj-bundle'), 'oj-bundle is unavailable')
    def test_light_fps_combined_budget(self):
        headers = [path.relative_to(ROOT).as_posix() for path in light_headers() if path.parent == ROOT / 'fps']
        self.assertTrue(headers)
        source = ''.join('#include "' + header + '"\n' for header in headers)
        source += '#include "components/fast_io.hpp"\n#include <cstdlib>\n'
        usage = (ROOT / 'test/unit/fps_light_public_headers.cpp').read_text()
        source += usage[usage.index('void require('):]
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'all_light_fps.cpp'
            path.write_text(source)
            generated = bundle(path)
            reference = b'//https://qandori.github.io/cp-library/lightweight/fps/fps.hpp.html\n'
            self.assertLess(len(reference + generated), LIGHT_LIMIT)

    @unittest.skipUnless(shutil.which('oj-bundle') and shutil.which('g++'), 'C++ bundle tools are unavailable')
    def test_light_fps_output_budget(self):
        from atcoder_flags import constexpr_flags

        source = '''#include "fps/fps_light.hpp"
#include "components/fps_write.hpp"
int main() {
    light_fps f{1, 2, 3};
    const auto restored = f.log(5).exp();
    FastOutput output;
    fps_detail::write_coefficients(output, restored.data(), restored.size());
}
'''
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'fps_output.cpp'
            path.write_text(source)
            generated = bundle(path)
            reference = b'//https://qandori.github.io/cp-library/fps/fps_light.hpp.html\n'
            self.assertLess(len(reference + generated), LIGHT_LIMIT)
            path.write_bytes(reference + generated)
            executable = Path(temporary) / 'fps_output'
            subprocess.run(['g++', '-std=c++23', '-O2', *constexpr_flags('g++'), str(path), '-o', str(executable)], check=True)
            self.assertEqual(subprocess.check_output([str(executable)]).split(), [b'1', b'2', b'3', b'0', b'0'])

    @unittest.skipUnless(shutil.which('oj-bundle'), 'oj-bundle is unavailable')
    def test_full_fps_public_header_budgets(self):
        examples = {
            'fps': '',
            'bostan_mori': 'require(bostan_mori(F{0, 1}, F{1, -1, -1}, 50) == T(12586269025LL));',
            'rational_fps_coefficients': 'require(rational_fps_coefficients(F{1}, F{1, -1, -1}, 10, 15) == F({89, 144, 233, 377, 610}));',
            'compose': 'require(compose(a, inner, 5) == F({1, 2, 5, 6, 3})); require(compose(F{}, F{}, 0).empty());',
            'compositional_inverse': 'auto reverse = compositional_inverse(inner, 129); require(reverse.size() == 129 && reverse[1] == T(1)); require(compositional_inverse(F{}, 0).empty());',
            'multipoint_eval': 'require(multipoint_eval(a, xs) == ys); require(multipoint_eval(F{}, xs) == std::vector<T>(xs.size()));',
            'interpolate': 'require(interpolate(xs, ys) == a);',
            'linear_recurrence': 'require(linear_recurrence(std::vector<T>{0, 1}, std::vector<T>{1, 1}, 50) == T(12586269025LL));',
            'polynomial_gcd': 'F common{1, 1}, left = common * F{2, 1}, right = common * F{3, 1}; require(polynomial_gcd(left, right) == common); auto [gcd, x, y] = polynomial_extended_gcd(left, right); require(gcd == common); auto bezout = left * x + right * y; while (!bezout.empty() && bezout.back() == T(0)) bezout.pop_back(); require(bezout == gcd); auto inverse_mod = polynomial_inv_mod(F{0, 1}, F{1, 1, 1}); require(inverse_mod && *inverse_mod == F({T(-1), T(-1)}));',
            'power_projection': 'require(power_projection(inner, 2, 4) == F({0, 1, 1, 0})); require(power_projection(inner, F{0, 0, 1}, 4) == F({0, 1, 1, 0}));',
            'taylor_shift': 'require(taylor_shift(a, T(1)) == F({6, 8, 3}));',
        }
        usage = (ROOT / 'test/unit/fps_public_headers.cpp').read_text()
        core = usage[usage.index('void require('):usage.index('\trequire(compose(a, inner, 5)')]
        common = 'const std::vector<T> xs{0, 1, 2}, ys{1, 6, 17};\n'
        main = 'int main() { check<mint>(); FastInput input; FastOutput output; int x; if (input.read(x)) output.println(x); }\n'
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'fps.cpp'
            for name, example in examples.items():
                header = 'fps/' + name + '.hpp'
                with self.subTest(header=header):
                    source = '#include "' + header + '"\n#include "components/fast_io.hpp"\n#include <cstdlib>\n'
                    source += core + common + example + '\n}\n' + main
                    path.write_text(source)
                    generated = bundle(path)
                    reference = ('//https://qandori.github.io/cp-library/' + header + '.html\n').encode()
                    self.assertLess(len(reference + generated), LIMIT)

    @unittest.skipUnless(shutil.which('oj-bundle') and shutil.which('g++'), 'C++ bundle tools are unavailable')
    def test_fps_matrix_budgets(self):
        from atcoder_flags import constexpr_flags

        program = '''#include "components/fast_io.hpp"
using F = POLYNOMIAL;
using M = MATRIX<F>;
int main() {
    FastInput input;
    FastOutput output;
    int n = 0, degree = 0;
    std::uint64_t k = 0;
    if (!input.read(n)) return 0;
    input.read(degree); input.read(k);
    M a(n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
        a(i, j).resize(degree);
        for (auto &x : a(i, j)) {
            unsigned value = 0;
            input.read(value);
            x = mint::raw(value);
        }
    }
    auto b = a.pow(k, [degree](M &m) {
        for (int i = 0; i < m.rows(); ++i) for (int j = 0; j < m.cols(); ++j)
            if (m(i, j).size() > std::size_t(degree)) m(i, j).resize(degree);
    });
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
        for (int d = 0; d < degree; ++d) {
            output.write(d < int(b(i, j).size()) ? b(i, j)[d].val() : 0u);
            output.write(' ');
        }
}
'''
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'fps_matrix.cpp'
            for light in (False, True):
                with self.subTest(lightweight=light):
                    header = 'math/matrix_light.hpp' if light else 'math/matrix.hpp'
                    polynomial = 'fps/fps_light.hpp' if light else 'fps/fps.hpp'
                    source = '#include "' + header + '"\n#include "' + polynomial + '"\n'
                    source += program.replace('POLYNOMIAL', 'light_fps' if light else 'fps').replace('MATRIX', 'LightMatrix' if light else 'Matrix')
                    path.write_text(source)
                    generated = bundle(path)
                    reference = ('//https://qandori.github.io/cp-library/' + header + '.html\n').encode()
                    self.assertLess(len(reference + generated), LIGHT_LIMIT if light else LIMIT)
                    path.write_bytes(reference + generated)
                    executable = Path(temporary) / 'fps_matrix'
                    subprocess.run(['g++', '-std=c++23', '-O2', *constexpr_flags('g++'), str(path), '-o', str(executable)], check=True)
                    actual = subprocess.check_output([str(executable)], input=b'2 2 3\n1 0 0 1 0 1 1 0\n')
                    self.assertEqual(actual.split(), [b'1', b'0', b'0', b'3', b'0', b'3', b'1', b'0'])

    def test_documentation_uses_same_limit(self):
        from documentation import source_pages
        from onlinejudge_verify.documentation.front_matter import merge_front_matter, split_front_matter

        source = '#line 1 "source.cpp"\nint f(int x);\n'
        metadata = {'data': {'code': source, 'bundledCode': source}, 'title': 'Example'}
        page = merge_front_matter(metadata, b'Example text\n')
        generated = source_pages({Path('source.md'): page, Path('plain.md'): b'Plain text\n'})
        result, body = split_front_matter(generated[Path('source.md')])
        self.assertLess(result['data']['bundledBytes'], LIMIT)
        self.assertNotIn('bundledCompressed', result['data'])
        self.assertEqual(result['data']['bundledCode'], source)
        self.assertEqual(result['data']['code'], source)
        self.assertEqual(body, b'Example text\n')
        self.assertEqual(generated[Path('plain.md')], b'Plain text\n')
        metadata['data']['bundledCode'] = source * (LIMIT // len(source) + 1)
        oversized = merge_front_matter(metadata, b'Example text\n')
        with self.assertRaises(ValueError):
            source_pages({Path('source.md'): oversized})

    def test_windows_line_endings(self):
        from documentation import source_pages
        from onlinejudge_verify.documentation.front_matter import merge_front_matter, split_front_matter
        source = '#line 1 "source.cpp"\r\nint main() {}\r\n'
        page = merge_front_matter({'data': {'bundledCode': source}}, b'Body\n')
        generated = source_pages({Path('source.md'): page})
        metadata, _ = split_front_matter(generated[Path('source.md')])
        self.assertEqual(metadata['data']['bundledCode'], source)
        self.assertEqual(metadata['data']['bundledBytes'], len(source.encode('utf-8')))


if __name__ == '__main__':
    unittest.main()
