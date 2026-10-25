#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "graph/range_edge_graph.hpp"
#include "graph/dijkstra.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, s = 0, t = 0;
	input.read(n), input.read(m), input.read(s), input.read(t);
	RangeEdgeGraph<> graph(n);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t w = 0;
		input.read(u), input.read(v), input.read(w);
		switch (i % 4) {
		case 0: graph.add_edge(u, v, w); break;
		case 1: graph.add_to_range(u, v, v + 1, w); break;
		case 2: graph.add_from_range(u, u + 1, v, w); break;
		case 3: graph.add_range_to_range(u, u + 1, v, v + 1, w); break;
		}
	}
	auto [cost, expanded] = dijkstra_path(graph.build(), s, t);
	if (cost == -1) { output.println(-1); return 0; }
	auto path = graph.original_path(expanded);
	output.write(cost), output.write(' '), output.println(path.size() - 1);
	for (std::size_t i = 1; i < path.size(); ++i) output.write(path[i - 1]), output.write(' '), output.println(path[i]);
}
