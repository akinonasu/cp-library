from pathlib import Path
import tempfile
import unittest

from library_categories import check


class LibraryCategories(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        config = self.root / '.verify-helper/docs/_config.yml'
        config.parent.mkdir(parents=True)
        config.write_text('library_categories: [tree, string, string/substring, data_structure/persistent, components]\n')
        (self.root / 'docs').mkdir()

    def document(self, name, category, advanced=None):
        header = self.root / name
        header.parent.mkdir(exist_ok=True)
        header.write_text('')
        (self.root / 'docs' / (header.stem + '.md')).write_text(
            '---\ndocumentation_of: //' + name + '\n' +
            (f'library_category: {category}\n' if category else '') +
            (f'library_advanced: {advanced}\n' if advanced is not None else '') + '---\n')

    def test_persistent_tree_uses_persistent_category(self):
        self.document('tree/persistent_link_cut_tree.hpp', 'data_structure/persistent')
        self.assertEqual(check(self.root)['tree/persistent_link_cut_tree.hpp'], 'data_structure/persistent')
        self.document('tree/persistent_link_cut_tree.hpp', 'tree')
        with self.assertRaisesRegex(AssertionError, 'persistent category required'):
            check(self.root)

    def test_missing_or_unknown_category(self):
        for category in (None, 'typo'):
            self.document('tree/a.hpp', category)
            with self.assertRaisesRegex(AssertionError, 'missing or unknown'):
                check(self.root)

    def test_string_comparisons_keep_the_string_category(self):
        name = 'string/data_dependent_tree.hpp'
        for category in ('string', 'string/substring'):
            self.document(name, category)
            (self.root / name).write_text('struct DataDependentTree { using root_type = int; };\n')
            self.assertEqual(check(self.root)[name], category)
        self.document(name, 'data_structure/persistent')
        with self.assertRaisesRegex(AssertionError, 'string category required'):
            check(self.root)

    def test_advanced_flag_keeps_its_purpose_category(self):
        self.document('tree/a.hpp', 'tree', True)
        self.assertEqual(check(self.root)['tree/a.hpp'], 'tree')

    def test_advanced_flag_requires_a_boolean(self):
        for advanced in ('"true"', '0', 'null'):
            self.document('tree/a.hpp', 'tree', advanced)
            with self.assertRaisesRegex(AssertionError, 'must be a boolean'):
                check(self.root)

    def test_all_headers_need_a_documented_category(self):
        self.document('tree/a.hpp', 'tree')
        (self.root / 'tree/b.hpp').write_text('')
        with self.assertRaisesRegex(AssertionError, 'Missing category'):
            check(self.root)

    def test_auxiliary_components_stay_auxiliary(self):
        self.document('components/a.hpp', 'tree')
        with self.assertRaises(AssertionError):
            check(self.root)

    def test_auxiliary_components_are_not_advanced(self):
        self.document('components/a.hpp', 'components', True)
        with self.assertRaisesRegex(AssertionError, 'auxiliary components cannot be advanced'):
            check(self.root)

    def test_variants_keep_the_same_category(self):
        self.document('tree/a.hpp', 'tree')
        self.document('tree/b.hpp', 'data_structure/persistent')
        config = self.root / '.verify-helper/docs/_config.yml'
        config.write_text(config.read_text() + 'implementation_variants: {tree/a.hpp: tree/b.hpp}\n')
        with self.assertRaisesRegex(AssertionError, 'Implementation categories differ'):
            check(self.root)

    def test_variants_keep_the_same_advanced_flag(self):
        self.document('tree/a.hpp', 'tree', True)
        self.document('tree/b.hpp', 'tree')
        config = self.root / '.verify-helper/docs/_config.yml'
        config.write_text(config.read_text() + 'implementation_variants: {tree/a.hpp: tree/b.hpp}\n')
        with self.assertRaisesRegex(AssertionError, 'Implementation advanced flags differ'):
            check(self.root)
        self.document('tree/b.hpp', 'tree', True)
        check(self.root)

    def test_readme_matches_the_site(self):
        self.document('tree/a.hpp', 'tree')
        readme = self.root / 'README.md'
        readme.write_text('### tree\n\n| `tree/a.hpp` | A |\n')
        check(self.root)
        readme.write_text('### data_structure/persistent\n\n| `tree/a.hpp` | A |\n')
        with self.assertRaisesRegex(AssertionError, 'README category differs'):
            check(self.root)

    def test_readme_contains_every_library(self):
        self.document('tree/a.hpp', 'tree')
        self.document('tree/b.hpp', 'tree')
        readme = self.root / 'README.md'
        readme.write_text('### tree\n\n| `tree/a.hpp` | A |\n')
        with self.assertRaisesRegex(AssertionError, 'Missing README entries'):
            check(self.root)
        readme.write_text(readme.read_text() + '| `tree/b.hpp` | B |\n')
        check(self.root)

    def test_readme_advanced_marker_matches_metadata(self):
        self.document('tree/a.hpp', 'tree', True)
        readme = self.root / 'README.md'
        readme.write_text('### tree\n\n| `tree/a.hpp` | A |\n')
        with self.assertRaisesRegex(AssertionError, 'README advanced flag differs'):
            check(self.root)
        readme.write_text('### tree\n\n| `tree/a.hpp` | A（応用） |\n')
        check(self.root)
        self.document('tree/a.hpp', 'tree', False)
        with self.assertRaisesRegex(AssertionError, 'README advanced flag differs'):
            check(self.root)

    def test_readme_rejects_unknown_libraries_without_a_section(self):
        self.document('tree/a.hpp', 'tree')
        (self.root / 'README.md').write_text('| `tree/unknown.hpp` | Unknown |\n')
        with self.assertRaisesRegex(AssertionError, 'Unknown README library'):
            check(self.root)


if __name__ == '__main__':
    unittest.main()
