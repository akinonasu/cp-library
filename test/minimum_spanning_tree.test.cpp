#define PROBLEM "https://judge.yosupo.jp/problem/minimum_spanning_tree"
#include "graph/kruskal.hpp"
#include "components/fast_io_bulk.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	Kruskal<std::uint32_t, std::uint64_t> graph(n);
	graph.reserve(m);
	read_unsigned_records<3>(input, m, [&](unsigned u, unsigned v, unsigned cost) { graph.add_edge(u, v, cost); });
	auto result = graph.solve();
	output.println(result.cost);
	write_unsigned_records<1>(output, result.edges.size(), [&](std::size_t i) { return std::array{unsigned(result.edges[i])}; });
}
