import hashlib
from pathlib import Path

from size_profiles import render_page, split_page


def highlight(code):
    from pygments import highlight as render
    from pygments.formatters import HtmlFormatter
    from pygments.lexers import CppLexer
    markup = render(code, CppLexer(stripnl=False, ensurenl=False), HtmlFormatter(nowrap=True))
    if not code.endswith('\n'):
        markup = markup.removesuffix('\n')
    markup = markup.replace('{', '&#123;').replace('}', '&#125;')
    return ('<div class="language-cpp highlighter-rouge"><div class="highlight">'
            '<pre class="highlight"><code>' + markup + '</code></pre></div></div>\n').encode('utf-8')


def prepare_highlights(pages, destination, cache_dir=None):
    from importlib.metadata import version
    renderer = hashlib.sha256(Path(__file__).read_bytes() + version('Pygments').encode()).hexdigest()
    if cache_dir is not None:
        cache_dir.mkdir(parents=True, exist_ok=True)
    result = dict(pages)
    generated = {}
    cached = set()
    for path, content in pages.items():
        if Path(path).suffix != '.md':
            continue
        front, body = split_page(content)
        data = front.get('data', {})
        if data.get('_pathExtension') not in ('hpp', 'cpp'):
            continue
        for source, field in [('code', 'codeHighlight'), ('bundledCode', 'bundleHighlight')]:
            code = data[source]
            identifier = hashlib.sha256(code.encode('utf-8')).hexdigest()
            name = 'generated-code/' + identifier + '.html'
            data[field] = name
            if name not in generated:
                cache_path = cache_dir / (renderer + '-' + identifier + '.html') if cache_dir is not None else None
                if cache_path is not None and cache_path.is_file():
                    generated[name] = cache_path.read_bytes()
                else:
                    generated[name] = highlight(code)
                    if cache_path is not None:
                        cache_path.write_bytes(generated[name])
                if cache_path is not None:
                    cached.add(cache_path)
        result[path] = render_page(front, body)
    result.update({destination / '_includes' / name: code for name, code in generated.items()})
    directory = destination / '_includes/generated-code'
    if directory.exists():
        for path in directory.glob('*.html'):
            if path not in result:
                path.unlink()
    if cache_dir is not None:
        for path in cache_dir.glob('*.html'):
            if path not in cached:
                path.unlink()
    return result
