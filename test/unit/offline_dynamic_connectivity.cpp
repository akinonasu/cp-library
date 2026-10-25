#include "graph/offline_dynamic_connectivity.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

std::vector<int> labels(const std::vector<std::vector<int>> &edges) {
	int n = static_cast<int>(edges.size());
	std::vector<int> label(n, -1), todo;
	for (int v = 0; v < n; ++v) if (label[v] < 0) {
		label[v] = v, todo = {v};
		for (std::size_t i = 0; i < todo.size(); ++i)
			for (int u = 0; u < n; ++u) if (edges[todo[i]][u] && label[u] < 0) {
				label[u] = v, todo.push_back(u);
			}
	}
	return label;
}

int main() {
	OfflineDynamicConnectivity empty;
	require(empty.vertex_count() == 0 && empty.query_count() == 0 && empty.solve().empty());
	bool called = false;
	empty.run([&](int, const auto &) { called = true; });
	require(!called);
	require(empty.query_component_count() == 0 && empty.solve() == std::vector<int>({0}));
	empty.reset(2);
	empty.cut(0, 1), empty.link(1, 0), empty.link(0, 1);
	empty.query_same(0, 1);
	empty.cut(1, 0), empty.query_size(1);
	empty.cut(0, 1), empty.cut(1, 0), empty.query_component_count();
	empty.link(0, 0), empty.cut(1, 1), empty.query_same(0, 1);
	require(empty.solve() == std::vector<int>({1, 2, 2, 0}));
	std::mt19937 rng(20261020);
	for (int trial = 0; trial < 250; ++trial) {
		int n = 1 + rng() % 18;
		OfflineDynamicConnectivity graph(n);
		if (trial & 1) graph.reserve(500, 500);
		std::vector<std::vector<int>> edges(n, std::vector<int>(n));
		std::vector<int> expected;
		std::vector<std::vector<int>> states;
		for (int step = 0; step < 400; ++step) {
			int type = rng() % 9, u = rng() % n, v = rng() % n;
			if (type < 3) {
				graph.link(u, v);
				if (u != v) ++edges[u][v], ++edges[v][u];
			} else if (type < 6) {
				graph.cut(u, v);
				if (u != v && edges[u][v]) --edges[u][v], --edges[v][u];
			} else {
				auto state = labels(edges);
				int id;
				if (type == 6) {
					id = graph.query_same(u, v), expected.push_back(state[u] == state[v]);
				} else if (type == 7) {
					id = graph.query_size(u), expected.push_back(static_cast<int>(std::count(state.begin(), state.end(), state[u])));
				} else {
					id = graph.query_component_count();
					int count = 0;
					for (int x = 0; x < n; ++x) count += state[x] == x;
					expected.push_back(count);
				}
				require(id == static_cast<int>(expected.size()) - 1);
				states.push_back(state);
			}
		}
		require(graph.solve() == expected && graph.query_count() == static_cast<int>(expected.size()));
		auto copy = graph;
		graph.reset(0);
		require(copy.solve() == expected && graph.solve().empty());
		int visits = 0;
		copy.run([&](int id, const RollbackUnionFind &uf) {
			require(id == visits++);
			for (int u = 0; u < n; ++u)
				for (int v = 0; v < n; ++v) require(uf.same(u, v) == (states[id][u] == states[id][v]));
		});
		require(visits == static_cast<int>(states.size()));
	}
	for (int trial = 0; trial < 150; ++trial) {
		int n = 1 + rng() % 15;
		OfflineDynamicConnectivity graph(n);
		std::vector<std::vector<int>> edges(n, std::vector<int>(n));
		std::vector<std::int64_t> initial(n);
		for (auto &x : initial) x = int(rng() % 201) - 100;
		auto values = initial;
		struct Query { int type, vertex; std::int64_t delta, expected; };
		std::vector<Query> queries;
		for (int step = 0; step < 300; ++step) {
			int type = rng() % 4, u = rng() % n, v = rng() % n;
			if (type == 0) {
				graph.link(u, v);
				if (u != v) ++edges[u][v], ++edges[v][u];
			} else if (type == 1) {
				graph.cut(u, v);
				if (u != v && edges[u][v]) --edges[u][v], --edges[v][u];
			} else {
				std::int64_t delta = type == 2 ? int(rng() % 2000001) - 1000000 : 0;
				values[u] += delta;
				auto state = labels(edges);
				std::int64_t expected = 0;
				for (int x = 0; x < n; ++x) if (state[u] == state[x]) expected += values[x];
				require(graph.query() == static_cast<int>(queries.size()));
				queries.push_back({type, u, delta, expected});
			}
		}
		auto sums = initial;
		std::vector<int> parent(n, -1);
		int visits = 0, links = 0, cuts = 0;
		graph.run([&](int root, int child) {
			require(parent[root] == -1 && parent[child] == -1);
			parent[child] = root, sums[root] += sums[child], ++links;
		}, [&](int root, int child) {
			require(parent[root] == -1 && parent[child] == root);
			sums[root] -= sums[child], parent[child] = -1, ++cuts;
		}, [&](int id, const RollbackUnionFind &uf) {
			require(id == visits++);
			auto q = queries[id];
			if (q.type == 2) for (int x = q.vertex; x != -1; x = parent[x]) sums[x] += q.delta;
			require(sums[uf.leader(q.vertex)] == q.expected);
		});
		require(visits == static_cast<int>(queries.size()) && links == cuts && sums == values);
		require(std::all_of(parent.begin(), parent.end(), [](int p) { return p == -1; }));
	}
	constexpr int n = 100000;
	OfflineDynamicConnectivity path(n);
	for (int v = 1; v < n; ++v) path.link(v - 1, v);
	path.query_component_count();
	for (int v = 1; v < n; ++v) path.cut(v, v - 1);
	path.query_component_count();
	require(path.solve() == std::vector<int>({1, n}));
}
