#ifndef FACTORIZE_HPP
#define FACTORIZE_HPP

#include "is_prime.hpp"
#include "../components/splitmix64.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

namespace factorize_detail {
using U = std::uint64_t;
using W = __uint128_t;

struct Factorizer {
	SplitMix64 rng;
	std::vector<U> result;
	explicit Factorizer(U n) : rng(n ^ 0x9e3779b97f4a7c15ULL) { result.reserve(64); }
	U random() { return rng(); }

	struct RhoArithmetic {
		U mod, inverse, one;
		explicit RhoArithmetic(const Montgomery64 &m) : mod(m.mod), inverse(m.inverse), one(m.one) {}
		U mul(U a, U b) const {
			W x = W(a) * b;
			return U(x >> 64) + mod - U((W(U(x) * inverse) * mod) >> 64);
		}
		U step(U x, U c) const {
			return mul(x, x) + c;
		}
	};

	U rho_parallel(const Montgomery64 &original) {
		RhoArithmetic m(original);
		const U n = m.mod;
		bool first = true;
		for (;;) {
			U y1 = first ? 1 : random() % n, y2 = first ? 2 : random() % n;
			const U c1 = first ? 1 : (random() & 31) + 1, c2 = first ? 2 : (random() & 31) + 1;
			first = false;
			bool restart = false;
			for (int length = 64; length <= (1 << 18) && !restart; length <<= 1) {
				const U x1 = y1 + 2 * n, x2 = y2 + 2 * n;
				for (int k = 0; k < length; k += 1024) {
					const U saved1 = y1, saved2 = y2;
					U q1 = m.one, q2 = m.one;
					const int count = std::min(1024, length - k);
					y1 = m.step(y1, c1), y2 = m.step(y2, c2);
					for (int i = 0; i < count; ++i) {
						const U d1 = x1 - y1, d2 = x2 - y2;
						y1 = m.step(y1, c1), y2 = m.step(y2, c2);
						q1 = m.mul(q1, d1), q2 = m.mul(q2, d2);
					}
					q1 = m.mul(q1, x1 - y1), q2 = m.mul(q2, x2 - y2);
					const U g = std::gcd(m.mul(q1, q2), n);
					if (g == 1) continue;
					if (g != n) return g;
					const U g1 = std::gcd(q1, n), g2 = std::gcd(q2, n);
					if (g1 > 1 && g1 < n) return g1;
					if (g2 > 1 && g2 < n) return g2;
					U y = g1 == n ? saved1 : saved2;
					const U x = g1 == n ? x1 : x2, c = g1 == n ? c1 : c2;
					for (int i = 0; i <= count; ++i) {
						y = m.step(y, c);
						const U d = std::gcd(x - y, n);
						if (d > 1 && d < n) return d;
						if (d == n) break;
					}
					restart = true;
					break;
				}
			}
		}
	}
	U rho(const Montgomery64 &m) {
		if ((m.mod >> 32) && m.mod < (U(1) << 61)) return rho_parallel(m);
		U n = m.mod;
		for (;;) {
			U y = m.in(random() % (n - 1) + 1), c = m.in(random() % (n - 1) + 1);
			U x = 0, saved = 0, g = 1;
			auto step = [&](U v) { return m.add(m.mul(v, v), c); };
			auto diff = [](U a, U b) { return a < b ? b - a : a - b; };
			for (int length = 1; g == 1 && length <= (1 << 18); length <<= 1) {
				x = y;
				for (int repeat_index = 0; repeat_index < length; ++repeat_index) y = step(y);
				for (int k = 0; k < length && g == 1; k += 128) {
					saved = y;
					U q = m.one;
					for (int repeat_index = 0; repeat_index < std::min(128, length - k); ++repeat_index) {
						y = step(y);
						q = m.mul(q, diff(x, y));
					}
					g = std::gcd(q, n);
				}
			}
			if (g == n)
				for (int repeat_index = 0; repeat_index < 128; ++repeat_index) {
					saved = step(saved);
					g = std::gcd(diff(x, saved), n);
					if (g != 1) break;
				}
			if (g > 1 && g < n) return g;
		}
	}
	void split(U n) {
		if (n < 1681) {
			result.push_back(n);
			return;
		}
		if ((n & 7) == 1) {
			U r = std::sqrt(static_cast<long double>(n));
			r -= W(r) * r > n;
			if (W(r) * r == n) {
				int begin = static_cast<int>(result.size());
				split(r);
				int end = static_cast<int>(result.size());
				for (int i = begin; i < end; ++i) result.push_back(result[i]);
				return;
			}
		}
		Montgomery64 m(n);
		if (is_prime_detail::fast_prime(m)) result.push_back(n);
		else {
			U d = rho(m);
			split(d);
			split(n / d);
		}
	}
};
}

inline std::vector<std::uint64_t> factorize(std::uint64_t n) {
	assert(n > 0);
	if (n < 2) return {};
	factorize_detail::Factorizer f(n);
	for (auto p : is_prime_detail::small_primes)
		while (n % p == 0) f.result.push_back(p), n /= p;
	if (n > 1) f.split(n);
	std::sort(f.result.begin(), f.result.end());
	return std::move(f.result);
}

inline std::vector<std::pair<std::uint64_t, int>> factorize_count(std::uint64_t n) {
	std::vector<std::pair<std::uint64_t, int>> result;
	for (auto p : factorize(n)) {
		if (result.empty() || result.back().first != p) result.emplace_back(p, 1);
		else ++result.back().second;
	}
	return result;
}

inline std::vector<std::uint64_t> divisors(const std::vector<std::pair<std::uint64_t, int>> &factors, bool sorted = true) {
	std::size_t count = 1;
	for (auto [p, e] : factors) {
		assert(p >= 2 && e > 0);
		count *= std::size_t(e) + 1;
	}
	std::vector<std::uint64_t> result{1};
	result.reserve(count);
	for (auto [p, e] : factors) {
		std::size_t size = result.size();
		std::uint64_t power = 1;
		for (int i = 0; i < e; ++i) {
			assert(power <= std::numeric_limits<std::uint64_t>::max() / p);
			power *= p;
			for (std::size_t j = 0; j < size; ++j) {
				assert(result[j] <= std::numeric_limits<std::uint64_t>::max() / power);
				result.push_back(result[j] * power);
			}
		}
	}
	if (sorted) std::sort(result.begin(), result.end());
	return result;
}

inline std::vector<std::uint64_t> divisors(std::uint64_t n, bool sorted = true) {
	return divisors(factorize_count(n), sorted);
}

#endif
