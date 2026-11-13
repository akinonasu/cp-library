#ifndef COMB_LIGHT_HPP
#define COMB_LIGHT_HPP

#include "../components/factorial_table_light.hpp"

#ifndef COMB_MAX
#define COMB_MAX 10000000
#endif

namespace light_combinatorics {
inline constexpr int COMB_INF = static_cast<int>(COMB_MAX);
static_assert(COMB_INF >= 0 && COMB_INF < mint::mod());
namespace detail {
template<class Mint>
const light_factorial_detail::Table<Mint, true>& table() {
	static light_factorial_detail::Table<Mint, true> value(COMB_INF);
	if (value.modulus != Mint::mod()) value = light_factorial_detail::Table<Mint, true>(COMB_INF);
	return value;
}
template<class Mint, int Kind> struct Factorial {
	Mint operator[](int i) const {
		if (i < 0 || (Kind == 2 && i == 0)) return Mint(0);
		assert(i <= COMB_INF);
		const auto& value = table<Mint>();
		assert(std::size_t(i) < value.factorial.size());
		if constexpr (Kind == 2) return value.inverse[i];
		else if constexpr (Kind == 1) return value.inverse_factorial[i];
		else return value.factorial[i];
	}
};
}

template<class Mint = mint>
Mint comb(int n, int k) {
	if (n < 0 || k < 0 || k > n) return Mint(0);
	assert(n <= COMB_INF);
	const auto& value = detail::table<Mint>();
	assert(std::size_t(n) < value.factorial.size());
	return value.factorial[n] * value.inverse_factorial[k] * value.inverse_factorial[n - k];
}

template<class Mint = mint> struct Comb {
	inline static constexpr detail::Factorial<Mint, 0> fact{};
	inline static constexpr detail::Factorial<Mint, 1> finv{};
	inline static constexpr detail::Factorial<Mint, 2> inv{};
	static Mint comb(int n, int k) { return light_combinatorics::comb<Mint>(n, k); }
};
inline constexpr auto& fact = Comb<>::fact;
inline constexpr auto& finv = Comb<>::finv;
inline constexpr auto& inv = Comb<>::inv;
}

#ifndef CP_COMB_API_HPP
#define CP_COMB_API_HPP 1
using light_combinatorics::COMB_INF;
using light_combinatorics::comb;
using light_combinatorics::Comb;
using light_combinatorics::fact;
using light_combinatorics::finv;
using light_combinatorics::inv;
#endif

#endif
