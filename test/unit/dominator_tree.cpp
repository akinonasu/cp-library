#include "graph/dominator_tree.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

using Edges = std::vector<std::pair<int, int>>;

std::vector<unsigned char> reach(int n, int root, const Edges &edges, int removed = -1) {
	std::vector<std::vector<int>> graph(n);
	for (auto [from, to] : edges) graph[from].push_back(to);
	std::vector<unsigned char> visited(n);
	if (root == removed) return visited;
	std::vector<int> queue{root};
	visited[root] = true;
	for (std::size_t i = 0; i < queue.size(); ++i)
		for (int to : graph[queue[i]]) if (to != removed && !visited[to]) {
			visited[to] = true;
			queue.push_back(to);
		}
	return visited;
}

void check(int n, int root, const Edges &edges) {
	DominatorTree graph(n);
	graph.reserve(edges.size());
	for (auto [from, to] : edges) {
		int id = graph.add_edge(from, to);
		assert(id == int(graph.edge_count()) - 1);
		auto edge = graph.get_edge(id);
		assert(edge.from == from && edge.to == to);
	}
	assert(graph.size() == n && graph.edges().size() == edges.size());
	auto result = graph.solve(root);
	assert(graph.idom(root) == result.idom);
	assert(result.root == root && result.size() == n && result.empty() == (n == 0));
	auto visited = reach(n, root, edges);
	std::vector<std::vector<unsigned char>> dominates(n, std::vector<unsigned char>(n));
	for (int u = 0; u < n; ++u) {
		auto without = reach(n, root, edges, u);
		for (int v = 0; v < n; ++v) dominates[u][v] = visited[u] && visited[v] && !without[v];
	}
	std::vector<int> expected(n, -1);
	expected[root] = root;
	for (int v = 0; v < n; ++v) if (visited[v] && v != root) {
		for (int u = 0; u < n; ++u) if (u != v && dominates[u][v]) {
			bool deepest = true;
			for (int d = 0; d < n; ++d)
				if (d != v && dominates[d][v] && !dominates[d][u]) deepest = false;
			if (deepest) { assert(expected[v] == -1); expected[v] = u; }
		}
		assert(expected[v] != -1);
	}
	assert(result.idom == expected);
	auto tree = result.tree();
	assert(int(tree.size()) == n);
	std::vector<int> incoming(n);
	for (int u = 0; u < n; ++u) {
		assert(result.reachable(u) == bool(visited[u]));
		for (int v = 0; v < n; ++v) assert(result.dominates(u, v) == bool(dominates[u][v]));
		for (int v : tree[u]) {
			assert(0 <= v && v < n && v != root && result.idom[v] == u && visited[v]);
			++incoming[v];
		}
	}
	for (int v = 0; v < n; ++v) assert(incoming[v] == (visited[v] && v != root));
	assert(graph.solve(root).idom == expected);
	auto saved = result;
	auto copy = graph;
	copy.clear();
	assert(copy.size() == n && copy.edge_count() == 0 && graph.edge_count() == int(edges.size()));
	auto empty_edges = copy.solve(root);
	assert(copy.idom(root) == empty_edges.idom);
	for (int v = 0; v < n; ++v) assert(empty_edges.idom[v] == (v == root ? root : -1));
	assert(saved.idom == expected);
	auto moved = std::move(saved);
	assert(moved.idom == expected);
	for (int v = 0; v < n; ++v) assert(moved.reachable(v) == bool(visited[v]));
}

