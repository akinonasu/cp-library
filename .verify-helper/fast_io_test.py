from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from atcoder_flags import constexpr_flags

ROOT = Path(__file__).resolve().parent.parent


class FastIOCompile(unittest.TestCase):
    def test_atcoder_constexpr_limits(self):
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        if not compilers:
            self.skipTest('C++ compiler not installed')
        with tempfile.TemporaryDirectory(prefix='fast-io-compile-') as directory:
            source = Path(directory) / 'main.cpp'
            source.write_text('#include "components/fast_io.hpp"\n'
                              'int main() { FastOutput out; out.println(4294967295u); }\n')
            for compiler in compilers:
                with self.subTest(compiler=compiler):
                    flags = constexpr_flags(compiler)
                    binary = Path(directory) / 'main.exe'
                    subprocess.run([compiler, '-std=c++2b', '-O2', *flags, '-I', str(ROOT),
                                    str(source), '-o', str(binary)], check=True)
                    self.assertEqual(subprocess.check_output([str(binary)], text=True), '4294967295\n')


if __name__ == '__main__':
    unittest.main()
