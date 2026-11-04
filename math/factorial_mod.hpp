#ifndef FACTORIAL_MOD_HPP
#define FACTORIAL_MOD_HPP

#include "mint.hpp"
#include "../components/factorial_mod_table.hpp"
#include <algorithm>
#include <cstdint>

inline mint factorial_mod(std::uint64_t n) {
	if (n >= mint::mod()) return 0;
	constexpr std::uint32_t step = 65536;
	std::uint32_t block = std::uint32_t((n + step / 2) / step), anchor = block * step;
	std::uint32_t first = std::uint32_t(std::min<std::uint64_t>(n, anchor)) + 1;
	std::uint32_t last = std::uint32_t(std::max<std::uint64_t>(n, anchor));
	std::uint64_t a = 1, b = 1, c = 1, d = 1;
	for (; last >= first && last - first >= 3; first += 4) {
		a = a * first % 998244353;
		b = b * (first + 1) % 998244353;
		c = c * (first + 2) % 998244353;
		d = d * (first + 3) % 998244353;
	}
	for (; first <= last; ++first) a = a * first % 998244353;
	mint product = mint::raw(std::uint32_t((a * b % 998244353) * (c * d % 998244353) % 998244353));
	mint result = mint::raw(factorial_mod_detail::table[block]);
	return anchor > n ? result / product : result * product;
}

#endif
