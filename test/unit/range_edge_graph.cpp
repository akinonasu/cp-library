#include "graph/range_edge_graph.hpp"
#include "graph/shortest_path/dijkstra.hpp"
#include "graph/shortest_path/bellman_ford.hpp"
#include <random>

int main() {
	RangeEdgeGraph<> empty;
	empty.add_range_to_range(0, 0, 0, 0, -1);
	assert(empty.size() == 0 && empty.vertex_count() == 0 && empty.edge_count() == 0);
	assert(empty.build().empty() && empty.adjacency_list().empty() && empty.original_path({}).empty());
	std::mt19937 rng(574498);
	constexpr long long inf = 1000000000000LL;
	for (int n = 1; n <= 20; ++n) for (int trial = 0; trial < 20; ++trial) {
		RangeEdgeGraph<long long> graph(n);
		assert(graph.vertex_count() == 3 * n - 2 && graph.edge_count() == std::size_t(4 * (n - 1)));
		std::vector<std::vector<long long>> weight(n, std::vector<long long>(n, inf));
		auto add = [&](int l1, int r1, int l2, int r2, long long w) {
			for (int u = l1; u < r1; ++u) for (int v = l2; v < r2; ++v) weight[u][v] = std::min(weight[u][v], w);
		};
		int count = trial ? 60 : 0;
		while (count--) {
			int l = rng() % (n + 1), r = rng() % (n + 1), x = rng() % (n + 1), y = rng() % (n + 1), u = rng() % n, v = rng() % n;
			if (l > r) std::swap(l, r);
			if (x > y) std::swap(x, y);
			long long w = rng() % 21;
			switch (rng() % 4) {
			case 0: graph.add_edge(u, v, w); add(u, u + 1, v, v + 1, w); break;
			case 1: graph.add_to_range(u, l, r, w); add(u, u + 1, l, r, w); break;
			case 2: graph.add_from_range(l, r, v, w); add(l, r, v, v + 1, w); break;
			case 3: graph.add_range_to_range(l, r, x, y, w); add(l, r, x, y, w); break;
			}
		}
		auto dist = weight;
		for (int v = 0; v < n; ++v) dist[v][v] = 0;
		for (int k = 0; k < n; ++k) for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) dist[u][v] = std::min(dist[u][v], dist[u][k] + dist[k][v]);
		auto csr = graph.build();
		auto adj = graph.adjacency_list();
		assert(csr.size() == graph.vertex_count() && csr.edge_count() == graph.edge_count());
		for (int s = 0; s < n; ++s) {
			auto actual = dijkstra(csr, s, inf);
			assert(actual == dijkstra(adj, s, inf));
			for (int t = 0; t < n; ++t) {
				assert(actual[t] == (dist[s][t] == inf ? -1 : dist[s][t]));
				auto [cost, path] = dijkstra_path(csr, s, t, inf);
				assert(cost == actual[t]);
				auto original = graph.original_path(path);
				if (cost == -1) { assert(original.empty()); continue; }
				assert(original.front() == s && original.back() == t);
				long long total = 0;
				for (int i = 1; i < int(original.size()); ++i) { assert(weight[original[i - 1]][original[i]] != inf); total += weight[original[i - 1]][original[i]]; }
				assert(total == cost);
			}
		}
		auto snapshot = graph.build();
		auto copy = graph;
		copy.add_edge(0, 0);
		assert(copy.edge_count() == graph.edge_count() + 1 && snapshot.edge_count() == graph.edge_count());
	}
	for (int n = 1; n < 15; ++n) for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
		RangeEdgeGraph<int> graph(n);
		graph.add_to_range(0, l, r, 7);
		auto dist = dijkstra(graph.build(), 0);
		for (int v = 1; v < n; ++v) assert(dist[v] == (l <= v && v < r ? 7 : -1));
	}
	RangeEdgeGraph<> negative(5);
	negative.add_to_range(0, 1, 3, -5);
	negative.add_range_to_range(1, 3, 3, 5, -2);
	auto negative_result = bellman_ford(negative.build(), 0);
	assert(!negative_result.has_negative_cycle());
	for (int v = 0; v < 5; ++v) assert(negative_result.distance[v] == (v == 0 ? 0 : v < 3 ? -5 : -7));
	negative.add_edge(4, 0, 0);
	assert(bellman_ford(negative.build(), 0).has_negative_cycle());
	RangeEdgeGraph<__int128_t> wide(2);
	wide.add_range_to_range(0, 1, 1, 2, __int128_t(1) << 90);
	assert(dijkstra(wide.build(), 0, __int128_t(1) << 100)[1] == (__int128_t(1) << 90));
	RangeEdgeGraph<double> floating(2);
	floating.add_edge(0, 1, 1.25);
	assert(dijkstra(floating.build(), 0, 100.0)[1] == 1.25);
	RangeEdgeGraph<> large(100000);
	large.add_to_range(0, 0, 100000, 7);
	large.add_from_range(0, 100000, 99999, 3);
	large.add_range_to_range(123, 56789, 45, 99876, 2);
	assert(large.vertex_count() == 299999 && large.edge_count() < 400100);
	assert(dijkstra(large.build(), 0)[99999] == 3);
}
