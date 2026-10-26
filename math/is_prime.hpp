#ifndef IS_PRIME_HPP
#define IS_PRIME_HPP

#include "../components/is_prime_core.hpp"
#include "../components/is_prime_bases.hpp"

namespace is_prime_detail {
inline bool fast_prime(const Montgomery32 &m) {
	using V = Montgomery32::U;
	V d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	const std::array<V, 1> bases{bases32[(m.mod * 0xad625b89u) >> 24]};
	if (m.mod < (V(1) << 30)) return witnesses(m, d, s, bases, [&](V a, V b) {
		std::uint64_t x = std::uint64_t(a) * b;
		return V(x >> 32) + m.mod - V((std::uint64_t(V(x) * m.inverse) * m.mod) >> 32);
	});
	return witnesses(m, d, s, bases, [&](V a, V b) { return m.mul(a, b); });
}

inline bool fast_prime(const Montgomery64 &m) {
	if (m.mod < (U(1) << 32)) return fast_prime(Montgomery32(m.mod));
	U d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	U base = bases64[(std::uint32_t(m.mod) * 0xad625b89u) >> 18];
	constexpr std::array<U, 8> second{15, 135, 13, 60, 15, 117, 65, 29};
	auto check = [&](auto multiply) {
		if (m.mod < (U(1) << 49)) return witnesses(m, d, s, std::array<U, 2>{2, base}, multiply);
		return witnesses(m, d, s, std::array<U, 3>{2, base, second[base >> 13]}, multiply);
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
	return is_prime_detail::check(n, [](const auto &m) { return is_prime_detail::fast_prime(m); });
}

#endif
