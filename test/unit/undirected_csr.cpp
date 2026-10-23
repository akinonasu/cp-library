#include "components/undirected_csr.hpp"
#include <algorithm>
#include <random>

int main() {
	std::mt19937 rng(931356);
	for (int n = 0; n <= 50; ++n) for (int trial = 0; trial < 30; ++trial) {
		int m = n ? rng() % 200 : 0;
		std::vector<std::pair<int, int>> edges;
		std::vector<std::vector<std::pair<int, int>>> expected(n);
		for (int i = 0; i < m; ++i) {
			int u = rng() % n, v = rng() % n;
			edges.emplace_back(u, v);
			expected[u].emplace_back(v, i), expected[v].emplace_back(u, i);
		}
		UndirectedCSR graph(n, edges);
		UndirectedVertexCSR compact(n, edges);
		assert(graph.n == n && graph.start.size() == std::size_t(n + 1));
		assert(graph.start[0] == 0 && graph.start[n] == 2 * m);
		assert(compact.n == n && compact.start == graph.start);
		for (int v = 0; v < n; ++v) {
			std::vector<std::pair<int, int>> result;
			for (auto [to, id] : graph[v]) result.emplace_back(to, id);
			assert(result == expected[v]);
			assert(graph.start[v + 1] - graph.start[v] == int(expected[v].size()));
			auto adjacent = compact[v];
			assert(adjacent.size() == expected[v].size());
			for (std::size_t i = 0; i < adjacent.size(); ++i) assert(adjacent[i] == expected[v][i].first);
		}
		auto moved = std::move(graph);
		assert(moved.start[n] == 2 * m);
		auto moved_compact = std::move(compact);
		assert(moved_compact.start == moved.start);
	}
}
