#define PROBLEM "https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root"
#include "data_structure/static_top_tree.hpp"
#include "components/fast_io.hpp"
#include "test/fixtures/static_top_tree_dp.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	using DP = StaticTopTreeSumDP;
	std::vector<DP::Vertex> values(n);
	for (auto& v : values) { unsigned x = 0; input.read(x); v.value = x; }
	HLD tree(n);
	std::vector<std::pair<mint, mint>> functions;
	for (int i = 0; i < n - 1; ++i) {
		int u = 0, v = 0;
		unsigned a = 0, b = 0;
		input.read(u), input.read(v), input.read(a), input.read(b);
		tree.add_edge(u, v), functions.emplace_back(a, b);
	}
	tree.build();
	for (int i = 0; i < n - 1; ++i) {
		auto [u, v] = tree.edges[i];
		if (tree.parent[u] == v) std::swap(u, v);
		values[v].a = functions[i].first, values[v].b = functions[i].second;
	}
	StaticTopTree<DP> dp(tree, std::move(values));
	for (int j = 0; j < q; ++j) {
		int type = 0, i = 0;
		unsigned x = 0, y = 0;
		input.read(type), input.read(i), input.read(x);
		if (type == 0) {
			auto value = dp.get(i); value.value = x; dp.set(i, value);
		} else {
			input.read(y);
			int v = dp.edge_vertex(i);
			auto value = dp.get(v); value.a = x, value.b = y; dp.set(v, value);
		}
		output.println(dp.all_prod().sum.val());
	}
}
