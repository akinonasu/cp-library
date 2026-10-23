from html.parser import HTMLParser
from importlib.util import find_spec
from pathlib import Path
import re
import tempfile
import unittest
from unittest.mock import patch

from code_highlight import highlight, prepare_highlights
from documentation import fast_front_matter
from size_profiles import render_page, split_page


class DocumentationExamples(unittest.TestCase):
    def test_repository_docs_do_not_open_liquid_variables(self):
        root = Path(__file__).resolve().parents[1]
        for path in sorted((root / 'docs').rglob('*.md')):
            body = re.sub(r'{%\s*raw\s*%}.*?{%\s*endraw\s*%}', '', path.read_text(), flags=re.DOTALL)
            with self.subTest(path=path.relative_to(root).as_posix()):
                self.assertNotIn('{{', body)


class Text(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.text = ''
        self.scripts = 0
    def handle_data(self, data):
        self.text += data
    def handle_starttag(self, tag, attrs):
        self.scripts += tag == 'script'


@unittest.skipUnless(find_spec('pygments'), 'Documentation dependencies are not installed')
class CodeHighlight(unittest.TestCase):
    def test_code_roundtrip_and_no_html_or_liquid_injection(self):
        for code in ['', 'int x;', '\n\nint x;\n\n', '\tint x = 2;\n', '// 日本語\n',
                     'auto s = "</code><script>x</script> & {{ page.title }} {% endraw %}";\n']:
            with self.subTest(code=code):
                markup = highlight(code).decode()
                parser = Text(); parser.feed(markup)
                self.assertEqual(parser.text.removesuffix('\n'), code)
                self.assertEqual(parser.scripts, 0)
                self.assertNotIn('{{', markup)
                self.assertNotIn('{%', markup)

    def test_shared_code_is_generated_once_and_stale_html_is_removed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            front = {'data': {'_pathExtension': 'hpp', 'code': 'int a;', 'bundledCode': 'int a;'}}
            pages = {root / 'a.md': render_page(front, b'prose'), root / 'b.md': render_page(front, b'other prose')}
            stale = root / '_includes/generated-code/stale.html'
            stale.parent.mkdir(parents=True); stale.write_text('old code')
            result = prepare_highlights(pages, root)
            self.assertFalse(stale.exists())
            self.assertEqual(sum(path.suffix == '.html' for path in result), 1)
            data, body = split_page(result[root / 'a.md'])
            self.assertEqual(body, b'prose')
            self.assertEqual(data['data']['codeHighlight'], data['data']['bundleHighlight'])
            self.assertEqual(data['data']['bundledCode'], 'int a;')

    def test_fast_yaml_preserves_values_and_restores_original_functions(self):
        from onlinejudge_verify.documentation import front_matter
        original = front_matter.merge_front_matter
        metadata = {'data': {'bundledCode': 'int x;\r\n', 'enabled': True}, 'title': '日本語', 'redirect_from': ['/old.html']}
        with fast_front_matter():
            source = front_matter.merge_front_matter(metadata, b'body\n')
            self.assertEqual(front_matter.split_front_matter(source), (metadata, b'body\n'))
        self.assertIs(front_matter.merge_front_matter, original)

    def test_cached_highlights_are_reused_and_changed_code_is_rebuilt(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            cache = root / 'cache'
            front = {'data': {'_pathExtension': 'hpp', 'code': 'int a;', 'bundledCode': 'int a;'}}
            pages = {root / 'a.md': render_page(front, b'prose')}
            first = prepare_highlights(pages, root, cache_dir=cache)
            with patch('code_highlight.highlight', side_effect=AssertionError('cached code highlighted')):
                self.assertEqual(prepare_highlights(pages, root, cache_dir=cache), first)
            front['data']['code'] = 'int b;'
            with patch('code_highlight.highlight', wraps=highlight) as execute:
                prepare_highlights({root / 'a.md': render_page(front, b'prose')}, root, cache_dir=cache)
                execute.assert_called_once_with('int b;')
