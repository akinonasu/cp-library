#define PROBLEM "https://judge.yosupo.jp/problem/minimum_spanning_tree"
#include "graph/kruskal.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	Kruskal<> graph(n);
	graph.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t cost = 0;
		input.read(u), input.read(v), input.read(cost);
		graph.add_edge(u, v, cost);
	}
	auto result = graph.solve();
	output.println(result.cost);
	for (std::size_t i = 0; i < result.edges.size(); ++i) {
		if (i) output.write(' ');
		output.write(result.edges[i]);
	}
	output.write('\n');
}
