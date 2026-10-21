import argparse
import json
from pathlib import Path

from size_profiles import LIGHTWEIGHT, LIMITS, metadata, split_page


def check(root):
    from affected import LIBRARIES

    destination = root / '.verify-helper/markdown'
    manifest = json.loads((destination / 'assets/library-sizes.json').read_text())
    config, _ = split_page(b'---\n' + (root / '.verify-helper/docs/_config.yml').read_bytes() + b'\n---\n')
    headers = {path.relative_to(root).as_posix() for folder in LIBRARIES for path in (root / folder).rglob('*.hpp')}
    assert manifest['limits'] == LIMITS
    assert set(manifest['libraries']) == headers, 'Size inventory does not match library headers'
    eligible = set()
    for name, size in manifest['libraries'].items():
        front, _ = split_page((destination / (name + '.md')).read_bytes())
        data = front['data']
        assert 'Traceback (most recent call last)' not in data['bundledCode'], name + ': bundle failed'
        assert front['size_profile'] == 'full'
        assert metadata(data['bundledCode'], name, config) == size, name + ': size mismatch'
        assert data['copyByteCount'] == size['copyBytes']['full']
        assert size['copyBytes']['full'] < LIMITS['full'], name + ': exceeds 256 KiB'
        page = destination / LIGHTWEIGHT / (name + '.md')
        assert page.exists() == size['lightweightAvailable'], name + ': lightweight page mismatch'
        if page.exists():
            eligible.add(name)
            light, _ = split_page(page.read_bytes())
            assert light['size_profile'] == LIGHTWEIGHT and light['code_limit'] == LIMITS[LIGHTWEIGHT]
            assert light['data']['copyByteCount'] <= LIMITS[LIGHTWEIGHT], name + ': exceeds 64 KiB'
            assert light['data']['bundledCode'] == data['bundledCode'], name + ': code differs'
            assert light['data']['copyByteCount'] == size['copyBytes'][LIGHTWEIGHT]
            assert not light.get('redirect_from') and not light.get('permalink')
    for profile, expected in [('full', headers), (LIGHTWEIGHT, eligible)]:
        prefix = '' if profile == 'full' else LIGHTWEIGHT
        for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
            front, _ = split_page((destination / prefix / page).read_bytes())
            entries = [item for category in front['data']['libraryCategories'] for item in category['pages']]
            assert {item['path'] for item in entries} == expected, page + ': catalog mismatch'
            assert len(entries) == len(expected), page + ': duplicate entry'
            for item in entries:
                target = destination / item['pagePath'].removesuffix('.html')
                assert target.with_name(target.name + '.md').is_file(), item['path'] + ': missing link'
    print(f'Checked {len(headers)} libraries: {len(eligible)} within 64 KiB; all within 256 KiB')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    check(args.root.resolve())


if __name__ == '__main__':
    main()
