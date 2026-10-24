#include "graph/connectivity/block_cut_tree.hpp"
#include <bit>
#include <numeric>
#include <random>

std::vector<int> labels(const std::vector<std::vector<int>>& g, int removed = -1) {
	std::vector<int> result(g.size(), -1), queue;
	int count = 0;
	for (int root = 0; root < int(g.size()); ++root) if (root != removed && result[root] == -1) {
		result[root] = count++;
		queue.assign(1, root);
		for (std::size_t i = 0; i < queue.size(); ++i) for (int to : g[queue[i]])
			if (to != removed && result[to] == -1) result[to] = result[root], queue.push_back(to);
	}
	return result;
}

int count(const std::vector<int>& id) { return id.empty() ? 0 : *std::max_element(id.begin(), id.end()) + 1; }

std::vector<unsigned> brute(const std::vector<std::vector<int>>& g) {
	int n = g.size();
	std::vector<unsigned> adj(n), candidates, result;
	for (int v = 0; v < n; ++v) for (int to : g[v]) if (to != v) adj[v] |= 1U << to;
	auto connected = [&](unsigned mask) {
		if (!mask) return true;
		unsigned reached = mask & -mask, pending = reached;
		while (pending) {
			int v = std::countr_zero(pending);
			pending &= pending - 1;
			unsigned next = adj[v] & mask & ~reached;
			reached |= next, pending |= next;
		}
		return reached == mask;
	};
	for (unsigned mask = 1; mask < (1U << n); ++mask) {
		if (std::popcount(mask) == 1) {
			if (!adj[std::countr_zero(mask)]) candidates.push_back(mask);
			continue;
		}
		if (!connected(mask)) continue;
		bool valid = true;
		for (int v = 0; v < n; ++v) if (mask >> v & 1U) valid &= connected(mask ^ (1U << v));
		if (valid) candidates.push_back(mask);
	}
	for (unsigned mask : candidates) {
		bool maximal = true;
		for (unsigned other : candidates) if (other != mask && (mask & other) == mask) maximal = false;
		if (maximal) result.push_back(mask);
	}
	return result;
}

void verify(const BlockCutTree& graph) {
	int n = graph.n, blocks = graph.block_count();
	std::vector<std::vector<int>> adj(n), membership(n);
	for (auto [u, v] : graph.edges) adj[u].push_back(v), adj[v].push_back(u);
	std::vector<unsigned> actual;
	auto groups = graph.groups();
	assert(int(groups.size()) == blocks);
	for (int c = 0; c < blocks; ++c) {
		auto members = graph.group(c);
		assert(!members.empty() && std::vector<int>(members.begin(), members.end()) == groups[c]);
		unsigned mask = 0;
		for (int v : members) {
			assert(0 <= v && v < n && !(mask >> v & 1U));
			mask |= 1U << v;
			membership[v].push_back(c);
		}
		actual.push_back(mask);
	}
	std::sort(actual.begin(), actual.end());
	assert(actual == brute(adj));
	int connected = count(labels(adj));
	std::vector<int> articulation;
	for (int v = 0; v < n; ++v) {
		bool cut = count(labels(adj, v)) > connected;
		assert(graph.is_articulation(v) == cut);
		assert((membership[v].size() > 1) == cut);
		if (cut) {
			assert(graph.node_id[v] == blocks + int(articulation.size()));
			articulation.push_back(v);
		} else assert(membership[v].size() == 1 && graph.node_id[v] == membership[v][0]);
	}
	assert(graph.articulation == articulation);
	assert(graph.node_count() == blocks + int(articulation.size()));
	std::vector<std::vector<int>> expected(graph.node_count());
	for (int c = 0; c < blocks; ++c) for (int v : groups[c]) if (graph.is_articulation(v)) {
		expected[c].push_back(graph.node_id[v]);
		expected[graph.node_id[v]].push_back(c);
	}
	std::size_t arcs = 0;
	for (int v = 0; v < graph.node_count(); ++v) {
		auto actual = graph.tree[v];
		std::sort(actual.begin(), actual.end());
		std::sort(expected[v].begin(), expected[v].end());
		assert(actual == expected[v] && std::adjacent_find(actual.begin(), actual.end()) == actual.end());
		assert(graph.is_block(v) == (v < blocks));
		assert(graph.vertex(v) == (v < blocks ? -1 : articulation[v - blocks]));
		arcs += actual.size();
	}
	assert(count(labels(graph.tree)) == connected);
	assert(arcs == 2 * std::size_t(graph.node_count() - connected));
	for (int removed = -1; removed < n; ++removed) {
		auto original = labels(adj, removed);
		int cut = removed != -1 && graph.is_articulation(removed) ? graph.node_id[removed] : -1;
		auto forest = labels(graph.tree, cut);
		for (int u = 0; u < n; ++u) if (u != removed)
			for (int v = 0; v < n; ++v) if (v != removed)
				assert((original[u] == original[v]) == (forest[graph.node_id[u]] == forest[graph.node_id[v]]));
	}
}

