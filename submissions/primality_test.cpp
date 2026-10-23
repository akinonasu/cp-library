#line 1 "test/is_prime.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
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


#line 4 "test/is_prime.test.cpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	while (q--) {
		std::uint64_t n;
		std::cin >> n;
		std::cout << (is_prime(n) ? "Yes" : "No") << '\n';
	}
}
