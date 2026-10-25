#ifndef COMB_MOD_HPP
#define COMB_MOD_HPP

#include "../modint.hpp"
#include "../../components/factorial_table_generic.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

#ifndef COMB_MOD
#define COMB_MOD 1000000007
#endif
#ifndef COMB_MAX
#define COMB_MAX 10000000
#endif

using comb_mint = StaticModint<COMB_MOD>;
static_assert(COMB_MOD > 1 && COMB_MAX >= 0 && COMB_MAX < std::numeric_limits<int>::max());

namespace comb_mod_detail {
struct Table {
	std::vector<comb_mint> fact, finv;
	Table() {
		const int n = int(std::min(std::uint64_t(COMB_MAX), std::uint64_t(COMB_MOD) - 1));
		fact.resize(n + 1);
		finv.resize(n + 1);
		factorial_table_detail::build(fact.data(), finv.data(), n);
	}
};
inline const Table &table() { static const Table value; return value; }
template <bool Inverse> struct Factorial {
	comb_mint operator[](int i) const {
		if (i < 0) return 0;
		assert(std::uint64_t(i) <= std::uint64_t(COMB_MAX) && std::uint64_t(i) < COMB_MOD);
		if constexpr (Inverse) return table().finv[i];
		else return table().fact[i];
	}
};
}

inline comb_mint comb(int n, int k) {
	if (n < 0 || k < 0 || k > n) return 0;
	assert(std::uint64_t(n) <= std::uint64_t(COMB_MAX) && std::uint64_t(n) < COMB_MOD);
	const auto &t = comb_mod_detail::table();
	return t.fact[n] * t.finv[k] * t.finv[n - k];
}
inline constexpr comb_mod_detail::Factorial<false> fact;
inline constexpr comb_mod_detail::Factorial<true> finv;

#endif
