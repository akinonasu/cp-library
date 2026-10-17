#define PROBLEM "https://judge.yosupo.jp/problem/min_cost_b_flow"
#include "graph/min_cost_flow.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> balance(n);
	std::int64_t sum = 0;
	for (auto& b : balance) input.read(b), sum += b;
	MinCostFlow<> graph(n + 2);
	graph.reserve(m + n);
	struct Edge { std::int64_t base; bool reverse; };
	std::vector<Edge> edges;
	__int128_t base_cost = 0;
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t lower = 0, upper = 0, cost = 0;
		input.read(u), input.read(v), input.read(lower), input.read(upper), input.read(cost);
		bool reverse = cost < 0;
		auto base = reverse ? upper : lower;
		balance[u] -= base, balance[v] += base;
		base_cost += __int128_t(base) * cost;
		if (reverse) graph.add_edge(v, u, upper - lower, -cost);
		else graph.add_edge(u, v, upper - lower, cost);
		edges.push_back({base, reverse});
	}
	if (sum) { output.println("infeasible"); return 0; }
	std::int64_t need = 0;
	for (int v = 0; v < n; ++v) {
		if (balance[v] > 0) graph.add_edge(n, v, balance[v], 0), need += balance[v];
		else if (balance[v] < 0) graph.add_edge(v, n + 1, -balance[v], 0);
	}
	auto [flow, cost] = graph.flow(n, n + 1, need);
	if (flow != need) { output.println("infeasible"); return 0; }
	output.println(base_cost + cost);
	const auto& potentials = graph.potentials();
	for (int v = 0; v < n; ++v) output.println(potentials[v]);
	for (int i = 0; i < m; ++i) {
		auto flow_value = graph.get_edge(i).flow;
		output.println(edges[i].base + (edges[i].reverse ? -flow_value : flow_value));
	}
}
