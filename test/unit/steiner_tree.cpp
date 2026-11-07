#include "graph/steiner_tree.hpp"
#include <cmath>
#include <numeric>
#include <random>

using Graph = SteinerTree<>;

std::optional<std::int64_t> exhaustive(int n, const std::vector<Graph::Edge> &edges, const std::vector<int> &terminals) {
	std::optional<std::int64_t> answer;
	for (unsigned mask = 0; mask < (1U << edges.size()); ++mask) {
		std::vector<int> label(n);
		std::iota(label.begin(), label.end(), 0);
		std::int64_t sum = 0;
		for (std::size_t i = 0; i < edges.size(); ++i) if ((mask >> i) & 1) {
			auto [u, v, cost] = edges[i];
			sum += cost;
			int from = label[u], to = label[v];
			for (int &x : label) if (x == from) x = to;
		}
		bool connected = true;
		for (int v : terminals) if (label[v] != label[terminals.front()]) connected = false;
		if (connected && (!answer || sum < *answer)) answer = sum;
	}
	return answer;
}

void check(int n, const std::vector<Graph::Edge> &edges, const std::vector<int> &terminals) {
	Graph graph(n);
	graph.reserve(edges.size());
	for (std::size_t i = 0; i < edges.size(); ++i) {
		auto e = edges[i];
		assert(graph.add_edge(e.from, e.to, e.cost) == int(i));
		const auto &stored = graph.get_edge(int(i));
		assert(stored.from == e.from && stored.to == e.to && stored.cost == e.cost);
	}
	assert(graph.size() == n && graph.edge_count() == int(edges.size()));
	auto expected = exhaustive(n, edges, terminals);
	const auto &view = graph;
	auto result = view.solve(terminals), again = view.solve(terminals);
	assert(view.minimum_cost(terminals) == expected);
	assert(bool(result) == bool(expected) && bool(again) == bool(expected));
	if (!result) return;
	assert(result->cost == *expected && result->edges == again->edges);
	UnionFind uf(n);
	std::int64_t sum = 0;
	for (int id : result->edges) {
		assert(0 <= id && id < int(edges.size()));
		auto [u, v, w] = edges[id];
		assert(!uf.same(u, v));
		uf.merge(u, v);
		sum += w;
	}
	assert(sum == result->cost);
	for (int v : terminals) assert(uf.same(v, terminals.front()));
	if (*expected > 0) assert(!graph.solve(terminals, *expected));
	assert(graph.minimum_cost(terminals, *expected + 1) == expected);
}

int main() {
	check(0, {}, {});
	check(1, {}, {0, 0});
	check(5, {}, {});
	check(5, {}, {3});
	check(5, {}, {0, 4});
	check(5, { {0, 1, 0}, {1, 2, 0}, {2, 0, 0}, {2, 3, 0}, {3, 4, 0}, {4, 0, 0} }, {4, 1, 3, 4});
	check(5, { {0, 0, 0}, {0, 1, 9}, {1, 0, 2}, {1, 2, 3}, {2, 3, 4} }, {0, 3});
	check(6, { {0, 1, 2}, {1, 2, 3}, {3, 4, 0} }, {0, 2});
	check(5, { {0, 4, 1}, {1, 4, 1}, {2, 4, 1}, {3, 4, 1} }, {0, 1, 2, 3});
	std::mt19937 random(391754);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + random() % 7, m = random() % 13;
		std::vector<Graph::Edge> edges;
		for (int i = 0; i < m; ++i) edges.push_back({int(random() % n), int(random() % n), std::int64_t(random() % 11)});
		std::vector<int> terminals;
		for (int v = 0; v < n; ++v) if (random() % 2) terminals.push_back(v);
		if (trial % 3 == 0 && !terminals.empty()) terminals.push_back(terminals.front());
		std::shuffle(terminals.begin(), terminals.end(), random);
		check(n, edges, terminals);
	}
	{
		Graph graph(4);
		graph.add_edge(0, 3, 8), graph.add_edge(3, 1, 7), graph.add_edge(3, 2, 6);
		auto old = graph.solve({0, 1, 2});
		assert(old && old->cost == 21);
		graph.add_edge(0, 1, 1);
		assert(graph.minimum_cost({0, 1, 2}) == 14 && old->cost == 21);
		graph.clear();
		assert(graph.edge_count() == 0 && graph.edges().empty() && graph.size() == 4 && !graph.solve({0, 2}));
		assert(graph.add_edge(1, 3, 7) == 0 && graph.solve({1, 3})->cost == 7);
	}
	{
		constexpr auto max = std::numeric_limits<std::int64_t>::max();
		Graph graph(4);
		graph.add_edge(0, 1, max - 5), graph.add_edge(1, 2, 4), graph.add_edge(2, 3, 9);
		assert(graph.minimum_cost({0, 2}) == max - 1);
		assert(!graph.minimum_cost({0, 3}));
		graph.add_edge(0, 3, 11);
		assert(graph.minimum_cost({0, 2, 3}) == 20);
	}
	{
		SteinerTree<std::uint32_t, std::uint64_t> graph(3);
		graph.add_edge(0, 1, 4'000'000'000U), graph.add_edge(1, 2, 4'000'000'000U);
		assert(graph.minimum_cost({0, 2}) == 8'000'000'000ULL);
		SteinerTree<std::uint64_t> large(3);
		large.add_edge(0, 1, std::uint64_t(1) << 63), large.add_edge(1, 2, 7);
		assert(large.solve({0, 2})->cost == (std::uint64_t(1) << 63) + 7);
		SteinerTree<std::int64_t, __int128_t> wide(3);
		wide.add_edge(0, 1, 9'000'000'000'000'000'000LL), wide.add_edge(1, 2, 9'000'000'000'000'000'000LL);
		assert(wide.minimum_cost({0, 2}) == __int128_t(9'000'000'000'000'000'000LL) * 2);
	}
	{
		SteinerTree<double> graph(4);
		graph.add_edge(0, 3, 0.5), graph.add_edge(1, 3, 0.25), graph.add_edge(2, 3, 1.25);
		assert(graph.solve({0, 1, 2})->cost == 2.0 && graph.minimum_cost({0, 1}) == 0.75);
		assert(!graph.solve({0, 1, 2}, 1.5));
	}
	{
		constexpr int n = 30000;
		Graph path(n);
		for (int i = 1; i < n; ++i) path.add_edge(i - 1, i, 1);
		auto result = path.solve({0, n / 2, n - 1});
		assert(result && result->cost == n - 1 && result->edges.size() == n - 1);
	}
}
