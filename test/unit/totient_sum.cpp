#include "math/totient_sum.hpp"
#include "math/mint.hpp"
#include "math/modint.hpp"
#include "test/fixtures/multiplicative_prefix_sum_types.hpp"
#include <cassert>
#include <cfenv>
#include <cstdint>
#include <functional>
#include <random>
#include <unordered_map>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;

struct WideMod {
	U v = 0;
	static constexpr U mod() { return UINT64_MAX - 58; }
	WideMod(U x = 0) : v(x % mod()) {}
	WideMod(unsigned x) : WideMod(U(x)) {}
	WideMod(std::int64_t x) {
		auto r = __int128_t(x) % __int128_t(mod());
		v = U(r < 0 ? r + mod() : r);
	}
	WideMod(int x) : WideMod(std::int64_t(x)) {}
	U val() const { return v; }
	WideMod &operator+=(WideMod b) { v = U((W(v) + b.v) % mod()); return *this; }
	WideMod &operator-=(WideMod b) { v = v >= b.v ? v - b.v : mod() - (b.v - v); return *this; }
	WideMod &operator*=(WideMod b) { v = U(W(v) * b.v % mod()); return *this; }
	friend WideMod operator+(WideMod a, WideMod b) { return a += b; }
	friend WideMod operator-(WideMod a, WideMod b) { return a -= b; }
	friend WideMod operator*(WideMod a, WideMod b) { return a *= b; }
};

struct NarrowMod {
	unsigned v = 0;
	static constexpr unsigned mod() { return 251; }
	NarrowMod(std::int8_t x = 0) {
		int r = int(x) % int(mod());
		v = unsigned(r < 0 ? r + int(mod()) : r);
	}
	unsigned val() const { return v; }
	NarrowMod &operator+=(NarrowMod b) { v = (v + b.v) % mod(); return *this; }
	NarrowMod &operator-=(NarrowMod b) { v = (v + mod() - b.v) % mod(); return *this; }
	NarrowMod &operator*=(NarrowMod b) { v = v * b.v % mod(); return *this; }
	friend NarrowMod operator+(NarrowMod a, NarrowMod b) { return a += b; }
	friend NarrowMod operator-(NarrowMod a, NarrowMod b) { return a -= b; }
	friend NarrowMod operator*(NarrowMod a, NarrowMod b) { return a *= b; }
};

static_assert(totient_detail::wide_modular<mint>);
static_assert(totient_detail::wide_modular<StaticModint<998244353>>);
static_assert(totient_detail::wide_modular<WideMod>);
static_assert(!totient_detail::wide_modular<NarrowMod>);
static_assert(!totient_detail::wide_modular<MultiplicativeSumModint<251>>);

int main() {
	constexpr unsigned limit = 1000000;
	std::vector<U> phi(limit + 1), prefix(limit + 1);
	for (unsigned i = 1; i <= limit; ++i) phi[i] = i;
	for (unsigned p = 2; p <= limit; ++p) if (phi[p] == p)
		for (unsigned i = p; i <= limit; i += p) phi[i] -= phi[i] / p;
	for (unsigned i = 1; i <= limit; ++i) prefix[i] = prefix[i - 1] + phi[i];
	for (unsigned n = 0; n <= 1000; ++n) assert(totient_sum(n) == prefix[n]);
	for (unsigned n = 0; n < 64; ++n) assert(totient_sum<NarrowMod>(n).val() == prefix[n] % NarrowMod::mod());
	assert(totient_sum<NarrowMod>(20).val() == 128);
	for (U n : {U(0), U(1), U(2), U(3), U(31), U(63), U(64), U(65), U(127), U(128)}) {
		assert(totient_sum<mint>(n).val() == prefix[n] % mint::mod());
		assert(totient_sum<MultiplicativeSumModint<1>>(n).val() == 0);
		assert(totient_sum<WideMod>(n).val() == prefix[n] % WideMod::mod());
	}
	std::mt19937_64 rng(20261023);
	for (int i = 0; i < 100; ++i) {
		U n = rng() % (limit + 1);
		assert(totient_sum(n) == prefix[n]);
		assert(totient_sum<mint>(n).val() == prefix[n] % mint::mod());
		assert(totient_sum<MultiplicativeSumModint<12>>(n).val() == prefix[n] % 12);
		assert(totient_sum<MultiplicativeSumModint<1000000007>>(n).val() == prefix[n] % 1000000007);
		assert(totient_sum<WideMod>(n).val() == prefix[n] % WideMod::mod());
		if (n >= 64) assert((totient_detail::compute<W, __int128_t>(n) == prefix[n]));
	}
	for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
		if (std::fesetround(mode)) continue;
		for (U n : {U(63), U(64), U(65), U(127), U(128), U(10000), U(limit)}) assert(totient_sum(n) == prefix[n]);
	}
	std::fesetround(FE_TONEAREST);
	std::unordered_map<U, W> cache;
	std::function<W(U)> reference = [&](U n) -> W {
		if (n <= limit) return prefix[n];
		if (auto found = cache.find(n); found != cache.end()) return found->second;
		W result = W(n) * (n + 1) / 2;
		for (U l = 2, r; l <= n; l = r + 1) {
			r = n / (n / l);
			result -= W(r - l + 1) * reference(n / l);
		}
		return cache[n] = result;
	};
	for (U n : {U(5000000000), U(10000000000)}) {
		W expected = reference(n);
		assert(totient_sum<W>(n) == expected);
		assert((totient_detail::compute<W, __int128_t>(n) == expected));
		assert(totient_sum<mint>(n).val() == expected % mint::mod());
		assert(totient_sum<WideMod>(n).val() == U(expected % WideMod::mod()));
		if (n == 5000000000) assert(W(totient_sum<std::int64_t>(n)) == expected);
	}
}
