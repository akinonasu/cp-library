#ifndef MONGE_D_EDGE_SHORTEST_PATH_HPP
#define MONGE_D_EDGE_SHORTEST_PATH_HPP

#include "../components/online_monge_minima.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <class Distance = std::int64_t, class Weight>
std::optional<Distance> monge_d_edge_shortest_path(int n, int d, Weight weight) {
	using T = std::remove_cvref_t<std::invoke_result_t<Weight &, int, int>>;
	static_assert(std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_signed && sizeof(T) <= 8);
	static_assert(std::numeric_limits<Distance>::is_integer && std::numeric_limits<Distance>::is_signed && sizeof(Distance) <= 16);
	using Wide = __int128_t;
	assert(n >= 0 && d >= 0);
	if (!n || d >= n || (!d && n > 1)) return std::nullopt;
	if (n == 1) return Distance(0);
	auto result = [](Wide value) -> std::optional<Distance> {
		if (value < Wide(std::numeric_limits<Distance>::lowest()) || value > Wide(std::numeric_limits<Distance>::max()))
			throw std::overflow_error("Monge shortest path distance");
		return Distance(value);
	};
	Wide single = weight(0, n - 1);
	if (d == 1) return result(single);
	std::vector<Wide> adjacent(n - 1);
	Wide full = 0;
	for (int i = 0; i < n - 1; ++i) full += adjacent[i] = weight(i, i + 1);
	if (d == n - 1) return result(full);
	Wide two = Wide(weight(0, 1)) + Wide(weight(1, n - 1));
	Wide removed = Wide(weight(0, 2)) - adjacent[0] - adjacent[1];
	for (int i = 2; i < n - 1; ++i) {
		two = std::min(two, Wide(weight(0, i)) + Wide(weight(i, n - 1)));
		removed = std::min(removed, Wide(weight(i - 1, i + 1)) - adjacent[i - 1] - adjacent[i]);
	}
	if (d == 2) return result(two);
	if (d == n - 2) return result(full + removed);
	Wide low = removed, high = single - two;
	assert(low <= high);
	std::vector<Wide> distance(n);
	std::vector<int> edges(n);
	auto solve = [&](Wide penalty) {
		online_monge_minima(n, [&](int row, int old_col, int new_col) {
			Wide a = distance[old_col] + Wide(weight(old_col, row));
			Wide b = distance[new_col] + Wide(weight(new_col, row));
			return b < a || (b == a && edges[new_col] < edges[old_col]);
		}, [&](int row, int column) {
			distance[row] = distance[column] + Wide(weight(column, row)) + penalty;
			edges[row] = edges[column] + 1;
		});
	};
	Wide best = single + high;
	while (low < high) {
		Wide mid = low + (high - low) / 2;
		solve(mid);
		if (edges[n - 1] == d) return result(distance[n - 1] - mid * d);
		if (edges[n - 1] < d) high = mid, best = distance[n - 1];
		else low = mid + 1;
	}
	return result(best - high * d);
}

#endif
