import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

from judge_data import build_manifest, generator_stack, mark_ready, normalize_sources, prepare_selected, problem_fingerprint


class JudgeDataTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for name, text in {
            'test/requirements.txt': 'helper==1\n',
            'test/a.test.cpp': '#define PROBLEM "https://judge.yosupo.jp/problem/alpha"\n',
            'test/b.test.cpp': '#define PROBLEM "https://judge.yosupo.jp/problem/alpha"\n',
            'test/c.test.cpp': '#define PROBLEM "https://yukicoder.me/problems/no/2362"\n',
        }.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
        self.plan = {'verification': ['test/a.test.cpp', 'test/b.test.cpp', 'test/c.test.cpp']}
        self.repo = self.root / 'checker'
        self.entries = {'data_structure/alpha/info.toml': 'a', 'common/random.h': 'b',
                        'generate.py': 'c', 'data_structure/beta/info.toml': 'd'}

    def manifest(self, toolchain='gcc13'):
        with patch('judge_data.source_entries', return_value=self.entries):
            return build_manifest(self.root, self.plan, self.repo, toolchain)

    def test_selected_urls_are_deduplicated(self):
        manifest = self.manifest()
        self.assertEqual(len(manifest['entries']), 2)
        first = next(x for x in manifest['entries'] if 'problem' in x)
        self.assertEqual(first['drivers'], ['test/a.test.cpp', 'test/b.test.cpp'])
        self.assertEqual(first['problem'], str(self.repo / 'data_structure/alpha'))
        self.assertEqual(len(first['paths']), 2)
        with patch('judge_data.source_entries') as read:
            empty = build_manifest(self.root, {'verification': []}, self.repo, '')
        read.assert_not_called()
        self.assertEqual(empty, {'repository': None, 'entries': []})

    def test_cache_key_tracks_only_problem_shared_inputs_and_compiler(self):
        before = self.manifest()
        self.entries['data_structure/beta/info.toml'] = 'unrelated'
        self.assertEqual(before, self.manifest())
        self.entries['common/random.h'] = 'changed'
        after = self.manifest()
        a, b = [next(x for x in m['entries'] if 'problem' in x)['key'] for m in [before, after]]
        self.assertNotEqual(a, b)
        self.assertNotEqual(after, self.manifest('gcc14'))
        original = problem_fingerprint(self.entries, 'data_structure/alpha')
        self.entries['data_structure/alpha/gen.cpp'] = 'new'
        self.assertNotEqual(original, problem_fingerprint(self.entries, 'data_structure/alpha'))

    def test_source_timestamps_are_normalized_only_after_content_check(self):
        subprocess.run(['git', 'init', '--quiet', str(self.repo)], check=True)
        for name in ['generate.py', 'common/test.h', 'tree/alpha/info.toml']:
            path = self.repo / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('source')
        subprocess.run(['git', '-C', str(self.repo), 'add', '.'], check=True)
        normalize_sources(self.repo, self.repo / 'tree/alpha')
        self.assertEqual((self.repo / 'common/test.h').stat().st_mtime_ns, 0)
        (self.repo / 'common/test.h').write_text('changed')
        with self.assertRaises(subprocess.CalledProcessError):
            normalize_sources(self.repo, self.repo / 'tree/alpha')

    def test_no_cache_scope_is_rejected(self):
        with self.assertRaises(ValueError):
            build_manifest(self.root, {**self.plan, 'mode': 'affected-no-cache-v1'}, self.repo, '')

    def test_generator_stack_uses_hard_limit_and_restores_original_limit(self):
        for hard in (-1, 64 * 1024 * 1024):
            with self.subTest(hard=hard):
                resource = Mock()
                resource.getrlimit.return_value = (8 * 1024 * 1024, hard)
                with patch.dict('sys.modules', resource=resource):
                    with generator_stack():
                        resource.setrlimit.assert_called_once_with(resource.RLIMIT_STACK, (hard, hard))
                    self.assertEqual(resource.setrlimit.call_args.args,
                                     (resource.RLIMIT_STACK, (8 * 1024 * 1024, hard)))

    def test_generator_stack_restores_limit_after_failure(self):
        resource = Mock()
        resource.getrlimit.return_value = (8 * 1024 * 1024, -1)
        with patch.dict('sys.modules', resource=resource):
            with self.assertRaisesRegex(RuntimeError, 'generator failed'):
                with generator_stack():
                    raise RuntimeError('generator failed')
        self.assertEqual(resource.setrlimit.call_args.args,
                         (resource.RLIMIT_STACK, (8 * 1024 * 1024, -1)))

    def test_generator_stack_without_resource_module(self):
        with patch.dict('sys.modules', resource=None):
            with generator_stack():
                pass

    @unittest.skipUnless(__import__('importlib.util').util.find_spec('resource'), 'Requires resource')
    def test_generator_child_inherits_stack_limit(self):
        import resource

        before = resource.getrlimit(resource.RLIMIT_STACK)
        with generator_stack():
            actual = subprocess.check_output([sys.executable, '-c',
                'import json, resource; print(json.dumps(resource.getrlimit(resource.RLIMIT_STACK)))'], text=True)
        self.assertEqual(json.loads(actual), [before[1], before[1]])
        self.assertEqual(resource.getrlimit(resource.RLIMIT_STACK), before)

    @unittest.skipUnless(__import__('importlib.util').util.find_spec('onlinejudge'), 'Requires helper')
    def test_prepare_uses_hardlinks_and_preserves_unselected_data(self):
        manifest = self.manifest()
        item = next(x for x in manifest['entries'] if 'problem' in x)
        problem = Path(item['problem'])
        for name, data in [('in/example_00.in', b'1\n'), ('out/example_00.out', b'2\n')]:
            path = problem / name; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(data)
        path = self.root / 'manifest.json'; path.write_text(json.dumps(manifest))
        with patch('judge_data.subprocess.run') as run, patch('judge_data.normalize_sources'), \
                patch('judge_data.generator_stack') as stack:
            prepare_selected(self.root, self.plan, path)
        stack.assert_called_once_with()
        self.assertEqual(run.call_count, 1)
        cases = Path(item['cases'])
        self.assertEqual((cases / 'example_00.in').stat().st_ino, (problem / 'in/example_00.in').stat().st_ino)
        self.assertEqual((cases / 'example_00.out').read_bytes(), b'2\n')
        self.assertTrue(next(x for x in json.loads(path.read_text())['entries'] if 'problem' in x)['ready'])
        mark_ready(path, 'test/c.test.cpp')
        self.assertTrue(all(x['ready'] for x in json.loads(path.read_text())['entries']))
        with self.assertRaises(ValueError):
            prepare_selected(self.root, {'verification': []}, path)


if __name__ == '__main__':
    unittest.main()
