#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"
#include "../data_structure/fenwick_tree.hpp"
#include "../data_structure/advanced/hld.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	std::vector<long long> a(n);
	for (auto &x : a) input.read(x);
	HLD tree(n);
	for (int v = 1, p; v < n; ++v) {
		input.read(p);
		tree.add_edge(p, v);
	}
	tree.build();
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
