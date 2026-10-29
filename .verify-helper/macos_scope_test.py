from pathlib import Path
import subprocess
import tempfile
import unittest

import macos_scope


class MacOSScopeTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.command('init', '-q')
        self.write('graph/a.hpp', 'original')
        self.base = self.commit()

    def command(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args],
                                       stderr=subprocess.PIPE).decode().strip()

    def write(self, name, content='changed'):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)

    def commit(self, message='change'):
        self.command('add', '.')
        self.command('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid',
                     'commit', '-qm', message)
        return self.command('rev-parse', 'HEAD')

    def select(self, paths, *, message='change', kind='push'):
        for path in paths:
            self.write(path)
        head = self.commit(message)
        event = {'before': self.base, 'pull_request': {
            'base': {'sha': self.base}, 'head': {'sha': head}}}
        return macos_scope.needed(self.root, event, {'GITHUB_SHA': head, 'GITHUB_EVENT_NAME': kind})

    def test_submission_and_documentation_only(self):
        self.assertFalse(self.select(['submissions/abc012_4.cpp', 'docs/example.md', 'README.md']))

    def test_headers_mixed_with_docs(self):
        self.assertTrue(self.select(['graph/a.hpp', 'docs/example.md']))

    def test_unit_and_control_inputs(self):
        for path in ['test/unit/a.cpp', 'test/fixtures/data.md', 'test/verify.py',
                     '.verify-helper/ci.py', '.github/workflows/macos-portability.yml']:
            with self.subTest(path=path):
                self.assertFalse(macos_scope.documentation_only(path))

    def test_header_deletion(self):
        (self.root / 'graph/a.hpp').unlink()
        self.assertTrue(self.select(['docs/example.md']))

    def test_header_moved_to_submission(self):
        (self.root / 'submissions').mkdir()
        self.command('mv', 'graph/a.hpp', 'submissions/a.cpp')
        self.assertTrue(self.select(['docs/example.md']))

    def test_manual_and_new_branch_are_not_skipped(self):
        self.assertTrue(macos_scope.needed(self.root, {}, {'GITHUB_EVENT_NAME': 'workflow_dispatch'}))
        self.assertTrue(macos_scope.needed(self.root, {'before': '0' * 40}, {
            'GITHUB_EVENT_NAME': 'push', 'GITHUB_SHA': self.base}))

    def test_unavailable_base_is_not_skipped(self):
        self.assertTrue(macos_scope.needed(self.root, {'before': 'a' * 40}, {
            'GITHUB_EVENT_NAME': 'push', 'GITHUB_SHA': self.base}))

    def test_no_cache_opt_in_runs_even_for_docs(self):
        self.assertTrue(self.select(['README.md'], message='change\n\nVerify-Mode: no-cache'))

    def test_pull_request_docs(self):
        self.assertFalse(self.select(['docs/example.md'], kind='pull_request'))

    def test_pull_request_merge_base_excludes_base_branch_changes(self):
        self.command('checkout', '-qb', 'topic')
        self.write('docs/example.md')
        head = self.commit()
        self.command('checkout', '-q', '-')
        self.write('graph/a.hpp', 'base branch change')
        base = self.commit()
        self.assertFalse(macos_scope.needed(self.root, {'pull_request': {
            'base': {'sha': base}, 'head': {'sha': head}}}, {
                'GITHUB_EVENT_NAME': 'pull_request', 'GITHUB_SHA': head}))

    def test_workflow_keeps_required_job_and_manual_trigger(self):
        import yaml
        root = Path(__file__).resolve().parent.parent
        workflow = yaml.load((root / '.github/workflows/macos-portability.yml').read_text(), Loader=yaml.BaseLoader)
        self.assertIn('workflow_dispatch', workflow['on'])
        job = workflow['jobs']['portable']
        self.assertEqual(job['runs-on'], 'macos-15')
        self.assertEqual(job['needs'], 'scope')
        self.assertEqual(job['if'], "needs.scope.outputs.needed == 'true'")
        self.assertNotIn('continue-on-error', job)
        self.assertEqual(workflow['jobs']['scope']['runs-on'], 'ubuntu-24.04')
        self.assertIn('needed', workflow['jobs']['scope']['outputs'])


if __name__ == '__main__':
    unittest.main()
