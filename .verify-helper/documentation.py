from pathlib import Path
import hashlib
import json
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager

from size_profiles import render_page, split_page


@contextmanager
def fast_front_matter():
    from onlinejudge_verify.documentation import front_matter
    split, merge = front_matter.split_front_matter, front_matter.merge_front_matter
    front_matter.split_front_matter, front_matter.merge_front_matter = split_page, render_page
    try:
        yield
    finally:
        front_matter.split_front_matter, front_matter.merge_front_matter = split, merge


def source_pages(pages):
    limit = 256 * 1024
    result = {}
    for path, content in pages.items():
        metadata, body = split_page(content)
        data = metadata.get("data", {})
        original = data.get("bundledCode")
        if isinstance(original, str):
            size = len(original.encode("utf-8"))
            if size >= limit:
                raise ValueError(f"{path}: {size} bytes is not below {limit} bytes")
            data.update(bundledBytes=size, bundledLimit=limit)
            data.pop("bundledCompressed", None)
            content = render_page(metadata, body)
        result[path] = content
    return result


def source_paths(plan, excluded):
    paths = set(plan["state"]["targets"]["headers"]) | set(plan["state"]["targets"]["verification"])
    return sorted(path for path in map(Path, paths) if not any(
        path == exclude or exclude in path.parents for exclude in excluded
    ))


class SourceCache:
    def __init__(self, root, plan, path):
        from ci import affected_inventory, no_cache_context
        self.root = root.resolve()
        self.path = path
        self.enabled = not affected_inventory(plan) and not no_cache_context()
        self.targets = plan['state']['targets']
        self.selected = set(plan.get('bundles', [])) | set(plan.get('verification', []))
        self.context = hashlib.sha256((self.root / '.verify-helper/documentation.py').read_bytes()).hexdigest()
        try:
            previous = json.loads(path.read_text()) if self.enabled else {}
            if previous.get('version') != 1:
                previous = {}
        except (OSError, ValueError, AttributeError):
            previous = {}
        self.previous = previous.get('sources', {})
        self.records = {}
        self.hits = set()

    def record(self, path):
        name = path.resolve().relative_to(self.root).as_posix()
        group = 'bundles' if name.endswith('.hpp') else 'verification'
        fingerprint = self.targets[group].get(name)
        if fingerprint is None:
            return {}
        signature = hashlib.sha256((self.context + fingerprint).encode()).hexdigest()
        if name not in self.records:
            old = self.previous.get(name, {})
            if self.enabled and name not in self.selected and isinstance(old, dict) and old.get('signature') == signature:
                self.records[name] = dict(old)
                self.hits.add(name)
            else:
                self.records[name] = {'signature': signature}
        return self.records[name]

    def dependencies(self, original, path, *, basedir):
        record = self.record(basedir / path)
        if 'dependencies' not in record:
            paths = original(path, basedir=basedir)
            encoded = []
            for dependency in paths:
                dependency = (basedir / dependency).resolve()
                try:
                    encoded.append([True, dependency.relative_to(self.root).as_posix()])
                except ValueError:
                    encoded.append([False, str(dependency)])
            record['dependencies'] = encoded
        return [self.root / name if relative else Path(name) for relative, name in record['dependencies']]

    def attributes(self, original, path, *, basedir):
        record = self.record(basedir / path)
        if 'attributes' not in record:
            record['attributes'] = original(path, basedir=basedir)
        return record['attributes']

    def bundle(self, original, path, *, basedir, options):
        if options != {'include_paths': [basedir]}:
            return original(path, basedir=basedir, options=options)
        record = self.record(basedir / path)
        if 'bundle' not in record:
            record['bundle'] = original(path, basedir=basedir, options=options).decode('utf-8')
        return record['bundle'].encode('utf-8')

    @contextmanager
    def use(self):
        from onlinejudge_verify.languages import list as languages
        language = languages.get(Path('source.cpp'))
        original = {name: getattr(language, name) for name in ('list_dependencies', 'list_attributes', 'bundle')}
        language.list_dependencies = lambda path, **kwargs: self.dependencies(original['list_dependencies'], path, **kwargs)
        language.list_attributes = lambda path, **kwargs: self.attributes(original['list_attributes'], path, **kwargs)
        language.bundle = lambda path, **kwargs: self.bundle(original['bundle'], path, **kwargs)
        try:
            yield
        finally:
            for name, method in original.items():
                setattr(language, name, method)

    def save(self):
        if self.enabled:
            temporary = self.path.with_suffix('.tmp')
            temporary.parent.mkdir(parents=True, exist_ok=True)
            temporary.write_text(json.dumps({'version': 1, 'sources': self.records}, ensure_ascii=False))
            temporary.replace(self.path)
        print(f'Documentation sources: reused {len(self.hits)}, rebuilt {len(self.records) - len(self.hits)}', flush=True)


def generate(plan):
    root = Path.cwd().resolve()
    cache = SourceCache(root, plan, root / '.verify-helper/documentation-cache.json')
    with cache.use(), fast_front_matter():
        generate_pages(plan, highlight_cache=root / '.verify-helper/highlight-cache' if cache.enabled else None)
    cache.save()


def generate_pages(plan, *, highlight_cache=None):
    from onlinejudge_verify.documentation import build, configure, main
    from onlinejudge_verify.documentation.type import PageRenderJob
    from verification import get_marker

    root = Path.cwd().resolve()
    config = main.load_render_config(basedir=root)
    excluded = [Path(path) for path in config.config_yml.get("exclude", [])]
    paths = source_paths(plan, excluded)
    verification = get_marker()
    from onlinejudge_verify.languages import list as languages
    def prefetch(path):
        language = languages.get(path)
        language.list_dependencies(path, basedir=root)
        language.list_attributes(path, basedir=root)
    with ThreadPoolExecutor(max_workers=2) as pool:
        list(pool.map(prefetch, paths))
    depends_on, required_by, verified_with = configure._build_dependency_graph(paths, basedir=root)
    status = configure._build_verification_status(paths, verified_with=verified_with, basedir=root, marker=verification)
    stats = [configure._get_source_code_stat(
        path, depends_on=depends_on, required_by=required_by, verified_with=verified_with,
        verification_status=status, marker=verification, basedir=root,
    ) for path in paths]
    markdown = configure.apply_exclude_list_to_paths(configure.find_markdown_paths(basedir=root), excluded_paths=excluded)
    jobs = configure.convert_to_page_render_jobs(source_code_stats=stats, markdown_paths=markdown, site_render_config=config)
    for path, title, kind in [("components/index.md", "補助部品", "components"),
                              ("data_structure/advanced/index.md", "応用ライブラリ", "advanced")]:
        jobs.append(PageRenderJob(
            path=Path(path),
            front_matter={"layout": "toppage", "title": title, kind: True},
            content=config.index_md.with_name(kind + ".md").read_bytes(),
        ))
    pages = source_pages(build.render_pages(page_render_jobs=jobs, source_code_stats=stats, site_render_config=config))
    from size_profiles import documentation_links, prepare_pages, remove_stale_pages
    targets, origins = documentation_links(root, markdown)
    pages = prepare_pages(pages, config.destination_dir, config.config_yml, targets=targets, origins=origins)
    pages.update(build.load_static_files(site_render_config=config))
    from code_highlight import prepare_highlights
    pages = prepare_highlights(pages, config.destination_dir, cache_dir=highlight_cache)
    remove_stale_pages(config.destination_dir, pages)
    for path, content in pages.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    print(f"Generated documentation for {len(paths)} library and judge sources", flush=True)
