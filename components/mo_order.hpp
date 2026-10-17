#ifndef MO_ORDER_HPP
#define MO_ORDER_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

inline std::vector<int> mo_order(const std::vector<std::pair<int, int>>& ranges, int block_size = 0) {
	assert(ranges.size() <= std::size_t(std::numeric_limits<int>::max()) && block_size >= 0);
	if (ranges.empty()) return {};
	int min_l = ranges[0].first, max_l = min_l, min_r = ranges[0].second, max_r = min_r;
	for (auto [l, r] : ranges) {
		assert(0 <= l && l <= r);
		min_l = std::min(min_l, l), max_l = std::max(max_l, l);
		min_r = std::min(min_r, r), max_r = std::max(max_r, r);
	}
	if (!block_size) block_size = std::max(1, int(std::sqrt((double(max_l) - min_l + 1) * (double(max_r) - min_r + 1) / ranges.size())));
	std::vector<std::pair<std::uint64_t, int>> keys;
	keys.reserve(ranges.size());
	for (int i = 0; i < int(ranges.size()); ++i) {
		auto [l, r] = ranges[i];
		std::uint32_t block = (l - min_l) / block_size;
		std::uint32_t right = block & 1 ? ~std::uint32_t(r) : std::uint32_t(r);
		keys.emplace_back((std::uint64_t(block) << 32) | right, i);
	}
	std::sort(keys.begin(), keys.end());
	std::vector<int> order;
	order.reserve(keys.size());
	for (auto [key, i] : keys) { (void)key; order.push_back(i); }
	return order;
}

#endif
