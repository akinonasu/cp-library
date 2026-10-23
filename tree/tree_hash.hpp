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
	int first = 0;
	for (int v = 1; v < n; ++v)
		if (tree.size[v] >= n / 2 + n % 2 && tree.size[v] < tree.size[first]) first = v;
	int second = tree.size[first] == n / 2 ? tree.parent[first] : -1;
	if (first != 0) tree = tree_hash_detail::root_tree(graph, first);
	auto hashes = tree_hash_detail::hashes(graph, tree, seed);
	result[0] = hashes[first];
	if (second == -1) result[1] = result[0];
	else {
		int height = 0;
		for (int u : graph[first]) if (u != second) height = std::max(height, tree.height[u] + 1);
		auto salt = tree_hash_detail::salt_at(height, seed);
		TreeHashValue branch{ {1, 1} };
		for (int u : graph[first]) if (u != second) tree_hash_detail::append(branch, hashes[u], salt);
		salt = tree_hash_detail::salt_at(std::max(tree.height[second], height + 1), seed);
		result[1] = { {1, 1} };
		for (int u : graph[second]) tree_hash_detail::append(result[1], u == first ? branch : hashes[u], salt);
		if (result[1] < result[0]) std::swap(result[0], result[1]);
	}
	return result;
}

#endif
