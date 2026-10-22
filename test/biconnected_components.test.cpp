#define PROBLEM "https://judge.yosupo.jp/problem/biconnected_components"
#include "graph/connectivity/block_cut_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	BlockCutTree graph(n);
	graph.edges.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		graph.add_edge(u, v);
	}
	graph.build();
	output.println(graph.block_count());
	for (int c = 0; c < graph.block_count(); ++c) {
		auto group = graph.group(c);
		output.write(group.size());
		for (int v : group) output.write(' '), output.write(v);
		output.write('\n');
	}
}
