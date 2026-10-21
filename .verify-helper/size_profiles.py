import copy
import json
import posixpath
import re
from pathlib import Path
from urllib.parse import urlsplit

import yaml


LIGHTWEIGHT = 'lightweight'
LIMITS = {'full': 256 * 1024, LIGHTWEIGHT: 64 * 1024}
LOADER = getattr(yaml, 'CSafeLoader', yaml.SafeLoader)
DUMPER = getattr(yaml, 'CSafeDumper', yaml.SafeDumper)


def split_page(source):
    match = re.match(rb'\A---\r?\n(.*?)\r?\n---(?:\r?\n|\Z)', source, re.DOTALL)
    if not match:
        return {}, source
    return yaml.load(match[1], Loader=LOADER) or {}, source[match.end():]


def render_page(front, body):
    return b'---\n' + yaml.dump(front, Dumper=DUMPER, allow_unicode=True, sort_keys=False).encode() + b'---\n' + body


def page_url(config, path, profile='full'):
    prefix = LIGHTWEIGHT + '/' if profile == LIGHTWEIGHT else ''
    base = config.get('url', '').rstrip('/') + '/' + config.get('baseurl', '').strip('/')
    return base.rstrip('/') + '/' + prefix + path


def copied_source(code, url):
    return '//' + url + '\n' + code


def copy_size(code, url):
    return len(copied_source(code, url).encode('utf-8'))


def metadata(code, path, config):
    sizes = {profile: copy_size(code, page_url(config, path + '.html', profile)) for profile in LIMITS}
    return {'bundleBytes': len(code.encode('utf-8')), 'copyBytes': sizes,
            'lightweightAvailable': sizes[LIGHTWEIGHT] <= LIMITS[LIGHTWEIGHT]}


def documentation_links(root, markdown):
    from onlinejudge_verify.documentation.configure import resolve_documentation_of

    targets, origins = {}, {}
    for source in markdown:
        source = Path(source)
        if source.is_absolute():
            source = source.relative_to(root)
        path = source.as_posix()
        front, _ = split_page((root / source).read_bytes())
        reference = front.get('documentation_of')
        target = resolve_documentation_of(reference, markdown_path=Path(source), basedir=root) if reference else None
        page = target.resolve().relative_to(root).as_posix() + '.html' if target else re.sub(r'\.md$', '.html', path)
        targets[path] = page
        origins[page] = path
        aliases = front.get('redirect_from', [])
        if isinstance(aliases, str):
            aliases = [aliases]
        for alias in aliases:
            targets[alias.lstrip('/')] = page
    for source in (root / 'test').rglob('*.test.cpp'):
        path = source.relative_to(root).as_posix()
        targets[path] = path + '.html'
    return targets, origins


def rewrite_links(body, origin, config, targets, available, profile, routes=None):
    root_url = page_url(config, '')
    base = '/' + config.get('baseurl', '').strip('/')
    base = base.rstrip('/') + '/'

    def replace(match):
        label, link = match.groups()
        url = urlsplit(link)
        if link.startswith(root_url):
            path = url.path[len(base):] if url.path.startswith(base) else None
        elif url.scheme or url.netloc or not url.path:
            return match[0]
        elif url.path.startswith(base):
            path = url.path[len(base):]
        elif url.path.startswith('/'):
            return match[0]
        else:
            path = posixpath.normpath(posixpath.join(posixpath.dirname(origin), url.path))
        target = targets.get(path, path)
        if target not in targets.values() and target not in available:
            return match[0]
        prefix = LIGHTWEIGHT + '/' if profile == LIGHTWEIGHT and target in available else ''
        result = base + (routes or {}).get(target, prefix + target)
        if url.query:
            result += '?' + url.query
        if url.fragment:
            result += '#' + url.fragment
        return '[' + label + '](' + result + ')'

    parts = re.split(r'(```[^\n]*\n[\s\S]*?\n```|`[^`\n]*`)', body.decode('utf-8'))
    return ''.join(part if i % 2 else re.sub(r'(?<!!)\[([^\]\n]+)\]\(([^\s)]+)\)', replace, part)
                   for i, part in enumerate(parts)).encode('utf-8')


