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
			for (int i = 0; i < n; ++i) assert(b[i] % mod == expected[i]);
		}
#endif
	}
	for (int n : {1, 2, 4, 8, 16, 128, 4096}) {
		std::vector<std::uint32_t> a(n), b(n), expected(n);
		for (int i = 0; i < n; ++i) {
			a[i] = i % 3 == 0 ? mod - 1 : rng() % mod;
			b[i] = i % 3 == 1 ? mod - 1 : rng() % mod;
			expected[i] = std::uint64_t(a[i]) * b[i] % mod;
		}
		auto scalar = a, vectorized = a;
		ntt_mod_detail::pointwise<mod>(scalar, b, false);
		std::uint32_t radix = (std::uint64_t(1) << 32) % mod;
		ntt_mod_detail::scale<mod>(scalar, radix, false);
		assert(scalar == expected);
#if defined(__x86_64__) || defined(__i386__)
		if (__builtin_cpu_supports("avx2")) {
			ntt_mod_detail::pointwise<mod>(vectorized, b, true);
			ntt_mod_detail::scale<mod>(vectorized, radix, true);
			assert(vectorized == expected);
			ntt_mod_detail::pointwise<mod>(vectorized, vectorized, true);
			ntt_mod_detail::scale<mod>(vectorized, radix, true);
			for (int i = 0; i < n; ++i) assert(vectorized[i] == std::uint64_t(expected[i]) * expected[i] % mod);
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
	check<257, 3>();
	check<65537, 3>();
	check<167772161, 3>();
	check<469762049, 3>();
	check<998244353, 3>();
	check<754974721, 11>();
	check<1224736769, 3>();
	check<2013265921, 31>();
	check<2130706433, 3>();
	check<2147483647, 7>();
}
