#include "graph/functional_graph.hpp"
#include <array>
#include <numeric>
#include <random>

struct Orbit {
	std::vector<int> vertices;
	int prefix;
	int jump(std::uint64_t k) const {
		if (k < vertices.size()) return vertices[k];
		return vertices[prefix + (k - prefix) % (vertices.size() - prefix)];
	}
	int dist(int v) const {
		auto it = std::find(vertices.begin(), vertices.end(), v);
		return it == vertices.end() ? -1 : static_cast<int>(it - vertices.begin());
	}
};

Orbit orbit(const std::vector<int> &to, int v) {
	std::vector<int> first(to.size(), -1);
	Orbit result;
	while (first[v] == -1) {
		first[v] = static_cast<int>(result.vertices.size());
		result.vertices.push_back(v);
		v = to[v];
	}
	result.prefix = first[v];
	return result;
}

int reference_lca(const std::vector<int> &to, const std::vector<Orbit> &walks, int u, int v) {
	int a = walks[u].prefix, b = walks[v].prefix;
	if (walks[u].vertices[a] != walks[v].vertices[b]) return -1;
	while (a > b) u = to[u], --a;
	while (b > a) v = to[v], --b;
	while (u != v) u = to[u], v = to[v];
	return u;
}

void check(const std::vector<int> &to) {
	const auto saved = to;
	const int n = static_cast<int>(to.size());
	FunctionalGraph graph(to);
	assert(graph.to == to && graph.size() == n && graph.empty() == !n);
	std::vector<Orbit> walks;
	for (int v = 0; v < n; ++v) walks.push_back(orbit(to, v));
	std::vector<int> seen(n), positions(n);
	for (int i = 0; i < n; ++i) {
		int v = graph.vertex_at(i);
		assert(0 <= v && v < n && !seen[v]++);
		positions[v] = i;
	}
	std::fill(seen.begin(), seen.end(), 0);
	int total_cycle_vertices = 0;
	for (int id = 0; id < graph.cycle_count(); ++id) {
		auto cycle = graph.cycle(id);
		assert(!cycle.empty());
		total_cycle_vertices += static_cast<int>(cycle.size());
		for (std::size_t i = 0; i < cycle.size(); ++i) {
			int v = cycle[i];
			assert(0 <= v && v < n && !seen[v]++);
			assert(graph.cycle_id[v] == id && graph.cycle_position[v] == static_cast<int>(i));
			assert(to[v] == cycle[(i + 1) % cycle.size()]);
		}
	}
	auto forest = graph.forest();
	assert(forest.size() == to.size());
	std::size_t forest_edges = 0;
	for (int v = 0; v < n; ++v) {
		const auto &walk = walks[v];
		assert(graph.depth[v] == walk.prefix && graph.entry[v] == walk.vertices[walk.prefix]);
		assert(graph.on_cycle(v) == !walk.prefix && seen[v] == !walk.prefix);
		assert(graph.parent[v] == (walk.prefix ? to[v] : -1));
		assert(graph.cycle_length(v) == static_cast<int>(walk.vertices.size()) - walk.prefix);
		assert(graph.reachable_count(v) == static_cast<int>(walk.vertices.size()));
		if (walk.prefix) assert(graph.cycle_position[v] == -1);
		for (std::uint64_t k = 0; k <= std::uint64_t(n) * 2; ++k) assert(graph.jump(v, k) == walk.jump(k));
		for (std::uint64_t k : {std::uint64_t(1000000000000000000ULL), std::uint64_t(1) << 63,
			std::numeric_limits<std::uint64_t>::max() - 1, std::numeric_limits<std::uint64_t>::max()})
			assert(graph.jump(v, k) == walk.jump(k));
		auto [l, r] = graph.subtree(v);
		assert(0 <= l && l < r && r <= n && graph.vertex_at(l) == v);
		assert(r - l == graph.subtree_size[v]);
		int expected_size = 0;
		for (int u = 0; u < n; ++u) {
			bool ancestor = walks[u].prefix >= walk.prefix && walks[u].jump(walks[u].prefix - walk.prefix) == v;
			assert((l <= positions[u] && positions[u] < r) == ancestor);
			expected_size += ancestor;
			assert(graph.dist(u, v) == walks[u].dist(v));
			assert(graph.lca(u, v) == reference_lca(to, walks, u, v));
			bool same = walks[u].dist(walk.vertices[walk.prefix]) != -1;
			assert(graph.same_component(u, v) == same);
		}
		assert(graph.subtree_size[v] == expected_size);
		for (int u : forest[v]) {
			assert(graph.parent[v] == u || graph.parent[u] == v);
			assert(std::count(forest[u].begin(), forest[u].end(), v) == 1);
		}
		forest_edges += forest[v].size();
	}
	assert(forest_edges == std::size_t(n - total_cycle_vertices) * 2);
	assert(to == saved);
}

