#ifndef TREE_HASH_HPP
#define TREE_HASH_HPP

#include "../components/tree_hash_core.hpp"
#include <utility>

inline std::array<TreeHashValue, 2> tree_hash(const std::vector<std::vector<int>> &graph,
	std::uint64_t seed = tree_hash_detail::default_seed()) {
	std::array<TreeHashValue, 2> result{};
	if (graph.empty()) return result;
	auto tree = tree_hash_detail::root_tree(graph, 0);
	const int n = static_cast<int>(graph.size());
	std::array<int, 2> centroid{};
	int count = 0;
	for (int v = 0; v < n; ++v) {
		int largest = n - tree.size[v];
		for (int u : graph[v]) if (tree.parent[u] == v) largest = std::max(largest, tree.size[u]);
		if (largest <= n / 2) {
			assert(count < 2);
			centroid[count++] = v;
		}
	}
	assert(count >= 1);
	for (int i = 0; i < count; ++i) {
		int root = centroid[i];
		if (tree.parent[root] != -1) tree = tree_hash_detail::root_tree(graph, root);
		result[i] = tree_hash_detail::hashes(graph, tree, seed)[root];
	}
	if (count == 1) result[1] = result[0];
	else if (result[1] < result[0]) std::swap(result[0], result[1]);
	return result;
}

#endif
