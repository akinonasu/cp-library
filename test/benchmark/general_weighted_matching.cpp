#include "graph/matching/general_weighted_matching.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <tuple>

int main(int argc, char **argv) {
	if (argc != 2) return 1;
	std::ifstream input(argv[1]);
	int n = 0, m = 0;
	input >> n >> m;
	std::vector<std::tuple<int, int, std::int64_t>> edges(m);
	for (auto &[u, v, w] : edges) input >> u >> v >> w;
	if (!input) return 1;
	auto start = std::chrono::steady_clock::now();
	GeneralWeightedMatching graph(n);
	for (auto [u, v, w] : edges) graph.add_edge(u, v, w);
	auto built = std::chrono::steady_clock::now();
	auto result = graph.build();
	auto end = std::chrono::steady_clock::now();
	std::vector<std::int64_t> weights(std::size_t(n) * n);
	for (auto [u, v, w] : edges) weights[std::size_t(u) * n + v] = weights[std::size_t(v) * n + u] = w;
	__int128_t total = 0;
	for (int u = 0; u < n; ++u) if (graph.mate[u] != -1) {
		int v = graph.mate[u];
		if (v < 0 || v >= n || u == v || graph.mate[v] != u || weights[std::size_t(u) * n + v] <= 0) std::abort();
		if (u < v) total += weights[std::size_t(u) * n + v];
	}
	if (total != result) std::abort();
	std::printf("{\"graph_ms\":%.6f,\"solve_ms\":%.6f,\"weight\":%lld,\"count\":%d}\n",
		std::chrono::duration<double, std::milli>(built - start).count(),
		std::chrono::duration<double, std::milli>(end - built).count(), (long long)result, graph.matching_size());
}
