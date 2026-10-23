#include "../../graph/connectivity/scc.hpp"
#include <cassert>
#include <random>

void check(int n, const std::vector<std::pair<int, int>> &edges) {
	SCCGraph graph(n);
	std::vector<std::vector<bool>> reaches(n, std::vector<bool>(n));
	for (int v = 0; v < n; ++v) reaches[v][v] = true;
	for (auto [u, v] : edges) graph.add_edge(u, v), reaches[u][v] = true;
	for (int k = 0; k < n; ++k) for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
		reaches[u][v] = reaches[u][v] || (reaches[u][k] && reaches[k][v]);
	auto result = graph.decomposition();
	auto old = graph.scc();
	assert(result.size() == int(old.size()) && result.id.size() == std::size_t(n));
	std::vector<bool> seen(n);
	for (int id = 0; id < result.size(); ++id) {
		auto group = result[id];
		assert(std::vector<int>(group.begin(), group.end()) == old[id]);
		for (int v : group) { assert(result.id[v] == id && !seen[v]); seen[v] = true; }
	}
	for (int v = 0; v < n; ++v) assert(seen[v]);
	for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) {
		assert((result.id[u] == result.id[v]) == (reaches[u][v] && reaches[v][u]));
		if (reaches[u][v]) assert(result.id[u] <= result.id[v]);
	}
	auto copy = result;
	auto moved = std::move(result);
	assert(copy.id == moved.id && result.size() == 0);
	for (int id = 0; id < moved.size(); ++id) {
		auto a = moved[id], b = copy[id];
		assert(std::equal(a.begin(), a.end(), b.begin(), b.end()));
	}
}

int main() {
	check(0, {});
	for (unsigned mask = 0; mask < 512; ++mask) {
		std::vector<std::pair<int, int>> edges;
		for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) if ((mask >> (u * 3 + v)) & 1) edges.emplace_back(u, v);
		check(3, edges);
	}
	std::mt19937 random(29);
	for (int repeat = 0; repeat < 500; ++repeat) {
		int n = random() % 35;
		std::vector<std::pair<int, int>> edges;
		for (int i = 0, m = n ? random() % 150 : 0; i < m; ++i) edges.emplace_back(random() % n, random() % n);
		check(n, edges);
	}
	check(200, {});
	std::vector<std::pair<int, int>> cycle;
	for (int v = 0; v < 200; ++v) cycle.emplace_back(v, (v + 1) % 200);
	check(200, cycle);
}
