#include "math/totient_sum.hpp"
#include "math/mint.hpp"
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

int main() {
	constexpr unsigned limit = 1000000;
	std::vector<U> phi(limit + 1), prefix(limit + 1);
	for (unsigned i = 1; i <= limit; ++i) phi[i] = i;
	for (unsigned p = 2; p <= limit; ++p) if (phi[p] == p)
		for (unsigned i = p; i <= limit; i += p) phi[i] -= phi[i] / p;
	for (unsigned i = 1; i <= limit; ++i) prefix[i] = prefix[i - 1] + phi[i];
	for (unsigned n = 0; n <= 1000; ++n) assert(totient_sum(n) == prefix[n]);
	std::mt19937_64 rng(20261023);
	for (int i = 0; i < 100; ++i) {
		U n = rng() % (limit + 1);
		assert(totient_sum(n) == prefix[n]);
		assert(totient_sum<mint>(n).val() == prefix[n] % mint::mod());
		assert(totient_sum<MultiplicativeSumModint<12>>(n).val() == prefix[n] % 12);
		assert(totient_sum<MultiplicativeSumModint<1000000007>>(n).val() == prefix[n] % 1000000007);
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
		if (n == 5000000000) assert(W(totient_sum<std::int64_t>(n)) == expected);
	}
}
