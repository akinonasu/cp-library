#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_path_sum"
#include "../data_structure/fenwick_tree.hpp"
#include "../data_structure/hld.hpp"
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
	for (int i = 1, u, v; i < n; ++i) {
		input.read(u), input.read(v);
		tree.add_edge(u, v);
	}
	tree.build();
	std::vector<long long> ordered(n);
	for (int v = 0; v < n; ++v) ordered[tree.in[v]] = a[v];
	FenwickTree<long long> seg(ordered);
	while (q--) {
		int type, u;
		long long v;
		input.read(type), input.read(u), input.read(v);
		if (type == 0) seg.add(tree.in[u], v);
		else {
			long long result = 0;
			tree.for_each_path(u, int(v), [&](int l, int r, bool) { result += seg.sum(l, r); });
			output.println(result);
		}
	}
}
