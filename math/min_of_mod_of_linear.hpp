#ifndef MIN_OF_MOD_OF_LINEAR_HPP
#define MIN_OF_MOD_OF_LINEAR_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>

namespace min_mod_detail {
inline std::uint32_t small(std::uint32_t n, std::uint32_t m, std::uint32_t a, std::uint32_t b) {
	std::uint32_t result = 0, p = 1, q = 1;
	bool forward = true;
	while (a) {
		const auto edge = (forward ? a : m) - 1, quotient = m / a, remainder = m % a;
		const auto next = quotient * p + q;
		if ((forward ? b + 1 : m - b) > a) {
			const auto step = (m - b + (forward ? a : 0) - 1) / a;
			const auto consumed = (step - forward) * p + (forward ? q : 0);
			if (n <= consumed) {
				if (!forward) result -= a * ((n - 1) / p);
				return result + (forward ? b : -b);
			}
			n -= consumed, b += a * step - (forward ? m : 0);
		}
		q = next, p = next - p;
		result += forward ? edge : -edge;
		m = a, a = remainder, b = edge - b, forward = !forward;
	}
	return result + (forward ? b : -b);
}
}

inline std::uint64_t min_of_mod_of_linear(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b) {
	assert(n > 0 && m > 0);
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	if (!a || !b || n == 1) return b;
	if (n >= m) return b % std::gcd(a, m);
	if (m <= (std::uint64_t(1) << 30)) return min_mod_detail::small(n, m, a, b);
	if (m <= std::numeric_limits<std::uint32_t>::max()) {
		std::uint64_t best = b;
		while (a) {
			std::uint64_t z = a * (n - 1) + b;
			if (z < m) break;
			n = z / m;
			b = z % m % a;
			if (!b) return 0;
			best = std::min(best, b);
			std::uint64_t next = m % a;
			m = a, a = next;
		}
		return best;
	}
	std::uint64_t p = m - a, q = a, sp = 1, sq = 1, remaining = n - 1;
	for (;;) {
		if (p <= b) {
			std::uint64_t k = b / p;
			if (__uint128_t(k) * sp > remaining) return b - remaining / sp * p;
			b %= p;
			remaining -= k * sp;
			if (!b) return 0;
		}
		if (sp > remaining || sq > remaining - sp) return b;
		if (p > q) {
			std::uint64_t k = (p - b - 1) / q + 1, d = k * q;
			if (d == p) return b;
			if (d > p) --k;
			sp += k * sq;
			if (sp > remaining) return b;
			p -= k * q;
		} else {
			std::uint64_t k = q / p;
			q %= p;
			if (!q) return b;
			sq += k * sp;
		}
	}
}

#endif
