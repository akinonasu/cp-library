#ifndef FACTORIAL_TABLE_LIGHT_HPP
#define FACTORIAL_TABLE_LIGHT_HPP

#include "../math/mint.hpp"
#include "factorial_table_generic.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace light_factorial_detail {
template<class Mint> struct Table {
	decltype(Mint::mod()) modulus;
	std::vector<Mint> factorial, inverse_factorial;

	explicit Table(int max_n) : modulus(Mint::mod()) {
		assert(0 <= max_n && max_n < std::numeric_limits<int>::max() && modulus > 1);
		int n = static_cast<int>(std::min(std::uint64_t(max_n), std::uint64_t(modulus) - 1));
		factorial.resize(n + 1), inverse_factorial.resize(n + 1);
		factorial_table_detail::build<Mint, !std::is_same_v<Mint, mint>>(factorial.data(), inverse_factorial.data(), n);
	}
};
}

#endif
