#include "graph/shortest_path/bellman_ford.hpp"
#include <numeric>
#include <random>

using Graph = std::vector<std::vector<std::pair<int, long long>>>;
constexpr long long INF = 1LL << 60;

template<class G>
void validate(const G &g, int source, const std::vector<std::vector<long long>> &expected) {
	auto verify = [&](const auto &result) {
		bool negative = false;
		for (int v = 0; v < int(g.size()); ++v)
			if (expected[source][v] != INF && expected[v][v] < 0) negative = true;
		assert(result.has_negative_cycle() == negative);
		if (negative) {
			const auto &cycle = result.negative_cycle;
			assert(cycle.size() >= 2 && cycle.front() == cycle.back());
			assert(result.negative_cycle_edges.size() + 1 == cycle.size());
			long long cost = 0;
			for (std::size_t i = 0; i + 1 < cycle.size(); ++i) {
				auto [to, w] = g[cycle[i]][result.negative_cycle_edges[i]];
				assert(to == cycle[i + 1] && expected[source][cycle[i]] != INF);
				cost += w;
			}
			assert(cost < 0);
		} else {
			for (int v = 0; v < int(g.size()); ++v) {
				assert(result.reachable(v) == (expected[source][v] != INF));
				auto path = result.path(v);
				auto edges = result.path_edges(v);
				if (!result.reachable(v)) { assert(path.empty() && edges.empty() && result.parent[v] == -1); continue; }
				assert(result.distance[v] == expected[source][v]);
				assert(path.front() == source && path.back() == v && edges.size() + 1 == path.size());
				assert(path.size() <= std::size_t(g.size()));
				long long cost = 0;
				for (std::size_t i = 0; i < edges.size(); ++i) {
					auto [to, w] = g[path[i]][edges[i]];
					assert(to == path[i + 1]);
					cost += w;
				}
				assert(cost == expected[source][v]);
			}
		}
	};
	verify(bellman_ford(g, source));
	verify(goldberg_radzik(g, source));
	verify(negative_path_detail::solve<true, false, std::int64_t>(g, source));
}

void check(const Graph &g) {
	int n = g.size();
	std::vector<std::vector<long long>> d(n, std::vector<long long>(n, INF));
	std::vector<DirectedCSR<long long>::Edge> edges;
	for (int v = 0; v < n; ++v) {
		d[v][v] = 0;
		for (auto [to, w] : g[v]) { d[v][to] = std::min(d[v][to], w); edges.push_back({v, to, w}); }
	}
	for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
		if (d[i][k] != INF && d[k][j] != INF) d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
	DirectedCSR<long long> csr(n, edges);
	for (int s = 0; s < n; ++s) {
		validate(g, s, d);
		validate(csr, s, d);
		auto comp = negative_path_detail::reachable_components(g, s);
		for (int v = 0; v < n; ++v) {
			assert((comp.id[v] != -1) == (d[s][v] != INF));
			if (comp.id[v] == -1) continue;
			for (auto [to, w] : g[v]) { (void)w; assert(comp.id[v] >= comp.id[to]); }
			for (int u = 0; u < n; ++u)
				if (comp.id[u] != -1) assert((comp.id[v] == comp.id[u]) == (d[u][v] != INF && d[v][u] != INF));
		}
	}
}

int main() {
	check(Graph(1));
	check(Graph{ { {0, 0} } });
	check(Graph{ { {0, -1} } });
	check(Graph{ { {1, -1}, {1, 2} }, { {0, 1} } });
	check(Graph{ { {1, 3}, {2, 2} }, { {2, -4} }, { {1, 4} }, { {3, -1} } });
	std::mt19937 rng(20261020);
	for (int n = 1; n <= 14; ++n) for (int trial = 0; trial < 120; ++trial) {
		Graph g(n);
		int m = rng() % (n * n + 1);
		for (int i = 0; i < m; ++i) g[rng() % n].push_back({int(rng() % n), int(rng() % 21) - 10});
		check(g);
	}
	for (int n = 2; n <= 30; n += 2) for (int trial = 0; trial < 30; ++trial) {
		Graph g(n);
		std::vector<int> potential(n);
		for (auto &x : potential) x = int(rng() % 2001) - 1000;
		for (int i = 0; i < 4 * n; ++i) {
			int v = rng() % n, to = rng() % n;
			g[v].push_back({to, int(rng() % 11) + potential[to] - potential[v]});
		}
		check(g);
	}
	for (int n = 1; n <= 9; ++n) for (int trial = 0; trial < 40; ++trial) {
		Graph g(n);
		for (int v = 0; v < n; ++v) for (int to = v + 1; to < n; ++to)
			if (rng() % 3) g[v].push_back({to, int(rng() % 101) - 50});
		check(g);
	}
	int n = 200000;
	Graph chain(n);
	for (int v = 0; v + 1 < n; ++v) chain[v].push_back({v + 1, -1});
	auto result = bellman_ford(chain, 0);
	assert(!result.has_negative_cycle() && result.distance.back() == 1 - n);
	assert(result.path(n - 1).size() == std::size_t(n));
	chain.back().push_back({n - 2, 0});
	assert(bellman_ford(chain, 0).has_negative_cycle());
	Graph positive{ { {1, -20} }, { {2, 7}, {3, 19} }, { {1, 0}, {3, 5} }, { {1, 0} } };
	check(positive);
	check(Graph{ { {1, -1} }, { {2, 2} }, { {1, 0} } });
	Graph overflow{ { {1, std::numeric_limits<long long>::max() - 1} }, { {2, 10} }, {} };
	bool caught = false;
	try { (void)bellman_ford(overflow, 0); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
#if defined(__SIZEOF_INT128__)
	auto wide = bellman_ford<__int128_t>(overflow, 0);
	assert(wide.distance[2] == __int128_t(std::numeric_limits<long long>::max()) + 9);
	Graph underflow{ { {1, std::numeric_limits<long long>::min()} }, { {2, -1} }, {} };
	assert(bellman_ford<__int128_t>(underflow, 0).distance[2] == __int128_t(std::numeric_limits<long long>::min()) - 1);
	Graph wide_cycle{ { {1, std::numeric_limits<long long>::min()} }, { {0, -1} } };
	assert(bellman_ford<__int128_t>(wide_cycle, 0).has_negative_cycle());
	std::vector<std::vector<std::pair<int, __int128_t>>> wide_positive(3);
	wide_positive[0].push_back({1, -(__int128_t(1) << 90)});
	wide_positive[1].push_back({2, 1});
	wide_positive[2].push_back({1, 0});
	auto heap_fallback = bellman_ford<__int128_t>(wide_positive, 0);
	assert(heap_fallback.distance[2] == -(__int128_t(1) << 90) + 1);
#endif
}
