#ifndef IS_PRIME_HPP
#define IS_PRIME_HPP

#include "../components/montgomery.hpp"
#include "../components/montgomery32.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

template <class Montgomery, class Multiply>
inline bool witness(const Montgomery &m, typename Montgomery::U x, int s, Multiply multiply) {
	if (x >= m.mod) x -= m.mod;
	if (x == m.one || x == m.mod - m.one) return true;
	for (int j = 1; j < s; ++j) {
		x = multiply(x, x);
		if (x >= m.mod) x -= m.mod;
		if (x == m.mod - m.one) return true;
	}
	return false;
}

template <class Multiply>
inline bool witness2(const Montgomery64 &m, U d, int s, Multiply multiply) {
	int shift = std::max(0, int(std::bit_width(d)) - 6);
	U x = m.in(U(1) << (d >> shift));
	const bool lazy = m.mod < (U(1) << 62);
	const U twice = m.mod + m.mod;
	for (int bit = shift - 1; bit >= 0; --bit) {
		x = multiply(x, x);
		if ((d >> bit) & 1) {
			if (lazy) {
				x += x;
				if (x >= twice) x -= twice;
			} else x = m.add(x, x);
		}
	}
	return witness(m, x, s, multiply);
}

template <class Montgomery, std::size_t Count, class Multiply>
inline bool witnesses(const Montgomery &m, typename Montgomery::U d, int s,
	const std::array<typename Montgomery::U, Count> &bases, Multiply multiply) {
	using V = typename Montgomery::U;
	std::array<V, Count> x;
	if (Count <= 2 || d < 4096) {
		std::array<V, Count> a;
#pragma GCC unroll 8
		for (std::size_t i = 0; i < Count; ++i) a[i] = x[i] = m.in(bases[i] % m.mod);
		for (V e = d >> 1; e; e >>= 1) {
#pragma GCC unroll 8
			for (std::size_t i = 0; i < Count; ++i) a[i] = multiply(a[i], a[i]);
			if (e & 1) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], a[i]);
			}
		}
	} else {
		constexpr int window = 3;
		std::array<std::array<V, Count>, 1 << window> powers;
		for (std::size_t i = 0; i < Count; ++i) powers[1][i] = m.in(bases[i] % m.mod);
		for (int k = 2; k < (1 << window); ++k)
			for (std::size_t i = 0; i < Count; ++i) powers[k][i] = multiply(powers[k - 1][i], powers[1][i]);
		int shift = ((std::bit_width(d) - 1) / window) * window;
		x = powers[d >> shift];
		for (shift -= window; shift >= 0; shift -= window) {
			for (int k = 0; k < window; ++k) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], x[i]);
			}
			int digit = (d >> shift) & ((1 << window) - 1);
			if (digit) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], powers[digit][i]);
			}
		}
	}
#pragma GCC unroll 8
	for (std::size_t i = 0; i < Count; ++i) {
		V y = x[i] >= m.mod ? x[i] - m.mod : x[i];
		if (!y && bases[i] % m.mod == 0) continue;
		if (!witness(m, y, s, multiply)) return false;
	}
	return true;
}

inline bool prime(const Montgomery32 &m) {
	using V = Montgomery32::U;
	V d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto check = [&](auto multiply) {
		if (!witnesses(m, d, s, std::array<V, 1>{2}, multiply)) return false;
		if (m.mod < 1373653) return witnesses(m, d, s, std::array<V, 1>{3}, multiply);
		return witnesses(m, d, s, std::array<V, 2>{7, 61}, multiply);
	};
	if (m.mod < (V(1) << 30)) return check([&](V a, V b) {
		std::uint64_t x = std::uint64_t(a) * b;
		return V(x >> 32) + m.mod - V((std::uint64_t(V(x) * m.inverse) * m.mod) >> 32);
	});
	return check([&](V a, V b) { return m.mul(a, b); });
}

inline bool prime(const Montgomery64 &m) {
	if (m.mod < (U(1) << 32)) return prime(Montgomery32(m.mod));
	U d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	const std::array<U, 6> bases{325, 9375, 28178, 450775, 9780504, 1795265022};
	if (m.mod < (U(1) << 62)) {
		if (!witness2(m, d, s, [&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U(x >> 64) + m.mod - U((__uint128_t(U(x) * m.inverse) * m.mod) >> 64);
		})) return false;
		U inverse = -m.inverse;
		return witnesses(m, d, s, bases, [&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U((x + __uint128_t(U(x) * inverse) * m.mod) >> 64);
		});
	}
	auto multiply = [&](U a, U b) { return m.mul(a, b); };
	return witnesses(m, d, s, std::array<U, 1>{2}, multiply) && witnesses(m, d, s, bases, multiply);
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
