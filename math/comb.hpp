#ifndef COMB_HPP
#define COMB_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>

#include "mint.hpp"
#include "../components/factorial_table.hpp"

#ifndef COMB_MAX
#define COMB_MAX 10000000
#endif

#ifndef CP_COMB_API_HPP
#define CP_COMB_API_HPP 1
constexpr int COMB_INF = static_cast<int>(COMB_MAX);
static_assert(COMB_INF >= 0 && COMB_INF < mint::mod());
namespace comb_detail {
inline mint storage[std::size_t(COMB_INF + 1) * 3];
inline constexpr mint *factorial = storage, *inverse_factorial = factorial + COMB_INF + 1, *inverse = inverse_factorial + COMB_INF + 1;
struct Initializer {
	Initializer() {
		[[maybe_unused]] static const bool initialized = [] {
			mint_detail::factorials(factorial, inverse_factorial, COMB_INF + 1);
			factorial_table_detail::inverses(inverse, factorial, inverse_factorial, COMB_INF);
			return true;
		}();
	}
};
[[maybe_unused]] static const Initializer initializer;

template<class Mint>
const factorial_detail::Table<Mint, true> &table() {
	static factorial_detail::Table<Mint, true> value(COMB_INF);
	if (value.modulus != Mint::mod()) value = factorial_detail::Table<Mint, true>(COMB_INF);
	return value;
}
template<class Mint, int Kind> struct Factorial {
	Mint operator[](int i) const {
		if (i < 0 || (Kind == 2 && i == 0)) return Mint(0);
		assert(i <= COMB_INF);
		if constexpr (std::is_same_v<Mint, mint>) {
			if constexpr (Kind == 2) return inverse[i];
			else return Kind == 1 ? inverse_factorial[i] : factorial[i];
		} else {
			const auto &value = table<Mint>();
			assert(std::size_t(i) < value.factorial.size());
			if constexpr (Kind == 2) return value.inverse[i];
			else if constexpr (Kind == 1) return value.inverse_factorial[i];
			else return value.factorial[i];
		}
	}
};
}
inline mint comb(int n, int k) {
	if (n < 0 || k < 0 || k > n) return 0;
	assert(n <= COMB_INF);
	return comb_detail::factorial[n] * comb_detail::inverse_factorial[k] * comb_detail::inverse_factorial[n - k];
}
template<class Mint>
Mint comb(int n, int k) {
	if constexpr (std::is_same_v<Mint, mint>) return comb(n, k);
	else {
		if (n < 0 || k < 0 || k > n) return Mint(0);
		assert(n <= COMB_INF);
		const auto &table = comb_detail::table<Mint>();
		assert(std::size_t(n) < table.factorial.size());
		return table.factorial[n] * table.inverse_factorial[k] * table.inverse_factorial[n - k];
	}
}

template<class Mint = mint> struct Comb {
	inline static constexpr comb_detail::Factorial<Mint, 0> fact{};
	inline static constexpr comb_detail::Factorial<Mint, 1> finv{};
	inline static constexpr comb_detail::Factorial<Mint, 2> inv{};
	static Mint comb(int n, int k) { return ::comb<Mint>(n, k); }
};
inline constexpr auto &fact = Comb<>::fact;
inline constexpr auto &finv = Comb<>::finv;
inline constexpr auto &inv = Comb<>::inv;

#endif

#endif
