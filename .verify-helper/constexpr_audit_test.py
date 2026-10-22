import contextlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location('constexpr_audit', ROOT / 'test/benchmark/constexpr_audit.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class ConstexprAudit(unittest.TestCase):
    def run_audit(self, fail=False):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('math/value.hpp', 'test/unit/value.cpp', 'test/value.test.cpp'):
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('int value;\n')
            output = root / 'report.json'
            commands = []

            def compile_source(command, **kwargs):
                self.assertIn('-fsyntax-only', command)
                self.assertIn('-test-constexpr-limit', command)
                source = Path(command[-1])
                self.assertTrue(source.is_file())
                if source.name == 'main.cpp':
                    self.assertIn('#include "math/value.hpp"', source.read_text())
                commands.append(command)
                if fail and source.name == 'value.test.cpp':
                    return subprocess.CompletedProcess(command, 1, '', 'constexpr limit exceeded')
                return subprocess.CompletedProcess(command, 0, '', '')

            arguments = ['constexpr_audit.py', '--cxx', 'g++', 'clang++', '--jobs', '2', '--output', str(output)]
            with patch.object(audit, 'ROOT', root), patch.object(audit, 'LIBRARIES', {'math'}), \
                    patch.object(audit, 'constexpr_flags', return_value=['-test-constexpr-limit']), \
                    patch('subprocess.check_output', side_effect=lambda command, **kwargs: command[0]), \
                    patch('subprocess.run', side_effect=compile_source), patch.object(sys, 'argv', arguments), \
                    contextlib.redirect_stdout(io.StringIO()):
                status = audit.main()
            self.assertEqual(len(commands), 6)
            return status, json.loads(output.read_text())

    def test_inventory_and_flags(self):
        status, report = self.run_audit()
        self.assertEqual(status, 0)
        self.assertEqual(report['counts'], {'header': 1, 'unit': 1, 'judge': 1})
        self.assertEqual(len(report['source_sha256']), 3)
        self.assertTrue(all(record['returncode'] == 0 for record in report['results']))

    def test_compile_failure_is_recorded_and_fails_audit(self):
        status, report = self.run_audit(fail=True)
        self.assertEqual(status, 1)
        failed = [record for record in report['results'] if record['returncode']]
        self.assertEqual(len(failed), 2)
        self.assertTrue(all(record['path'] == 'test/value.test.cpp' and record['diagnostic'] for record in failed))


if __name__ == '__main__':
    unittest.main()
