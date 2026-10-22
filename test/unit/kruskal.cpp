#include "graph/kruskal.hpp"
#include <bit>
#include <numeric>
#include <random>

using Edge = Kruskal<>::Edge;

bool join(std::vector<int> &labels, int u, int v) {
	int a = labels[u], b = labels[v];
	if (a == b) return false;
	for (int &x : labels) if (x == b) x = a;
	return true;
}

std::int64_t exhaustive(int n, const std::vector<Edge> &edges, int components) {
	std::int64_t best = std::numeric_limits<std::int64_t>::max();
	for (unsigned mask = 0; mask < (1U << edges.size()); ++mask) {
		if (std::popcount(mask) != n - components) continue;
		std::vector<int> labels(n);
		std::iota(labels.begin(), labels.end(), 0);
		std::int64_t cost = 0;
		bool forest = true;
		for (std::size_t i = 0; i < edges.size(); ++i) if ((mask >> i) & 1) {
			auto [u, v, w] = edges[i];
			if (!join(labels, u, v)) { forest = false; break; }
			cost += w;
		}
		if (forest) best = std::min(best, cost);
	}
	return best;
}

std::int64_t prim(int n, const std::vector<Edge> &edges) {
	std::vector<bool> used(n);
	std::int64_t cost = 0;
	for (int root = 0; root < n; ++root) if (!used[root]) {
		used[root] = true;
		while (true) {
			int next = -1;
			std::int64_t weight = 0;
			for (const auto &e : edges) if (used[e.from] != used[e.to]) {
				if (next == -1 || e.cost < weight) {
					next = used[e.from] ? e.to : e.from;
					weight = e.cost;
				}
			}
			if (next == -1) break;
			used[next] = true;
			cost += weight;
		}
	}
	return cost;
}

void check(int n, const std::vector<Edge> &edges, bool brute = true) {
	Kruskal<> graph(n);
	graph.reserve(edges.size());
	std::vector<int> original(n);
	std::iota(original.begin(), original.end(), 0);
	int components = n;
	for (std::size_t i = 0; i < edges.size(); ++i) {
		auto [u, v, cost] = edges[i];
		assert(graph.add_edge(u, v, cost) == int(i));
		components -= join(original, u, v);
	}
	assert(graph.size() == n && graph.edge_count() == int(edges.size()));
	const auto &view = graph;
	auto result = view.solve(), again = view.solve();
	assert(result.cost == again.cost && result.edges == again.edges && result.components == again.components);
	assert(result.components == components && result.connected() == (components <= 1));
	assert(int(result.edges.size()) == n - components);
	std::vector<int> chosen(n);
	std::iota(chosen.begin(), chosen.end(), 0);
	std::int64_t cost = 0;
	int previous = -1;
	for (int id : result.edges) {
		assert(0 <= id && id < int(edges.size()));
		auto [u, v, w] = graph.get_edge(id);
		assert(join(chosen, u, v));
		cost += w;
		if (previous != -1) assert(edges[previous].cost < w || (edges[previous].cost == w && previous < id));
		previous = id;
	}
	for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
		assert((chosen[u] == chosen[v]) == (original[u] == original[v]));
	assert(cost == result.cost && cost == prim(n, edges));
	if (brute) assert(cost == exhaustive(n, edges, components));
	for (std::size_t i = 0; i < edges.size(); ++i) {
		const auto &a = graph.edges()[i], &b = edges[i];
		assert(a.from == b.from && a.to == b.to && a.cost == b.cost);
	}
}

template<class Cost>
void overflow(Cost a, Cost b) {
	Kruskal<Cost> graph(3);
	graph.add_edge(0, 1, a);
	graph.add_edge(1, 2, b);
	bool caught = false;
	try { graph.solve(); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
}

int main() {
	check(0, {});
	check(1, {});
	check(6, {});
	check(1, { {0, 0, -10}, {0, 0, 5} });
	check(4, { {0, 1, 3}, {1, 2, 1}, {0, 2, 2}, {2, 3, 4} });
	check(5, { {0, 1, -1}, {1, 0, -1}, {2, 3, -3}, {4, 4, -9} });
	for (int n = 1; n <= 4; ++n) {
		int pairs = n * (n - 1) / 2, total = 1;
		for (int i = 0; i < pairs; ++i) total *= 3;
		for (int code = 0; code < total; ++code) {
			std::vector<Edge> edges;
			int value = code;
			for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v, value /= 3)
				if (value % 3) edges.push_back({u, v, value % 3 == 1 ? -2 : 3});
			check(n, edges);
		}
	}
	std::mt19937 rng(271884);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + rng() % 7, m = rng() % 13;
		std::vector<Edge> edges;
		for (int i = 0; i < m; ++i) edges.push_back({int(rng() % n), int(rng() % n), int(rng() % 13) - 6});
		check(n, edges);
	}
	for (int trial = 0; trial < 100; ++trial) {
		int n = 10 + rng() % 60;
		std::vector<Edge> edges;
		for (int i = 0; i < 500; ++i) edges.push_back({int(rng() % n), int(rng() % n), int(rng() % 101) - 50});
		check(n, edges, false);
	}
	Kruskal<> graph(3);
	graph.add_edge(0, 1, 7);
	auto old = graph.solve();
	graph.add_edge(1, 2, -3);
	assert(graph.solve().cost == 4 && graph.solve().connected());
	assert(old.cost == 7 && old.components == 2);
	graph.clear();
	assert(graph.size() == 3 && graph.edge_count() == 0 && graph.solve().components == 3);
	assert(graph.add_edge(2, 0, 5) == 0);
	Kruskal<double> real(3);
	real.add_edge(0, 1, 1.5);
	real.add_edge(1, 2, -0.25);
	real.add_edge(0, 2, 3.0);
	assert(real.solve().cost == 1.25);
	Kruskal<__int128_t> wide(3);
	__int128_t large = __int128_t(1) << 100;
	wide.add_edge(0, 1, large);
	wide.add_edge(1, 2, large + 1);
	assert(wide.solve().cost == 2 * large + 1);
	Kruskal<std::uint64_t> unsigned_graph(2);
	unsigned_graph.add_edge(0, 1, std::numeric_limits<std::uint64_t>::max());
	assert(unsigned_graph.solve().cost == std::numeric_limits<std::uint64_t>::max());
	overflow<std::int64_t>(std::numeric_limits<std::int64_t>::max(), 1);
	overflow<std::int64_t>(std::numeric_limits<std::int64_t>::lowest(), -1);
	overflow<std::uint64_t>(std::numeric_limits<std::uint64_t>::max(), 1);
	overflow<__int128_t>(std::numeric_limits<__int128_t>::max(), 1);
	graph.clear();
	graph.add_edge(0, 1, std::numeric_limits<std::int64_t>::lowest());
	graph.add_edge(1, 2, std::numeric_limits<std::int64_t>::max());
	assert(graph.solve().cost == -1);
	const int n = 500000;
	Kruskal<> path(n);
	path.reserve(n - 1);
	for (int i = 1; i < n; ++i) path.add_edge(i - 1, i, n - i);
	auto result = path.solve();
	assert(result.connected() && result.edges.size() == n - 1);
	assert(result.cost == std::int64_t(n) * (n - 1) / 2);
}
