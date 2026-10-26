#include "math/factorize.hpp"
#include "math/is_prime_light.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;

U power(U a, U e, U n) {
	U r = 1;
	for (; e; e >>= 1, a = W(a) * a % n)
		if (e & 1) r = W(r) * a % n;
	return r;
}

bool prime(U n) {
	if (n < 2 || !(n & 1)) return n == 2;
	U d = n - 1;
	int s = 0;
	while (!(d & 1)) d >>= 1, ++s;
	for (U a : {2, 325, 9375, 28178, 450775, 9780504, 1795265022}) {
		if (a % n == 0) continue;
		U x = power(a % n, d, n);
		if (x == 1 || x == n - 1) continue;
		bool pass = false;
		for (int i = 1; i < s; ++i) {
			x = W(x) * x % n;
			if (x == n - 1) pass = true;
		}
		if (!pass) return false;
	}
	return true;
}

template <class M>
void check_montgomery(U n, std::mt19937_64 &random) {
	M m(n);
	assert(typename M::U(n * m.inverse) == 1);
	for (int i = 0; i < 16; ++i) {
		U a = i < 3 ? (i == 0 ? 0 : n - i) : random() % n;
		U b = i < 3 ? n - 1 : random() % n;
		U x = m.in(a), y = m.in(b);
		assert(m.reduce(x) == a);
		assert(m.reduce(m.mul(x, y)) == W(a) * b % n);
		assert(m.reduce(m.add(x, y)) == (W(a) + b) % n);
		U e = random() & 255;
		assert(m.reduce(m.pow(x, e)) == power(a, e, n));
		assert(m.reduce(m.pow(x, 0)) == 1);
	}
}

int main() {
	std::mt19937_64 random(20261017);
	auto check_prime = [](U n) {
		const bool expected = prime(n);
		assert(is_prime(n) == expected);
		assert(light_is_prime(n) == expected);
		if (n > 1 && (n & 1)) {
			assert(is_prime_detail::prime(Montgomery64(n)) == expected);
			assert(is_prime_detail::fast_prime(Montgomery64(n)) == expected);
			if (n <= std::numeric_limits<std::uint32_t>::max()) {
				assert(is_prime_detail::prime(Montgomery32(n)) == expected);
				assert(is_prime_detail::fast_prime(Montgomery32(n)) == expected);
			}
		}
	};
	assert(!is_prime(0) && !is_prime(1));
	assert((factorize(4294967295ULL) == std::vector<U>{3, 5, 17, 257, 65537}));
	assert((factorize(4294967291ULL) == std::vector<U>{4294967291ULL}));
	assert((factorize(4294967297ULL) == std::vector<U>{641, 6700417}));
	std::vector<bool> composite(1000001);
	for (U i = 2; i < composite.size(); ++i) {
		assert(is_prime(i) == !composite[i]);
		assert(light_is_prime(i) == !composite[i]);
		if (!composite[i])
			for (U j = i * i; j < composite.size(); j += i) composite[j] = true;
	}
	for (U n : {3ULL, 41ULL, 1681ULL, 2147483647ULL, 2147483649ULL, 4294967291ULL, 4294967295ULL})
		check_montgomery<Montgomery64>(n, random);
	for (U n : {3ULL, 4294967297ULL, 9223372036854775783ULL, 9223372036854775809ULL,
				18446744073709551557ULL, 18446744073709551615ULL})
		check_montgomery<Montgomery64>(n, random);
	for (int i = 0; i < 20000; ++i) {
		U n = std::uint32_t(random()) | 1;
		if (n == 1) n = 3;
		check_montgomery<Montgomery64>(n, random);
		check_prime(n);
		n = random();
		check_prime(n);
	}
	for (U n = 0; n <= 257; ++n) check_prime(n);
	for (U n : {61ULL, 73ULL, 193ULL, 407521ULL, 299210837ULL, 8651776913431ULL,
				1152965996591997761ULL, 3315293452192821991ULL}) check_prime(n);
	for (unsigned bit : {17, 30, 32, 49, 62, 63}) {
		const U center = U(1) << bit;
		for (U n = center - 4096; n < center + 4096; ++n) check_prime(n);
	}
	for (U n = 1373653 - 256; n <= 1373653 + 256; ++n) check_prime(n);
	const U maximum = std::numeric_limits<U>::max();
	for (U offset = 0; offset <= 4096; ++offset) check_prime(maximum - offset);
	for (unsigned shift = 1; shift < 64; ++shift) {
		const U limit = (maximum - 1) >> shift;
		for (U k = 1; k <= 31 && k <= limit; k += 2) check_prime((k << shift) + 1);
	}
	for (U n : {341ULL, 561ULL, 1105ULL, 1729ULL, 2047ULL, 1373653ULL, 25326001ULL,
				3215031751ULL, 2152302898747ULL, 3474749660383ULL, 341550071728321ULL,
				3825123056546413051ULL, 18446744073709551615ULL}) assert(!is_prime(n));
}
