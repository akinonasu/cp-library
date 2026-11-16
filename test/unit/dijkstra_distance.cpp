#include "graph/dijkstra.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

template<class W, class D>
void check(const std::vector<std::vector<std::pair<int, W>>> &graph, D inf) {
	const auto saved = graph;
	int n = int(graph.size());
	std::vector<std::vector<D>> expected(n, std::vector<D>(n, inf));
	for (int v = 0; v < n; ++v) {
		expected[v][v] = 0;
		for (auto [u, weight] : graph[v]) if (__uint128_t(weight) < __uint128_t(inf))
			expected[v][u] = std::min(expected[v][u], D(weight));
	}
	for (int k = 0; k < n; ++k)
		for (int v = 0; v < n; ++v)
			for (int u = 0; u < n; ++u) {
				if (expected[v][k] == inf || expected[k][u] >= inf - expected[v][k]) continue;
				expected[v][u] = std::min(expected[v][u], D(expected[v][k] + expected[k][u]));
			}
	std::vector<typename DirectedCSR<W>::Edge> edges;
	for (int v = 0; v < n; ++v) for (auto [u, w] : graph[v]) edges.push_back({v, u, w});
	DirectedCSR<W> csr(n, edges);
	for (int source = 0; source < n; ++source) {
		for (auto &value : expected[source]) if (value == inf) value = D(-1);
		assert(dijkstra(graph, source, inf) == expected[source]);
		assert(dijkstra<true>(graph, source, inf) == expected[source]);
		assert(dijkstra(csr, source, inf) == expected[source]);
		for (int target = 0; target < n; ++target) {
			for (bool topological : {false, true}) {
				auto [distance, path] = topological ? dijkstra_path<true>(graph, source, target, inf) : dijkstra_path(graph, source, target, inf);
				assert(distance == expected[source][target]);
				if (distance == D(-1)) { assert(path.empty()); continue; }
				assert(path.front() == source && path.back() == target && path.size() <= graph.size());
				D total = 0;
				for (std::size_t i = 1; i < path.size(); ++i) {
					D best = inf;
					for (auto [u, weight] : graph[path[i - 1]]) if (u == path[i] && __uint128_t(weight) < __uint128_t(inf)) best = std::min(best, D(weight));
					assert(best < inf && best < inf - total);
					total += best;
				}
				assert(total == distance);
			}
		}
	}
	assert(graph == saved);
}

template<class W, class D>
void random_cases(std::mt19937_64 &random, D inf) {
	for (int trial = 0; trial < 60; ++trial) {
		int n = 1 + int(random() % 13);
		std::vector<std::vector<std::pair<int, W>>> graph(n);
		for (int repeat = 0, m = int(random() % (n * n + 1)); repeat < m; ++repeat) {
			int v = int(random() % n), u = int(random() % n);
			W weight = trial % 3 == 0 ? W(random() % 5) : trial % 3 == 1 ? W(random() % 2'000'000'000) : W(random() & std::numeric_limits<W>::max());
			graph[v].push_back({u, weight});
		}
		check(graph, inf);
	}
}

int main() {
	std::mt19937_64 random(292914);
	using Graph = std::vector<std::vector<std::pair<int, int>>>;
	Graph graph{ { {1, 1'500'000'000} }, { {2, 1'500'000'000} }, {} };
	static_assert(std::is_same_v<decltype(dijkstra(graph, 0)), std::vector<int>>);
	static_assert(std::is_same_v<decltype(dijkstra<true>(graph, 0)), std::vector<int>>);
	static_assert(std::is_same_v<decltype(dijkstra(graph, 0, INT64_MAX)), std::vector<std::int64_t>>);
	assert((dijkstra(graph, 0, INT64_MAX) == std::vector<std::int64_t>{0, 1'500'000'000, 3'000'000'000}));
	check(graph, std::int64_t(INT64_MAX));
	random_cases<int, std::int64_t>(random, INT64_MAX);
	random_cases<std::uint32_t, std::int64_t>(random, INT64_MAX);
	random_cases<std::int64_t, __int128_t>(random, __int128_t(1) << 100);
	random_cases<std::uint64_t, __int128_t>(random, __int128_t(1) << 100);
	random_cases<std::uint64_t, __uint128_t>(random, __uint128_t(1) << 100);
	check<std::uint64_t, int>({ { {1, UINT64_MAX}, {2, 7} }, {}, { {1, 9} } }, INT32_MAX);
	std::vector<std::vector<std::pair<int, float>>> fractional{ { {1, .25f}, {2, .75f} }, { {2, .25f} }, {} };
	assert((dijkstra(fractional, 0, 100.) == std::vector<double>{0, .25, .5}));
	assert((dijkstra_path<true>(fractional, 0, 2, 100.) == std::pair<double, std::vector<int>>{.5, {0, 1, 2}}));
}
