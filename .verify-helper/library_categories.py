from pathlib import Path
import re

import yaml

from affected import LIBRARIES


def check(root):
    root = Path(root)
    config = yaml.safe_load((root / '.verify-helper/docs/_config.yml').read_text())
    allowed = config['library_categories']
    assert len(allowed) == len(set(allowed)), 'Duplicate library categories'
    headers = {p.relative_to(root).as_posix() for folder in LIBRARIES
               for p in (root / folder).rglob('*.hpp')}
    categories, advanced_libraries = {}, {}
    for path in (root / 'docs').rglob('*.md'):
        source = path.read_text()
        if not source.startswith('---'):
            continue
        front = yaml.safe_load(source.split('---', 2)[1])
        if not front.get('documentation_of'):
            continue
        name = front['documentation_of'].removeprefix('//')
        category = front.get('library_category')
        advanced = front.get('library_advanced', False)
        assert category in allowed, f'{path}: library_category is missing or unknown: {category}'
        assert type(advanced) is bool, f'{path}: library_advanced must be a boolean'
        assert name not in categories, f'Duplicate category declaration: {name}'
        assert name in headers, f'Unknown library: {name}'
        if name.startswith('components/'):
            assert category == 'components' or category.startswith('components/'), name
            assert not advanced, name + ': auxiliary components cannot be advanced'
        else:
            assert not category.startswith('components'), name
            if Path(name).stem.startswith('persistent_'):
                assert category == 'data_structure/persistent', name + ': persistent category required'
        if name == 'string/data_dependent_tree.hpp':
            assert category == 'string' or category.startswith('string/'), name + ': string category required'
        categories[name] = category
        advanced_libraries[name] = advanced
    assert set(categories) == headers, 'Missing category declarations: ' + str(sorted(headers - categories.keys()))
    for normal, light in config.get('implementation_variants', {}).items():
        assert categories[normal] == categories[light], f'Implementation categories differ: {normal}, {light}'
        assert advanced_libraries[normal] == advanced_libraries[light], f'Implementation advanced flags differ: {normal}, {light}'
    readme = root / 'README.md'
    if readme.exists():
        section, seen = None, set()
        for line in readme.read_text().splitlines():
            if line.startswith('### '):
                section = line[4:]
            match = re.match(r'\| `([^`]+\.hpp)`', line)
            if match:
                name = match[1]
                assert name in categories, f'Unknown README library: {name}'
                assert name not in seen, f'Duplicate README entry: {name}'
                assert categories.get(name) == section, f'README category differs: {name}'
                assert ('（応用）' in line) == advanced_libraries[name], f'README advanced flag differs: {name}'
                seen.add(name)
        assert seen == headers, 'Missing README entries: ' + str(sorted(headers - seen))
    return categories


if __name__ == '__main__':
    categories = check(Path(__file__).resolve().parent.parent)
    print(f'Checked explicit categories for {len(categories)} libraries')
