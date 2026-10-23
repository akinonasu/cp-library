#include "../../tree/tree_diameter.hpp"
#include <cassert>
#include <random>
#include <type_traits>

int main() {
	TreeDiameter<> empty;
	assert(empty.solve().distance == 0 && empty.solve().path.empty());
	std::mt19937 random(883);
	for (int n = 1; n <= 70; ++n) for (int repeat = 0; repeat < 40; ++repeat) {
		TreeDiameter<> tree(n);
		std::vector<std::vector<std::pair<int, long long>>> graph(n);
		for (int v = 1; v < n; ++v) {
			int p = random() % v;
			long long weight = int(random() % 31) - 15;
			tree.add_edge(v, p, weight);
			graph[v].emplace_back(p, weight), graph[p].emplace_back(v, weight);
		}
		long long best = 0;
		for (int s = 0; s < n; ++s) {
			std::vector<int> parent(n, -1), order{s};
			std::vector<long long> distance(n);
			parent[s] = s;
			for (std::size_t i = 0; i < order.size(); ++i) {
				int v = order[i];
				best = std::max(best, distance[v]);
				for (auto [u, w] : graph[v]) if (u != parent[v]) parent[u] = v, distance[u] = distance[v] + w, order.push_back(u);
			}
		}
		auto result = tree.solve();
		assert(result.distance == best && !result.path.empty());
		std::vector<bool> seen(n);
		long long length = 0;
		for (std::size_t i = 0; i < result.path.size(); ++i) {
			int v = result.path[i];
			assert(0 <= v && v < n && !seen[v]);
			seen[v] = true;
			if (!i) continue;
			bool adjacent = false;
			for (auto [u, w] : graph[v]) if (u == result.path[i - 1]) length += w, adjacent = true;
			assert(adjacent);
		}
		assert(length == result.distance && tree.solve().distance == result.distance);
		auto copy = tree;
		assert(copy.solve().distance == best);
	}
	TreeDiameter<__int128_t> large(3);
	__int128_t weight = __int128_t(1) << 80;
	large.add_edge(0, 1, weight), large.add_edge(1, 2, weight);
	assert(large.solve().distance == weight * 2);
	TreeDiameter<double> real(3);
	real.add_edge(0, 1, 1.25), real.add_edge(1, 2, 2.5);
	assert(real.solve().distance == 3.75);
}
