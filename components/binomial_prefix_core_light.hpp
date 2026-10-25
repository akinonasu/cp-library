#ifndef BINOMIAL_PREFIX_CORE_LIGHT_HPP
#define BINOMIAL_PREFIX_CORE_LIGHT_HPP

#include "factorial_table_light.hpp"
#include <type_traits>

namespace light_binomial_prefix_detail {
template<class Mint> struct Table : light_factorial_detail::Table<Mint> {
	using Base = light_factorial_detail::Table<Mint>;
	using Base::factorial, Base::inverse_factorial;
	std::vector<Mint> power2;
	Mint half;

	explicit Table(int n = 0) : Base(n) {
		assert(std::uint64_t(n) < Mint::mod());
		power2.resize(n + 1);
		power2[0] = Mint(1);
		for (int i = 1; i <= n; ++i) power2[i] = power2[i - 1] + power2[i - 1];
		half = Mint((std::uint64_t(Mint::mod()) + 1) / 2);
	}
	void build(int n) { *this = Table(n); }
	Mint binom(int n, int k) const {
		if (k < 0 || k > n) return Mint(0);
		return factorial[n] * inverse_factorial[k] * inverse_factorial[n - k];
	}
	Mint center(int n) const {
		if (!n) return Mint(0);
		return n & 1 ? power2[n - 1] : (power2[n] - binom(n, n / 2)) * half;
	}
	Mint range(int n, int l, int r) const {
		assert(0 <= l && l <= r && r <= n + 1);
		Mint sum(0);
		for (int i = l; i < r; ++i) sum += inverse_factorial[i] * inverse_factorial[n - i];
		return factorial[n] * sum;
	}
};
}

#endif
