import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
LIMIT = 256 * 1024
LIGHT_LIMIT = 64 * 1024
from tools.compact_fps import compact, lex


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
    def test_compact_requires_line_directive(self):
        with self.assertRaises(ValueError):
            compact('int main() {}\n')

    def test_distribution_headers_are_current(self):
        subprocess.run([sys.executable, str(ROOT / 'tools/update_fps_headers.py'), '--check'],
                       cwd=ROOT, check=True, capture_output=True)

    def test_light_fps_uses_readable_cpp(self):
        headers = ['fps/fps_light.hpp', 'fps/fps_light_algorithms.hpp',
                   'components/fps_light_ntt.hpp', 'components/fps_light_product_tree.hpp',
                   'components/fps_light_backend.hpp', 'math/mint.hpp']
        for name in headers:
            source = (ROOT / name).read_text()
            self.assertTrue(source.isascii(), name)
            definitions = [line for line in source.splitlines() if line.startswith('#define ')]
            self.assertEqual(len(definitions), 1, name)
            self.assertEqual(definitions[0].split()[1], source.splitlines()[0].split()[1])
        self.assertGreater((ROOT / 'components/fps_light_backend.hpp').read_text().count('\n'), 500)

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
                    generated = subprocess.check_output(['oj-bundle', '-I', str(ROOT), str(path)], cwd=ROOT)
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
            generated = subprocess.check_output(['oj-bundle', '-I', str(ROOT), str(path)], cwd=ROOT)
            reference = b'//https://qandori.github.io/cp-library/lightweight/fps/fps.hpp.html\n'
            self.assertLess(len(reference + generated), LIGHT_LIMIT)

    def test_preprocessor_equivalence(self):
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        self.assertTrue(compilers)
        prefix = '#line 1 "fixture.cpp"\n#include <cstdio>\n#include <cstdint>\n#include <source_location>\n'
        prefix += '#define TEXT(x) #x\n#define APPLY(F,X) F(X)\n#define ROW __LINE__\n#define ADD(x,y) ((x)+(y))\n'
        prefix += ''.join(f'inline int f{i}(int x) {{ const int y=x+123; return y^(y>>3); }}\n' for i in range(24))
        prefix += ''.join(f'inline int pair{i}(int x) {{ const int z=(x*17371+99127)^(x*839+11203); return z+(z>>7); }}\n' for i in range(2))
        raw = 'R"raw(before\\\n' + '\n' * 40 + '#define NOT_A_DIRECTIVE 1\nafter)raw"'
        conditional = '#if PICK\n#undef ADD\n#define ADD(x,y) \\\n ((x)+(y))\n#else\n#undef ADD\n#define ADD(x,y) ((x)+(y))\n#endif\n'
        clobbers = ', '.join(f'"xmm{i}"' for i in range(16)) + ', "memory", "cc"'
        barriers = '#if defined(__x86_64__)\n' + ''.join(
            f'inline void barrier{i}() {{ asm volatile("" ::: {clobbers}); }}\n' for i in range(2)
        ) + '#endif\n'
        main = ('int main() {\nconst char* a=APPLY(TEXT,std :: uint32_t);\n'
                f'const char* raw={raw};\n'
                'const auto where=std::source_location::current();\n'
                'std::printf("%s %s %d %u %d\\n",a,raw,ROW,where.line(),ADD(f0(1),2));\n}\n')
        indirect = prefix + '#define INDIRECT() \\\n ROW\n#if PICK\n#define SELECTED 5\n'
        indirect += '#define LONG 1 \\\n' + '+1 \\\n' * 30 + '+1\n'
        indirect += '#elif INDIRECT() == EXPECTED\n#define SELECTED 7\n#else\n#define SELECTED 9\n#endif\n'
        expected = next(i for i, line in enumerate(indirect.splitlines()) if line.startswith('#elif'))
        indirect = indirect.replace('EXPECTED', str(expected))
        indirect += 'int selected = SELECTED;\n' + main.replace('ADD(f0(1),2)', 'ADD(f0(1),2)+selected')
        quoted = '"before\\\n#if PICK\\\n#define STRING_TEXT 1\\\nafter"'
        fixtures = [prefix + conditional + barriers + main,
                    prefix + '#if PICK\n#line 800 "conditional.cpp"\n#endif\n' + main,
                    prefix + '#if ROW > 0\n#define OBSERVED 1\n#endif\n' + main, indirect,
                    prefix + main.replace(raw, quoted)]
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary)
            for index, source in enumerate(fixtures):
                packed = compact(source)
                for compiler in compilers:
                    for pick in (0, 1):
                        with self.subTest(fixture=index, compiler=compiler, pick=pick):
                            streams, outputs = [], []
                            for variant, text in [('before', source), ('after', packed)]:
                                folder = work / variant
                                folder.mkdir(exist_ok=True)
                                path = folder / 'source.cpp'
                                path.write_text(text, encoding='utf-8')
                                command = [compiler, '-std=c++23', '-O2', '-DPICK=' + str(pick),
                                           '-ffile-prefix-map=' + str(folder) + '=/source', str(path)]
                                result = subprocess.check_output([*command, '-E', '-P'], text=True)
                                streams.append(lex(result))
                                executable = folder / 'program'
                                subprocess.run([*command, '-o', str(executable)], check=True, capture_output=True)
                                outputs.append(subprocess.check_output([str(executable)]))
                            self.assertEqual(streams[0], streams[1])
                            self.assertEqual(outputs[0], outputs[1])

    @unittest.skipUnless(shutil.which('oj-bundle'), 'oj-bundle is unavailable')
    def test_fps_normal_and_basic_light_combined_budget(self):
        headers = ['fps/' + name + '.hpp' for name in (
            'bostan_mori', 'compose', 'compositional_inverse', 'fps', 'fps_light', 'fps_light_algorithms',
            'interpolate', 'linear_recurrence', 'multipoint_eval', 'polynomial_gcd', 'power_projection', 'taylor_shift')]
        source = ''.join('#include "' + header + '"\n' for header in headers)
        source += '#include "components/fast_io.hpp"\n'
        usage = (ROOT / 'test/unit/fps_public_headers.cpp').read_text()
        usage = usage[usage.index('void require('):]
        usage = usage.replace('check<mint>();', 'light_fps lf{1,2,1}; require(lf.inv(8).size()==8); check<mint>();')
        source += usage
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'all_fps.cpp'
            path.write_text(source)
            generated = subprocess.check_output(['oj-bundle', '-I', str(ROOT), str(path)], cwd=ROOT)
            reference = b'//https://qandori.github.io/cp-library/fps/fps.hpp.html\n'
            self.assertLess(len(reference + generated), LIMIT)

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
