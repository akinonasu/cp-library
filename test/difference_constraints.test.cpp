#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_B"
#include "graph/difference_constraints.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0, source = 0;
	input.read(n);
	input.read(m);
	input.read(source);
	struct Edge { int from, to; std::int64_t cost; };
	std::vector<Edge> edges(m);
	std::vector<std::vector<int>> graph(n);
	for (auto &edge : edges) {
		input.read(edge.from);
		input.read(edge.to);
		input.read(edge.cost);
		graph[edge.from].push_back(edge.to);
	}
	std::vector<bool> reachable(n);
	std::vector<int> order{source};
	reachable[source] = true;
	for (std::size_t i = 0; i < order.size(); ++i) for (int to : graph[order[i]])
		if (!reachable[to]) reachable[to] = true, order.push_back(to);
	DifferenceConstraints<> system(n);
	system.reserve(m);
	for (auto edge : edges) if (reachable[edge.from]) system.add_le(edge.from, edge.to, edge.cost);
	auto result = system.solve();
	if (!result.feasible()) {
		output.write("NEGATIVE CYCLE\n");
		return 0;
	}
	auto bounds = result.upper_bounds(source);
	for (auto bound : bounds) {
		if (bound) {
			output.write(*bound);
			output.write('\n');
		} else output.write("INF\n");
	}
}
