#ifndef IS_PRIME_HPP
#define IS_PRIME_HPP

#include "../../components/montgomery.hpp"
#include "../../components/montgomery32.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <initializer_list>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

template <class Montgomery, std::size_t Count, class Multiply>
inline bool witnesses(const Montgomery &m, typename Montgomery::U d, int s,
	const std::array<typename Montgomery::U, Count> &bases, Multiply multiply) {
	using V = typename Montgomery::U;
	std::array<V, Count> a, x;
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC unroll 8
#endif
	for (std::size_t i = 0; i < Count; ++i) a[i] = x[i] = m.in(bases[i] % m.mod);
	for (V e = d >> 1; e; e >>= 1) {
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC unroll 8
#endif
		for (std::size_t i = 0; i < Count; ++i) a[i] = multiply(a[i], a[i]);
		if (e & 1) {
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC unroll 8
#endif
			for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], a[i]);
		}
	}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC unroll 8
#endif
	for (std::size_t i = 0; i < Count; ++i) {
		V y = x[i] >= m.mod ? x[i] - m.mod : x[i];
		if (!y && bases[i] % m.mod == 0) continue;
		if (y == m.one || y == m.mod - m.one) continue;
		bool passed = false;
		for (int j = 1; j < s; ++j) {
			y = multiply(y, y);
			if (y >= m.mod) y -= m.mod;
			if (y == m.mod - m.one) { passed = true; break; }
		}
		if (!passed) return false;
	}
	return true;
}

inline bool prime(const Montgomery32 &m) {
	using V = Montgomery32::U;
	V d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto multiply = [&](V a, V b) { return m.mul(a, b); };
	return witnesses(m, d, s, std::array<V, 1>{2}, multiply)
		&& witnesses(m, d, s, std::array<V, 2>{7, 61}, multiply);
}

inline bool prime(const Montgomery64 &m) {
	if (m.mod < (U(1) << 32)) return prime(Montgomery32(m.mod));
	U d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto check = [&](auto multiply) {
		return witnesses(m, d, s, std::array<U, 1>{2}, multiply)
			&& witnesses(m, d, s, std::array<U, 6>{325, 9375, 28178, 450775, 9780504, 1795265022}, multiply);
	};
	if (m.mod < (U(1) << 62)) {
		U inverse = -m.inverse;
		return check([&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U((x + __uint128_t(U(x) * inverse) * m.mod) >> 64);
		});
	}
	return check([&](U a, U b) { return m.mul(a, b); });
}
}

inline bool is_prime(std::uint64_t n) {
	if (n < 2) return false;
	if (n % 2 == 0) return n == 2;
	if (n % 3 == 0) return n == 3;
	if (n % 5 == 0) return n == 5;
	if (n % 7 == 0) return n == 7;
	if (n % 11 == 0) return n == 11;
	if (n % 13 == 0) return n == 13;
	if (n % 17 == 0) return n == 17;
	if (n % 19 == 0) return n == 19;
	if (n % 23 == 0) return n == 23;
	if (n % 29 == 0) return n == 29;
	if (n % 31 == 0) return n == 31;
	if (n % 37 == 0) return n == 37;
	if (n < 1681) return true;
	if (n < (std::uint64_t(1) << 32)) return is_prime_detail::prime(Montgomery32(n));
	return is_prime_detail::prime(Montgomery64(n));
}

#endif
