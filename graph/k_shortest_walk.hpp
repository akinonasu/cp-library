#ifndef K_SHORTEST_WALK_HPP
#define K_SHORTEST_WALK_HPP

#include "../components/dijkstra_queue.hpp"
#include "../components/directed_csr.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace k_shortest_walk_detail {
template<class T> class SidetrackHeap {
  public:
	struct Node { T cost; int to, left = 0, right = 0, rank = 1; };
	std::vector<Node> nodes{{T(0), 0, 0, 0, 0}};
	int make(Node node) {
		if (nodes.size() == std::size_t(std::numeric_limits<int>::max()))
			throw std::length_error("K shortest walk heap");
		int id = int(nodes.size());
		nodes.push_back(std::move(node));
		return id;
	}
	template<bool Persistent> int meld(int a, int b) {
		std::array<int, 64> path;
		int depth = 0;
		while (a && b) {
			if (nodes[b].cost < nodes[a].cost) std::swap(a, b);
			path[depth++] = a;
			a = nodes[a].right;
		}
		int root = a ? a : b;
		while (depth) {
			int id = path[--depth];
			if constexpr (Persistent) id = make(nodes[id]);
			auto &node = nodes[id];
			node.right = root;
			if (nodes[node.left].rank < nodes[root].rank) std::swap(node.left, node.right);
			node.rank = nodes[node.right].rank + 1;
			root = id;
		}
		return root;
	}
};
}

template<class Distance = std::int64_t, class G>
std::vector<Distance> k_shortest_walk(const G &g, int start, int target, int k,
	Distance inf = std::numeric_limits<Distance>::max()) {
	using T = Distance;
	using W = std::remove_cvref_t<decltype(g[0][0].second)>;
	static_assert(std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded &&
		sizeof(T) <= 16 && !std::is_same_v<T, bool>);
	static_assert(std::numeric_limits<W>::is_integer && std::numeric_limits<W>::is_bounded &&
		sizeof(W) <= 16 && !std::is_same_v<W, bool>);
	assert(k >= 0 && inf > 0);
	if (!k) return {};
	assert(std::size_t(g.size()) <= std::size_t(std::numeric_limits<int>::max()));
	int n = int(g.size());
	assert(0 <= start && start < n && 0 <= target && target < n);
	std::size_t m = 0;
	for (int v = 0; v < n; ++v) m += g[v].size();
	DirectedCSR<W> reverse(n, m, [&](auto emit) {
		for (int v = 0; v < n; ++v)
			for (const auto &[to, weight] : g[v]) {
				assert(0 <= to && to < n && weight >= 0);
				emit(typename DirectedCSR<W>::Edge{to, v, weight});
			}
	});
	std::vector<T> distance(n, inf);
	std::vector<int> parent(n, -1), order;
	order.reserve(n);
	distance[target] = 0;
	{
		shortest_path_detail::DistanceQueue<T> queue;
		queue.push(0, target);
		while (!queue.empty()) {
			auto [cost, v] = queue.pop();
			if (cost != distance[v]) continue;
			order.push_back(v);
			for (const auto &[to, weight] : reverse[v]) {
				if (shortest_path_detail::reaches_limit(weight, T(inf - cost))) continue;
				T next = cost + T(weight);
				if (next >= distance[to]) continue;
				distance[to] = next;
				parent[to] = v;
				queue.push(next, to);
			}
		}
	}
	if (distance[start] == inf) return {};
	std::vector<T> answer{distance[start]};
	if (k == 1) return answer;
	k_shortest_walk_detail::SidetrackHeap<T> heap;
	std::vector<int> roots(n), local;
	using Compare = std::common_type_t<T, W>;
	for (int v : order) {
		local.clear();
		bool skipped = false;
		for (const auto &[to, weight] : g[v]) {
			if (distance[to] == inf) continue;
			if (!skipped && to == parent[v] && Compare(weight) == Compare(distance[v] - distance[to])) {
				skipped = true;
				continue;
			}
			T extra;
			if (distance[to] >= distance[v]) {
				extra = distance[to] - distance[v];
				if (shortest_path_detail::reaches_limit(weight, T(inf - extra))) continue;
				extra += T(weight);
			} else {
				Compare difference = Compare(weight) - Compare(distance[v] - distance[to]);
				if (shortest_path_detail::reaches_limit(difference, inf)) continue;
				extra = T(difference);
			}
			local.push_back(heap.make({extra, to}));
		}
		for (std::size_t size = local.size(); size > 1;) {
			std::size_t next = 0, i = 0;
			for (; i + 1 < size; i += 2) local[next++] = heap.template meld<false>(local[i], local[i + 1]);
			if (i < size) local[next++] = local[i];
			size = next;
		}
		int inherited = parent[v] == -1 ? 0 : roots[parent[v]];
		roots[v] = heap.template meld<true>(local.empty() ? 0 : local[0], inherited);
	}
	shortest_path_detail::DistanceQueue<T> queue;
	auto push = [&](T cost, int id) {
		if (id && heap.nodes[id].cost < inf - cost) queue.push(cost + heap.nodes[id].cost, id);
	};
	push(distance[start], roots[start]);
	while (int(answer.size()) < k && !queue.empty()) {
		auto [cost, id] = queue.pop();
		answer.push_back(cost);
		if (int(answer.size()) == k) break;
		const auto &node = heap.nodes[id];
		push(cost, roots[node.to]);
		push(cost - node.cost, node.left);
		push(cost - node.cost, node.right);
	}
	return answer;
}

#endif
