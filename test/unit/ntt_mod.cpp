#include "components/ntt_mod.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

template <std::uint32_t mod, std::uint32_t root>
void check() {
	std::mt19937 rng(mod);
	for (int half : {1, 2, 4, 8, 16, 64, 1024}) {
		int n = 4 * half;
		std::vector<std::uint32_t> a(n), roots(half), quotients(half);
		for (auto &x : a) x = rng() % mod;
		a[0] = a[half] = mod - 1;
		a[1] = a[half + 1] = 0;
		for (int j = 0; j < half; ++j) {
			roots[j] = j == 0 ? mod - 1 : rng() % mod;
			quotients[j] = (std::uint64_t(roots[j]) << 32) / mod;
		}
		auto expected = a;
		for (int start = 0; start < n; start += 2 * half)
			for (int j = 0; j < half; ++j) {
				std::uint64_t x = a[start + j], y = std::uint64_t(a[start + j + half]) * roots[j] % mod;
				expected[start + j] = (x + y) % mod;
				expected[start + j + half] = (x + mod - y) % mod;
			}
		auto b = a;
		ntt_mod_detail::butterfly<mod>(b.data(), n, half, roots.data(), quotients.data());
		assert(b == expected);
#if defined(__x86_64__) || defined(__i386__)
		if (half >= 8 && __builtin_cpu_supports("avx2")) {
			b = a;
			ntt_mod_detail::butterfly_avx2<mod>(b.data(), n, half, roots.data(), quotients.data());
			assert(b == expected);
		}
#endif
	}
	for (int n = 1; n <= 131072 && (mod - 1) % n == 0; n *= 2) {
		std::vector<std::uint32_t> a(n);
		for (auto &x : a) x = rng() % mod;
		for (bool inverse : {false, true}) {
			auto scalar = a, dispatched = a;
			ntt_mod_detail::transform<mod, root>(scalar, inverse, false);
			ntt_mod<mod, root>(dispatched, inverse);
			assert(scalar == dispatched);
			ntt_mod_detail::transform<mod, root>(scalar, !inverse, false);
			ntt_mod<mod, root>(dispatched, !inverse);
			assert(scalar == a && dispatched == a);
		}
	}
}

int main() {
	check<3, 2>();
	check<17, 3>();
	check<998244353, 3>();
	check<754974721, 11>();
	check<1224736769, 3>();
	check<2013265921, 31>();
	check<2130706433, 3>();
	check<2147483647, 7>();
}
