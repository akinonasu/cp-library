#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "tree/link_cut_tree.hpp"
#include "components/fast_io.hpp"

int op(int a, int b) { return a + b; }
int e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::pair<int, int>> edges;
	edges.reserve(n - 1);
	for (int v = 1; v < n; ++v) {
		int parent = 0;
		input.read(parent);
		edges.emplace_back(parent, v);
	}
	LinkCutTree<int, op, e, true> tree(n);
	tree.build_forest(edges);
	while (q--) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		output.println(tree.lca(0, u, v));
	}
}
