#include "graph/shortest_path/bellman_ford.hpp"

int main() {
	int n = 200000;
	std::vector<std::vector<std::pair<int, int>>> g(n);
	for (int v = 1; v < n; ++v) g[v].push_back({v - 1, -1});
	auto comp = negative_path_detail::reachable_components(g, n - 1);
	assert(comp.offset.size() == std::size_t(n + 1));
	for (int v = 0; v < n; ++v) assert(comp.id[v] == v);
	auto hybrid = bellman_ford(g, n - 1);
	auto raw = goldberg_radzik(g, n - 1);
	assert(hybrid.distance[0] == 1 - n && raw.distance == hybrid.distance);
	assert(hybrid.path(0).size() == std::size_t(n));
	g[0].push_back({n - 1, n - 1});
	assert(!bellman_ford(g, n - 1).has_negative_cycle());
	g[0][0].second = n - 2;
	auto negative = bellman_ford(g, n - 1);
	assert(negative.has_negative_cycle() && negative.negative_cycle.size() == std::size_t(n + 1));
}
