#include "graph/incremental_msf.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <utility>
#include <vector>

struct DisjointSet {
	std::vector<int> parent;
	int components;
	explicit DisjointSet(int n) : parent(n, -1), components(n) {}
	int root(int v) { return parent[v] < 0 ? v : parent[v] = root(parent[v]); }
	bool join(int u, int v) {
		u = root(u), v = root(v);
		if (u == v) return false;
		if (parent[u] > parent[v]) std::swap(u, v);
		parent[u] += parent[v], parent[v] = u;
		--components;
		return true;
	}
};

template<class Cost> struct InputEdge { int from, to; Cost cost; };

struct Oracle {
	std::vector<int> selected, root, size;
	__int128_t cost = 0;
	int components;
};

template<class Cost>
Oracle rebuild(int n, const std::vector<InputEdge<Cost>> &edges) {
	std::vector<int> order(edges.size());
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int a, int b) {
		return edges[a].cost != edges[b].cost ? edges[a].cost < edges[b].cost : a < b;
	});
	DisjointSet uf(n);
	Oracle result{{}, {}, {}, 0, n};
	for (int id : order) {
		auto e = edges[id];
		if (uf.join(e.from, e.to)) result.selected.push_back(id), result.cost += __int128_t(e.cost);
	}
	std::sort(result.selected.begin(), result.selected.end());
	result.components = uf.components;
	for (int v = 0; v < n; ++v) {
		result.root.push_back(uf.root(v));
		result.size.push_back(-uf.parent[uf.root(v)]);
	}
	return result;
}

template<class Cost>
__int128_t exhaustive(int n, const std::vector<InputEdge<Cost>> &edges, int components) {
	std::optional<__int128_t> best;
	for (unsigned mask = 0; mask < (1U << edges.size()); ++mask) {
		if (std::popcount(mask) != n - components) continue;
		DisjointSet uf(n);
		__int128_t cost = 0;
		bool valid = true;
		for (std::size_t i = 0; i < edges.size(); ++i) if ((mask >> i) & 1) {
			auto e = edges[i];
			if (!uf.join(e.from, e.to)) { valid = false; break; }
			cost += __int128_t(e.cost);
		}
		if (valid && (!best || cost < *best)) best = cost;
	}
	assert(best);
	return *best;
}

template<class Cost>
std::optional<int> path_maximum(const std::vector<InputEdge<Cost>> &edges,
	const Oracle &oracle, int from, int to) {
	if (from == to || oracle.root[from] != oracle.root[to]) return std::nullopt;
	int n = int(oracle.root.size());
	std::vector<std::vector<std::pair<int, int>>> adjacency(n);
	for (int id : oracle.selected) {
		auto e = edges[id];
		adjacency[e.from].push_back({e.to, id});
		adjacency[e.to].push_back({e.from, id});
	}
	std::vector<int> parent(n, -1), edge(n, -1), queue{from};
	parent[from] = from;
	for (std::size_t i = 0; i < queue.size(); ++i) {
		int v = queue[i];
		for (auto [u, id] : adjacency[v]) if (parent[u] == -1) {
			parent[u] = v, edge[u] = id;
			queue.push_back(u);
		}
	}
	int best = -1;
	for (int v = to; v != from; v = parent[v]) {
		int id = edge[v];
		if (best == -1 || edges[best].cost < edges[id].cost ||
			(edges[best].cost == edges[id].cost && best < id)) best = id;
	}
	return best;
}

template<class Cost, class Sum>
void check_path(IncrementalMSF<Cost, Sum> &graph, const std::vector<InputEdge<Cost>> &edges,
	const Oracle &oracle, int u, int v) {
	assert(graph.same(u, v) == (oracle.root[u] == oracle.root[v]));
	auto expected = path_maximum(edges, oracle, u, v);
	auto actual = graph.path_max(u, v);
	assert(bool(actual) == bool(expected));
	if (actual) assert(actual->id == *expected && actual->cost == edges[*expected].cost);
}

template<class Cost, class Sum>
void check_state(IncrementalMSF<Cost, Sum> &graph, const std::vector<InputEdge<Cost>> &edges,
	const Oracle &oracle, bool all_paths) {
	int n = int(oracle.root.size());
	assert(graph.size() == n && graph.edge_count() == int(edges.size()));
	assert(graph.component_count() == oracle.components);
	assert(graph.forest_edge_count() == n - oracle.components);
	assert(graph.connected() == (oracle.components <= 1));
	assert(__int128_t(graph.total_cost()) == oracle.cost);
	auto forest = graph.forest_edges();
	std::vector<int> selected;
	for (auto e : forest) {
		assert(0 <= e.id && e.id < int(edges.size()));
		assert(e.cost == edges[e.id].cost);
		selected.push_back(e.id);
	}
	std::sort(selected.begin(), selected.end());
	assert(selected == oracle.selected);
	for (int v = 0; v < n; ++v) {
		assert(graph.component_size(v) == oracle.size[v]);
		if (all_paths) for (int u = 0; u < n; ++u) check_path(graph, edges, oracle, u, v);
	}
}

