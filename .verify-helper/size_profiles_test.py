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

    def test_separate_catalogs_and_unchanged_sources(self):
        destination, pages = self.fixture()
        originals = dict(pages)
        result = prepare_pages(pages, destination, CONFIG)
        self.assertEqual(pages, originals)
        self.assertNotIn(destination / LIGHTWEIGHT / 'fps/large.hpp.md', result)
        full, _ = split_page(result[destination / 'index.md'])
        light, _ = split_page(result[destination / LIGHTWEIGHT / 'index.md'])
        self.assertEqual(full['data']['profileLibraryCount'], 2)
        self.assertEqual(light['data']['profileLibraryCount'], 1)
        for front, profile in [(full, 'full'), (light, LIGHTWEIGHT)]:
            entries = [p for c in front['data']['libraryCategories'] for p in c['pages']]
            self.assertEqual(len(entries), 2 if profile == 'full' else 1)
            self.assertEqual(entries[0]['pagePath'], ('lightweight/' if profile == LIGHTWEIGHT else '') + 'tree/a.hpp.html')
        for name in ['tree/a.hpp', 'fps/large.hpp']:
            old, _ = split_page(pages[destination / (name + '.md')])
            new, _ = split_page(result[destination / (name + '.md')])
            self.assertEqual(old['data']['bundledCode'], new['data']['bundledCode'])
            self.assertEqual(old['data']['code'], new['data']['code'])
            self.assertEqual(old['redirect_from'], new['redirect_from'])
        light, body = split_page(result[destination / LIGHTWEIGHT / 'tree/a.hpp.md'])
        self.assertNotIn('redirect_from', light)
        self.assertEqual(light['data']['_extendedDependsOn'][0]['pagePath'], 'lightweight/tree/a.hpp.html')
        self.assertEqual(light['data']['_extendedVerifiedWith'][0]['pagePath'], 'test/a.test.cpp.html')
        self.assertIn(b'/cp-library/lightweight/tree/a.hpp.html', body)
        self.assertEqual(result[destination / 'test/a.test.cpp.md'], pages[destination / 'test/a.test.cpp.md'])

    def test_downgraded_library_disappears_from_all_lightweight_catalogs(self):
        destination, pages = self.fixture()
        path = destination / 'tree/a.hpp.md'
        front, body = split_page(pages[path])
        front['data']['bundledCode'] = 'x' * 65536
        pages[path] = render_page(front, body)
        result = prepare_pages(pages, destination, CONFIG)
        self.assertNotIn(destination / LIGHTWEIGHT / 'tree/a.hpp.md', result)
        for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
            front, _ = split_page(result[destination / LIGHTWEIGHT / page])
            self.assertTrue(all(not category['pages'] for category in front['data']['libraryCategories']))

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
            path.write_bytes(render_page({'documentation_of': '//tree/a.hpp', 'redirect_from': ['/old/a.hpp.html']}, b''))
            targets, origins = documentation_links(root, [path])
            self.assertEqual(targets['docs/a.md'], 'tree/a.hpp.html')
            self.assertEqual(targets['old/a.hpp.html'], 'tree/a.hpp.html')
            self.assertEqual(targets['test/a.test.cpp'], 'test/a.test.cpp.html')
            self.assertEqual(origins['tree/a.hpp.html'], 'docs/a.md')

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
            for path in [keep, asset, normal]:
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
