import argparse
from datetime import datetime, timezone
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import urllib.request


class Rows(HTMLParser):
    def __init__(self, source):
        super().__init__()
        self.rows, self.row, self.cell = [], [], None
        self.feed(source)
    def handle_starttag(self, tag, attrs):
        if tag == 'tr': self.row = []
        if tag == 'td': self.cell = []
    def handle_data(self, data):
        if self.cell is not None: self.cell.append(data)
    def handle_endtag(self, tag):
        if tag == 'td' and self.cell is not None:
            self.row.append(' '.join(''.join(self.cell).split())); self.cell = None
        if tag == 'tr' and self.row: self.rows.append(self.row)


def get(url):
    with urllib.request.urlopen(url, timeout=60) as response: return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    url = 'https://yukicoder.me/problems/no/3668/submissions?status=AC&sort_time=enabled'
    listing = get(url)
    (args.cache / 'listing.html').write_bytes(listing)
    rows = Rows(listing.decode()).rows
    selected = [r for r in rows if any(c == 'AC' for c in r) and any('C++' in c for c in r)][:3]
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problem': 'https://yukicoder.me/problems/no/3668',
              'listing_url': url, 'listing_sha256': hashlib.sha256(listing).hexdigest(), 'columns': ['id', 'date', 'filter', 'user', 'problem', 'language', 'status', 'time', 'length'], 'references': []}
    for row in selected:
        identity = int(row[0])
        source = get(f'https://yukicoder.me/submissions/{identity}/source')
        (args.cache / f'{identity}.cpp').write_bytes(source)
        result['references'].append({'id': identity, 'row': row, 'url': f'https://yukicoder.me/submissions/{identity}',
                                     'source_sha256': hashlib.sha256(source).hexdigest()})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + '\n')
    print(result['references'])


if __name__ == '__main__': main()
