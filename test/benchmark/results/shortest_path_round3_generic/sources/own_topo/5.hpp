#ifndef DIJKSTRA_TOPOLOGICAL_HPP
#define DIJKSTRA_TOPOLOGICAL_HPP

#include "3.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace shortest_path_detail {
template<bool Path, class G, class T>
std::vector<T> topological_run(const G &g, int start, int target, T inf, std::vector<int> &parent) {
	std::size_t n = std::size_t(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(start >= 0 && static_cast<std::size_t>(start) < n);
	assert(inf > 0);
	std::vector<T> distance(n, inf);
	std::vector<std::size_t> indegree(n);
	std::vector<int> ready;
	ready.reserve(n);
	for (std::size_t v = 0; v < n; ++v)
		for (const auto &[to, weight] : g[int(v)]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(weight >= 0);
			++indegree[to];
		}
	for (std::size_t v = 0; v < n; ++v)
		if (!indegree[v]) ready.push_back(int(v));
	distance[start] = 0;
	auto improve = [&](int from, const T &cost, const T &allowance, int to, const auto &weight) {
		using Weight = std::remove_cvref_t<decltype(weight)>;
		if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
					  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
			using Compare = std::common_type_t<Weight, T>;
			if (Compare(weight) >= Compare(allowance)) return false;
		} else if (weight >= allowance) return false;
		T next = cost + weight;
		if (next >= distance[to]) return false;
		if constexpr (Path) {
			if (next >= distance[target]) return false;
			parent[to] = from;
		}
		distance[to] = std::move(next);
		return true;
	};
	for (std::size_t head = 0; head < ready.size(); ++head) {
		int v = ready[head];
		if constexpr (Path) if (v == target) return distance;
		T cost = distance[v];
		bool active = cost < inf;
		if constexpr (Path) active &= cost < distance[target];
		T allowance = active ? inf - cost : T(0);
		for (const auto &[to, weight] : g[v]) {
			if (active) improve(v, cost, allowance, to, weight);
			if (--indegree[to] == 0) ready.push_back(to);
		}
	}
	using State = std::pair<T, int>;
	constexpr bool radix = std::is_integral_v<T> && sizeof(T) <= 8;
	std::conditional_t<radix, shortest_path_detail::RadixQueue<T>, std::vector<State>> queue;
	auto push = [&](T cost, int v) {
		if constexpr (radix) queue.push(cost, v);
		else {
			queue.emplace_back(std::move(cost), v);
			std::push_heap(queue.begin(), queue.end(), std::greater<State>());
		}
	};
	for (std::size_t v = 0; v < n; ++v) {
		if (!indegree[v] || distance[v] == inf || g[v].empty()) continue;
		if constexpr (Path) if (int(v) == target || distance[v] >= distance[target]) continue;
		push(distance[v], int(v));
	}
	while (!queue.empty()) {
		const auto [cost, v] = [&] {
			if constexpr (radix) return queue.pop();
			else {
				std::pop_heap(queue.begin(), queue.end(), std::greater<State>());
				State state = std::move(queue.back());
				queue.pop_back();
				return state;
			}
		}();
		if (cost != distance[v]) continue;
		if constexpr (Path) if (cost >= distance[target]) break;
		T allowance = inf - cost;
		for (const auto &[to, weight] : g[v]) {
			if (!improve(v, cost, allowance, to, weight)) continue;
			if constexpr (Path) if (to == target) continue;
			if (!g[to].empty()) push(distance[to], to);
		}
	}
	return distance;
}
}

#endif
