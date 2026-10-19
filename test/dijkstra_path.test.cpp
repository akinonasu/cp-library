#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include "../graph/dijkstra.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

#ifndef DIJKSTRA_USE_TOPOLOGICAL
#define DIJKSTRA_USE_TOPOLOGICAL false
#endif

int main() {
	FastInput in;
	FastOutput out;
	int n, m, s, t;
	in.read(n), in.read(m), in.read(s), in.read(t);
	using Graph = DirectedCSR<std::uint32_t>;
	std::vector<Graph::Edge> edges(m);
	for (auto &edge : edges) {
		in.read(edge.from), in.read(edge.to), in.read(edge.weight);
	}
	Graph g(n, edges);
	auto [distance, path] = dijkstra_path<DIJKSTRA_USE_TOPOLOGICAL>(g, s, t);
	if (distance == -1) out.println(-1);
	else {
		out.write(distance), out.write(' '), out.println(path.size() - 1);
		for (std::size_t i = 1; i < path.size(); ++i)
			out.write(path[i - 1]), out.write(' '), out.println(path[i]);
	}
}
