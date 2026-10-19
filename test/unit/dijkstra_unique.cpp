#include "graph/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

template<class W, class T>
T add_cost(T cost, W weight, T inf) {
	if constexpr (std::numeric_limits<W>::is_integer && std::numeric_limits<T>::is_integer) {
		auto sum = static_cast<__uint128_t>(cost) + static_cast<__uint128_t>(weight);
		if (sum >= static_cast<__uint128_t>(inf)) return inf;
		return static_cast<T>(sum);
	} else {
		if (static_cast<long double>(cost) + static_cast<long double>(weight) >= static_cast<long double>(inf)) return inf;
		T sum = T(cost + weight);
		return sum < inf ? sum : inf;
	}
}

template<class W, class T>
std::vector<T> oracle(int n, const std::vector<typename DirectedCSR<W>::Edge> &edges, int source, T inf) {
	std::vector<T> distance(n, inf);
	distance[source] = 0;
	for (int step = 1; step < n; ++step) {
		bool changed = false;
		for (const auto &edge : edges) if (distance[edge.from] < inf) {
			T candidate = add_cost(distance[edge.from], edge.weight, inf);
			if (candidate < distance[edge.to]) distance[edge.to] = candidate, changed = true;
		}
		if (!changed) break;
	}
	for (T &value : distance) if (value == inf) value = T(-1);
	return distance;
}

template<class W, class T>
void check_path(const DirectedCSR<W> &graph, int source, int target, T inf, T expected) {
	for (bool topological : {false, true}) {
		auto [cost, path] = topological ? dijkstra_path<true>(graph, source, target, inf) : dijkstra_path(graph, source, target, inf);
		assert(cost == expected);
		if (expected == T(-1)) { assert(path.empty()); continue; }
		assert(!path.empty() && path.front() == source && path.back() == target);
		assert(path.size() <= std::size_t(graph.size()));
		std::vector<bool> seen(graph.size());
		T sum = 0;
		for (std::size_t i = 0; i < path.size(); ++i) {
			assert(!seen[path[i]]);
			seen[path[i]] = true;
			if (!i) continue;
			bool found = false;
			W minimum{};
			for (const auto &[to, weight] : graph[path[i - 1]])
				if (to == path[i] && (!found || weight < minimum)) minimum = weight, found = true;
			assert(found);
			sum = add_cost(sum, minimum, inf);
			assert(sum < inf);
		}
		assert(sum == cost);
	}
}

template<class W, class T>
void check_graph(int n, const std::vector<typename DirectedCSR<W>::Edge> &edges, T inf) {
	DirectedCSR<W> graph(n, edges);
	DirectedCSR<W> produced(n, edges.size(), [&](auto emit) { for (auto edge : edges) emit(edge); });
	std::vector<int> indegree(n);
	for (const auto &edge : edges) ++indegree[edge.to];
	bool unique = std::all_of(indegree.begin(), indegree.end(), [](int degree) { return degree <= 1; });
	assert(directed_csr_detail::Access::unique_incoming(graph) == unique);
	assert(directed_csr_detail::Access::unique_incoming(produced) == unique);
	for (int source = 0; source < n; ++source) {
		auto expected = oracle<W>(n, edges, source, inf);
		for (int target = 0; target < n; ++target) {
			check_path(graph, source, target, inf, expected[target]);
			check_path(produced, source, target, inf, expected[target]);
		}
	}
}

template<class W, class T>
void random_graphs() {
	std::mt19937_64 random(20261019);
	for (int trial = 0; trial < 140; ++trial) {
		int n = 3 + int(random() % 17);
		std::vector<typename DirectedCSR<W>::Edge> edges;
		for (int to = 0; to < n; ++to) if (random() % 4) {
			W weight = W(random() % 50);
			if constexpr (std::is_floating_point_v<W>) weight /= 4;
			edges.push_back({int(random() % n), to, weight});
		}
		if (trial % 5 == 0) {
			edges.clear();
			for (int v = 0; v < n; ++v) edges.push_back({v, (v + 1) % n, W(trial % 2)});
		}
		if (trial % 7 == 0) {
			edges.clear();
			for (int v = 1; v < n; ++v) edges.push_back({0, v, W(v % 3)});
		}
		if (trial % 3 == 0) {
			edges.push_back({0, 2, W(8)});
			edges.push_back({1, 2, W(3)});
		}
		std::shuffle(edges.begin(), edges.end(), random);
		check_graph<W>(n, edges, T(trial % 2 ? 31 : 1000));
	}
}

void rebuild() {
	using Graph = DirectedCSR<std::uint32_t>;
	Graph graph;
	for (int trial = 0; trial < 30; ++trial) {
		std::vector<Graph::Edge> edges;
		if (trial % 3 == 0) edges = { {0, 1, 2}, {0, 2, 3}, {2, 3, 4}, {4, 5, 0}, {5, 4, 0} };
		if (trial % 3 == 1) edges = { {0, 1, 8}, {0, 2, 2}, {2, 1, 1}, {1, 3, 5} };
		if (trial % 2) graph.build(6, edges);
		else graph.build(6, edges.size(), [&](auto emit) { for (auto edge : edges) emit(edge); });
		auto expected = oracle<std::uint32_t>(6, edges, 0, std::int64_t(100));
		for (int target = 0; target < 6; ++target) check_path(graph, 0, target, std::int64_t(100), expected[target]);
		Graph copied = graph, moved = std::move(copied);
		for (int target = 0; target < 6; ++target) check_path(moved, 0, target, std::int64_t(100), expected[target]);
	}
}

int main() {
	random_graphs<std::uint32_t, std::int64_t>();
	random_graphs<std::uint64_t, std::uint64_t>();
	random_graphs<std::int64_t, __int128_t>();
	random_graphs<double, double>();
	random_graphs<double, std::int64_t>();
	random_graphs<std::uint64_t, std::int32_t>();
	random_graphs<__uint128_t, std::int64_t>();
	check_graph<std::uint32_t>(10, { {0, 1, 0}, {1, 2, 0}, {2, 0, 0}, {1, 3, 5}, {1, 4, 0}, {3, 5, 1}, {4, 6, 3}, {6, 7, 0} }, std::int64_t(100));
	check_graph<std::uint32_t>(6, { {0, 1, 3}, {1, 2, 4}, {2, 3, 5}, {2, 4, 1} }, std::int64_t(12));
	check_graph<std::uint64_t>(5, { {0, 1, 5}, {0, 2, 4}, {1, 3, std::numeric_limits<std::uint64_t>::max()}, {2, 4, 1} }, std::int32_t(100));
	const auto maximum = std::numeric_limits<std::uint64_t>::max();
	check_graph<std::uint64_t>(6, { {0, 1, maximum - 5}, {0, 2, maximum - 1}, {1, 3, 3}, {1, 4, 5}, {3, 5, 1} }, maximum);
	check_graph<__uint128_t>(6, { {0, 1, 3}, {0, 2, __uint128_t(1) << 100}, {1, 3, 4}, {1, 4, 97} }, std::int64_t(100));
	check_graph<__int128_t>(6, { {0, 1, __int128_t(1) << 100}, {0, 2, 0}, {1, 3, __int128_t(1) << 99}, {1, 4, 3} }, __int128_t(1) << 120);
	check_graph<double>(6, { {0, 1, 1e16}, {1, 2, 1.5}, {1, 3, 0.25}, {3, 4, 1.5} }, std::int64_t(DIJKSTRA_INF));
	rebuild();
}
