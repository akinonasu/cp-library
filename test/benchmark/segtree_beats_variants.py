import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'data_structure/segment_tree/segtree_beats.hpp').read_text()
source = source.replace('#include "../components/default_init_allocator.hpp"\n', '').replace('std::vector<Node, DefaultInitAllocator<Node>>', 'std::vector<Node>').replace('\n\t\ttree[k].add = 0;', '')
unit = (root / 'test/unit/segtree_beats.cpp').read_text()
variants = {'compact': source}
heap = source.replace('return k + 1;', 'return 2 * k + 1;').replace('return k + 2 * (m - l);', '(void)l; (void)m; return 2 * k + 2;').replace('2 * std::size_t(n) - 1', '4 * std::size_t(n)')
variants['heap'] = heap
variants['aligned'] = source.replace('struct Node {', 'struct alignas(64) Node {')
a = source.index('\t\tif (a.lo == a.hi) {', source.index('\tvoid push('))
b = source.index('\t\tif (a.add)', a)
variants['plain_push'] = source[:a] + '''\t\tif (a.lo == a.hi) {
			if (b.lo != a.lo || b.hi != a.hi) fill(b, a.lo, m - l);
			if (c.lo != a.lo || c.hi != a.hi) fill(c, a.lo, r - m);
			a.add = 0;
			return;
		}
''' + source[b:]
a = source.index('\t\tif (a.lo == a.hi) {', source.index('\tResult<op> query('))
b = source.index('\t\tint m =', a)
variants['plain_query'] = source[:a] + source[b:]
variants['prefetch'] = source.replace('\t\tpush(k, l, m, r);\n\t\tif (ql < m)', '\t\t__builtin_prefetch(&tree[lc]);\n\t\t__builtin_prefetch(&tree[rc]);\n\t\tpush(k, l, m, r);\n\t\tif (ql < m)')
iterative = heap.replace('#include <algorithm>', '#include <algorithm>\n#include <bit>')
iterative = iterative.replace('int n = 0;', 'int n = 0, capacity = 1;')
iterative = iterative.replace(': n(n), tree(n > 0 ? 4 * std::size_t(n) : 0)', ': n(n), capacity(n > 0 ? int(std::bit_ceil(unsigned(n))) : 1), tree(n > 0 ? 2 * std::size_t(capacity) - 1 : 0)')
iterative = iterative.replace(': n(int(a.size())), tree(n ? 4 * std::size_t(n) : 0)', ': n(int(a.size())), capacity(n ? int(std::bit_ceil(unsigned(n))) : 1), tree(n ? 2 * std::size_t(capacity) - 1 : 0)')
iterative = iterative.replace('fill(tree[k], f(l), 1)', 'fill(tree[k], l < n ? f(l) : T(0), 1)').replace('build(0, 0, n,', 'build(0, 0, capacity,').replace('collect(0, 0, n,', 'collect(0, 0, capacity,')
iterative = iterative.replace('if (tree[k].lo == tree[k].hi) {', 'if (l >= n) return;\n\t\tif (tree[k].lo == tree[k].hi) {').replace('a.begin() + r, tree[k].lo', 'a.begin() + std::min(r, n), tree[k].lo')
iterative = iterative.replace('return n ? tree[0].lo : inf;', 'return min(0, n);').replace('return n ? tree[0].hi : neg_inf;', 'return max(0, n);')
a = iterative.index('\ttemplate <int op>\n\tvoid update(')
b = iterative.index('\ttemplate <int op>\n\tusing Result', a)
iterative = iterative[:a] + '''	template <int op>
	void apply(int k, int len, T x) {
		auto &a = tree[k];
		if constexpr (op == 0) {
			if (a.hi <= x) return;
			if (x <= a.lo) return fill(a, x, len);
			if (a.hi2 < x) return lower(a, x);
		}
		if constexpr (op == 1) {
			if (a.lo >= x) return;
			if (x >= a.hi) return fill(a, x, len);
			if (a.lo2 > x) return raise(a, x);
		}
		if constexpr (op == 2) return add_node(a, x, len);
		if constexpr (op == 3) return fill(a, x, len);
		push(k, 0, len / 2, len);
		apply<op>(left(k), len / 2, x);
		apply<op>(right(k, 0, 0), len / 2, x);
		pull(k, left(k), right(k, 0, 0));
	}
	void boundaries(int l, int r) const {
		for (int h = std::countr_zero(unsigned(capacity)); h; --h) {
			int len = 1 << h;
			if (l & (len - 1)) push((l >> h) - 1, 0, len / 2, len);
			if ((r & (len - 1)) && (l >> h != (r - 1) >> h || !(l & (len - 1))))
				push(((r - 1) >> h) - 1, 0, len / 2, len);
		}
	}
	template <int op>
	void update(int, int, int, int ql, int qr, T x) {
		int l = ql + capacity, r = qr + capacity, len = 1;
		boundaries(l, r);
		for (int a = l, b = r; a < b; a >>= 1, b >>= 1, len <<= 1) {
			if (a & 1) apply<op>(a++ - 1, len, x);
			if (b & 1) apply<op>(--b - 1, len, x);
		}
		for (int h = 1; h <= std::countr_zero(unsigned(capacity)); ++h) {
			int mask = (1 << h) - 1;
			if (l & mask) {
				int k = (l >> h) - 1;
				pull(k, left(k), right(k, 0, 0));
			}
			if ((r & mask) && (l >> h != (r - 1) >> h || !(l & mask))) {
				int k = ((r - 1) >> h) - 1;
				pull(k, left(k), right(k, 0, 0));
			}
		}
	}
''' + iterative[b:]
variants['iterative_update'] = iterative
# Recursive queries need the padded extent.
variants['iterative_update'] = iterative.replace('query<0>(0, 0, n,', 'query<0>(0, 0, capacity,').replace('query<1>(0, 0, n,', 'query<1>(0, 0, capacity,').replace('query<2>(0, 0, n,', 'query<2>(0, 0, capacity,')
a = iterative.index('\ttemplate <int op>\n\tResult<op> query(')
b = iterative.index('\tvoid collect(', a)
iterative = iterative[:a] + '''	template <int op>
	Result<op> query(int, int, int, int l, int r) const {
		if (tree[0].lo == tree[0].hi) {
			if constexpr (op == 0) return Sum(tree[0].lo) * (r - l);
			else return tree[0].lo;
		}
		l += capacity;
		r += capacity;
		boundaries(l, r);
		Result<op> result;
		if constexpr (op == 0) result = 0;
		if constexpr (op == 1) result = inf;
		if constexpr (op == 2) result = neg_inf;
		auto merge = [&](int k) {
			if constexpr (op == 0) result += tree[k].sum;
			if constexpr (op == 1) result = std::min(result, tree[k].lo);
			if constexpr (op == 2) result = std::max(result, tree[k].hi);
		};
		for (; l < r; l >>= 1, r >>= 1) {
			if (l & 1) merge(l++ - 1);
			if (r & 1) merge(--r - 1);
		}
		return result;
	}
''' + iterative[b:]
variants['iterative'] = iterative

