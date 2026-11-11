#define PROBLEM "https://judge.yosupo.jp/problem/dominatortree"
#include "graph/dominator_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, root = 0;
	input.read(n), input.read(m), input.read(root);
	DominatorTree graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int from = 0, to = 0;
		input.read(from), input.read(to);
		graph.add_edge(from, to);
	}
	auto parent = graph.idom(root);
	for (int v = 0; v < n; ++v) {
		if (v) output.write(' ');
		output.write(parent[v]);
	}
	output.write('\n');
}