void check(int n, const std::vector<std::pair<int, int>>& edges) {
	BlockCutTree graph(n);
	for (auto [u, v] : edges) {
		int id = graph.add_edge(u, v);
		assert(id == int(graph.edges.size()) - 1);
	}
	graph.build();
	verify(graph);
	graph.build();
	verify(graph);
	auto copy = graph;
	graph.build_components();
	assert(graph.tree.empty() && graph.node_count() == copy.node_count());
	assert(graph.groups() == copy.groups() && graph.articulation == copy.articulation);
	assert(graph.node_id == copy.node_id);
	for (int v = 0; v < graph.node_count(); ++v) {
		assert(graph.is_block(v) == copy.is_block(v));
		assert(graph.vertex(v) == copy.vertex(v));
	}
	graph.build();
	verify(graph);
	verify(copy);
}

int main() {
	auto build = &BlockCutTree::build;
	BlockCutTree empty;
	(empty.*build)();
	assert(empty.node_count() == 0);
	for (int n = 0; n <= 5; ++n) {
		std::vector<std::pair<int, int>> all;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) all.emplace_back(u, v);
		for (unsigned mask = 0; mask < (1U << all.size()); ++mask) {
			std::vector<std::pair<int, int>> edges;
			for (std::size_t i = 0; i < all.size(); ++i) if (mask >> i & 1U) edges.push_back(all[i]);
			check(n, edges);
		}
	}
	check(1, { {0, 0}, {0, 0} });
	check(2, { {0, 1}, {1, 0}, {0, 0}, {1, 1} });
	check(6, { {0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 2}, {5, 5} });
	std::mt19937 random(603191);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + random() % 8, m = random() % 35;
		std::vector<std::pair<int, int>> edges;
		for (int i = 0; i < m; ++i) edges.emplace_back(random() % n, random() % n);
		check(n, edges);
	}
	BlockCutTree changed(3);
	changed.add_edge(0, 1), changed.add_edge(1, 2), changed.build();
	auto old = changed;
	changed.add_edge(2, 0), changed.build();
	verify(changed), verify(old);
	assert(changed.block_count() == 1 && old.block_count() == 2);
	BlockCutTree path(300000);
	path.edges.reserve(path.n);
	for (int v = 1; v < path.n; ++v) path.add_edge(v - 1, v);
	path.build();
	assert(path.block_count() == path.n - 1 && path.node_count() == 2 * path.n - 3);
	for (int v = 0; v < path.n; ++v) assert(path.is_articulation(v) == (0 < v && v + 1 < path.n));
	path.add_edge(0, path.n - 1), path.build();
	assert(path.block_count() == 1 && path.node_count() == 1 && path.group(0).size() == std::size_t(path.n));
	assert(path.articulation.empty());
	BlockCutTree star(200000);
	for (int v = 1; v < star.n; ++v) star.add_edge(0, v);
	star.build();
	assert(star.block_count() == star.n - 1 && star.node_count() == star.n);
	assert(star.articulation == std::vector<int>{0});
	assert(star.tree[star.node_id[0]].size() == std::size_t(star.n - 1));
}
