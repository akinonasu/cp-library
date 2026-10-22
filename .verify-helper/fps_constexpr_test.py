from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class FPSCompile(unittest.TestCase):
    def test_atcoder_constexpr_limits(self):
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        if not compilers:
            self.skipTest('C++ compiler not installed')
        with tempfile.TemporaryDirectory(prefix='fps-compile-') as directory:
            for compiler in compilers:
                with self.subTest(compiler=compiler):
                    version = subprocess.check_output([compiler, '--version'], text=True).lower()
                    flags = ['-fconstexpr-depth=1024']
                    if 'clang' in version:
                        flags += ['-fconstexpr-steps=524288']
                    else:
                        flags += ['-fconstexpr-loop-limit=524288', '-fconstexpr-ops-limit=2097152']
                    binary = Path(directory) / 'main.exe'
                    subprocess.run([compiler, '-std=c++2b', '-O2', *flags, '-I', str(ROOT),
                                    str(ROOT / 'test/unit/fps_taylor_factorials.cpp'),
                                    '-o', str(binary)], check=True)
                    subprocess.run([str(binary)], check=True, timeout=120)


if __name__ == '__main__':
    unittest.main()