void large_shapes() {
	constexpr int n = 200000;
	DominatorTree chain(n);
	chain.reserve(n + 2);
	for (int v = 1; v < n; ++v) chain.add_edge(v - 1, v);
	auto result = chain.solve();
	assert(chain.idom() == result.idom);
	assert(result.idom[0] == 0);
	for (int v = 1; v < n; ++v) {
		assert(result.idom[v] == v - 1 && result.reachable(v));
		assert(result.dominates(v - 1, v) && !result.dominates(v, v - 1));
	}
	assert(result.dominates(0, n - 1));
	chain.add_edge(n - 1, 0);
	chain.add_edge(n - 1, n - 1);
	assert(chain.solve().idom == result.idom);
	assert(chain.idom() == result.idom);
	auto reverse_root = chain.solve(n - 1);
	assert(chain.idom(n - 1) == reverse_root.idom);
	assert(reverse_root.idom[n - 1] == n - 1 && reverse_root.idom[0] == n - 1);
	for (int v = 1; v < n - 1; ++v) assert(reverse_root.idom[v] == v - 1);
	DominatorTree star(n);
	star.reserve(n - 1);
	for (int v = 0; v < n - 1; ++v) star.add_edge(n - 1, v);
	auto rays = star.solve(n - 1);
	assert(star.idom(n - 1) == rays.idom);
	for (int v = 0; v < n; ++v) assert(rays.idom[v] == n - 1);
	assert(!rays.dominates(0, 1) && rays.dominates(0, 0));
	constexpr int layers = 60000;
	DominatorTree diamonds(3 * layers + 1);
	diamonds.reserve(5 * layers);
	for (int i = 0; i < layers; ++i) {
		int from = 3 * i, left = from + 1, right = from + 2, join = from + 3;
		diamonds.add_edge(from, left); diamonds.add_edge(from, right);
		diamonds.add_edge(left, join); diamonds.add_edge(right, join);
		diamonds.add_edge(join, from);
	}
	auto joined = diamonds.solve();
	assert(diamonds.idom() == joined.idom);
	for (int i = 0; i < layers; ++i) {
		int from = 3 * i;
		assert(joined.idom[from + 1] == from && joined.idom[from + 2] == from && joined.idom[from + 3] == from);
		assert(joined.dominates(from, from + 3) && !joined.dominates(from + 1, from + 3));
	}
}

int main() {
	DominatorTree empty;
	for (int root : {-1, 0}) {
		auto result = empty.solve(root);
		assert(empty.idom(root) == result.idom);
		assert(result.root == -1 && result.empty() && result.size() == 0 && result.idom.empty() && result.tree().empty());
	}
	for (int n = 1; n <= 3; ++n) {
		int bits = n * n;
		for (unsigned mask = 0; mask < (1U << bits); ++mask) {
			Edges edges;
			for (int from = 0; from < n; ++from) for (int to = 0; to < n; ++to)
				if (mask >> (from * n + to) & 1U) edges.push_back({from, to});
			for (int root = 0; root < n; ++root) check(n, root, edges);
		}
	}
	std::mt19937 random(813719);
	for (int trial = 0; trial < 1500; ++trial) {
		int n = 1 + random() % 16, m = random() % (3 * n + 1);
		Edges edges;
		for (int i = 0; i < m; ++i) edges.push_back({random() % n, random() % n});
		check(n, random() % n, edges);
		std::shuffle(edges.begin(), edges.end(), random);
		check(n, random() % n, edges);
	}
	for (int trial = 0; trial < 60; ++trial) {
		int n = 20 + random() % 25;
		Edges edges;
		for (int i = 0; i < 5 * n; ++i) edges.push_back({random() % n, random() % n});
		check(n, random() % n, edges);
	}
	check(8, 0, {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}, {4, 1}, {4, 2}, {5, 3}, {6, 4}, {5, 6}, {6, 5}, {7, 7}});
	check(7, 6, {{6, 0}, {6, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 1}, {3, 4}, {4, 5}, {5, 2}, {6, 0}});
	check(9, 5, {{5, 5}, {0, 5}, {1, 5}, {1, 2}, {2, 1}});
	DominatorTree incremental(4);
	incremental.add_edge(0, 1); incremental.add_edge(1, 2); incremental.add_edge(2, 3);
	auto before = incremental.solve();
	assert(incremental.idom() == before.idom);
	incremental.add_edge(0, 3);
	assert((before.idom == std::vector<int>{0, 0, 1, 2}));
	assert((incremental.solve().idom == std::vector<int>{0, 0, 1, 0}));
	assert((incremental.idom() == std::vector<int>{0, 0, 1, 0}));
	large_shapes();
}
