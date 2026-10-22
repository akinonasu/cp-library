import copy
from pathlib import Path
import tempfile
import unittest
from unittest.mock import Mock, patch

from documentation import SourceCache


class DocumentationCache(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root / '.verify-helper').mkdir()
        (self.root / '.verify-helper/documentation.py').write_text('helper\n')
        self.path = self.root / 'sources.json'
        self.source = self.root / 'math/a.hpp'
        self.plan = {'bundles': [], 'verification': [], 'state': {'targets': {
            'bundles': {'math/a.hpp': 'header'}, 'verification': {'test/a.test.cpp': 'driver'}}}}

    def cache(self, plan=None):
        return SourceCache(self.root, plan or self.plan, self.path)

    def populate(self):
        cache = self.cache()
        cache.dependencies(Mock(return_value=[self.source]), self.source, basedir=self.root)
        cache.attributes(Mock(return_value={'PROBLEM': 'https://judge.yosupo.jp/problem/unionfind'}), self.source, basedir=self.root)
        cache.bundle(Mock(return_value=b'int a;\n'), self.source, basedir=self.root, options={'include_paths': [self.root]})
        cache.save()

    def test_reuses_content_without_reusing_status_or_timestamps(self):
        self.populate()
        cache = self.cache()
        unexpected = Mock(side_effect=AssertionError('unchanged source analyzed'))
        self.assertEqual(cache.dependencies(unexpected, self.source, basedir=self.root), [self.source])
        self.assertIn('PROBLEM', cache.attributes(unexpected, self.source, basedir=self.root))
        self.assertEqual(cache.bundle(unexpected, self.source, basedir=self.root, options={'include_paths': [self.root]}), b'int a;\n')
        self.assertEqual(cache.hits, {'math/a.hpp'})
        self.assertNotIn('verification_status', cache.record(self.source))
        self.assertNotIn('timestamp', cache.record(self.source))

    def test_changed_dependency_fingerprint_and_selected_sources_rebuild(self):
        self.populate()
        for selected in [False, True]:
            with self.subTest(selected=selected):
                plan = copy.deepcopy(self.plan)
                if selected: plan['bundles'] = ['math/a.hpp']
                else: plan['state']['targets']['bundles']['math/a.hpp'] = 'changed transitive dependency or tools'
                cache = self.cache(plan)
                original = Mock(return_value={'PROBLEM': 'new'})
                self.assertEqual(cache.attributes(original, self.source, basedir=self.root), {'PROBLEM': 'new'})
                original.assert_called_once()
                self.assertEqual(cache.hits, set())

    def test_helper_changes_and_no_cache_disable_reuse(self):
        self.populate()
        (self.root / '.verify-helper/documentation.py').write_text('new renderer\n')
        self.assertNotIn('bundle', self.cache().record(self.source))
        self.populate()
        before = self.path.read_bytes()
        with patch('ci.no_cache_context', return_value=True):
            cache = self.cache()
            self.assertNotIn('bundle', cache.record(self.source))
            cache.save()
        self.assertEqual(before, self.path.read_bytes())

    def test_dependencies_are_relocated_and_other_bundle_options_bypass_cache(self):
        self.populate()
        cache = self.cache()
        call = Mock(return_value=b'other options')
        self.assertEqual(cache.bundle(call, self.source, basedir=self.root, options={'include_paths': []}), b'other options')
        call.assert_called_once()
        self.assertNotIn(str(self.root), self.path.read_text())
