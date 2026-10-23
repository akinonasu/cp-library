#ifndef FACTORIAL_TABLE_HPP
#define FACTORIAL_TABLE_HPP

#include "mint_factorials.hpp"
#include "barrett32.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace factorial_detail {
template<class Mint> struct Table {
	decltype(Mint::mod()) modulus;
	std::vector<Mint> factorial, inverse_factorial;
	explicit Table(int max_n) : modulus(Mint::mod()) {
		assert(0 <= max_n && max_n < std::numeric_limits<int>::max() && modulus > 1);
		int n = static_cast<int>(std::min(std::uint64_t(max_n), std::uint64_t(modulus) - 1));
		factorial.resize(n + 1), inverse_factorial.resize(n + 1);
		if constexpr (std::is_same_v<Mint, mint>) {
			mint_detail::factorials(factorial.data(), inverse_factorial.data(), n + 1);
		} else {
			auto build = [&] {
				factorial[0] = Mint(1);
				for (int i = 1; i <= n; ++i) factorial[i] = factorial[i - 1] * Mint(i);
				inverse_factorial[n] = factorial[n].inv();
				for (int i = n; i > 0; --i) inverse_factorial[i - 1] = inverse_factorial[i] * Mint(i);
			};
			if constexpr (requires(const Mint &x) { { x.val() } -> std::integral; }) {
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
				} else build();
			} else build();
		}
	}
};
}

#endif
