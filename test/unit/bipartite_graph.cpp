#include "graph/bipartite_graph.hpp"
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <functional>
#include <random>
#include <utility>

void require(bool value) { if (!value) std::abort(); }

void covering_check(const std::vector<std::vector<int>>& graph) {
	int n = int(graph.size());
	std::vector<unsigned char> possible(1u << n);
	std::function<void(unsigned, unsigned)> enumerate = [&](unsigned remaining, unsigned covered) {
		if (!remaining) { possible[covered] = 1; return; }
		int u = std::countr_zero(remaining);
		remaining ^= 1u << u;
		enumerate(remaining, covered);
		for (int v : graph[u]) if (remaining >> v & 1)
			enumerate(remaining ^ (1u << v), covered | (1u << u) | (1u << v));
	};
	enumerate((1u << n) - 1, 0);
	for (int bit = 0; bit < n; ++bit)
		for (unsigned mask = 0; mask < possible.size(); ++mask)
			if (!(mask >> bit & 1)) possible[mask] |= possible[mask | (1u << bit)];
	for (bool built : {false, true}) {
		BipartiteGraph solver(graph);
		require(solver.is_bipartite());
		std::vector<int> mates;
		if (built) {
			solver.minimum_vertex_cover();
			mates = solver.mates();
		}
		auto before = solver;
		auto color = solver.coloring();
		const auto& query = solver;
		for (unsigned mask = 0; mask < possible.size(); ++mask) {
			std::vector<int> required;
			for (int u = n - 1; u >= 0; --u) if (mask >> u & 1) {
				required.push_back(u);
				if ((mask + u) % 3 == 0) required.push_back(u);
			}
			auto result = query.matching_covering(required);
			require(bool(result) == bool(possible[mask]));
			if (result) {
				std::vector<int> used(n);
				for (auto [u, v] : *result) {
					require(0 <= u && u < n && 0 <= v && v < n && !used[u]++ && !used[v]++);
					require(color[u] == 0 && color[v] == 1);
					require(std::find(graph[u].begin(), graph[u].end(), v) != graph[u].end());
				}
				for (int u : required) require(used[u]);
				if (!mask) require(result->empty());
			}
			if (built) require(solver.mates() == mates);
		}
		require(solver.maximum_matching() == before.maximum_matching());
		require(solver.minimum_vertex_cover() == before.minimum_vertex_cover());
		require(solver.maximum_independent_set() == before.maximum_independent_set() && solver.coloring() == color);
	}
}

void covering_tests() {
	covering_check({});
	covering_check({{}, {}, {}});
	covering_check({{1, 2}, {0}, {0}});
	covering_check({{3, 1, 3}, {0}, {}, {0, 0}});
	std::mt19937 random(20261115);
	for (int trial = 0; trial < 180; ++trial) {
		int n = random() % 9;
		std::vector<std::vector<int>> graph(n);
		std::vector<int> color(n);
		for (int& c : color) c = random() & 1;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
			if (color[u] != color[v] && random() % 3 == 0) {
				int copies = 1 + random() % 3;
				while (copies--) graph[u].push_back(v), graph[v].push_back(u);
			}
		for (auto& adj : graph) std::shuffle(adj.begin(), adj.end(), random);
		covering_check(graph);
	}
	BipartiteGraph graph({{1, 2}, {0}, {0}});
	const auto& query = graph;
	auto before = graph.maximum_matching();
	auto result = query.matching_covering({0, 2, 0, 2});
	require(result && (*result == std::vector<std::pair<int, int>>{{0, 2}}));
	require(graph.maximum_matching() == before);
	auto copy = graph;
	require(graph.build({{1}, {0}, {}}));
	require(!query.matching_covering({2}) && graph.matching_size() == 1);
	require(copy.matching_covering({0, 2}) == result);
	require(graph.build({}));
	require(query.matching_covering({}) && query.matching_covering({})->empty());
	require(graph.empty() && graph.matching_size() == 0);
}

