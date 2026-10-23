import argparse
import json
from pathlib import Path

from size_profiles import LIGHTWEIGHT, LIMITS, copy_size, metadata, page_url, split_page


def check(root):
    from affected import LIBRARIES

    destination = root / '.verify-helper/markdown'
    manifest = json.loads((destination / 'assets/library-sizes.json').read_text())
    config, _ = split_page(b'---\n' + (root / '.verify-helper/docs/_config.yml').read_bytes() + b'\n---\n')
    headers = {path.relative_to(root).as_posix() for folder in LIBRARIES for path in (root / folder).rglob('*.hpp')}
    assert manifest['limits'] == LIMITS
    assert set(manifest['libraries']) == headers, 'Size inventory does not match library headers'
    actual_full = {path.relative_to(destination).as_posix().removesuffix('.md') for path in destination.rglob('*.hpp.md')
                   if path.relative_to(destination).parts[0] != LIGHTWEIGHT}
    assert actual_full == headers, 'Stale full-site library pages remain'
    variants = config.get('implementation_variants', {})
    assert manifest['implementationVariants'] == variants
    aliases = set(variants.values())
    expected = headers - aliases
    assert set(manifest['catalog']) == expected
    full_pages = {}
    for name, size in manifest['libraries'].items():
        front, _ = split_page((destination / (name + '.md')).read_bytes())
        data = front['data']
        full_pages[name] = front
        assert 'Traceback (most recent call last)' not in data['bundledCode'], name + ': bundle failed'
        assert front['size_profile'] == 'full'
        assert metadata(data['bundledCode'], name, config, lightweight_path=name + '.html') == size, name + ': size mismatch'
        assert data['copyByteCount'] == size['copyBytes']['full']
        assert size['copyBytes']['full'] < LIMITS['full'], name + ': exceeds 256 KiB'
    different = {name for name, other in variants.items()
                 if full_pages[name]['data']['bundledCode'] != full_pages[other]['data']['bundledCode']}
    assert manifest['differentImplementations'] == sorted(different)
    page_titles = {name + '.html': page['title'] for name, page in full_pages.items()}
    documents = list(full_pages.values())
    eligible = set()
    for name, size in manifest['catalog'].items():
        implementation = variants.get(name, name)
        assert size['lightweightImplementation'] == implementation
        code = full_pages[implementation]['data']['bundledCode']
        page_path = (LIGHTWEIGHT + '/' if name in different else '') + name + '.html'
        assert size['lightweightPage'] == page_path
        assert size['differentImplementation'] == (name in different)
        light_size = copy_size(code, page_url(config, page_path))
        assert size['copyBytes'] == {'full': manifest['libraries'][name]['copyBytes']['full'], LIGHTWEIGHT: light_size}
        assert size['lightweightAvailable'] == (light_size <= LIMITS[LIGHTWEIGHT])
        page = destination / LIGHTWEIGHT / (name + '.md')
        separate = name in different and size['lightweightAvailable']
        assert page.exists() == separate, name + ': lightweight page mismatch'
        full = full_pages[name]
        if name in different:
            assert full['title'].endswith('（高速版）'), name + ': missing implementation label'
        counterpart = full['data']['counterpartPage']
        assert counterpart == (page_path if separate else None)
        assert full['data']['sharedPage'] == (name not in different)
        if size['lightweightAvailable']:
            eligible.add(name)
            if not separate:
                assert '/' + LIGHTWEIGHT + '/' + name + '.html' in full['redirect_from']
                assert full['data']['implementationLinks'] == []
                assert full['data']['copyBundled'] and full['code_limit'] == LIMITS[LIGHTWEIGHT]
                assert 'fps_backend.hpp"' not in full['data']['bundledCode'], name + ': common lightweight page includes full FPS'
        if page.exists():
            light, _ = split_page(page.read_bytes())
            documents.append(light)
            assert light['title'] == full['title'].removesuffix('（高速版）') + '（軽量版）'
            assert light['documentation_of'] == name
            page_titles[page_path] = light['title']
            assert light['size_profile'] == LIGHTWEIGHT and light['code_limit'] == LIMITS[LIGHTWEIGHT]
            assert light['data']['path'] == implementation
            assert light['data']['libraryPath'] == name
            assert light['data']['bundledCode'] == code, name + ': wrong implementation'
            assert 'fps_backend.hpp"' not in code, name + ': lightweight page includes full FPS'
            assert light['data']['copyByteCount'] == light_size
            assert light['data']['counterpartPage'] == name + '.html'
            assert not light.get('redirect_from') and not light.get('permalink')
            assert light['data']['copyBundled'] and not light['data']['sharedPage']
            if name in different:
                assert full['data']['implementationKind'] == '高速実装'
                assert light['data']['implementationKind'] == '軽量実装'
                assert full['data']['implementationLinks'][0]['pagePath'] == counterpart
                assert light['data']['implementationLinks'][0]['pagePath'] == name + '.html'
    for front in documents:
        for key in ['_extendedDependsOn', '_extendedRequiredBy']:
            for item in front['data'].get(key, []):
                if item.get('pagePath') in page_titles:
                    assert item['title'] == page_titles[item['pagePath']], item['path'] + ': dependency title differs from page'
    expected_light = eligible & different
    actual_light = {path.relative_to(destination / LIGHTWEIGHT).as_posix().removesuffix('.md')
                    for path in (destination / LIGHTWEIGHT).rglob('*.hpp.md')}
    assert actual_light == expected_light, 'Stale lightweight library pages remain'
    for name in aliases:
        page = destination / LIGHTWEIGHT / (name + '.md')
        assert not page.exists(), name + ': duplicate implementation page remains'
        full = full_pages[name]
        assert full['data']['sharedPage']
        if manifest['libraries'][name]['lightweightAvailable']:
            assert '/' + LIGHTWEIGHT + '/' + name + '.html' in full['redirect_from']
            assert full['data']['copyBundled'] and full['code_limit'] == LIMITS[LIGHTWEIGHT]
        assert {item['pagePath'] for item in full['data']['implementationLinks']} == {
            normal + '.html' for normal in different if variants[normal] == name}
    for page in ['index.md', 'components/index.md', 'data_structure/advanced/index.md']:
        structures = []
        for profile in ['full', LIGHTWEIGHT]:
            prefix = '' if profile == 'full' else LIGHTWEIGHT
            front, _ = split_page((destination / prefix / page).read_bytes())
            categories = front['data']['libraryCategories']
            structures.append([(category['name'], [item['path'] for item in category['pages']]) for category in categories])
            entries = [item for category in categories for item in category['pages']]
            assert {item['path'] for item in entries} == expected, page + ': catalog mismatch'
            assert len(entries) == len(expected), page + ': duplicate entry'
            for item in entries:
                name = item['path']
                usable = profile == 'full' or name in eligible
                assert item['profileAvailable'] == usable
                assert item['pagePath'] == (manifest['catalog'][name]['lightweightPage'] if profile == LIGHTWEIGHT and usable else name + '.html')
                assert (destination / (item['pagePath'].removesuffix('.html') + '.md')).is_file(), name + ': missing link'
                assert item['title'] == page_titles[item['pagePath']], name + ': catalog title differs from page'
        assert structures[0] == structures[1], page + ': profile structures differ'
    print(f'Checked {len(headers)} headers, {len(expected)} identical catalog positions, {len(eligible)} lightweight implementations and {len(different)} separate implementation pairs')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    check(args.root.resolve())


if __name__ == '__main__':
    main()
