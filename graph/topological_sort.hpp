#ifndef TOPOLOGICAL_SORT_HPP
#define TOPOLOGICAL_SORT_HPP

#include "../components/predecessor_set.hpp"
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
		std::optional<PredecessorSet> ready_set;
		bool use_set = ready.size() > 128;
		if (use_set) {
			ready_set.emplace(n);
			for (int v : ready) ready_set->insert(v);
			ready.clear();
		} else std::make_heap(ready.begin(), ready.end(), std::greater<int>{});
		while (use_set || !ready.empty()) {
			int v;
			if (use_set) {
				v = ready_set->min();
				ready_set->erase(v);
			} else {
				std::pop_heap(ready.begin(), ready.end(), std::greater<int>{});
				v = ready.back();
				ready.pop_back();
			}
			order.push_back(v);
			for (int to : graph[v]) if (--indegree[to] == 0) {
				if (use_set) ready_set->insert(to);
				else {
					ready.push_back(to);
					std::push_heap(ready.begin(), ready.end(), std::greater<int>{});
				}
			}
			if (use_set && ready_set->size() <= 32) {
				while (!ready_set->empty()) {
					int x = ready_set->min();
					ready_set->erase(x);
					ready.push_back(x);
				}
				std::make_heap(ready.begin(), ready.end(), std::greater<int>{});
				use_set = false;
			} else if (!use_set && ready.size() > 128) {
				if (!ready_set) ready_set.emplace(n);
				for (int x : ready) ready_set->insert(x);
				ready.clear();
				use_set = true;
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
