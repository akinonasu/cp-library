#ifndef BITWISE_ZETA_HPP
#define BITWISE_ZETA_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace bitwise_zeta_detail {
template <bool inverse, bool superset, class T>
void stages(std::vector<T> &a, std::size_t start, std::size_t n, std::size_t half) {
	auto step = [&](std::size_t low, std::size_t high) {
		T &to = a[superset ? low : high];
		const T &from = a[superset ? high : low];
		if constexpr (inverse) to -= from;
		else to += from;
	};
	for (; half < n / 2; half *= 4)
		for (std::size_t block = start; block < start + n; block += 4 * half)
			for (std::size_t j = block; j < block + half; ++j) {
				step(j, j + half);
				step(j + 2 * half, j + 3 * half);
				step(j, j + 2 * half);
				step(j + half, j + 3 * half);
			}
	if (half < n)
		for (std::size_t j = start; j < start + half; ++j) step(j, j + half);
}
}

template <bool inverse = false, bool superset = false, class T>
void bitwise_zeta(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	constexpr std::size_t block = 1024;
	if (a.size() <= block) bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), 1);
	else {
		for (std::size_t start = 0; start < a.size(); start += block)
			bitwise_zeta_detail::stages<inverse, superset>(a, start, block, 1);
		bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), block);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
