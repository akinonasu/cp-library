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
	std::int64_t equal_pairs = 0;
	std::size_t run = 1, reversed = 1;
	for (; reversed < n && !compare(first[reversed - 1], first[reversed]); ++reversed) {
		if (compare(first[reversed], first[reversed - 1])) run = 1;
		else equal_pairs += static_cast<std::int64_t>(run++);
	}
	if (reversed == n) return static_cast<std::int64_t>(n) * (n - 1) / 2 - equal_pairs;
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
	constexpr std::size_t block = 8;
	for (std::size_t l = 0; l < n; l += block) {
		const auto end = std::min(l + block, n);
		for (std::size_t i = l + 1; i < end; ++i) {
			Stored x = current[i];
			std::size_t j = i;
			while (j > l && compare(value(x), value(current[j - 1]))) current[j] = current[j - 1], --j;
			current[j] = x;
			result += static_cast<std::int64_t>(i - j);
		}
	}
	for (std::size_t width = block; width < n; width *= 2) {
		for (std::size_t l = 0; l < n; l += 2 * width) {
			const auto mid = std::min(l + width, n), r = std::min(l + 2 * width, n);
			if (mid == r || !compare(value(current[mid]), value(current[mid - 1]))) {
				std::copy(current.begin() + l, current.begin() + r, buffer.begin() + l);
				continue;
			}
			if (compare(value(current[r - 1]), value(current[l]))) {
				result += static_cast<std::int64_t>(mid - l) * (r - mid);
				std::copy(current.begin() + mid, current.begin() + r, buffer.begin() + l);
				std::copy(current.begin() + l, current.begin() + mid, buffer.begin() + (l + r - mid));
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
