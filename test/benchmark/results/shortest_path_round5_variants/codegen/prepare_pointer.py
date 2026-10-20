import os
from pathlib import Path
import shutil

HERE = Path(os.environ.get('CP_SP_ROUND5_CODEGEN_WORK', Path(__file__).resolve().parent))
for name, limit in [('pointer', True), ('pointer_all', False)]:
    tree = HERE / name
    shutil.copytree(HERE / 'current', tree, dirs_exist_ok=True)
    header = tree / 'components/directed_csr.hpp'
    text = header.read_text()
    insert = text.index('\n\t\tstd::size_t i = edges.size();')
    condition = 'n < 8192 && edges.size() >= n * 16 && !edges.empty()' if limit else '!edges.empty()'
    fragment = '''
\t\tif (CONDITION) {
\t\t\tauto base = arcs_.data();
\t\t\tstd::vector<Arc *> cursor(n + 1);
\t\t\tfor (std::size_t v = 0; v <= n; ++v) cursor[v] = base + offset_[v];
\t\t\tfor (std::size_t i = edges.size(); i; --i) {
\t\t\t\tconst auto &edge = edges[i - 1];
\t\t\t\t*--cursor[edge.from] = {edge.to, edge.weight};
\t\t\t}
\t\t\tfor (std::size_t v = 0; v <= n; ++v) offset_[v] = cursor[v] - base;
\t\t\treturn;
\t\t}'''.replace('CONDITION', condition)
    header.write_text(text[:insert] + fragment + text[insert:])
