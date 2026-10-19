import json
import shutil
from pathlib import Path
W=Path(__file__).resolve().parent
metadata=json.loads((W/'metadata.json').read_text())
base=W/'current'

def variant(name, patch):
    dest=W/name
    if dest.exists(): shutil.rmtree(dest)
    shutil.copytree(base,dest)
    files={k: dest/v for k,v in metadata['current']['manifest'].items()}
    patch(files)

def attr(files, source, old, new):
    p=files[source]; text=p.read_text(); assert old in text, (source,old); p.write_text(text.replace(old,new))

def deferred(files):
    p=files['components/directed_csr.hpp']; t=p.read_text()
    start=t.index('\tvoid count_edge('); end=t.index('\tvoid finish(',start)
    t=t[:start]+'''\tvoid count_edge(const Edge &edge) {
\t\tassert(0 <= edge.from && edge.from < size() && 0 <= edge.to && edge.to < size());
\t\t++offset_[edge.from];
\t}
\tvoid check_incoming(std::span<const Edge> edges) {
\t\tif (!unique_incoming_) return;
\t\tstd::vector<std::uint64_t> seen((std::size_t(size()) + 63) / 64);
\t\tfor (const auto &edge : edges) {
\t\t\tauto mask = std::uint64_t(1) << (edge.to % 64);
\t\t\tauto &word = seen[edge.to / 64];
\t\t\tif (word & mask) { unique_incoming_ = false; return; }
\t\t\tword |= mask;
\t\t}
\t}
'''+t[end:]
    t=t.replace('\t\tconst auto n = offset_.size() - 1;','\t\tcheck_incoming(edges);\n\t\tconst auto n = offset_.size() - 1;',1)
    t=t.replace('\t\t\tstd::vector<std::uint64_t> seen(unique_incoming_ ? (std::size_t(n) + 63) / 64 : 0);\n','')
    t=t.replace('count_edge(edge, seen)','count_edge(edge)')
    p.write_text(t)

variant('deferred',deferred)
variant('build_noinline',lambda f:attr(f,'components/directed_csr.hpp','\tvoid build(int n, std::size_t m, Producer &&produce) {','\t[[gnu::noinline]] void build(int n, std::size_t m, Producer &&produce) {'))
variant('records_noinline',lambda f:attr(f,'components/fast_io_bulk.hpp','void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {','[[gnu::noinline]] void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {'))
variant('run_noinline',lambda f:attr(f,'graph/dijkstra.hpp','std::vector<T> run(const G &g, int st, int target, T inf, std::vector<int> &parent) {','[[gnu::noinline]] std::vector<T> run(const G &g, int st, int target, T inf, std::vector<int> &parent) {'))
variant('path_noinline',lambda f:attr(f,'graph/dijkstra.hpp','std::pair<T, std::vector<int>> path(const G &g, int st, int target, T inf) {','[[gnu::noinline]] std::pair<T, std::vector<int>> path(const G &g, int st, int target, T inf) {'))
variant('queue_old',lambda f: f['components/dijkstra_queue.hpp'].write_text((W/'old'/metadata['old']['manifest']['components/dijkstra_queue.hpp']).read_text().replace('"3.hpp"','"3.hpp"')))
variant('early_resize',lambda f:attr(f,'components/directed_csr.hpp','\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);','\t\t\tarcs_.resize(m);\n\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);'))
variant('early_resize_after_edges',lambda f:attr(f,'components/directed_csr.hpp','\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);','\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);\n\t\t\tarcs_.resize(m);'))
variant('early_reserve',lambda f:attr(f,'components/directed_csr.hpp','\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);','\t\t\tarcs_.reserve(m);\n\t\t\tauto edges = std::make_unique_for_overwrite<Edge[]>(m);'))
for distance in (8,32,64):
    for arcs in (False,True):
        line='\t\tfor (std::size_t i = edges.size(); i; --i) {\n'
        code=line+f'\t\t\tif (i > {distance}) {{\n\t\t\t\tconst auto from = edges[i - {distance+1}].from;\n\t\t\t\t__builtin_prefetch(&offset_[from], 1, 3);\n'
        if arcs: code+='\t\t\t\t__builtin_prefetch(&arcs_[offset_[from] - 1], 1, 3);\n'
        code+='\t\t\t}\n'
        variant(f'prefetch_{distance}_'+('arc' if arcs else 'offset'),lambda f,code=code:attr(f,'components/directed_csr.hpp',line,code))

def guarded(files):
    p=files['components/directed_csr.hpp']; t=p.read_text()
    old='''\t\tfor (std::size_t i = edges.size(); i; --i) {
\t\t\tconst auto &edge = edges[i - 1];
\t\t\tarcs_[--offset_[edge.from]] = {edge.to, edge.weight};
\t\t}
'''
    new='''\t\tstd::size_t i = edges.size();
\t\tif (n >= 8192) {
\t\t\tfor (; i > 32; --i) {
\t\t\t\tconst auto from = edges[i - 33].from;
\t\t\t\t__builtin_prefetch(&offset_[from], 1, 3);
\t\t\t\t__builtin_prefetch(&arcs_[offset_[from] - 1], 1, 3);
\t\t\t\tconst auto &edge = edges[i - 1];
\t\t\t\tarcs_[--offset_[edge.from]] = {edge.to, edge.weight};
\t\t\t}
\t\t}
\t\tfor (; i; --i) {
\t\t\tconst auto &edge = edges[i - 1];
\t\t\tarcs_[--offset_[edge.from]] = {edge.to, edge.weight};
\t\t}
'''
    assert old in t
    p.write_text(t.replace(old,new))

variant('guard32',guarded)
variant('deferred_guard32',lambda f:(deferred(f),guarded(f)))
import sys
sys.path.insert(0,str(W.parents[4]/'test/benchmark'))
from shortest_path_round3 import freeze_reference
dest=W/'narrow'
if dest.exists(): shutil.rmtree(dest)
freeze_reference(dest,W.parent/'narrow/test/driver.cpp')
print('Variants prepared')
