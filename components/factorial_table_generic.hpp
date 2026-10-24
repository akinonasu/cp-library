#ifndef FACTORIAL_TABLE_GENERIC_HPP
#define FACTORIAL_TABLE_GENERIC_HPP

#include "barrett32.hpp"
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace factorial_table_detail {
template<class Mint, bool Canonical = true> void build(Mint *factorial, Mint *inverse_factorial, int n) {
	if constexpr (Canonical && requires(const Mint &x) { { x.val() } -> std::integral; }) {
		const auto modulus = Mint::mod();
		if (std::uint64_t(modulus) <= std::numeric_limits<std::uint32_t>::max()) {
			Barrett32 arithmetic(modulus);
			auto make = [](std::uint32_t value) { return Mint(value); };
			std::uint32_t value = 1;
			factorial[0] = make(value);
			for (int i = 1; i <= n; ++i) {
				value = arithmetic.mul(value, i);
				factorial[i] = make(value);
			}
			inverse_factorial[n] = factorial[n].inv();
			if constexpr (std::is_signed_v<decltype(inverse_factorial[n].val())>) {
				std::int64_t residue = inverse_factorial[n].val() % std::int64_t(modulus);
				value = residue < 0 ? residue + modulus : residue;
			} else value = arithmetic.reduce(inverse_factorial[n].val());
			for (int i = n; i > 0; --i) {
				value = arithmetic.mul(value, i);
				inverse_factorial[i - 1] = make(value);
			}
			return;
		}
	}
	factorial[0] = Mint(1);
	for (int i = 1; i <= n; ++i) factorial[i] = factorial[i - 1] * Mint(i);
	inverse_factorial[n] = factorial[n].inv();
	for (int i = n; i > 0; --i) inverse_factorial[i - 1] = inverse_factorial[i] * Mint(i);
}
}

#endif
