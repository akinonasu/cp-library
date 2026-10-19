#define PROBLEM "https://judge.yosupo.jp/problem/tree_path_composite_sum"
#include "tree/rerooting.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	Rerooting<std::pair<mint, mint>> tree(n);
	std::vector<mint> a(n), b(n - 1), c(n - 1);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = value; }
	for (int id = 0; id < n - 1; ++id) {
		int u = 0, v = 0;
		unsigned x = 0, y = 0;
		input.read(u), input.read(v), input.read(x), input.read(y);
		b[id] = x, c[id] = y;
		tree.add_edge(u, v);
	}
	using Data = std::pair<mint, mint>;
	tree.build(Data{0, 0}, [](Data x, Data y) { return Data{x.first + y.first, x.second + y.second}; },
		[&](Data x, int v) { return Data{x.first + a[v], x.second + 1}; },
		[&](Data x, int, int, int id) { return Data{x.first * b[id] + x.second * c[id], x.second}; });
	for (auto x : tree.values()) output.write(x.first.val()), output.write(' ');
	output.write('\n');
}
