import os
from pathlib import Path
import shutil
import sys
import subprocess

HERE = Path(os.environ.get('CP_SP_ROUND5_STORAGE_WORK', Path(__file__).resolve().parent))
ROOT = Path(__file__).resolve().parents[5]
sys.path.insert(0, str(ROOT / 'test/benchmark'))
from shortest_path_round3 import freeze
phase = subprocess.check_output(['git', 'show', 'c075fb733c2954ce303eb6a7fa1ef78977a611f9:test/benchmark/shortest_path_round4.cpp'], cwd=ROOT, text=True)
actual = subprocess.check_output(['git', 'show', 'c075fb733c2954ce303eb6a7fa1ef78977a611f9:test/dijkstra_path.test.cpp'], cwd=ROOT, text=True).replace('"../graph/', '"graph/').replace('"../components/', '"components/')
storage = '''#ifndef OVERWRITE_ARRAY_HPP
#define OVERWRITE_ARRAY_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template<class T>
class OverwriteArray {
\tstatic_assert(std::is_trivially_destructible_v<T> && std::is_nothrow_copy_constructible_v<T>);
\tT *data_ = nullptr;
\tstd::size_t size_ = 0, capacity_ = 0;
\tstd::allocator<T> allocator_;
\tvoid release() {
\t\tif (data_) allocator_.deallocate(data_, capacity_);
\t}
  public:
\tOverwriteArray() = default;
\tOverwriteArray(const OverwriteArray &other) {
\t\tresize(other.size_);
\t\tstd::uninitialized_copy_n(other.data_, size_, data_);
\t}
\tOverwriteArray(OverwriteArray &&other) noexcept : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)) {}
\tOverwriteArray &operator=(const OverwriteArray &other) {
\t\tif (this != &other) {
\t\t\tresize(other.size_);
\t\t\tstd::uninitialized_copy_n(other.data_, size_, data_);
\t\t}
\t\treturn *this;
\t}
\tOverwriteArray &operator=(OverwriteArray &&other) noexcept {
\t\tif (this != &other) {
\t\t\trelease();
\t\t\tdata_ = std::exchange(other.data_, nullptr);
\t\t\tsize_ = std::exchange(other.size_, 0);
\t\t\tcapacity_ = std::exchange(other.capacity_, 0);
\t\t}
\t\treturn *this;
\t}
\t~OverwriteArray() { release(); }
\tvoid resize(std::size_t n) {
\t\tif (n > capacity_) {
\t\t\tconst auto max = std::allocator_traits<std::allocator<T>>::max_size(allocator_);
\t\t\tconst auto capacity = std::max(n, size_ <= max / 2 ? size_ * 2 : max);
\t\t\tauto next = allocator_.allocate(capacity);
\t\t\trelease();
\t\t\tdata_ = next;
\t\t\tcapacity_ = capacity;
\t\t}
\t\tsize_ = n;
\t}
\tvoid clear() { size_ = 0; }
\tT *data() { return data_; }
\tconst T *data() const { return data_; }
\tT *begin() { return data_; }
\tconst T *begin() const { return data_; }
\tT *end() { return size_ ? data_ + size_ : data_; }
\tconst T *end() const { return size_ ? data_ + size_ : data_; }
\tstd::size_t size() const { return size_; }
\tbool empty() const { return size_ == 0; }
\tT &operator[](std::size_t i) { return data_[i]; }
\tconst T &operator[](std::size_t i) const { return data_[i]; }
};

#endif
'''
for name in ('current', 'overwrite'):
    tree = HERE / name
    freeze(tree, phase, revision='c075fb733c2954ce303eb6a7fa1ef78977a611f9')
    (tree / 'phase.cpp').write_text(phase)
    (tree / 'actual.cpp').write_text(actual)
    if name == 'overwrite':
        (tree / 'components/overwrite_array.hpp').write_text(storage)
        header = tree / 'components/directed_csr.hpp'
        text = header.read_text().replace('#include <algorithm>', '#include "overwrite_array.hpp"\n\n#include <algorithm>')
        text = text.replace('\tstd::vector<Arc> arcs_;', '\tstatic constexpr bool overwrite = std::is_trivial_v<T> && std::is_nothrow_constructible_v<Arc, int, const T &> && std::is_trivially_destructible_v<Arc> && std::is_nothrow_copy_constructible_v<Arc>;\n\tstd::conditional_t<overwrite, OverwriteArray<Arc>, std::vector<Arc>> arcs_;')
        text = text.replace('\tvoid finish(std::span<const Edge> edges) {', '\tvoid store_arc(std::size_t i, const Edge &edge) {\n\t\tif constexpr (overwrite) std::construct_at(arcs_.data() + i, edge.to, edge.weight);\n\t\telse arcs_[i] = {edge.to, edge.weight};\n\t}\n\tvoid finish(std::span<const Edge> edges) {')
        text = text.replace('arcs_[edge.from - (single_gap_ >= 0 && edge.from > single_gap_)] = {edge.to, edge.weight};', 'store_arc(edge.from - (single_gap_ >= 0 && edge.from > single_gap_), edge);')
        text = text.replace('arcs_[--offset_[edge.from]] = {edge.to, edge.weight};', 'store_arc(--offset_[edge.from], edge);')
        text = text.replace('&arcs_[offset_[from] - 1]', 'arcs_.data() + offset_[from] - 1')
        header.write_text(text)
runner = (Path(__file__).resolve().parent.parent / 'codegen/run.py').read_text().replace('CP_SP_ROUND5_CODEGEN_WORK', 'CP_SP_ROUND5_STORAGE_WORK')
runner = runner.replace("parser.add_argument('--trials'", "parser.add_argument('--problem', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path')\nparser.add_argument('--trials'")
runner = runner.replace("problem = Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/shortest_path'", 'problem = args.problem')
(HERE / 'run.py').write_text(runner)
print(HERE)
