import copy
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from size_profiles import LIGHTWEIGHT, LIMITS, copied_source, copy_size, documentation_links, metadata, page_url, prepare_pages, remove_stale_pages, render_page, rewrite_links, split_page


ROOT = Path(__file__).resolve().parent.parent
CONFIG = {'url': 'https://qandori.github.io', 'baseurl': '/cp-library'}


class SizeProfiles(unittest.TestCase):
    def test_byte_limit_includes_utf8_and_reference(self):
        name = 'tree/example.hpp'
        url = page_url(CONFIG, name + '.html', LIGHTWEIGHT)
        overhead = copy_size('', url)
        code = 'a' * (LIMITS[LIGHTWEIGHT] - overhead)
        self.assertTrue(metadata(code, name, CONFIG)['lightweightAvailable'])
        self.assertFalse(metadata(code + 'a', name, CONFIG)['lightweightAvailable'])
        self.assertEqual(copy_size('円\n', url), overhead + 4)
        self.assertEqual(copied_source('x\n', url), '//' + url + '\nx\n')

    def test_longer_lightweight_url_changes_classification(self):
        name = 'a.hpp'
        code = 'x' * (LIMITS[LIGHTWEIGHT] - copy_size('', page_url(CONFIG, name + '.html')))
        sizes = metadata(code, name, CONFIG)
        self.assertEqual(sizes['copyBytes']['full'], LIMITS[LIGHTWEIGHT])
        self.assertFalse(sizes['lightweightAvailable'])

    def fixture(self):
        destination = Path('/tmp/generated')
        names = {'tree/a.hpp': '#line 1 "tree/a.hpp"\nint a;\n', 'fps/large.hpp': 'x' * 65536}
        pages = {}
        categories = [{'name': name.split('/')[0], 'pages': [{'path': name, 'title': name, 'icon': 'OK'}]}
                      for name in names]
        for name, code in names.items():
            front = {'layout': 'document', 'documentation_of': name, 'title': name,
                     'redirect_from': ['/old/' + name + '.html'], 'data': {'path': name, 'code': 'original',
                     'bundledCode': code, 'isVerificationFile': False,
                     '_extendedDependsOn': [{'path': 'tree/a.hpp', 'title': 'a'}],
                     '_extendedVerifiedWith': [{'path': 'test/a.test.cpp', 'title': 'test'}]}}
            pages[destination / (name + '.md')] = render_page(front, b'[a](https://qandori.github.io/cp-library/tree/a.hpp.html)\n')
        for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
            front = {'layout': 'toppage', 'data': {'libraryCategories': copy.deepcopy(categories)}}
            pages[destination / page] = render_page(front, b'catalog\n')
        pages[destination / 'test/a.test.cpp.md'] = render_page({'data': {'path': 'test/a.test.cpp'}}, b'test')
        return destination, pages

    def test_common_pages_and_unchanged_sources(self):
        destination, pages = self.fixture()
        originals = dict(pages)
        result = prepare_pages(pages, destination, CONFIG)
        self.assertEqual(pages, originals)
        self.assertNotIn(destination / LIGHTWEIGHT / 'fps/large.hpp.md', result)
        full, _ = split_page(result[destination / 'index.md'])
        light, _ = split_page(result[destination / LIGHTWEIGHT / 'index.md'])
        self.assertEqual(full['data']['profileLibraryCount'], 2)
        self.assertEqual(light['data']['profileLibraryCount'], 2)
        self.assertEqual(light['data']['profileAvailableCount'], 1)
        for front, profile in [(full, 'full'), (light, LIGHTWEIGHT)]:
            entries = [p for c in front['data']['libraryCategories'] for p in c['pages']]
            self.assertEqual(len(entries), 2)
            large = next(item for item in entries if item['path'] == 'fps/large.hpp')
            self.assertEqual(large['profileAvailable'], profile == 'full')
            self.assertEqual(large['pagePath'], 'fps/large.hpp.html')
            self.assertEqual(entries[0]['pagePath'], 'tree/a.hpp.html')
        for name in ['tree/a.hpp', 'fps/large.hpp']:
            old, _ = split_page(pages[destination / (name + '.md')])
            new, _ = split_page(result[destination / (name + '.md')])
            self.assertEqual(old['data']['bundledCode'], new['data']['bundledCode'])
            self.assertEqual(old['data']['code'], new['data']['code'])
            self.assertEqual(new['redirect_from'][:len(old['redirect_from'])], old['redirect_from'])
        self.assertNotIn(destination / LIGHTWEIGHT / 'tree/a.hpp.md', result)
        shared, body = split_page(result[destination / 'tree/a.hpp.md'])
        self.assertIn('/lightweight/tree/a.hpp.html', shared['redirect_from'])
        self.assertTrue(shared['data']['sharedPage'])
        self.assertTrue(shared['data']['copyBundled'])
        self.assertEqual(shared['code_limit'], LIMITS[LIGHTWEIGHT])
        self.assertIsNone(shared['data']['counterpartPage'])
        self.assertEqual(shared['data']['implementationLinks'], [])
        self.assertEqual(shared['data']['_extendedDependsOn'][0]['pagePath'], 'tree/a.hpp.html')
        self.assertEqual(shared['data']['_extendedVerifiedWith'][0]['pagePath'], 'test/a.test.cpp.html')
        self.assertIn(b'/cp-library/tree/a.hpp.html', body)
        self.assertEqual(result[destination / 'test/a.test.cpp.md'], pages[destination / 'test/a.test.cpp.md'])

    def test_over_limit_library_keeps_its_catalog_position_and_full_link(self):
        destination, pages = self.fixture()
        path = destination / 'tree/a.hpp.md'
        front, body = split_page(pages[path])
        front['data']['bundledCode'] = 'x' * 65536
        pages[path] = render_page(front, body)
        result = prepare_pages(pages, destination, CONFIG)
        self.assertNotIn(destination / LIGHTWEIGHT / 'tree/a.hpp.md', result)
        for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
            front, _ = split_page(result[destination / LIGHTWEIGHT / page])
            entries = [item for category in front['data']['libraryCategories'] for item in category['pages']]
            self.assertEqual([item['path'] for item in entries], ['tree/a.hpp', 'fps/large.hpp'])
            self.assertTrue(all(not item['profileAvailable'] for item in entries))
            self.assertTrue(all(item['pagePath'] == item['path'] + '.html' for item in entries))

    def test_categories_are_independent_of_header_directories(self):
        destination, pages = self.fixture()
        expected = [('math/number_theory', ['tree/a.hpp']), ('math/combinatorics', ['fps/large.hpp'])]
        for category, names in expected:
            path = destination / (names[0] + '.md')
            front, body = split_page(pages[path])
            front['library_category'] = category
            pages[path] = render_page(front, body)
        path = destination / 'index.md'
        front, body = split_page(pages[path])
        front['data']['libraryCategories'] = [{'name': 'math', 'pages': [
            item for category in front['data']['libraryCategories'] for item in category['pages']]}]
        pages[path] = render_page(front, body)
        result = prepare_pages(pages, destination, CONFIG)
        for profile in ['', LIGHTWEIGHT]:
            front, _ = split_page(result[destination / profile / 'index.md'])
            self.assertEqual([(category['name'], [item['path'] for item in category['pages']])
                              for category in front['data']['libraryCategories'] if category['pages']], expected)

    def test_legacy_urls_keep_their_implementation_profile(self):
        destination, pages = self.fixture()
        path = destination / 'fps/large.hpp.md'
        front, body = split_page(pages[path])
        old = '/math/combinatorics/example.hpp.html'
        front['redirect_from'] = [old, '/' + LIGHTWEIGHT + old]
        pages[path] = render_page(front, body)
        config = dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp'})
        result = prepare_pages(pages, destination, config)
        full, _ = split_page(result[path])
        light, _ = split_page(result[destination / LIGHTWEIGHT / 'fps/large.hpp.md'])
        self.assertEqual(full['redirect_from'], [old])
        self.assertEqual(light['redirect_from'], ['/' + LIGHTWEIGHT + old])
        result = prepare_pages(pages, destination, CONFIG)
        full, _ = split_page(result[path])
        self.assertIn(old, full['redirect_from'])
        self.assertIn('/' + LIGHTWEIGHT + old, full['redirect_from'])

    def test_variants_share_paths_structure_and_reciprocal_links(self):
        destination, pages = self.fixture()
        config = dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp'})
        result = prepare_pages(pages, destination, config)
        full, _ = split_page(result[destination / 'fps/large.hpp.md'])
        light, _ = split_page(result[destination / LIGHTWEIGHT / 'fps/large.hpp.md'])
        original, _ = split_page(pages[destination / 'tree/a.hpp.md'])
        self.assertEqual(full['title'], 'fps/large.hpp（高速版）')
        self.assertEqual(light['title'], 'fps/large.hpp（軽量版）')
        self.assertEqual(light['documentation_of'], 'fps/large.hpp')
        self.assertEqual(light['data']['path'], 'tree/a.hpp')
        self.assertEqual(light['data']['libraryPath'], 'fps/large.hpp')
        self.assertEqual(light['data']['bundledCode'], original['data']['bundledCode'])
        self.assertEqual(full['data']['implementationKind'], '高速実装')
        self.assertEqual(light['data']['implementationKind'], '軽量実装')
        self.assertEqual(light['data']['_extendedDependsOn'][0]['pagePath'], 'lightweight/fps/large.hpp.html')
        self.assertEqual(light['data']['_extendedDependsOn'][0]['title'], light['title'])
        self.assertEqual(full['data']['implementationLinks'][0]['pagePath'], 'lightweight/fps/large.hpp.html')
        self.assertEqual(light['data']['implementationLinks'][0]['pagePath'], 'fps/large.hpp.html')
        self.assertEqual(light['data']['copyByteCount'], copy_size(original['data']['bundledCode'], page_url(config, 'fps/large.hpp.html', LIGHTWEIGHT)))
        self.assertIn(destination / 'tree/a.hpp.md', result)
        self.assertNotIn(destination / LIGHTWEIGHT / 'tree/a.hpp.md', result)
        for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
            for prefix in ['', LIGHTWEIGHT]:
                front, _ = split_page(result[destination / prefix / page])
                entries = [item for category in front['data']['libraryCategories'] for item in category['pages']]
                self.assertEqual([item['path'] for item in entries], ['fps/large.hpp'])
                self.assertTrue(entries[0]['profileAvailable'])
                self.assertEqual(entries[0]['title'], light['title'] if prefix else full['title'])

    def test_variant_titles_follow_dependency_destinations_and_fallbacks(self):
        destination, pages = self.fixture()
        normal = destination / 'fps/large.hpp.md'
        front, body = split_page(pages[normal])
        front['data']['_extendedRequiredBy'] = [{'path': 'fps/large.hpp', 'title': 'old'}]
        pages[normal] = render_page(front, body)
        judge = destination / 'test/a.test.cpp.md'
        pages[judge] = render_page({'data': {'path': 'test/a.test.cpp',
            '_extendedDependsOn': [{'path': 'fps/large.hpp', 'title': 'old'}]}}, b'test')
        config = dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp'})
        for available in [True, False]:
            if not available:
                alias = destination / 'tree/a.hpp.md'
                front, body = split_page(pages[alias])
                front['data']['bundledCode'] = 'y' * 65536
                pages[alias] = render_page(front, body)
            result = prepare_pages(pages, destination, config)
            full, _ = split_page(result[normal])
            self.assertEqual(full['data']['_extendedRequiredBy'][0]['title'], full['title'])
            verification, _ = split_page(result[judge])
            self.assertEqual(verification['data']['_extendedDependsOn'][0]['title'], full['title'])
            index, _ = split_page(result[destination / LIGHTWEIGHT / 'index.md'])
            entry = index['data']['libraryCategories'][1]['pages'][0]
            linked, _ = split_page(result[destination / (entry['pagePath'].removesuffix('.html') + '.md')])
            self.assertEqual(entry['title'], linked['title'])
            if available:
                self.assertEqual(linked['title'], 'fps/large.hpp（軽量版）')
            else:
                self.assertEqual(linked['title'], 'fps/large.hpp（高速版）')

    def test_shared_lightweight_header_can_back_separate_algorithm_pages(self):
        destination, pages = self.fixture()
        second = destination / 'fps/second.hpp.md'
        front, body = split_page(pages[destination / 'fps/large.hpp.md'])
        front['data']['path'] = 'fps/second.hpp'
        front['title'] = 'Second algorithm'
        pages[second] = render_page(front, body)
        config = dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp', 'fps/second.hpp': 'tree/a.hpp'})
        result = prepare_pages(pages, destination, config)
        legacy, _ = split_page(result[destination / 'tree/a.hpp.md'])
        self.assertEqual({item['pagePath'] for item in legacy['data']['implementationLinks']}, {'fps/large.hpp.html', 'fps/second.hpp.html'})
        for name in config['implementation_variants']:
            front, _ = split_page(result[destination / LIGHTWEIGHT / (name + '.md')])
            self.assertEqual(front['data']['counterpartPage'], name + '.html')

    def test_identical_variant_uses_one_page(self):
        destination, pages = self.fixture()
        normal = destination / 'fps/large.hpp.md'
        front, body = split_page(pages[normal])
        shared, _ = split_page(pages[destination / 'tree/a.hpp.md'])
        front['data']['bundledCode'] = shared['data']['bundledCode']
        pages[normal] = render_page(front, body)
        result = prepare_pages(pages, destination, dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp'}))
        self.assertNotIn(destination / LIGHTWEIGHT / 'fps/large.hpp.md', result)
        front, _ = split_page(result[normal])
        self.assertEqual(front['title'], 'fps/large.hpp')
        self.assertTrue(front['data']['sharedPage'])
        self.assertIsNone(front['data']['counterpartPage'])
        self.assertEqual(front['data']['implementationLinks'], [])
        self.assertIn('/lightweight/fps/large.hpp.html', front['redirect_from'])
        for prefix in ['', LIGHTWEIGHT]:
            front, _ = split_page(result[destination / prefix / 'index.md'])
            entry = front['data']['libraryCategories'][1]['pages'][0]
            self.assertEqual(entry['pagePath'], 'fps/large.hpp.html')
            self.assertEqual(entry['title'], 'fps/large.hpp')

    def test_common_page_uses_its_shorter_canonical_url_at_limit(self):
        destination, pages = self.fixture()
        name = 'tree/a.hpp'
        path = destination / (name + '.md')
        front, body = split_page(pages[path])
        front['data']['bundledCode'] = 'x' * (LIMITS[LIGHTWEIGHT] - copy_size('', page_url(CONFIG, name + '.html')))
        front['redirect_from'] = '/legacy.html'
        pages[path] = render_page(front, body)
        result = prepare_pages(pages, destination, CONFIG)
        shared, _ = split_page(result[path])
        self.assertTrue(shared['data']['lightweightAvailable'])
        self.assertEqual(shared['data']['copyByteCount'], LIMITS[LIGHTWEIGHT])
        self.assertEqual(shared['redirect_from'], ['/legacy.html', '/lightweight/tree/a.hpp.html'])
        self.assertNotIn(destination / LIGHTWEIGHT / (name + '.md'), result)

    def test_light_variant_links_to_common_dependencies_and_legacy_urls(self):
        destination, pages = self.fixture()
        mint = destination / 'math/mint.hpp.md'
        front, _ = split_page(pages[destination / 'tree/a.hpp.md'])
        front['data']['path'] = 'math/mint.hpp'
        front['data']['bundledCode'] = 'int mint;'
        pages[mint] = render_page(front, b'')
        alias = destination / 'tree/a.hpp.md'
        front, _ = split_page(pages[alias])
        front['data']['_extendedDependsOn'] = [{'path': 'math/mint.hpp'}]
        pages[alias] = render_page(front, b'[mint](https://qandori.github.io/cp-library/lightweight/math/mint.hpp.html#usage) [fast](../fps/large.hpp.html)')
        config = dict(CONFIG, implementation_variants={'fps/large.hpp': 'tree/a.hpp'})
        result = prepare_pages(pages, destination, config)
        light, body = split_page(result[destination / LIGHTWEIGHT / 'fps/large.hpp.md'])
        self.assertEqual(light['data']['_extendedDependsOn'][0]['pagePath'], 'math/mint.hpp.html')
        self.assertIn(b'[mint](/cp-library/math/mint.hpp.html#usage)', body)
        self.assertIn(b'[fast](/cp-library/fps/large.hpp.html)', body)

    def test_invalid_variant_pair_is_rejected(self):
        destination, pages = self.fixture()
        for variants in [{'missing.hpp': 'tree/a.hpp'}, {'tree/a.hpp': 'missing.hpp'}, {'tree/a.hpp': 'tree/a.hpp'}]:
            with self.assertRaises(ValueError):
                prepare_pages(pages, destination, dict(CONFIG, implementation_variants=variants))

    def test_explicit_implementation_link_keeps_its_destination(self):
        body = b'[fast](../fps/large.hpp.html) [other](../tree/a.hpp.html)'
        actual = rewrite_links(body, 'docs/light.md', CONFIG, {}, {'fps/large.hpp.html', 'tree/a.hpp.html'}, LIGHTWEIGHT,
                               {'fps/large.hpp.html': 'fps/large.hpp.html'})
        self.assertIn(b'[fast](/cp-library/fps/large.hpp.html)', actual)
        self.assertIn(b'[other](/cp-library/lightweight/tree/a.hpp.html)', actual)

    def test_standalone_guides_link_to_generated_library_pages(self):
        destination, pages = self.fixture()
        guide = destination / 'docs/source_size.md'
        pages[guide] = render_page({'title': 'Guide'}, b'[a](a.md)\n')
        result = prepare_pages(pages, destination, CONFIG, targets={'docs/a.md': 'tree/a.hpp.html'})
        _, body = split_page(result[guide])
        self.assertEqual(body, b'[a](/cp-library/tree/a.hpp.html)\n')

    def test_relative_and_old_links_stay_in_matching_site(self):
        body = b'[a](geometry_point.md#usage) [b](../tree/b.hpp.html) [old](/cp-library/old/a.hpp.html) [external](https://example.com/a) [test](../test/a.test.cpp)\n`[code](geometry_point.md)`\n```cpp\n[code](geometry_point.md)\n```\n'
        targets = {'docs/geometry_point.md': 'geometry/point.hpp.html', 'old/a.hpp.html': 'geometry/point.hpp.html', 'test/a.test.cpp': 'test/a.test.cpp.html'}
        available = {'geometry/point.hpp.html'}
        actual = rewrite_links(body, 'docs/geometry.md', CONFIG, targets, available, LIGHTWEIGHT)
        self.assertIn(b'[a](/cp-library/lightweight/geometry/point.hpp.html#usage)', actual)
        self.assertIn(b'[b](../tree/b.hpp.html)', actual)
        self.assertIn(b'[old](/cp-library/lightweight/geometry/point.hpp.html)', actual)
        self.assertIn(b'[external](https://example.com/a)', actual)
        self.assertIn(b'[test](/cp-library/test/a.test.cpp.html)', actual)
        self.assertIn(b'`[code](geometry_point.md)`', actual)
        self.assertIn(b'```cpp\n[code](geometry_point.md)', actual)

    def test_root_baseurl_and_full_only_destination(self):
        actual = rewrite_links(b'[a](a.md)', 'docs/index.md', {'url': 'https://example.com'},
                               {'docs/a.md': 'tree/a.hpp.html'}, set(), LIGHTWEIGHT)
        self.assertEqual(actual, b'[a](/tree/a.hpp.html)')

    def test_documentation_paths_are_relative_even_with_absolute_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            (root / 'tree').mkdir()
            (root / 'docs').mkdir()
            (root / 'test').mkdir()
            (root / 'tree/a.hpp').write_text('int a;\n')
            (root / 'test/a.test.cpp').write_text('int main() {}\n')
            path = root / 'docs/a.md'
            for aliases in ['/old/a.hpp.html', ['/old/a.hpp.html']]:
                with self.subTest(aliases=aliases):
                    path.write_bytes(render_page({'documentation_of': '//tree/a.hpp', 'redirect_from': aliases}, b''))
                    targets, origins = documentation_links(root, [path])
                    self.assertEqual(targets['docs/a.md'], 'tree/a.hpp.html')
                    self.assertEqual(targets['old/a.hpp.html'], 'tree/a.hpp.html')
                    self.assertEqual(targets['test/a.test.cpp'], 'test/a.test.cpp.html')
                    self.assertEqual(origins['tree/a.hpp.html'], 'docs/a.md')
                    self.assertEqual(set(targets), {'docs/a.md', 'old/a.hpp.html', 'test/a.test.cpp'})

    def test_stale_pages_are_removed_without_touching_other_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory)
            keep = destination / LIGHTWEIGHT / 'tree/a.hpp.md'
            stale = destination / LIGHTWEIGHT / 'fps/large.hpp.md'
            asset = destination / LIGHTWEIGHT / 'image.png'
            normal = destination / 'fps/large.hpp.md'
            for path in [keep, stale, asset, normal]:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b'content')
            remove_stale_pages(destination, {keep: b'new'})
            remove_stale_pages(destination, {keep: b'new'})
            self.assertFalse(stale.exists())
            self.assertFalse(normal.exists())
            for path in [keep, asset]:
                self.assertEqual(path.read_bytes(), b'content')

    def test_size_cli_checks_file_bytes_at_boundary(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'submission.cpp'
            command = [sys.executable, str(ROOT / 'tools/check_source_size.py'), str(source), '--limit', '64']
            source.write_bytes(b'x' * 65536)
            self.assertEqual(subprocess.run(command, capture_output=True).returncode, 0)
            source.write_bytes(b'x' * 65537)
            self.assertEqual(subprocess.run(command, capture_output=True).returncode, 1)
            source.write_bytes(('円' * 22000).encode())
            self.assertEqual(subprocess.run(command, capture_output=True).returncode, 1)


if __name__ == '__main__':
    unittest.main()
