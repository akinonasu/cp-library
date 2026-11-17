#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_3_A"
#include "graph/block_cut_tree.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	BlockCutTree graph(n);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		graph.add_edge(u, v);
	}
	graph.build();
	for (int v = 0; v < n; ++v) if (graph.is_articulation(v)) output.println(v);
}
