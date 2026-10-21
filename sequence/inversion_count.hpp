#ifndef INVERSION_COUNT_HPP
#define INVERSION_COUNT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <vector>

template<class Range, class Compare = std::less<>>
	requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
std::int64_t inversion_count(const Range &a, Compare compare = {}) {
	assert(std::ranges::size(a) <= std::size_t(std::numeric_limits<int>::max()));
	const std::size_t n = std::ranges::size(a);
	auto first = std::ranges::begin(a);
	std::size_t ordered = 1;
	while (ordered < n && !compare(first[ordered], first[ordered - 1])) ++ordered;
	if (ordered >= n) return 0;
	using T = std::ranges::range_value_t<const Range>;
	constexpr bool cached = std::is_trivially_copyable_v<T> && sizeof(T) <= 16 &&
		std::is_default_constructible_v<T> && std::is_copy_constructible_v<T> &&
		std::is_copy_assignable_v<T> && !std::is_same_v<T, bool>;
	using Stored = std::conditional_t<cached, T, int>;
	std::vector<Stored> current(n), buffer(n);
	for (std::size_t i = 0; i < n; ++i) {
		if constexpr (cached) current[i] = first[i];
		else current[i] = static_cast<int>(i);
	}
	auto value = [&](const Stored &x) -> decltype(auto) {
		if constexpr (cached) return (x);
		else return first[x];
	};
	std::int64_t result = 0;
	for (std::size_t width = 1; width < n; width *= 2) {
		for (std::size_t l = 0; l < n; l += 2 * width) {
			const auto mid = std::min(l + width, n), r = std::min(l + 2 * width, n);
			if (mid == r || !compare(value(current[mid]), value(current[mid - 1]))) {
				std::copy(current.begin() + l, current.begin() + r, buffer.begin() + l);
				continue;
			}
			std::size_t i = l, j = mid, k = l;
			while (i < mid && j < r) {
				if (compare(value(current[j]), value(current[i]))) {
					buffer[k++] = current[j++];
					result += static_cast<std::int64_t>(mid - i);
				} else buffer[k++] = current[i++];
			}
			std::copy(current.begin() + i, current.begin() + mid, buffer.begin() + k);
			std::copy(current.begin() + j, current.begin() + r, buffer.begin() + (k + mid - i));
		}
		current.swap(buffer);
	}
	return result;
}

#endif
