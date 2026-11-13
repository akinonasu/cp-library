#ifndef FACTORIAL_TABLE_HPP
#define FACTORIAL_TABLE_HPP

#include "mint_factorials.hpp"
#include "factorial_table_generic.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace factorial_detail {
template<class Mint, bool WithInverse = false> struct Table {
	decltype(Mint::mod()) modulus;
	std::vector<Mint> factorial, inverse_factorial, inverse;
	explicit Table(int max_n) : modulus(Mint::mod()) {
		assert(0 <= max_n && max_n < std::numeric_limits<int>::max() && modulus > 1);
		int n = static_cast<int>(std::min(std::uint64_t(max_n), std::uint64_t(modulus) - 1));
		factorial.resize(n + 1), inverse_factorial.resize(n + 1);
		if constexpr (std::is_same_v<Mint, mint>) {
			mint_detail::factorials(factorial.data(), inverse_factorial.data(), n + 1);
		} else factorial_table_detail::build(factorial.data(), inverse_factorial.data(), n);
		if constexpr (WithInverse) {
			inverse.resize(n + 1);
			factorial_table_detail::inverses(inverse.data(), factorial.data(), inverse_factorial.data(), n);
		}
	}
};
}

#endif
