#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"
#include "../data_structure/fenwick_tree.hpp"
#include "../data_structure/advanced/hld.hpp"
#include "components/fast_io_bulk.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<long long> a(n);
	std::size_t offset = 0;
	read_unsigned_blocks(input, n, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) a[offset++] = values[i];
	});
	std::vector<int> parents(n, -1);
	offset = 1;
	read_unsigned_blocks(input, n - 1, [&](const std::uint32_t *values, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) parents[offset++] = values[i];
	});
	HLD tree;
	tree.build_from_parents(std::move(parents));

	std::vector<long long> ordered(n);
	for (int v = 0; v < n; ++v) ordered[tree.in[v]] = a[v];
	FenwickTree<long long> seg(ordered);
	while (q--) {
		int type, v;
		input.read(type), input.read(v);
		if (type == 0) {
			long long x;
			input.read(x);
			seg.add(tree.in[v], x);
		} else {
			auto [l, r] = tree.subtree(v);
			output.println(seg.sum(l, r));
		}
	}
}
