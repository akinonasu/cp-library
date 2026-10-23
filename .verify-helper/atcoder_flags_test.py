from pathlib import Path
import shutil
import subprocess
import tomllib
import unittest
from unittest.mock import patch

from atcoder_flags import constexpr_flags

ROOT = Path(__file__).resolve().parent.parent


class AtCoderFlags(unittest.TestCase):
    def setUp(self):
        constexpr_flags.cache_clear()
        self.addCleanup(constexpr_flags.cache_clear)

    def test_compiler_identity_and_cache(self):
        with patch('subprocess.check_output', return_value='Apple clang version 17') as version:
            self.assertIn('-fconstexpr-steps=524288', constexpr_flags('g++'))
            self.assertEqual(constexpr_flags('g++'), constexpr_flags('g++'))
            version.assert_called_once_with(['g++', '--version'], text=True)

    def test_judge_configuration_matches(self):
        with (ROOT / '.verify-helper/config.toml').open('rb') as source:
            environments = tomllib.load(source)['languages']['cpp']['environments']
        for environment in environments:
            with self.subTest(compiler=environment['CXX']):
                with patch('subprocess.check_output', return_value=environment['CXX']):
                    actual = [flag for flag in environment['CXXFLAGS'] if flag.startswith('-fconstexpr-')]
                    self.assertEqual(actual, constexpr_flags(environment['CXX']))

    def test_limits_reject_expensive_evaluation(self):
        compilers = [compiler for compiler in ('g++', 'clang++') if shutil.which(compiler)]
        if not compilers:
            self.skipTest('C++ compiler not installed')
        source = ('constexpr int value = [] { int n = 0; '
                  'for (int i = 0; i < 1000000; ++i) n += i % 3; return n; }();\n')
        for compiler in compilers:
            with self.subTest(compiler=compiler):
                result = subprocess.run([compiler, '-std=c++23', *constexpr_flags(compiler),
                                         '-fsyntax-only', '-x', 'c++', '-'], input=source,
                                        text=True, capture_output=True, timeout=60)
                self.assertNotEqual(result.returncode, 0)
                self.assertTrue('operation count exceeds limit' in result.stderr or
                                'maximum step limit' in result.stderr, result.stderr)


if __name__ == '__main__':
    unittest.main()
