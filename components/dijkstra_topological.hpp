#ifndef DIJKSTRA_TOPOLOGICAL_HPP
#define DIJKSTRA_TOPOLOGICAL_HPP

#include "dijkstra_queue.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
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
		if (reaches_limit(weight, allowance)) return false;
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
	DistanceQueue<T> queue;
	for (std::size_t v = 0; v < n; ++v) {
		if (!indegree[v] || distance[v] == inf || g[v].empty()) continue;
		if constexpr (Path) if (int(v) == target || distance[v] >= distance[target]) continue;
		queue.push(distance[v], int(v));
	}
	while (!queue.empty()) {
		const auto [cost, v] = queue.pop();
		if (cost != distance[v]) continue;
		if constexpr (Path) if (cost >= distance[target]) break;
		T allowance = inf - cost;
		for (const auto &[to, weight] : g[v]) {
			if (!improve(v, cost, allowance, to, weight)) continue;
			if constexpr (Path) if (to == target) continue;
			if (!g[to].empty()) queue.push(distance[to], to);
		}
	}
	return distance;
}
}

#endif
