#ifndef BELLMAN_FORD_HPP
#define BELLMAN_FORD_HPP

#include "../../components/shortest_path_scc.hpp"
#include "../../components/directed_csr.hpp"
#include "../../components/RadixHeap.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<class Distance>
struct BellmanFordResult {
	std::vector<Distance> distance;
	std::vector<int> parent, negative_cycle;
	std::vector<std::size_t> parent_edge, negative_cycle_edges;
	Distance infinity;
	int source;
	bool has_negative_cycle() const { return !negative_cycle.empty(); }
	bool reachable(int v) const {
		assert(!has_negative_cycle() && 0 <= v && std::size_t(v) < distance.size());
		return distance[v] != infinity;
	}
	std::vector<int> path(int v) const {
		if (!reachable(v)) return {};
		std::vector<int> result;
		for (; v != source; v = parent[v]) result.push_back(v);
		result.push_back(source);
		std::reverse(result.begin(), result.end());
		return result;
	}
	std::vector<std::size_t> path_edges(int v) const {
		if (!reachable(v)) return {};
		std::vector<std::size_t> result;
		for (; v != source; v = parent[v]) result.push_back(parent_edge[v]);
		std::reverse(result.begin(), result.end());
		return result;
	}
};

namespace negative_path_detail {
template<class Distance, class Weight>
Distance add(Distance a, Weight b) {
	static_assert(std::numeric_limits<Distance>::is_integer && std::numeric_limits<Distance>::is_signed);
	static_assert(std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_signed);
	static_assert(std::numeric_limits<Distance>::digits >= std::numeric_limits<Weight>::digits);
	Distance value;
#if defined(__GNUC__) || defined(__clang__)
	if (__builtin_add_overflow(a, Distance(b), &value)) throw std::overflow_error("Bellman-Ford distance");
#else
	Distance w = b;
	if ((w > 0 && a > std::numeric_limits<Distance>::max() - w) ||
		(w < 0 && a < std::numeric_limits<Distance>::lowest() - w)) throw std::overflow_error("Bellman-Ford distance");
	value = a + w;
#endif
	if (value == std::numeric_limits<Distance>::max()) throw std::overflow_error("Bellman-Ford infinity");
	return value;
}

template<bool Decompose, int Passes, class Distance, class Graph>
BellmanFordResult<Distance> solve(const Graph &g, int source) {
	assert(std::size_t(g.size()) <= std::size_t(std::numeric_limits<int>::max()));
	int n = static_cast<int>(g.size());
	assert(0 <= source && source < n);
	Distance inf = std::numeric_limits<Distance>::max();
	BellmanFordResult<Distance> result{std::vector<Distance>(n, inf), std::vector<int>(n, -1), {},
		std::vector<std::size_t>(n, std::size_t(-1)), {}, inf, source};
	result.distance[source] = 0;
	result.parent[source] = source;
	auto &d = result.distance;
	std::vector<int> active, order;
	auto relax = [&](int v, std::size_t i) {
		const auto &[to, w] = g[v][i];
		Distance next = add(d[v], w);
		if (next >= d[to]) return false;
		d[to] = next;
		result.parent[to] = v;
		result.parent_edge[to] = i;
		return true;
	};
	for (int pass = 0; pass < Passes; ++pass) {
		bool changed = false;
		for (int v = 0; v < n; ++v) if (d[v] != inf)
			for (std::size_t i = 0; i < g[v].size(); ++i) changed |= relax(v, i);
		if (!changed) return result;
	}
	std::vector<int> position(n, -1);
	std::vector<unsigned char> labeled(n);
	Components components;
	if constexpr (Decompose) components = reachable_components(g, source);
	else {
		components.id.assign(n, 0);
		components.vertices.resize(n);
		for (int v = 0; v < n; ++v) components.vertices[v] = v;
		components.offset.push_back(n);
	}
	struct Frame { int vertex; std::size_t next, incoming; };
	std::vector<Frame> stack;
	using State = std::pair<Distance, int>;
	constexpr bool radix = std::is_integral_v<Distance> && sizeof(Distance) <= 8;
	std::conditional_t<radix, RadixHeap<Distance>, std::vector<State>> heap;
	for (int c = static_cast<int>(components.offset.size()) - 2; c >= 0; --c) {
		auto begin = components.offset[c], end = components.offset[c + 1];
		auto internal = [&](int to) { return components.id[to] == c; };
		bool negative = !Decompose;
		Distance minimum = inf;
		for (auto j = begin; j < end; ++j) {
			int v = components.vertices[j];
			minimum = std::min(minimum, d[v]);
			if constexpr (Decompose)
				for (const auto &[to, w] : g[v]) if (internal(to) && w < 0) negative = true;
		}
		if (minimum == inf) continue;
		if (negative) {
			active.clear();
			for (auto j = begin; j < end; ++j) {
				int v = components.vertices[j];
				if (d[v] != inf) active.push_back(v);
			}
			while (!active.empty()) {
				order.clear();
				for (int v : active) labeled[v] = 0;
				for (int root : active) {
					if (position[root] != -1) continue;
					bool useful = false;
					for (const auto &[to, w] : g[root])
						if (internal(to) && add(d[root], w) < d[to]) { useful = true; break; }
					if (!useful) continue;
					position[root] = 0;
					stack.push_back({root, 0, std::size_t(-1)});
					while (!stack.empty()) {
						int v = stack.back().vertex;
						if (stack.back().next == g[v].size()) {
							position[v] = -2;
							order.push_back(v);
							stack.pop_back();
							continue;
						}
						std::size_t i = stack.back().next++;
						int to = g[v][i].first;
						if (!internal(to) || !relax(v, i)) continue;
						if (position[to] >= 0) {
							for (auto j = std::size_t(position[to]); j < stack.size(); ++j) {
								result.negative_cycle.push_back(stack[j].vertex);
								if (j + 1 < stack.size()) result.negative_cycle_edges.push_back(stack[j + 1].incoming);
							}
							result.negative_cycle.push_back(to);
							result.negative_cycle_edges.push_back(i);
							return result;
						}
						if (position[to] == -1) {
							position[to] = static_cast<int>(stack.size());
							stack.push_back({to, 0, i});
						}
					}
				}
				active.clear();
				for (auto it = order.rbegin(); it != order.rend(); ++it) {
					int v = *it;
					for (std::size_t i = 0; i < g[v].size(); ++i) {
						int to = g[v][i].first;
						if (internal(to) && relax(v, i) && !labeled[to]) active.push_back(to), labeled[to] = 1;
					}
				}
				for (int v : order) position[v] = -1;
			}
		} else if (end - begin > 1) {
			auto push = [&](Distance cost, int v) {
				if constexpr (radix) heap.push({cost, v});
				else { heap.emplace_back(cost, v); std::push_heap(heap.begin(), heap.end(), std::greater<State>()); }
			};
			if constexpr (radix) heap.clear(minimum);
			for (auto j = begin; j < end; ++j) {
				int v = components.vertices[j];
				if (d[v] != inf) push(d[v], v);
			}
			while (!heap.empty()) {
				auto [cost, v] = [&] {
					if constexpr (radix) return heap.pop_value();
					else { std::pop_heap(heap.begin(), heap.end(), std::greater<State>()); auto state = heap.back(); heap.pop_back(); return state; }
				}();
				if (cost != d[v]) continue;
				for (std::size_t i = 0; i < g[v].size(); ++i) {
					int to = g[v][i].first;
					if (internal(to) && relax(v, i)) push(d[to], to);
				}
			}
		}
		if constexpr (Decompose)
			for (auto j = begin; j < end; ++j) {
				int v = components.vertices[j];
				for (std::size_t i = 0; i < g[v].size(); ++i) if (!internal(g[v][i].first)) relax(v, i);
			}
	}
	return result;
}
}

template<class Distance = std::int64_t, class Graph>
BellmanFordResult<Distance> bellman_ford(const Graph &g, int source) {
	return negative_path_detail::solve<true, 2, Distance>(g, source);
}

template<class Distance = std::int64_t, class Graph>
BellmanFordResult<Distance> goldberg_radzik(const Graph &g, int source) {
	return negative_path_detail::solve<false, 0, Distance>(g, source);
}

#endif
