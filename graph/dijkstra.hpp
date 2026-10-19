#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "../components/dijkstra_queue.hpp"
#include "../components/dijkstra_topological.hpp"
#include "../components/dijkstra_unique.hpp"
#include "../components/directed_csr.hpp"
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

template<bool Path, class G, class T>
std::vector<T> run(const G &g, int st, int target, T inf, std::vector<int> &parent) {
	const auto n = static_cast<std::size_t>(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < n);
	assert(inf > 0);
	std::vector<T> d(n, inf);
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
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(c >= 0);
			using Weight = std::remove_cvref_t<decltype(c)>;
			if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
						  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
				using Compare = std::common_type_t<Weight, T>;
				if (Compare(c) >= Compare(allowance)) continue;
			} else if (c >= allowance) continue;
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

template<bool Topological, class G, class T>
std::vector<T> distances(const G &g, int st, T inf) {
	std::vector<int> parent;
	auto d = [&] {
		if constexpr (Topological) return topological_run<false>(g, st, -1, inf, parent);
		else return run<false>(g, st, -1, inf, parent);
	}();
	for (T &val : d)
		if (val == inf) val = -1;
	return d;
}

template<bool Topological, class G, class T>
std::pair<T, std::vector<int>> path(const G &g, int st, int target, T inf) {
	[[maybe_unused]] const auto n = static_cast<std::size_t>(g.size());
	assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	assert(st >= 0 && static_cast<std::size_t>(st) < n);
	assert(target >= 0 && static_cast<std::size_t>(target) < n);
	assert(inf > 0);
	if (st == target) return {T(0), {st}};
	if (g[st].empty()) return {T(-1), {}};
	if (g[st].size() == 1) {
		using Weight = std::remove_cvref_t<decltype(g[st][0].second)>;
		std::vector<int> direct{st};
		T cost = 0;
		int v = st, checkpoint = st;
		std::size_t power = 1, length = 0;
		while (g[v].size() == 1) {
			const auto &[to, weight] = g[v][0];
			assert(to >= 0 && static_cast<std::size_t>(to) < n);
			assert(weight >= 0);
			if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
						  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
				using Compare = std::common_type_t<Weight, T>;
				if (Compare(weight) >= Compare(inf - cost)) return {T(-1), {}};
			} else if (weight >= inf - cost) return {T(-1), {}};
			cost += weight;
			v = to;
			direct.push_back(v);
			if (v == target) return {std::move(cost), std::move(direct)};
			if (v == checkpoint) return {T(-1), {}};
			if (++length == power) {
				checkpoint = v;
				power *= 2;
				length = 0;
			}
		}
		if (g[v].empty()) return {T(-1), {}};
		if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
			auto suffix = shortest_path_detail::path<Topological>(g, v, target, T(inf - cost));
			if (suffix.first == T(-1)) return {T(-1), {}};
			direct.pop_back();
			direct.insert(direct.end(), suffix.second.begin(), suffix.second.end());
			return {cost + suffix.first, std::move(direct)};
		}
	}
	std::vector<int> parent(g.size(), -1);
	if constexpr (requires { directed_csr_detail::Access::unique_incoming(g); })
		if (directed_csr_detail::Access::unique_incoming(g)) return unique_path(g, st, target, inf, parent);
	auto d = [&] {
		if constexpr (Topological) return topological_run<true>(g, st, target, inf, parent);
		else return run<true>(g, st, target, inf, parent);
	}();
	if (d[target] == inf) return {T(-1), {}};
	std::vector<int> path;
	for (int v = target; v != st; v = parent[v]) path.push_back(v);
	path.push_back(st);
	std::reverse(path.begin(), path.end());
	return {std::move(d[target]), std::move(path)};
}
}

template<bool Topological, class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::distances<Topological>(g, st, inf);
}

template<class T>
std::vector<T> dijkstra(const std::vector<std::vector<std::pair<int, T>>> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra<false>(g, st, inf);
}

template<bool Topological, class T>
std::pair<T, std::vector<int>> dijkstra_path(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::path<Topological>(g, st, target, inf);
}

template<class T>
std::pair<T, std::vector<int>> dijkstra_path(const std::vector<std::vector<std::pair<int, T>>> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra_path<false>(g, st, target, inf);
}

template<bool Topological, class W, class T = typename DirectedCSR<W>::distance_type>
std::vector<T> dijkstra(const DirectedCSR<W> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return shortest_path_detail::distances<Topological>(g, st, inf);
}

template<class W, class T = typename DirectedCSR<W>::distance_type>
std::vector<T> dijkstra(const DirectedCSR<W> &g, int st, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra<false>(g, st, inf);
}

template<bool Topological, class W, class T = typename DirectedCSR<W>::distance_type>
std::pair<T, std::vector<int>> dijkstra_path(const DirectedCSR<W> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	if (directed_csr_detail::Access::has_single_edge_layout(g))
		return shortest_path_detail::path<Topological>(directed_csr_detail::Access::single_edge_view(g), st, target, inf);
	return shortest_path_detail::path<Topological>(g, st, target, inf);
}

template<class W, class T = typename DirectedCSR<W>::distance_type>
std::pair<T, std::vector<int>> dijkstra_path(const DirectedCSR<W> &g, int st, int target, T inf = shortest_path_detail::infinity<T>()) {
	return dijkstra_path<false>(g, st, target, inf);
}

#endif
