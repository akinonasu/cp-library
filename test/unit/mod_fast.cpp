#include "math/mod_pow.hpp"
#include "math/mint.hpp"
#include <array>
#include <limits>
#include <random>

using U = std::uint32_t;
using W = std::uint64_t;

U reference(U a, W e, U p) {
	U r = 1;
	for (; e; e >>= 1, a = W(a) * a % p) if (e & 1) r = W(r) * a % p;
	return r;
}

void check(U p) {
	ModInv inverse(p);
	ModPow power(p);
	assert(inverse.mod() == p && power.mod() == p);
	U g = power.primitive_root();
	std::mt19937_64 rng(731 + p);
	std::array<W, 10> exponents{0, 1, 2, p - 2, p - 1, p, 2ULL * (p - 1),
		W(std::numeric_limits<std::int64_t>::max()), W(1) << 63, std::numeric_limits<W>::max()};
	for (U a = 0; a < std::min(p, U(5000)); ++a) {
		for (W e : exponents) assert(power.pow(a, e) == reference(a, e, p));
		if (a) {
			assert(W(a) * inverse.inv(a) % p == 1 && inverse.inv(a) == power.inv(a));
			assert(reference(g, power.log(a), p) == a && power.log(a) < p - 1);
		}
	}
	for (int i = 0; i < 20000; ++i) {
		U a = 1 + rng() % (p - 1);
		W e = rng();
		assert(power.pow(a, e) == reference(a, e, p));
		assert(inverse.inv(a) == reference(a, p - 2, p));
		assert(power.pow_root(e) == reference(g, e, p));
		assert(power.pow_root(power.log(a)) == a);
	}
	if (p < 10000) {
		U x = 1;
		for (U e = 0; e < p - 1; ++e, x = W(x) * g % p) {
			assert(power.log(x) == e && inverse.inv(x) == reference(x, p - 2, p));
		}
		ModPow supplied(p, g);
		assert(supplied.primitive_root() == g);
		for (U x = 1; x < p; ++x) assert(supplied.log(x) == power.log(x));
	}
	if (p == 998244353) for (int i = 0; i < 2000; ++i) {
		mint a = mint::raw(1 + rng() % (p - 1));
		assert(mint::raw(inverse.inv(a.val())) == a.inv());
		assert(mint::raw(power.pow(a.val(), i)) == a.pow(i));
	}
}

int main() {
	std::mt19937_64 rng(731);
	for (U p : {1U, 2U, 3U, 4U, 998244352U, 998244353U, 1000000007U, 2147483647U}) {
		mod_fast_detail::Reducer r(p);
		for (W x : {W(0), W(1), W(p - 1), W(p), W(p) * p, std::numeric_limits<W>::max()}) assert(r.reduce(x) == x % p);
		for (int i = 0; i < 100000; ++i) {
			W x = rng();
			assert(r.reduce(x) == x % p);
		}
	}
	for (U p = 2; p < 100; ++p) if (is_prime(p)) check(p);
	for (U p : {127U, 257U, 4093U, 4099U, 8191U, 8209U, 65521U, 65537U, 1000003U,
		998244353U, 1000000007U, 1000000009U, 2147483647U}) check(p);
	for (U p : {4099U, 65537U, 1000003U}) {
		mod_fast_detail::RationalTable table(p);
		for (U x = 1; x < p; ++x) {
			auto [a, b] = table.fraction(x);
			assert(0 < b && b < p && (W(b) * x + p - (a < 0 ? p + a : a)) % p == 0);
			assert(a && -int(table.limit()) <= a && a <= int(table.limit()));
		}
	}
	ModPow first(101);
	auto copy = first;
	ModPow moved = std::move(copy);
	for (U x = 1; x < 101; ++x) assert(moved.log(x) == first.log(x) && moved.inv(x) == first.inv(x));
}
