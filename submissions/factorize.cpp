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


#line 5 "math/number_theory/is_prime.hpp"
#include <bit>
#line 7 "math/number_theory/is_prime.hpp"
#include <initializer_list>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

inline bool prime(const Montgomery64 &m) {
	U n = m.mod, d = n - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto witness = [&](U a) {
		if (!(a %= n)) return true;
		U x = m.pow(m.in(a), d);
		if (x == m.one || x == n - m.one) return true;
		for (int i = 1; i < s; ++i) {
			x = m.mul(x, x);
			if (x == n - m.one) return true;
		}
		return false;
	};
	if (n < (U(1) << 32)) {
		for (U a : {2, 7, 61})
			if (!witness(a)) return false;
	} else {
		for (U a : {2, 325, 9375, 28178, 450775, 9780504, 1795265022})
			if (!witness(a)) return false;
	}
	return true;
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
	return n < 1681 || is_prime_detail::prime(Montgomery64(n));
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
	U rho(const Montgomery64 &m) {
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
