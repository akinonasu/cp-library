#ifndef BINOMIAL_PREFIX_CORE_HPP
#define BINOMIAL_PREFIX_CORE_HPP

#include "../math/mint.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace binomial_prefix_detail {
template<class Mint> struct Table {
	std::vector<Mint> factorial, inverse_factorial, power2;
	Mint half;
	explicit Table(int n = 0) { build(n); }
	void build(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max() && std::uint64_t(n) < Mint::mod() && Mint::mod() > 1);
		factorial.resize(n + 1), inverse_factorial.resize(n + 1), power2.resize(n + 1);
		factorial[0] = power2[0] = Mint(1);
		for (int i = 1; i <= n; ++i) factorial[i] = factorial[i - 1] * Mint(i), power2[i] = power2[i - 1] + power2[i - 1];
		inverse_factorial[n] = factorial[n].inv();
		for (int i = n; i > 0; --i) inverse_factorial[i - 1] = inverse_factorial[i] * Mint(i);
		half = Mint((std::uint64_t(Mint::mod()) + 1) / 2);
	}
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
		if constexpr (std::is_same_v<Mint, fps_mint>) {
			__uint128_t sum = 0;
			for (int i = l; i < r; ++i) sum += std::uint64_t(inverse_factorial[i].v) * inverse_factorial[n - i].v;
			return factorial[n] * Mint::word(Mint::reduce(std::uint64_t(sum % Mint::mod())));
		} else {
			Mint sum(0);
			for (int i = l; i < r; ++i) sum += inverse_factorial[i] * inverse_factorial[n - i];
			return factorial[n] * sum;
		}
	}
};
}

#endif
