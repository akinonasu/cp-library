#define PROBLEM "https://judge.yosupo.jp/problem/tree_path_composite_sum"
#include "tree/rerooting.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"

using Data = std::pair<mint, mint>;
std::vector<mint> a, b, c;
Data merge(Data x, Data y) { return {x.first + y.first, x.second + y.second}; }
Data e() { return {0, 0}; }
Data put_edge(Data x, int id) { return {x.first * b[id] + x.second * c[id], x.second}; }
Data put_vertex(Data x, int v) { return {x.first + a[v], x.second + 1}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	RerootingDP<Data, Data, merge, e, put_edge, put_vertex> tree(n);
	a.resize(n), b.resize(n - 1), c.resize(n - 1);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = value; }
	for (int id = 0; id < n - 1; ++id) {
		int u = 0, v = 0;
		unsigned x = 0, y = 0;
		input.read(u), input.read(v), input.read(x), input.read(y);
		b[id] = x, c[id] = y;
		tree.add_edge(u, v, id, id);
	}
	tree.build();
	for (auto x : tree.reroot()) output.write(x.first.val()), output.write(' ');
	output.write('\n');
}
