#include "graph/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

template<class G, class T>
void check(const G &g, int start, int target, T inf) {
	auto expected = dijkstra(g, start, inf)[target];
	for (bool topological : {false, true}) {
		auto [cost, path] = topological ? dijkstra_path<true>(g, start, target, inf) : dijkstra_path(g, start, target, inf);
		assert(cost == expected);
		if (cost == T(-1)) { assert(path.empty()); continue; }
		assert(!path.empty() && path.front() == start && path.back() == target);
		assert(path.size() <= std::size_t(g.size()));
		T sum = 0;
		for (std::size_t i = 1; i < path.size(); ++i) {
			using W = std::remove_cvref_t<decltype(g[path[i - 1]][0].second)>;
			W edge{};
			bool found = false;
			for (const auto &[to, weight] : g[path[i - 1]])
				if (to == path[i] && (!found || weight < edge)) edge = weight, found = true;
			assert(found && edge < inf - sum);
			sum = T(sum + edge);
		}
		assert(sum == cost);
	}
}

template<class T>
void typed() {
	using G = std::vector<std::vector<std::pair<int, T>>>;
	const T inf = T(10000000);
	G g(80);
	for (int v = 0; v < 59; ++v) g[v].emplace_back(v + 1, T(v % 3));
	g[59] = { {60, T(0)}, {61, T(10)} };
	g[60] = { {61, T(2)}, {4, T(0)} };
	g[61] = { {62, T(1)}, {63, T(5)} };
	g[62] = { {61, T(0)}, {63, T(1)} };
	for (int target = 0; target < 80; ++target) check(g, 0, target, inf);
	g[59] = { {7, T(0)} };
	for (int target : {0, 1, 7, 58, 59, 60, 79}) check(g, 0, target, inf);
	g[59] = { {59, T(0)} };
	check(g, 0, 79, inf);
	g[59].clear();
	check(g, 0, 79, inf);
}

int main() {
	typed<int>();
	typed<std::int64_t>();
	typed<std::uint64_t>();
	typed<__int128_t>();
	typed<double>();
	std::mt19937_64 random(20261019);
	using G = std::vector<std::vector<std::pair<int, std::int64_t>>>;
	for (int trial = 0; trial < 300; ++trial) {
		int n = 2 + int(random() % 45), prefix = int(random() % (n - 1));
		G g(n);
		for (int v = 0; v < prefix; ++v) g[v].emplace_back(v + 1, random() % 11);
		for (int v = prefix; v < n; ++v)
			for (int j = 0, degree = int(random() % 4); j < degree; ++j)
				g[v].emplace_back(random() % n, random() % 11);
		for (int target = 0; target < n; ++target) check(g, 0, target, std::int64_t(1000000));
	}
	const auto maximum = std::numeric_limits<std::uint64_t>::max();
	using U = std::vector<std::vector<std::pair<int, std::uint64_t>>>;
	U huge = { { {1, maximum - 5} }, { {2, 1}, {3, 4} }, { {3, 1} }, {} };
	for (int target = 0; target < 4; ++target) check(huge, 0, target, maximum);
	huge[0][0].second = maximum;
	check(huge, 0, 3, maximum);
	using F = DirectedCSR<double>;
	std::vector<F::Edge> mixed_edges{ {0, 1, 1e16}, {1, 2, 1.5}, {1, 3, 0} };
	F mixed(4, mixed_edges);
	check(mixed, 0, 2, std::int64_t(DIJKSTRA_INF));
	assert(dijkstra_path(mixed, 0, 2, std::int64_t(DIJKSTRA_INF)).first == 10000000000000002LL);
	using Graph = DirectedCSR<std::uint32_t>;
	std::vector<Graph::Edge> edges;
	for (int v = 0; v < 199999; ++v) edges.push_back({v, v + 1, 1000000000U});
	Graph line(200001, edges);
	check(line, 0, 199999, std::int64_t(DIJKSTRA_INF));
	check(line, 0, 200000, std::int64_t(DIJKSTRA_INF));
	edges.back().to = 1234;
	Graph cycle(200001, edges);
	check(cycle, 0, 200000, std::int64_t(DIJKSTRA_INF));
}
