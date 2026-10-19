#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "../components/dijkstra_queue.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

inline constexpr std::int64_t DIJKSTRA_INF = 1'000'000'000'000'000'000;

namespace shortest_path_detail {
template<class T> T infinity() {
	if constexpr (std::numeric_limits<T>::is_bounded && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::digits < 60)
		return std::numeric_limits<T>::max();
	else return T(DIJKSTRA_INF);
}

template<bool Path, class T>
std::vector<T> run(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf, std::vector<int> &parent) {
	assert(g.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < g.size());
	assert(inf > 0);
	std::vector<T> d(g.size(), inf);
	d[st] = 0;
	using State = std::pair<T, int>;
	constexpr bool radix = std::is_integral_v<T> && sizeof(T) <= 8;
	std::conditional_t<radix, RadixQueue<T>, std::vector<State>> pq;
	auto push = [&](T cost, int v) {
		if constexpr (radix) pq.push(cost, v);
		else {
			pq.emplace_back(std::move(cost), v);
			std::push_heap(pq.begin(), pq.end(), std::greater<State>());
		}
	};
	push(0, st);
	while (!pq.empty()) {
		const auto [cost, v] = [&] {
			if constexpr (radix) return pq.pop();
			else {
				std::pop_heap(pq.begin(), pq.end(), std::greater<State>());
				State s = std::move(pq.back());
				pq.pop_back();
				return s;
			}
		}();
		if (d[v] != cost) continue;
		if constexpr (Path) if (cost >= d[target]) break;
		T allowance = inf - cost;
		for (const auto &[to, c] : g[v]) {
			assert(to >= 0 && static_cast<std::size_t>(to) < g.size());
			assert(c >= 0);
			if (c >= allowance) continue;
			T next = cost + c;
			if (next >= d[to]) continue;
			if constexpr (Path) {
				if (next >= d[target]) continue;
				parent[to] = v;
			}
			d[to] = next;
			if constexpr (Path) if (to == target) continue;
			if (!g[to].empty()) push(std::move(next), to);
		}
	}
	return d;
}
}

template<class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	std::vector<int> parent;
	auto d = shortest_path_detail::run<false>(g, st, -1, inf, parent);
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

template<class T>
std::pair<T, std::vector<int>> dijkstra_path(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	assert(st >= 0 && static_cast<std::size_t>(st) < g.size());
	assert(target >= 0 && static_cast<std::size_t>(target) < g.size());
	assert(inf > 0);
	if (st == target) return {T(0), {st}};
	std::vector<int> parent(g.size(), -1);
	auto d = shortest_path_detail::run<true>(g, st, target, inf, parent);
	if (d[target] == inf) return {T(-1), {}};
	std::vector<int> path;
	for (int v = target; v != st; v = parent[v]) path.push_back(v);
	path.push_back(st);
	std::reverse(path.begin(), path.end());
	return {std::move(d[target]), std::move(path)};
}

#endif
