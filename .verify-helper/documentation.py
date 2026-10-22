from pathlib import Path


def source_pages(pages):
    from onlinejudge_verify.documentation.front_matter import merge_front_matter, split_front_matter

    limit = 256 * 1024
    result = {}
    for path, content in pages.items():
        metadata, body = split_front_matter(content)
        data = metadata.get("data", {})
        original = data.get("bundledCode")
        if isinstance(original, str):
            size = len(original.encode("utf-8"))
            if size >= limit:
                raise ValueError(f"{path}: {size} bytes is not below {limit} bytes")
            data.update(bundledBytes=size, bundledLimit=limit)
            data.pop("bundledCompressed", None)
            content = merge_front_matter(metadata, body)
        result[path] = content
    return result


def source_paths(plan, excluded):
    paths = set(plan["state"]["targets"]["headers"]) | set(plan["state"]["targets"]["verification"])
    return sorted(path for path in map(Path, paths) if not any(
        path == exclude or exclude in path.parents for exclude in excluded
    ))


def generate(plan):
    from onlinejudge_verify.documentation import build, configure, main
    from onlinejudge_verify.documentation.type import PageRenderJob
    from verification import get_marker

    root = Path.cwd().resolve()
    config = main.load_render_config(basedir=root)
    excluded = [Path(path) for path in config.config_yml.get("exclude", [])]
    paths = source_paths(plan, excluded)
    verification = get_marker()
    depends_on, required_by, verified_with = configure._build_dependency_graph(paths, basedir=root)
    status = configure._build_verification_status(paths, verified_with=verified_with, basedir=root, marker=verification)
    stats = [configure._get_source_code_stat(
        path, depends_on=depends_on, required_by=required_by, verified_with=verified_with,
        verification_status=status, marker=verification, basedir=root,
    ) for path in paths]
    markdown = configure.apply_exclude_list_to_paths(configure.find_markdown_paths(basedir=root), excluded_paths=excluded)
    jobs = configure.convert_to_page_render_jobs(source_code_stats=stats, markdown_paths=markdown, site_render_config=config)
    jobs.append(PageRenderJob(
        path=Path("components/index.md"),
        front_matter={"layout": "toppage", "title": "補助部品", "components": True},
        content=config.index_md.with_name("components.md").read_bytes(),
    ))
    pages = source_pages(build.render_pages(page_render_jobs=jobs, source_code_stats=stats, site_render_config=config))
    from size_profiles import documentation_links, prepare_pages, remove_stale_pages
    targets, origins = documentation_links(root, markdown)
    pages = prepare_pages(pages, config.destination_dir, config.config_yml, targets=targets, origins=origins)
    pages.update(build.load_static_files(site_render_config=config))
    remove_stale_pages(config.destination_dir, pages)
    for path, content in pages.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    print(f"Generated documentation for {len(paths)} library and judge sources", flush=True)
