from html.parser import HTMLParser
from pathlib import Path
import re

import yaml

ROOT = Path(__file__).resolve().parents[2]
DIRECTORY = 'data_structure/advanced'


class Catalog(HTMLParser):
    def __init__(self, text):
        super().__init__()
        self.sections = []
        self.current = None
        self.heading = False
        self.feed(text)

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == 'section' and 'library-category' in attrs.get('class', '').split():
            self.current = {'title': '', 'links': []}
            self.sections.append(self.current)
        elif self.current is not None:
            if tag == 'h2':
                self.heading = True
            elif tag == 'a':
                self.current['links'].append(attrs.get('href'))

    def handle_endtag(self, tag):
        if tag == 'section':
            self.current = None
        elif tag == 'h2':
            self.heading = False

    def handle_data(self, data):
        if self.current is not None and self.heading:
            self.current['title'] += data


def main():
    headers = sorted(path.relative_to(ROOT).as_posix() for path in (ROOT / DIRECTORY).glob('*.hpp'))
    expected = {'/cp-library/' + name + '.html' for name in headers}
    site = ROOT / '.verify-helper/site'
    main_catalog = Catalog((site / 'index.html').read_text()).sections
    assert main_catalog[-1]['title'].strip() == '応用ライブラリ'
    assert set(main_catalog[-1]['links']) == expected
    all_links = [link for section in main_catalog for link in section['links']]
    assert all(all_links.count(link) == 1 for link in expected)
    advanced = Catalog((site / DIRECTORY / 'index.html').read_text()).sections
    assert len(advanced) == 1 and set(advanced[0]['links']) == expected
    auxiliary = Catalog((site / 'components/index.html').read_text()).sections
    assert not expected.intersection(link for section in auxiliary for link in section['links'])
    for name in headers:
        page = (site / (name + '.html')).read_text()
        assert 'id="code-bundle"' in page and 'id="code-copy"' in page
        docs = [path for path in (ROOT / 'docs').glob('*.md')
                if 'documentation_of: //' + name + '\n' in path.read_text()]
        assert len(docs) == 1, name
        front = yaml.safe_load(docs[0].read_text().split('---', 2)[1])
        for redirect in front.get('redirect_from', []):
            content = (site / redirect.lstrip('/')).read_text()
            assert '/cp-library/' + name + '.html' in content, redirect
    print(f'Passed: {len(headers)} advanced libraries, last category, dedicated page, redirects, Bundle and Copy')


if __name__ == '__main__':
    main()
