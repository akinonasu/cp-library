#ifndef DIJKSTRA_UNIQUE_HPP
#define DIJKSTRA_UNIQUE_HPP

#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace shortest_path_detail {
template<class G, class T>
std::pair<T, std::vector<int>> unique_path(const G &g, int st, int target, T inf, std::vector<int> &parent) {
	using State = std::pair<T, int>;
	std::vector<State> pending;
	pending.emplace_back(T(0), st);
	while (!pending.empty()) {
		auto [cost, v] = std::move(pending.back());
		pending.pop_back();
		for (const auto &first : g[v]) {
			const auto *edge = &first;
			int from = v;
			T prefix = cost;
			for (;;) {
				const auto &[to, weight] = *edge;
				assert(weight >= 0);
				if (to == st) break;
				using Weight = std::remove_cvref_t<decltype(weight)>;
				T allowance = inf - prefix;
				if constexpr (std::numeric_limits<Weight>::is_integer && std::numeric_limits<Weight>::is_bounded &&
							  std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_bounded) {
					using Compare = std::common_type_t<Weight, T>;
					if (Compare(weight) >= Compare(allowance)) break;
				} else if (weight >= allowance) break;
				T next = prefix + weight;
				if (to == target) {
					std::vector<int> path{to};
					for (int u = from; u != st; u = parent[u]) path.push_back(u);
					path.push_back(st);
					std::reverse(path.begin(), path.end());
					return {std::move(next), std::move(path)};
				}
				const auto &outgoing = g[to];
				if (outgoing.empty()) break;
				parent[to] = from;
				if (outgoing.size() != 1) {
					pending.emplace_back(std::move(next), to);
					break;
				}
				from = to;
				edge = &outgoing[0];
				prefix = std::move(next);
			}
		}
	}
	return {T(-1), {}};
}
}

#endif