def prepare_pages(pages, destination, config, *, targets=None, origins=None):
    destination = Path(destination)
    targets, origins = targets or {}, origins or {}
    parsed = {Path(path): split_page(source) for path, source in pages.items()}
    sources = {front['data']['path']: (front, body)
               for front, body in parsed.values()
               if front.get('data', {}).get('path', '').endswith('.hpp')
               and not front['data'].get('isVerificationFile')}
    libraries = {name: metadata(front['data']['bundledCode'], name, config)
                 for name, (front, _) in sources.items()}
    variants = config.get('implementation_variants', {})
    for normal, light in variants.items():
        if normal not in sources or light not in sources or normal == light or light in variants:
            raise ValueError('Invalid implementation pair: ' + normal + ' / ' + light)
    aliases = set(variants.values())
    alias_targets = {light: [normal for normal, source in variants.items() if source == light] for light in aliases}
    catalog = {}
    for name in sorted(libraries.keys() - aliases):
        implementation = variants.get(name, name)
        size = metadata(sources[implementation][0]['data']['bundledCode'], name, config)
        catalog[name] = {'lightweightImplementation': implementation,
                         'lightweightAvailable': size['lightweightAvailable'],
                         'copyBytes': {'full': libraries[name]['copyBytes']['full'],
                                       LIGHTWEIGHT: size['copyBytes'][LIGHTWEIGHT]}}
    available = {name + '.html' for name, size in libraries.items() if size['lightweightAvailable']}
    available.update(name + '.html' for name, size in catalog.items() if size['lightweightAvailable'])
    result = {}

    def transform(front, body, relative, profile, logical=None):
        front = copy.deepcopy(front)
        front['size_profile'] = profile
        front['library_prefix'] = LIGHTWEIGHT + '/' if profile == LIGHTWEIGHT else ''
        front['code_limit'] = LIMITS[profile]
        if profile == LIGHTWEIGHT:
            front.pop('redirect_from', None)
            front.pop('permalink', None)
        data = front.get('data', {})
        if front.get('layout') == 'toppage':
            for category in data.get('libraryCategories', []):
                entries = []
                for item in category['pages']:
                    name = item['path']
                    if name not in catalog:
                        continue
                    size = catalog[name]
                    item.update(size)
                    usable = profile == 'full' or size['lightweightAvailable']
                    item['profileAvailable'] = usable
                    item['implementationPath'] = size['lightweightImplementation'] if profile == LIGHTWEIGHT else name
                    item['copyByteCount'] = size['copyBytes'][profile] if usable else None
                    item['pagePath'] = (front['library_prefix'] if usable else '') + name + '.html'
                    entries.append(item)
                category['pages'] = entries
            data['profileLibraryCount'] = len(catalog)
            data['profileAvailableCount'] = sum(profile == 'full' or size['lightweightAvailable'] for size in catalog.values())
            if relative == 'index.md':
                front['title'] = '軽量ライブラリ（64 KiB）' if profile == LIGHTWEIGHT else 'ライブラリ（全体・256 KiB向け）'
        elif data.get('path') in libraries:
            name = logical or data['path']
            data.update(metadata(data['bundledCode'], name, config))
            data['libraryPath'] = name
            data['copyByteCount'] = data['copyBytes'][profile]
            data['implementationLinks'] = []
            data['implementationApi'] = config.get('implementation_variant_apis', {}).get(name) if profile == LIGHTWEIGHT else None
            if name in catalog:
                normal_front = sources[name][0]
                front['title'] = normal_front['title']
                front['documentation_of'] = name
                other_available = profile == LIGHTWEIGHT or catalog[name]['lightweightAvailable']
                data['counterpartPage'] = ('' if profile == LIGHTWEIGHT else LIGHTWEIGHT + '/') + name + '.html' if other_available else None
                if name in variants:
                    data['implementationKind'] = '軽量実装' if profile == LIGHTWEIGHT else '高速実装'
                    if other_available:
                        data['implementationLinks'].append({'label': ('高速実装（256 KiB向け）' if profile == LIGHTWEIGHT else '軽量実装（64 KiB）') + 'はこちら',
                                                            'pagePath': data['counterpartPage']})
            else:
                data['counterpartPage'] = ('' if profile == LIGHTWEIGHT else LIGHTWEIGHT + '/') + name + '.html' if name + '.html' in available else None
                data['implementationKind'] = '軽量実装'
                for normal, light in variants.items():
                    if light == name:
                        data['implementationLinks'].append({'label': sources[normal][0]['title'] + 'の高速実装はこちら',
                                                            'pagePath': normal + '.html'})
            for key in ['_extendedDependsOn', '_extendedRequiredBy']:
                for item in data.get(key, []):
                    target = item['path']
                    if profile == LIGHTWEIGHT and len(alias_targets.get(target, [])) == 1:
                        target = alias_targets[target][0]
                    item['pagePath'] = (LIGHTWEIGHT + '/' if profile == LIGHTWEIGHT and target + '.html' in available else '') + target + '.html'
            for item in data.get('_extendedVerifiedWith', []):
                item['pagePath'] = item['path'] + '.html'
        origin = origins.get(data.get('path', '') + '.html', origins.get(re.sub(r'\.md$', '.html', relative), relative))
        routes = {}
        for normal, light in variants.items():
            if data.get('path') == light:
                routes[normal + '.html'] = normal + '.html'
            elif data.get('path') == normal and catalog[normal]['lightweightAvailable']:
                routes[light + '.html'] = LIGHTWEIGHT + '/' + normal + '.html'
        body = rewrite_links(body, origin, config, targets, available, profile, routes)
        return render_page(front, body)

    for path, (front, body) in parsed.items():
        relative = path.relative_to(destination).as_posix()
        data = front.get('data', {})
        name = data.get('path')
        if not (front.get('layout') == 'toppage' or name in libraries):
            origin = origins.get(re.sub(r'\.md$', '.html', relative), relative)
            result[path] = render_page(front, rewrite_links(body, origin, config, targets, available, 'full'))
            continue
        result[path] = transform(front, body, relative, 'full')
        if front.get('layout') == 'toppage':
            result[destination / LIGHTWEIGHT / relative] = transform(front, body, relative, LIGHTWEIGHT)
        elif name in catalog and catalog[name]['lightweightAvailable']:
            light_front, light_body = sources[catalog[name]['lightweightImplementation']]
            result[destination / LIGHTWEIGHT / relative] = transform(light_front, light_body, relative, LIGHTWEIGHT, name)
        elif name in aliases and libraries[name]['lightweightAvailable']:
            result[destination / LIGHTWEIGHT / relative] = transform(front, body, relative, LIGHTWEIGHT)
    manifest = {'limits': LIMITS, 'libraries': libraries, 'catalog': catalog, 'implementationVariants': variants}
    result[destination / 'assets/library-sizes.json'] = (json.dumps(manifest, ensure_ascii=False, indent=2) + '\n').encode()
    return result


def remove_stale_pages(destination, pages):
    destination = Path(destination).resolve()
    if not destination.exists():
        return
    kept = {Path(path).resolve() for path in pages}
    for path in destination.rglob('*.md'):
        if path.resolve() not in kept:
            path.unlink()
