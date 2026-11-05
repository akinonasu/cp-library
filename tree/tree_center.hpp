#ifndef TREE_CENTER_HPP
#define TREE_CENTER_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

[[nodiscard]] inline std::vector<int> tree_center(const std::vector<std::vector<int>> &graph) {
	assert(graph.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(graph.size());
	if (!n) return {};
	if (n == 1) { assert(graph[0].empty()); return {0}; }
	std::vector<int> degree(n), leaves;
	leaves.reserve(n);
	[[maybe_unused]] std::size_t edges = 0;
	for (int v = 0; v < n; ++v) {
		assert(graph[v].size() < std::size_t(n));
		degree[v] = static_cast<int>(graph[v].size());
		edges += graph[v].size();
		if (degree[v] == 1) leaves.push_back(v);
	}
	assert(edges == std::size_t(n - 1) * 2);
	int remaining = n, head = 0;
	while (remaining > 2) {
		const int end = static_cast<int>(leaves.size());
		assert(head < end);
		remaining -= end - head;
		for (; head < end; ++head) for (int u : graph[leaves[head]]) {
			assert(0 <= u && u < n);
			if (--degree[u] == 1) leaves.push_back(u);
		}
	}
	assert(leaves.size() - std::size_t(head) == std::size_t(remaining));
	std::vector<int> result(leaves.begin() + head, leaves.end());
	if (result.size() == 2 && result[1] < result[0]) std::swap(result[0], result[1]);
	return result;
}

#endif