void check(const std::vector<std::vector<int>>& graph) {
	int n = static_cast<int>(graph.size()), independent_size = 0;
	bool bipartite = false;
	for (unsigned mask = 0; mask < (1u << n); ++mask) {
		bool independent = true, coloring = true;
		for (int u = 0; u < n; ++u) for (int v : graph[u]) {
			if ((mask >> u & 1) && (mask >> v & 1)) independent = false;
			if ((mask >> u & 1) == (mask >> v & 1)) coloring = false;
		}
		if (independent) independent_size = std::max(independent_size, std::popcount(mask));
		bipartite |= coloring;
	}
	BipartiteGraph solver(graph);
	require(solver.size() == n && solver.empty() == graph.empty() && solver.is_bipartite() == bipartite);
	if (!bipartite) {
		require(solver.build({}) && solver.empty() && solver.is_bipartite());
		require(solver.maximum_independent_set().empty() && solver.maximum_matching().empty());
		return;
	}
	const auto& color = solver.coloring();
	for (int u = 0; u < n; ++u) {
		require(color[u] == 0 || color[u] == 1);
		for (int v : graph[u]) require(color[u] != color[v]);
	}
	auto chosen = solver.maximum_independent_set(), cover = solver.minimum_vertex_cover();
	require(int(chosen.size()) == independent_size && int(cover.size()) == n - independent_size);
	std::vector<int> used(n), selected(n);
	for (int v : chosen) require(0 <= v && v < n && !used[v]++), selected[v] = 1;
	for (int v : cover) require(0 <= v && v < n && !used[v]++);
	for (int u = 0; u < n; ++u) {
		require(used[u] == 1);
		for (int v : graph[u]) require(!selected[u] || !selected[v]);
	}
	auto matching = solver.maximum_matching();
	auto mates = solver.mates();
	require(solver.matching_size() == n - independent_size && int(matching.size()) == solver.matching_size());
	std::fill(used.begin(), used.end(), 0);
	for (auto [u, v] : matching) {
		require(0 <= u && u < n && 0 <= v && v < n && !used[u]++ && !used[v]++);
		require(color[u] == 0 && color[v] == 1 && std::find(graph[u].begin(), graph[u].end(), v) != graph[u].end());
		require(mates[u] == v && mates[v] == u);
	}
	for (int u = 0; u < n; ++u) require((mates[u] >= 0) == bool(used[u]));
	auto edges = solver.minimum_edge_cover();
	bool isolated = std::any_of(graph.begin(), graph.end(), [](const auto& adj) { return adj.empty(); });
	require(bool(edges) == !isolated);
	if (edges) {
		require(int(edges->size()) == n - solver.matching_size());
		std::fill(used.begin(), used.end(), 0);
		for (auto [u, v] : *edges) {
			require(0 <= u && u < n && 0 <= v && v < n && color[u] == 0 && color[v] == 1);
			require(std::find(graph[u].begin(), graph[u].end(), v) != graph[u].end());
			used[u] = used[v] = 1;
		}
		for (int x : used) require(x == 1);
	}
	auto copy = solver;
	require(copy.matching_size() == solver.matching_size() && copy.maximum_independent_set() == chosen);
	require(!solver.build({{0}}) && !solver.is_bipartite());
	require(solver.build(graph) && solver.maximum_independent_set() == chosen);
}

int main() {
	covering_tests();
	BipartiteGraph empty;
	require(empty.is_bipartite() && empty.empty() && empty.matching_size() == 0);
	require(empty.maximum_independent_set().empty() && empty.minimum_vertex_cover().empty());
	require(empty.minimum_edge_cover() && empty.minimum_edge_cover()->empty());
	for (int n = 0; n <= 6; ++n) {
		int count = n * (n - 1) / 2;
		for (unsigned mask = 0; mask < (1u << count); ++mask) {
			std::vector<std::vector<int>> graph(n);
			int bit = 0;
			for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v, ++bit)
				if (mask >> bit & 1) graph[u].push_back(v), graph[v].push_back(u);
			check(graph);
		}
	}
	std::mt19937 rng(20261023);
	for (int t = 0; t < 600; ++t) {
		int n = rng() % 12;
		std::vector<std::vector<int>> graph(n);
		std::vector<int> color(n);
		for (int& x : color) x = rng() & 1;
		for (int u = 0; u < n; ++u) for (int v = u; v < n; ++v)
			if ((t & 1 || color[u] != color[v]) && rng() % 4 == 0) {
				int count = 1 + rng() % 3;
				while (count--) graph[u].push_back(v), graph[v].push_back(u);
			}
		for (auto& row : graph) std::shuffle(row.begin(), row.end(), rng);
		check(graph);
	}
	const int n = 200000;
	std::vector<std::vector<int>> path(n);
	for (int i = 1; i < n; ++i) path[i - 1].push_back(i), path[i].push_back(i - 1);
	BipartiteGraph long_path(path);
	require(long_path.matching_size() == n / 2 && long_path.maximum_independent_set().size() == std::size_t(n / 2));
	require(long_path.minimum_vertex_cover().size() == std::size_t(n / 2));
}
