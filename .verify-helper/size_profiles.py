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
        for alias in front.get('redirect_from', []):
            targets[alias.lstrip('/')] = page
    for source in (root / 'test').rglob('*.test.cpp'):
        path = source.relative_to(root).as_posix()
        targets[path] = path + '.html'
    return targets, origins


def rewrite_links(body, origin, config, targets, available, profile):
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
        result = base + prefix + target
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
    libraries = {}
    for path, (front, _) in parsed.items():
        data = front.get('data', {})
        if data.get('path', '').endswith('.hpp') and not data.get('isVerificationFile'):
            libraries[data['path']] = metadata(data['bundledCode'], data['path'], config)
    available = {name + '.html' for name, size in libraries.items() if size['lightweightAvailable']}
    result = dict(pages)

    def transform(front, body, relative, profile):
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
                    size = libraries[item['path']]
                    if profile == LIGHTWEIGHT and not size['lightweightAvailable']:
                        continue
                    item.update(size)
                    item['pagePath'] = front['library_prefix'] + item['path'] + '.html'
                    entries.append(item)
                category['pages'] = entries
            data['profileLibraryCount'] = sum(1 for size in libraries.values() if profile == 'full' or size['lightweightAvailable'])
            if relative == 'index.md':
                front['title'] = '軽量ライブラリ（64 KiB）' if profile == LIGHTWEIGHT else 'ライブラリ（全体・256 KiB向け）'
        elif data.get('path') in libraries:
            data.update(libraries[data['path']])
            data['copyByteCount'] = data['copyBytes'][profile]
            for key in ['_extendedDependsOn', '_extendedRequiredBy']:
                for item in data.get(key, []):
                    item['pagePath'] = (LIGHTWEIGHT + '/' if profile == LIGHTWEIGHT and item['path'] + '.html' in available else '') + item['path'] + '.html'
            for item in data.get('_extendedVerifiedWith', []):
                item['pagePath'] = item['path'] + '.html'
        origin = origins.get(re.sub(r'\.md$', '.html', relative), relative)
        body = rewrite_links(body, origin, config, targets, available, profile)
        return render_page(front, body)

    for path, (front, body) in parsed.items():
        relative = path.relative_to(destination).as_posix()
        data = front.get('data', {})
        if not (front.get('layout') == 'toppage' or data.get('path') in libraries):
            origin = origins.get(re.sub(r'\.md$', '.html', relative), relative)
            result[path] = render_page(front, rewrite_links(body, origin, config, targets, available, 'full'))
            continue
        result[path] = transform(front, body, relative, 'full')
        if front.get('layout') == 'toppage' or libraries[data['path']]['lightweightAvailable']:
            result[destination / LIGHTWEIGHT / relative] = transform(front, body, relative, LIGHTWEIGHT)
    manifest = {'limits': LIMITS, 'libraries': libraries}
    result[destination / 'assets/library-sizes.json'] = (json.dumps(manifest, ensure_ascii=False, indent=2) + '\n').encode()
    return result


def remove_stale_pages(destination, pages):
    destination = Path(destination).resolve()
    directory = destination / LIGHTWEIGHT
    if not directory.exists():
        return
    directory.resolve().relative_to(destination)
    kept = {Path(path).resolve() for path in pages}
    for path in directory.rglob('*.md'):
        if path.resolve() not in kept:
            path.unlink()
