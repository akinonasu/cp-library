import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.bundle import LIMIT, fit_source
from tools.compact_fps import compact, lex


class BundleTest(unittest.TestCase):
    def test_limit_and_passthrough(self):
        source = '#line 1 "source.cpp"\nint main() {}\n'
        self.assertEqual(fit_source(source), source)
        for limit in (0, -1, 1):
            with self.assertRaises(ValueError):
                fit_source(source, limit)
        with self.assertRaises(ValueError):
            compact('int main() {}\n')

    def test_preprocessor_equivalence(self):
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        self.assertTrue(compilers)
        prefix = '#line 1 "fixture.cpp"\n#include <cstdio>\n#include <cstdint>\n#include <source_location>\n'
        prefix += '#define TEXT(x) #x\n#define APPLY(F,X) F(X)\n#define ROW __LINE__\n#define ADD(x,y) ((x)+(y))\n'
        prefix += ''.join(f'inline int f{i}(int x) {{ const int y=x+123; return y^(y>>3); }}\n' for i in range(24))
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
    def test_command_and_failure_preserves_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary)
            source, target = work / 'input.cpp', work / 'output.cpp'
            source.write_bytes('int main() {} // 日本語\n'.encode('utf-8'))
            command = [sys.executable, str(ROOT / 'tools/bundle.py'), str(source), '-o', str(target)]
            subprocess.run(command, check=True, capture_output=True)
            previous = target.read_bytes()
            self.assertLess(len(previous), LIMIT)
            environment = dict(os.environ, PYTHONIOENCODING='ascii')
            piped = subprocess.check_output([*command[:-2]], env=environment)
            self.assertEqual(piped, previous)
            packed = subprocess.run([sys.executable, str(ROOT / 'tools/compact_fps.py'), '--limit', str(LIMIT)],
                                    input=previous, capture_output=True, env=environment, check=True).stdout
            packed.decode('utf-8')
            self.assertNotIn(b'\r', packed)
            result = subprocess.run([*command, '--limit', '1'], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(target.read_bytes(), previous)

    def test_documentation_uses_same_limit(self):
        from documentation import compact_source_pages
        from onlinejudge_verify.documentation.front_matter import merge_front_matter, split_front_matter

        source = '#line 1 "source.cpp"\n' + 'int f(int x);\n' * 25000
        metadata = {'data': {'code': source, 'bundledCode': source}, 'title': 'Example'}
        page = merge_front_matter(metadata, b'Example text\n')
        generated = compact_source_pages({Path('source.md'): page, Path('plain.md'): b'Plain text\n'})
        result, body = split_front_matter(generated[Path('source.md')])
        self.assertLess(result['data']['bundledBytes'], LIMIT)
        self.assertTrue(result['data']['bundledCompressed'])
        self.assertEqual(result['data']['code'], source)
        self.assertEqual(body, b'Example text\n')
        self.assertEqual(generated[Path('plain.md')], b'Plain text\n')


if __name__ == '__main__':
    unittest.main()
