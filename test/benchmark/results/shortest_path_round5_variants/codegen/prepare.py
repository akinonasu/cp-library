import os
from pathlib import Path
import sys
import subprocess

ROOT = Path(__file__).resolve().parents[5]
sys.path.insert(0, str(ROOT / 'test/benchmark'))
from shortest_path_round3 import freeze

HERE = Path(os.environ.get('CP_SP_ROUND5_CODEGEN_WORK', Path(__file__).resolve().parent))
phase = subprocess.check_output(['git', 'show', 'c075fb733c2954ce303eb6a7fa1ef78977a611f9:test/benchmark/shortest_path_round4.cpp'], cwd=ROOT, text=True)
actual = subprocess.check_output(['git', 'show', 'c075fb733c2954ce303eb6a7fa1ef78977a611f9:test/dijkstra_path.test.cpp'], cwd=ROOT, text=True).replace('"../graph/', '"graph/').replace('"../components/', '"components/')
for name in ('old', 'current', 'skip', 'split', 'split_skip'):
    tree = HERE / name
    freeze(tree, phase, revision='bd29f70c70e24e27770b9e9a3777b6cf85664b63' if name == 'old' else 'c075fb733c2954ce303eb6a7fa1ef78977a611f9')
    (tree / 'phase.cpp').write_text(phase)
    (tree / 'actual.cpp').write_text(actual)
    if name in ('skip', 'split_skip'):
        header = tree / 'components/directed_csr.hpp'
        text = header.read_text()
        start = text.index('\t\tstd::size_t active = offset_[0] != 0;')
        end = text.index('\n\t\tarcs_.resize(edges.size());', start)
        text = text[:start] + '''\t\tstd::size_t active = 0;
\t\tif (n >= 8192) {
\t\t\tactive = offset_[0] != 0;
\t\t\tfor (std::size_t v = 1; v < offset_.size(); ++v) {
\t\t\t\tactive += offset_[v] != 0;
\t\t\t\toffset_[v] += offset_[v - 1];
\t\t\t}
\t\t} else {
\t\t\tfor (std::size_t v = 1; v < offset_.size(); ++v) offset_[v] += offset_[v - 1];
\t\t}''' + text[end:]
        header.write_text(text)
    if name in ('split', 'split_skip'):
        header = tree / 'components/directed_csr.hpp'
        text = header.read_text()
        start = text.index('\t\tstd::size_t i = edges.size();')
        end = text.index('\n\t}\n\tvoid clear_edges()', start)
        original = text[start:end]
        prefetch_start = original.index('\t\t\tfor (; i > 32; --i) {')
        prefetch_end = original.index('\n\t\t}', prefetch_start)
        loop = original[prefetch_start:prefetch_end]
        loop = loop.replace('\t\t\t', '\t\t')
        replacement = '''#if defined(__GNUC__) || defined(__clang__)
\t\tif (active >= 8192) {
\t\t\tscatter_prefetched(edges);
\t\t\treturn;
\t\t}
#endif
\t\tfor (std::size_t i = edges.size(); i; --i) {
\t\t\tconst auto &edge = edges[i - 1];
\t\t\tarcs_[--offset_[edge.from]] = {edge.to, edge.weight};
\t\t}'''
        helper = '''
\tvoid scatter_prefetched(std::span<const Edge> edges) {
\t\tstd::size_t i = edges.size();
''' + loop + '''
\t\tfor (; i; --i) {
\t\t\tconst auto &edge = edges[i - 1];
\t\t\tarcs_[--offset_[edge.from]] = {edge.to, edge.weight};
\t\t}
\t}'''
        text = text[:start] + replacement + text[end:]
        position = text.index('\n\tvoid clear_edges()')
        text = text[:position] + '\n#if defined(__GNUC__) || defined(__clang__)' + helper + '\n#endif\n' + text[position:]
        header.write_text(text)
print(HERE)
