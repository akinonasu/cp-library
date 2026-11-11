#include "graph/directed_mst.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using I = __int128_t;
struct TestEdge { int from, to; std::int64_t cost; };

std::optional<I> exhaustive(int n, int root, const std::vector<TestEdge> &edges) {
	std::vector<std::vector<int>> incoming(n);
	for (int i = 0; i < int(edges.size()); ++i)
		if (edges[i].from != edges[i].to && edges[i].to != root) incoming[edges[i].to].push_back(i);
	std::vector<int> parent(n, -1);
	parent[root] = root;
	std::optional<I> best;
	auto visit = [&](auto &&self, int v, I cost) -> void {
		if (v == n) {
			for (int start = 0; start < n; ++start) {
				int u = start, steps = 0;
				while (u != root && steps++ < n) u = parent[u];
				if (u != root) return;
			}
			if (!best || cost < *best) best = cost;
			return;
		}
		if (v == root) { self(self, v + 1, cost); return; }
		for (int id : incoming[v]) {
			parent[v] = edges[id].from;
			self(self, v + 1, cost + edges[id].cost);
		}
	};
	visit(visit, 0, 0);
	return best;
}

std::optional<I> slow(int n, int root, const std::vector<TestEdge> &input) {
	struct Edge { int from, to; I cost; };
	std::vector<Edge> edges;
	for (auto e : input) edges.push_back({e.from, e.to, e.cost});
	I total = 0;
	for (;;) {
		std::vector<std::optional<I>> incoming(n);
		std::vector<int> parent(n, -1), component(n, -1), seen(n, -1);
		for (auto e : edges) if (e.from != e.to && (!incoming[e.to] || e.cost < *incoming[e.to]))
			incoming[e.to] = e.cost, parent[e.to] = e.from;
		incoming[root] = 0;
		for (int v = 0; v < n; ++v) {
			if (!incoming[v]) return std::nullopt;
			total += *incoming[v];
		}
		int count = 0;
		for (int start = 0; start < n; ++start) {
			int v = start;
			while (v != root && component[v] < 0 && seen[v] != start) seen[v] = start, v = parent[v];
			if (v == root || component[v] >= 0) continue;
			int u = v;
			do { component[u] = count; u = parent[u]; } while (u != v);
			++count;
		}
		if (!count) return total;
		for (int v = 0; v < n; ++v) if (component[v] < 0) component[v] = count++;
		std::vector<Edge> contracted;
		for (auto e : edges) if (component[e.from] != component[e.to])
			contracted.push_back({component[e.from], component[e.to], e.cost - *incoming[e.to]});
		root = component[root], n = count;
		edges = std::move(contracted);
	}
}

template<class Cost, class Sum>
void verify_result(const DirectedMST<Cost, Sum> &graph, int root,
	const typename DirectedMST<Cost, Sum>::Result &result, I expected) {
	int n = graph.size();
	assert(I(result.cost) == expected);
	assert(int(result.parent.size()) == n && int(result.parent_edge.size()) == n);
	assert(int(result.edges.size()) == std::max(0, n - 1));
	if (!n) return;
	assert(result.parent[root] == root && result.parent_edge[root] == -1);
	std::vector<unsigned char> chosen(graph.edge_count());
	I actual = 0;
	for (int id : result.edges) {
		assert(0 <= id && id < graph.edge_count() && !chosen[id]);
		chosen[id] = 1;
		auto e = graph.get_edge(id);
		assert(e.to != root && result.parent_edge[e.to] == id && result.parent[e.to] == e.from);
		actual += e.cost;
	}
	assert(actual == expected);
	std::vector<std::vector<int>> children(n);
	for (int v = 0; v < n; ++v) if (v != root) {
		assert(0 <= result.parent_edge[v] && result.parent_edge[v] < graph.edge_count());
		assert(chosen[result.parent_edge[v]]);
		assert(0 <= result.parent[v] && result.parent[v] < n);
		children[result.parent[v]].push_back(v);
	}
	std::vector<int> reached{root};
	for (std::size_t i = 0; i < reached.size(); ++i) for (int v : children[reached[i]]) reached.push_back(v);
	assert(int(reached.size()) == n);
}

template<class Cost = std::int64_t>
void check(int n, int root, const std::vector<TestEdge> &edges, bool brute = false) {
	DirectedMST<Cost, I> graph(n);
	graph.reserve(edges.size());
	for (int i = 0; i < int(edges.size()); ++i) {
		auto e = edges[i];
		assert(graph.add_edge(e.from, e.to, e.cost) == i);
	}
	assert(graph.size() == n && graph.edge_count() == int(edges.size()));
	assert(graph.edges().size() == edges.size());
	auto expected = n ? slow(n, root, edges) : std::optional<I>(0);
	if (brute && n) assert(expected == exhaustive(n, root, edges));
	const auto &view = graph;
	for (int repeat = 0; repeat < 2; ++repeat) {
		auto result = view.solve(root);
		assert(bool(result) == bool(expected));
		if (result) verify_result(graph, root, *result, *expected);
	}
	for (int i = 0; i < int(edges.size()); ++i) {
		auto e = graph.get_edge(i), f = graph.edges()[i];
		assert(e.from == edges[i].from && e.to == edges[i].to && e.cost == edges[i].cost);
		assert(e.from == f.from && e.to == f.to && e.cost == f.cost);
	}
}