int main() {
	check({});
	for (int n = 1; n <= 6; ++n) {
		std::vector<int> to(n);
		while (true) {
			check(to);
			int i = 0;
			while (i < n && ++to[i] == n) to[i++] = 0;
			if (i == n) break;
		}
	}
	std::mt19937_64 random(20261022);
	for (int trial = 0; trial < 400; ++trial) {
		int n = 7 + random() % 74;
		std::vector<int> to(n);
		for (int &v : to) v = random() % n;
		check(to);
	}
	FunctionalGraph reused;
	for (int trial = 0; trial < 100; ++trial) {
		int n = random() % 200;
		std::vector<int> to(n);
		for (int &v : to) v = random() % n;
		reused.build(to);
		FunctionalGraph fresh(to);
		assert(reused.to == fresh.to && reused.parent == fresh.parent && reused.depth == fresh.depth);
		assert(reused.entry == fresh.entry && reused.cycle_id == fresh.cycle_id && reused.cycle_position == fresh.cycle_position);
		assert(reused.subtree_size == fresh.subtree_size);
		for (int v = 0; v < n; ++v) {
			auto k = random();
			assert(reused.jump(v, k) == orbit(to, v).jump(k));
		}
	}
	FunctionalGraph example(std::vector<int>{1, 2, 0, 1, 3, 3, 6});
	auto copy = example;
	copy.build(copy.to);
	assert(copy.to == example.to && copy.depth == example.depth && copy.entry == example.entry);
	auto moved = std::move(copy);
	assert(moved.to == example.to && moved.dist(4, 2) == 3 && moved.lca(4, 5) == 3);
	copy.build({0});
	assert(copy.size() == 1 && copy.cycle_count() == 1 && copy.jump(0, random()) == 0);
	copy = example;
	assert(copy.jump(4, 4) == 0);
	copy.build({});
	assert(copy.empty() && copy.cycle_count() == 0 && copy.forest().empty() && copy.parent.empty());
	const int n = 500000;
	for (int cycle_start : {0, n / 2, n - 1}) {
		std::vector<int> to(n);
		std::iota(to.begin(), to.end(), 1);
		to.back() = cycle_start;
		FunctionalGraph graph(std::move(to));
		assert(graph.cycle_count() == 1 && graph.cycle_length(0) == n - cycle_start);
		for (int trial = 0; trial < 10000; ++trial) {
			int v = random() % n;
			std::uint64_t k = trial % 3 == 0 ? random() % (n + 1) : trial % 3 == 1 ? random() : std::numeric_limits<std::uint64_t>::max();
			int actual = graph.jump(v, k);
			int expected;
			if (v < cycle_start && k < std::uint64_t(cycle_start - v)) expected = v + k;
			else {
				if (v < cycle_start) k -= cycle_start - v, v = cycle_start;
				expected = cycle_start + ((v - cycle_start) + k % (n - cycle_start)) % (n - cycle_start);
			}
			assert(actual == expected);
		}
		assert(graph.reachable_count(0) == n);
	}
	for (int shape = 0; shape < 3; ++shape) {
		std::vector<int> to(n);
		for (int v = 0; v < n; ++v) to[v] = !v ? 0 : shape == 0 ? v : shape == 1 ? 0 : (v - 1) / 2;
		FunctionalGraph graph(to);
		assert(graph.cycle_count() == (shape == 0 ? n : 1));
		for (int trial = 0; trial < 10000; ++trial) {
			int v = random() % n;
			std::uint64_t k = trial & 1 ? random() : random() % 20;
			int expected = v;
			for (std::uint64_t i = 0; i < k && to[expected] != expected; ++i) expected = to[expected];
			assert(graph.jump(v, k) == expected);
		}
	}
}
