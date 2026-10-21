#ifndef TOPOLOGICAL_SORT_HPP
#define TOPOLOGICAL_SORT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <vector>

template<bool Lexicographical = false, class G>
std::optional<std::vector<int>> topological_sort(const G &graph) {
	assert(graph.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = static_cast<int>(graph.size());
	std::vector<std::size_t> indegree(n);
	for (int v = 0; v < n; ++v) for (int to : graph[v]) {
		assert(0 <= to && to < n);
		++indegree[to];
	}
	std::vector<int> order;
	order.reserve(n);
	if constexpr (Lexicographical) {
		std::vector<int> ready;
		for (int v = 0; v < n; ++v) if (!indegree[v]) ready.push_back(v);
		std::make_heap(ready.begin(), ready.end(), std::greater<int>{});
		while (!ready.empty()) {
			std::pop_heap(ready.begin(), ready.end(), std::greater<int>{});
			int v = ready.back();
			ready.pop_back();
			order.push_back(v);
			for (int to : graph[v]) if (--indegree[to] == 0) {
				ready.push_back(to);
				std::push_heap(ready.begin(), ready.end(), std::greater<int>{});
			}
		}
	} else {
		for (int v = 0; v < n; ++v) if (!indegree[v]) order.push_back(v);
		for (std::size_t head = 0; head < order.size(); ++head)
			for (int to : graph[order[head]]) if (--indegree[to] == 0) order.push_back(to);
	}
	if (order.size() != std::size_t(n)) return std::nullopt;
	return order;
}

#endif