template<class Cost>
void types() {
	DirectedMST<Cost, I> wide(4);
	wide.add_edge(0, 1, std::numeric_limits<Cost>::max());
	wide.add_edge(1, 2, std::numeric_limits<Cost>::min());
	wide.add_edge(2, 3, 1);
	assert(wide.solve()->cost == 0);
	DirectedMST<Cost> narrow(3);
	narrow.add_edge(0, 1, std::numeric_limits<Cost>::max());
	narrow.add_edge(0, 2, 1);
	bool caught = false;
	try { (void)narrow.solve(); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
}

int main() {
	types<std::int8_t>(); types<std::int16_t>(); types<std::int32_t>(); types<std::int64_t>();
	check(0, 0, {}); check(0, -1, {}); check(1, 0, {}, true);
	check(1, 0, {{0, 0, -10}}, true);
	check(4, 0, {{0, 1, 1}, {1, 2, 2}}, true);
	check(4, 0, {{0, 0, -1}, {1, 1, -2}, {2, 2, -3}, {3, 3, -4}}, true);
	check(4, 0, {{0, 1, 3}, {0, 1, 2}, {0, 1, 1}, {2, 3, -2}}, true);
	check(3, 0, {{1, 2, -4}, {2, 1, -3}}, true);
	check(4, 0, {{0, 1, 7}, {1, 2, -5}, {2, 1, -6}, {2, 3, 2}, {3, 2, -4}, {0, 3, 20}}, true);
	for (int n = 1; n <= 3; ++n) {
		int total = 1;
		for (int i = 0; i < n * (n - 1); ++i) total *= 3;
		for (int code = 0; code < total; ++code) {
			int value = code;
			std::vector<TestEdge> edges;
			for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) if (u != v) {
				if (value % 3) edges.push_back({u, v, value % 3 == 1 ? -2 : 3});
				value /= 3;
			}
			for (int root = 0; root < n; ++root) check(n, root, edges, true);
		}
	}
	std::mt19937 random(721849);
	for (int trial = 0; trial < 1600; ++trial) {
		int n = 1 + random() % 7, m = random() % 15;
		std::vector<TestEdge> edges;
		for (int i = 0; i < m; ++i) edges.push_back({int(random() % n), int(random() % n), int(random() % 19) - 9});
		check(n, random() % n, edges, true);
		check<std::int32_t>(n, random() % n, edges, true);
	}
	for (int trial = 0; trial < 180; ++trial) {
		int n = 8 + random() % 70, m = random() % 700;
		std::vector<TestEdge> edges;
		for (int i = 0; i < m; ++i) edges.push_back({int(random() % n), int(random() % n), int(random() % 2001) - 1000});
		check(n, random() % n, edges);
		check<std::int32_t>(n, random() % n, edges);
	}
	constexpr auto low = std::numeric_limits<std::int64_t>::min(), high = std::numeric_limits<std::int64_t>::max();
	check(4, 0, {{0, 1, low}, {1, 2, high}, {2, 3, 1}, {0, 2, high}, {0, 3, high}}, true);
	check(4, 0, {{0, 1, high}, {0, 2, high}, {0, 3, high}, {1, 2, low}, {2, 1, low}, {2, 3, low}, {3, 2, low}}, true);
	check<std::int32_t>(4, 0, {{0, 1, INT32_MAX}, {0, 2, INT32_MAX}, {1, 2, INT32_MIN},
		{2, 1, INT32_MIN}, {2, 3, INT32_MIN}, {3, 2, INT32_MIN}}, true);
	for (int sign : {-1, 1}) {
		DirectedMST<> overflow(3);
		overflow.add_edge(0, 1, sign < 0 ? low : high);
		overflow.add_edge(0, 2, sign);
		bool caught = false;
		try { (void)overflow.solve(); } catch (const std::overflow_error &) { caught = true; }
		assert(caught);
	}
	DirectedMST<> cancellation(4);
	cancellation.add_edge(0, 1, high); cancellation.add_edge(0, 2, high); cancellation.add_edge(0, 3, low);
	assert(cancellation.solve()->cost == high - 1);
	DirectedMST<> unreachable(4);
	unreachable.add_edge(0, 1, high); unreachable.add_edge(0, 2, high);
	assert(!unreachable.solve());
	DirectedMST<std::int32_t, std::int64_t> small(3);
	small.add_edge(0, 1, INT32_MIN); small.add_edge(1, 2, INT32_MIN);
	assert(small.solve()->cost == std::int64_t(INT32_MIN) * 2);
	auto copy = small;
	copy.clear(); assert(copy.size() == 3 && copy.edge_count() == 0 && !copy.solve());
	assert(small.edge_count() == 2);
	assert(copy.add_edge(2, 1, 5) == 0);
	copy = small;
	auto moved = std::move(copy);
	assert(moved.solve()->cost == std::int64_t(INT32_MIN) * 2);
	std::vector<TestEdge> cycle{{0, 1, -5}, {1, 2, -4}, {2, 0, -3}, {0, 2, 6}, {1, 0, 7}, {2, 1, 8}};
	DirectedMST<> roots(3);
	for (auto e : cycle) roots.add_edge(e.from, e.to, e.cost);
	for (int root : {0, 2, 1, 0, 1}) verify_result(roots, root, *roots.solve(root), *exhaustive(3, root, cycle));
	for (int mode = 0; mode < 3; ++mode) {
		const int n = 100000;
		DirectedMST<std::int32_t, std::int64_t> graph(n);
		graph.reserve(3 * n);
		for (int v = 1; v < n; ++v) graph.add_edge(mode == 0 ? v - 1 : 0, v, 2);
		if (mode == 1) for (int v = 1; v < n; ++v) graph.add_edge(v, v == n - 1 ? 1 : v + 1, -1);
		if (mode == 2) for (int v = 2; v < n; ++v) {
			graph.add_edge(v, v - 1, -1);
			graph.add_edge(v - 1, v, -1);
		}
		auto result = graph.solve();
		assert(result);
		I expected = mode ? 2 - I(n - 2) : I(n - 1) * 2;
		verify_result(graph, 0, *result, expected);
	}
}
