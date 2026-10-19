#ifndef LIS_HPP
#define LIS_HPP

#include "../components/binary_search.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace lis_detail {
template<class Range>
concept Sequence = std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>;

template<bool Strict, bool Record, Sequence Range, class Compare>
int scan(const Range &a, Compare &compare, int *lengths) {
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	int n = static_cast<int>(std::ranges::size(a));
	using T = std::ranges::range_value_t<const Range>;
	constexpr bool cached = std::is_trivially_copyable_v<T> && sizeof(T) <= 16 &&
		std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && !std::is_same_v<T, bool>;
	using Stored = std::conditional_t<cached, T, int>;
	std::vector<Stored> tails;
	if constexpr (Record) tails.reserve(n);
	auto first = std::ranges::begin(a);
	auto tail = [&](int p) -> decltype(auto) {
		if constexpr (cached) return (tails[p]);
		else return first[tails[p]];
	};
	auto before = [&](const auto &x, const auto &y) {
		if constexpr (Strict) return bool(compare(x, y));
		else return !compare(y, x);
	};
	for (int i = 0; i < n; ++i) {
		auto &&value = first[i];
		int p = static_cast<int>(tails.size());
		if (p && !before(tail(p - 1), value))
			p = binary_search_detail::partition_point(p - 1, [&](int k) { return before(tail(k), value); });
		if constexpr (cached) {
			if (p == static_cast<int>(tails.size())) tails.emplace_back(value);
			else tails[p] = value;
		} else {
			if (p == static_cast<int>(tails.size())) tails.push_back(i);
			else tails[p] = i;
		}
		if constexpr (Record) lengths[i] = p + 1;
	}
	return static_cast<int>(tails.size());
}
}

template<bool Strict = true, lis_detail::Sequence Range, class Compare = std::less<>>
int lis_length(const Range &a, Compare compare = {}) {
	return lis_detail::scan<Strict, false>(a, compare, nullptr);
}

template<bool Strict = true, lis_detail::Sequence Range, class Compare = std::less<>>
std::vector<int> lis_lengths(const Range &a, Compare compare = {}) {
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> lengths(std::ranges::size(a));
	lis_detail::scan<Strict, true>(a, compare, lengths.data());
	return lengths;
}

template<bool Strict = true, lis_detail::Sequence Range, class Compare = std::less<>>
std::vector<int> lis(const Range &a, Compare compare = {}) {
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> lengths(std::ranges::size(a));
	int remaining = lis_detail::scan<Strict, true>(a, compare, lengths.data());
	std::vector<int> result(remaining);
	for (int i = static_cast<int>(lengths.size()); remaining && i--;) {
		if (lengths[i] == remaining) result[--remaining] = i;
	}
	return result;
}

#endif