variants['noinit'] = source.replace('#include <vector>', '#include <vector>\n#include "components/default_init_allocator.hpp"').replace('std::vector<Node> tree;', 'std::vector<Node, DefaultInitAllocator<Node>> tree;').replace('build(rc, m, r, f);', 'build(rc, m, r, f);\n\t\ttree[k].add = 0;')
a = source.index('\t\tif (b.lo < c.lo)')
b = source.index('\n\t}', a)
variants['equal_first'] = source[:a] + '''		if (b.lo == c.lo) a.lo = b.lo, a.lo2 = std::min(b.lo2, c.lo2), a.nlo = b.nlo + c.nlo;
		else if (b.lo < c.lo) a.lo = b.lo, a.lo2 = std::min(b.lo2, c.lo), a.nlo = b.nlo;
		else a.lo = c.lo, a.lo2 = std::min(b.lo, c.lo2), a.nlo = c.nlo;
		if (b.hi == c.hi) a.hi = b.hi, a.hi2 = std::max(b.hi2, c.hi2), a.nhi = b.nhi + c.nhi;
		else if (b.hi > c.hi) a.hi = b.hi, a.hi2 = std::max(b.hi2, c.hi), a.nhi = b.nhi;
		else a.hi = c.hi, a.hi2 = std::max(b.hi, c.hi2), a.nhi = c.nhi;''' + source[b:]
variants['branchless'] = source[:a] + '''		a.lo = std::min(b.lo, c.lo);
		a.hi = std::max(b.hi, c.hi);
		a.lo2 = std::min(b.lo == a.lo ? b.lo2 : b.lo, c.lo == a.lo ? c.lo2 : c.lo);
		a.hi2 = std::max(b.hi == a.hi ? b.hi2 : b.hi, c.hi == a.hi ? c.hi2 : c.hi);
		a.nlo = (b.lo == a.lo ? b.nlo : 0) + (c.lo == a.lo ? c.nlo : 0);
		a.nhi = (b.hi == a.hi ? b.nhi : 0) + (c.hi == a.hi ? c.nhi : 0);''' + source[b:]
for name, code in variants.items():
    cls = 'Beats_' + name
    code = code.replace('SEGTREE_BEATS_HPP', 'SEGTREE_BEATS_' + name.upper() + '_HPP').replace('SegtreeBeats', cls)
    (out / (name + '.hpp')).write_text(code)
    (out / (name + '_test.cpp')).write_text(unit.replace('"data_structure/segment_tree/segtree_beats.hpp"', '"' + name + '.hpp"').replace('SegtreeBeats', cls))