template<class Cost>
void sequence(int n, const std::vector<InputEdge<Cost>> &input, bool brute) {
	IncrementalMSF<Cost> graph(n);
	std::vector<InputEdge<Cost>> edges;
	Oracle previous = rebuild(n, edges);
	check_state(graph, edges, previous, brute);
	for (auto e : input) {
		int id = int(edges.size());
		edges.push_back(e);
		Oracle next = rebuild(n, edges);
		int expected = id;
		if (std::binary_search(next.selected.begin(), next.selected.end(), id)) {
			expected = -1;
			for (int old : previous.selected)
				if (!std::binary_search(next.selected.begin(), next.selected.end(), old)) expected = old;
		}
		assert(graph.add_edge(e.from, e.to, e.cost) == expected);
		check_state(graph, edges, next, brute);
		if (brute) assert(next.cost == exhaustive(n, edges, next.components));
		else if (n) for (int repeat = 0; repeat < 4; ++repeat)
			check_path(graph, edges, next, (id * 17 + repeat * 31) % n, (id * 43 + repeat * 13) % n);
		previous = std::move(next);
	}
}

template<class Cost> void random_tests() {
	std::mt19937_64 rng(719343);
	for (int trial = 0; trial < 300; ++trial) {
		int n = 1 + rng() % 7, m = rng() % 12;
		std::vector<InputEdge<Cost>> edges;
		for (int i = 0; i < m; ++i) {
			Cost cost = std::numeric_limits<Cost>::is_signed ? Cost(int(rng() % 11) - 5) : Cost(rng() % 11);
			edges.push_back({int(rng() % n), int(rng() % n), cost});
		}
		sequence(n, edges, true);
	}
	for (int trial = 0; trial < 12; ++trial) {
		int n = 10 + rng() % 70;
		std::vector<InputEdge<Cost>> edges;
		for (int i = 0; i < 240; ++i) {
			Cost cost;
			if (trial % 3 == 0) cost = Cost(1000 - i);
			else if (trial % 3 == 1) cost = Cost(i / 3);
			else cost = std::numeric_limits<Cost>::is_signed ? Cost(int(rng() % 101) - 50) : Cost(rng() % 101);
			edges.push_back({int(rng() % n), int(rng() % n), cost});
		}
		sequence(n, edges, false);
	}
}

int main() {
	sequence<std::int64_t>(0, {}, true);
	sequence<std::int64_t>(1, { {0, 0, -10}, {0, 0, 5} }, true);
	sequence<int>(5, { {0, 1, 5}, {1, 2, 5}, {0, 2, 4}, {2, 0, 4}, {3, 4, -1}, {2, 3, 0} }, true);
	sequence<std::int64_t>(3, {
		{0, 1, std::numeric_limits<std::int64_t>::max()},
		{0, 1, std::numeric_limits<std::int64_t>::min()},
		{1, 2, std::numeric_limits<std::int64_t>::min()},
		{0, 2, std::numeric_limits<std::int64_t>::min()},
		{0, 0, std::numeric_limits<std::int64_t>::min()}
	}, true);
	sequence<std::int64_t>(4, {
		{0, 1, std::numeric_limits<std::int64_t>::max()},
		{1, 2, std::numeric_limits<std::int64_t>::max()},
		{2, 3, std::numeric_limits<std::int64_t>::max()}
	}, true);
	sequence<std::uint64_t>(3, {
		{0, 1, std::numeric_limits<std::uint64_t>::max()},
		{1, 2, std::numeric_limits<std::uint64_t>::max()},
		{0, 1, 0}, {2, 0, 0}, {2, 2, 0}
	}, true);
	random_tests<int>();
	random_tests<std::int64_t>();
	random_tests<std::uint64_t>();
	{
		IncrementalMSF<int, std::int64_t> graph(4);
		assert(graph.add_edge(0, 1, 7) == -1);
		assert(graph.add_edge(2, 3, -2) == -1);
		auto saved = graph.forest_edges();
		auto copy = graph;
		assert(copy.add_edge(1, 2, 1) == -1 && copy.total_cost() == 6);
		assert(graph.component_count() == 2 && graph.total_cost() == 5);
		assert(graph.add_edge(1, 0, -3) == 0 && graph.total_cost() == -5);
		assert(saved.size() == 2 && saved[0].cost + saved[1].cost == 5);
		copy = graph;
		assert(copy.edge_count() == 3 && copy.total_cost() == -5);
		auto moved = std::move(copy);
		assert(moved.size() == 4 && moved.total_cost() == -5);
		assert(copy.size() == 0 && copy.edge_count() == 0 && copy.component_count() == 0 && copy.connected());
		copy = std::move(moved);
		assert(copy.size() == 4 && copy.total_cost() == -5 && moved.size() == 0);
		auto *self = &copy;
		copy = std::move(*self);
		assert(copy.size() == 4 && copy.edge_count() == 3 && copy.total_cost() == -5);
		copy.clear();
		assert(copy.size() == 4 && copy.edge_count() == 0 && copy.forest_edge_count() == 0);
		assert(copy.component_count() == 4 && copy.total_cost() == 0 && copy.forest_edges().empty());
		assert(!copy.path_max(0, 1) && !copy.path_max(1, 1) && copy.same(0, 0));
		assert(copy.add_edge(1, 0, 4) == -1 && copy.edge_count() == 1);
	}
	{
		const int n = 20000;
		IncrementalMSF<int, std::int64_t> graph(n);
		for (int v = 1; v < n; ++v) assert(graph.add_edge(v - 1, v, v) == -1);
		assert(graph.path_max(0, n - 1)->id == n - 2);
		for (int v = 2; v < n; ++v) {
			assert(graph.add_edge(0, v, -v) == v - 1);
			assert(graph.path_max(v, 1)->id == 0);
		}
		assert(graph.connected() && graph.forest_edge_count() == n - 1);
		assert(graph.total_cost() == 2 - std::int64_t(n) * (n - 1) / 2);
		for (int i = 0; i < 20000; ++i) {
			int id = graph.edge_count();
			assert(graph.add_edge(i % n, (i * 17) % n, n + i) == id);
		}
	}
}
