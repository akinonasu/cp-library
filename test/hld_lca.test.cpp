#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include "tree/hld.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> parents(n, -1);
	for (int v = 1; v < n; ++v) input.read(parents[v]);
	HLD tree;
	tree.build_from_parents(std::move(parents));
	while (q--) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		output.println(tree.lca(u, v));
	}
}
