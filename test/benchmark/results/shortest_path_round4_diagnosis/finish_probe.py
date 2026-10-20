from pathlib import Path
source=Path(__file__).with_name('fault_probe.py').read_text()
source=source.replace("/fault_probe'", "/finish_probe'").replace('round4-fault-probe','round4-finish-probe').replace('TRIALS=41','TRIALS=41').replace('SEED=2026101905','SEED=2026101906')
source=source.replace("for name,revision in REVISIONS.items():", "for name,revision in REVISIONS.items():")
source=source.replace("    csr.write_text(text)", '''    text=text.replace('inline std::uintptr_t offset, arcs, edges;', 'inline std::uintptr_t offset, arcs, edges; inline std::chrono::steady_clock::time_point t0,t1,t2,t3; inline rusage r0,r1,r2,r3;')
    text=text.replace('void finish(std::span<const Edge> edges) {', 'void finish(std::span<const Edge> edges) { reproduce::t0=std::chrono::steady_clock::now(); getrusage(RUSAGE_SELF, &reproduce::r0);')
    text=text.replace('arcs_.resize(edges.size());', 'reproduce::t1=std::chrono::steady_clock::now(); getrusage(RUSAGE_SELF, &reproduce::r1); arcs_.resize(edges.size()); reproduce::t2=std::chrono::steady_clock::now(); getrusage(RUSAGE_SELF, &reproduce::r2);')
    marker='arcs_[--offset_[edge.from]] = {edge.to, edge.weight};'
    text=text.replace(marker, marker+'if(i==1) { reproduce::t3=std::chrono::steady_clock::now(); getrusage(RUSAGE_SELF, &reproduce::r3); }')
    driver=driver.rstrip()[:-1]+\"\\n\\tstd::fprintf(stderr, \\\"{\\\\\\\"prefix_ms\\\\\\\":%.9f,\\\\\\\"alloc_ms\\\\\\\":%.9f,\\\\\\\"scatter_ms\\\\\\\":%.9f,\\\\\\\"prefix_fault\\\\\\\":%ld,\\\\\\\"alloc_fault\\\\\\\":%ld,\\\\\\\"scatter_fault\\\\\\\":%ld}\\\\n\\\", elapsed(reproduce::t0,reproduce::t1),elapsed(reproduce::t1,reproduce::t2),elapsed(reproduce::t2,reproduce::t3), reproduce::r1.ru_minflt-reproduce::r0.ru_minflt,reproduce::r2.ru_minflt-reproduce::r1.ru_minflt,reproduce::r3.ru_minflt-reproduce::r2.ru_minflt);\\n}\\n\"
    csr.write_text(text)''')
source=source.replace("value.update(json.loads(completed.stderr.splitlines()[1]))", "value.update(json.loads(completed.stderr.splitlines()[1])); value.update(json.loads(completed.stderr.splitlines()[2]))")
source=source.replace("print(name,[(round(row['build_ms'],3),row['build_minor'],hex(row['arcs_addr']%2097152),row['minor_fault']) for row in samples],flush=True)", "print(name,[(round(row['build_ms'],3),round(row['prefix_ms'],3),round(row['alloc_ms'],3),round(row['scatter_ms'],3),row['alloc_fault'],row['scatter_fault']) for row in samples],flush=True)")
exec(compile(source,str(Path(__file__)), 'exec'))
