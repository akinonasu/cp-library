#ifndef IS_PRIME_CORE_HPP
#define IS_PRIME_CORE_HPP

#include "montgomery.hpp"
#include "montgomery32.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr U small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

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
	const U twice = m.mod + m.mod;
	for (int bit = shift - 1; bit >= 0; --bit) {
		x = multiply(x, x);
		if ((d >> bit) & 1) {
			x += x;
			if (x >= twice) x -= twice;
		}
	}
	return witness(m, x, s, multiply);
}

template <class Montgomery, std::size_t Count, class Multiply>
inline bool witnesses(const Montgomery &m, typename Montgomery::U d, int s,
	const std::array<typename Montgomery::U, Count> &bases, Multiply multiply) {
	using V = typename Montgomery::U;
	std::array<V, Count> x;
	if (Count <= 3 || d < 4096) {
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
		U x = U(a) * b;
		return V(x >> 32) + m.mod - V((U(V(x) * m.inverse) * m.mod) >> 32);
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

template <class Check>
inline bool check(U n, Check prime) {
	if (n < 2) return false;
#pragma GCC unroll 16
	for (U p : small_primes)
		if (n % p == 0) return n == p;
	if (n < 1681) return true;
	if (n < (std::uint64_t(1) << 32)) return prime(Montgomery32(n));
	return prime(Montgomery64(n));
}

}

#endif
