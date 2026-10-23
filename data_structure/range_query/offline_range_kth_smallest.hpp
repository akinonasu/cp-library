#ifndef OFFLINE_RANGE_KTH_SMALLEST_HPP
#define OFFLINE_RANGE_KTH_SMALLEST_HPP

#include "../../components/offline_range_kth.hpp"
#include "../../components/wavelet_compression.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <span>
#include <vector>

template <class T>
std::vector<T> offline_range_kth_smallest(const std::vector<T> &values, std::span<const std::array<int, 3>> queries) {
	assert(values.size() <= std::size_t(std::numeric_limits<int>::max()) && queries.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (queries.empty()) return {};
	offline_range_kth_detail::Frontier frontier(queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [l, r, k] = queries[i];
		assert(0 <= l && l < r && std::size_t(r) <= values.size() && 0 <= k && k < r - l);
		frontier.left[i] = unsigned(l); frontier.right[i] = unsigned(r); frontier.rank[i] = unsigned(k);
	}
	std::vector<T> sorted;
	std::vector<int> positions, offsets;
	int ordered = 0;
	auto codes = wavelet_detail::compress<T, false>(values, sorted, ordered, positions, offsets);
	unsigned bits = std::bit_width(unsigned(sorted.size() - 1));
	if (bits && !ordered) offline_range_kth_detail::run(frontier, std::move(codes), bits);
	std::vector<T> result;
	result.reserve(queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		if (ordered) {
			auto [l, r, k] = queries[i];
			result.push_back(values[ordered > 0 ? l + k : r - 1 - k]);
		} else result.push_back(sorted[frontier.answer[i]]);
	}
	return result;
}

#endif
