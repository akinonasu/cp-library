#line 1 "test/factorize.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/factorize"
#line 1 "math/number_theory/factorize.hpp"



#line 1 "math/number_theory/is_prime.hpp"



#line 1 "components/montgomery.hpp"



#include <cassert>
#include <cstdint>

struct Montgomery64 {
	using U = std::uint64_t;
	using W = __uint128_t;
	U mod, inverse = 1, r2, one;
	explicit Montgomery64(U n) : mod(n), r2(-W(n) % n) {
		for (int repeat_index = 0; repeat_index < 6; ++repeat_index) inverse *= 2 - n * inverse;
		assert(n > 1 && (n & 1));
		one = reduce(r2);
	}
	U reduce(W x) const {
		U a = x >> 64, b = (W(U(x) * inverse) * mod) >> 64;
		return a < b ? a - b + mod : a - b;
	}
	U in(U x) const { return reduce(W(x) * r2); }
	U mul(U a, U b) const { return reduce(W(a) * b); }
	U add(U a, U b) const {
		U c = a + b;
		return c < a || c >= mod ? c - mod : c;
	}
	U pow(U a, U e) const {
		U r = one;
		for (; e; e >>= 1, a = mul(a, a))
			if (e & 1) r = mul(r, a);
		return r;
	}
};


#line 1 "components/montgomery32.hpp"



#line 6 "components/montgomery32.hpp"

struct Montgomery32 {
	using U = std::uint32_t;
	using W = std::uint64_t;
	U mod, inverse = 1, r2, one;

	explicit Montgomery32(U n) : mod(n), r2(-W(n) % n) {
		for (int i = 0; i < 5; ++i) inverse *= 2 - n * inverse;
		assert(n > 1 && (n & 1));
		one = reduce(r2);
	}
	U reduce(W x) const {
		U a = x >> 32, b = (W(U(x) * inverse) * mod) >> 32;
		return a < b ? a - b + mod : a - b;
	}
	U in(U x) const { return reduce(W(x) * r2); }
	U mul(U a, U b) const { return reduce(W(a) * b); }
	U add(U a, U b) const {
		U c = a + b;
		return c < a || c >= mod ? c - mod : c;
	}
	U pow(U a, U e) const {
		U r = one;
		if (mod < (U(1) << 30)) {
			U inverse = -this->inverse;
			auto product = [&](U a, U b) {
				W x = W(a) * b;
				return U((x + W(U(x) * inverse) * mod) >> 32);
			};
			for (; e; e >>= 1, a = product(a, a)) if (e & 1) r = product(r, a);
			return r < mod ? r : r - mod;
		}
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};


#line 6 "math/number_theory/is_prime.hpp"
#include <array>
#include <bit>
#line 9 "math/number_theory/is_prime.hpp"
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


#line 1 "components/splitmix64.hpp"



#line 5 "components/splitmix64.hpp"

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 6 "math/number_theory/factorize.hpp"
#include <algorithm>
#line 8 "math/number_theory/factorize.hpp"
#include <cmath>
#line 10 "math/number_theory/factorize.hpp"
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
		if (is_prime_detail::prime(m)) result.push_back(n);
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


#line 4 "test/factorize.test.cpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		uint64_t n;
		cin >> n;
		auto factors = factorize(n);
		cout << static_cast<int>(factors.size());
		for (auto p : factors) cout << ' ' << p;
		cout << '\n';
	}
}
