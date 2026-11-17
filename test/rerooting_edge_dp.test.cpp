#define PROBLEM "https://judge.yosupo.jp/problem/tree_path_composite_sum"
#include "tree/rerooting.hpp"
#include "math/mint.hpp"
#include "components/fast_io.hpp"

using Data = std::pair<mint, mint>;

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<mint> value(n), scale(n - 1), shift(n - 1);
	for (auto& x : value) { unsigned a = 0; input.read(a); x = a; }
	Rerooting<Data> tree(n);
	for (int id = 0; id < n - 1; ++id) {
		int u = 0, v = 0;
		unsigned a = 0, b = 0;
		input.read(u), input.read(v), input.read(a), input.read(b);
		tree.add_edge(u, v);
		scale[id] = a, shift[id] = b;
	}
	auto merge = [](Data a, Data b) { return Data{a.first + b.first, a.second + b.second}; };
	tree.build(Data{0, 0}, merge,
		[&](Data x, int v) { return Data{x.first + value[v], x.second + 1}; },
		[&](Data x, int, int, int id) { return Data{x.first * scale[id] + x.second * shift[id], x.second}; });
	std::vector<mint> answer = value;
	for (int id = 0; id < tree.edge_count(); ++id) {
		auto [u, v] = tree.edge(id);
		answer[v] += tree.edge_dp(id).first;
		answer[u] += tree.edge_dp(id, true).first;
	}
	for (auto x : answer) output.write(x.val()), output.write(' ');
	output.write('\n');
}
