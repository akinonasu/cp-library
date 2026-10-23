import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from support_tests import fingerprint, run


class SupportTestsCache(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        subprocess.run(['git', 'init', '--quiet', str(self.root)], check=True)
        self.write('math/a.hpp', 'int a;\n')
        self.write('.verify-helper/helper_test.py', 'tests\n')
        self.state = self.root / 'state.json'

    def write(self, name, source):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source)

    def test_documentation_changes_reuse_but_sources_and_tools_invalidate(self):
        key = fingerprint(self.root, 'tools')
        self.write('docs/a.md', 'new prose\n')
        self.write('README.md', 'new catalog\n')
        self.write('AGENTS.md', 'new writing rule\n')
        self.assertEqual(key, fingerprint(self.root, 'tools'))
        self.assertNotEqual(key, fingerprint(self.root, 'new tools'))
        for name in ['math/a.hpp', '.github/workflows/verify.yml', '.verify-helper/helper_test.py', 'test/a.cpp']:
            with self.subTest(name=name):
                before = fingerprint(self.root, 'tools')
                self.write(name, 'changed\n')
                self.assertNotEqual(before, fingerprint(self.root, 'tools'))

    def test_new_untracked_tests_and_deleted_tracked_inputs_invalidate(self):
        subprocess.run(['git', '-C', str(self.root), 'add', '.'], check=True)
        before = fingerprint(self.root, 'tools')
        self.write('.verify-helper/new_test.py', 'new test\n')
        self.assertNotEqual(before, fingerprint(self.root, 'tools'))
        (self.root / '.verify-helper/new_test.py').unlink()
        (self.root / 'math/a.hpp').unlink()
        self.assertNotEqual(before, fingerprint(self.root, 'tools'))

    def test_reuse_only_successful_matching_tests(self):
        with patch('support_tests.subprocess.run') as execute:
            run(self.root, self.state, 'first')
            execute.assert_called_once()
            execute.reset_mock()
            run(self.root, self.state, 'first')
            execute.assert_not_called()
            run(self.root, self.state, 'second')
            execute.assert_called_once()
        before = self.state.read_text()
        with patch('support_tests.subprocess.run', side_effect=subprocess.CalledProcessError(1, 'tests')):
            with self.assertRaises(subprocess.CalledProcessError):
                run(self.root, self.state, 'failed')
        self.assertEqual(self.state.read_text(), before)

    def test_explicit_no_cache_always_runs_and_does_not_save(self):
        self.state.write_text(json.dumps({'version': 1, 'fingerprint': 'same'}))
        before = self.state.read_text()
        with patch('support_tests.subprocess.run') as execute:
            run(self.root, self.state, 'same', enabled=False)
            execute.assert_called_once()
        self.assertEqual(self.state.read_text(), before)

    def test_full_run_does_not_reuse_prior_success(self):
        self.state.write_text(json.dumps({'version': 1, 'fingerprint': 'same'}))
        with patch('support_tests.subprocess.run') as execute:
            run(self.root, self.state, 'same', force=True)
            execute.assert_called_once()
        self.assertEqual(json.loads(self.state.read_text())['fingerprint'], 'same')

    def test_workflow_only_installs_missing_boost(self):
        import yaml
        root = Path(__file__).resolve().parent.parent
        workflow = yaml.load((root / '.github/workflows/verify.yml').read_text(), Loader=yaml.BaseLoader)
        steps = workflow['jobs']['verify']['steps']
        command = next(step['run'] for step in steps if step.get('name') == 'Install Boost')
        for installed in [True, False]:
            with self.subTest(installed=installed):
                fake = "dpkg-query() { echo installed; };" if installed else "dpkg-query() { return 1; };"
                fake += 'sudo() { echo "INSTALL $*"; };'
                result = subprocess.run(['bash', '-e', '-c', fake + command], capture_output=True, text=True, check=True)
                self.assertEqual(result.stdout.count('INSTALL'), 0 if installed else 2)
