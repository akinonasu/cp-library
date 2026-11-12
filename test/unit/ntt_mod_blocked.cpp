#include "convolution/convolution_mod.hpp"
#include "components/ntt_mod_blocked.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
template <std::uint32_t P, std::uint32_t Root>
std::vector<std::uint32_t> reference(const std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b) {
	const std::size_t n = std::bit_ceil(a.size() + b.size() - 1);
	auto x = a, y = b;
	x.resize(n); y.resize(n);
	ntt_mod<P, Root>(x); ntt_mod<P, Root>(y);
	for (std::size_t i = 0; i < n; ++i) x[i] = std::uint64_t(x[i]) * y[i] % P;
	ntt_mod<P, Root>(x, true);
	x.resize(a.size() + b.size() - 1);
	return x;
}

template <std::uint32_t P, std::uint32_t Root>
void check() {
	std::mt19937 random(P);
	for (int lg = 7; lg <= 17; ++lg) {
		const std::size_t n = std::size_t(1) << lg;
		std::vector<std::uint32_t> a(n / 2), b(n / 3);
		for (auto &x : a) x = random() % P;
		for (auto &x : b) x = random() % P;
		for (int shape = 0; shape < 4; ++shape) {
			if (shape == 1) std::fill(a.begin(), a.end(), P - 1), std::fill(b.begin(), b.end(), P - 1);
			if (shape == 2) std::fill(b.begin(), b.end(), 0);
			const auto &other = shape == 3 ? a : b;
			const auto expected = reference<P, Root>(a, other), saved_a = a, saved_b = b;
			const auto actual = ntt_mod_detail::multiply_blocked<P, Root>(a, other, 1);
			assert(actual == expected && a == saved_a && b == saved_b);
			const auto scaled = ntt_mod_detail::multiply_blocked<P, Root>(a, other, P - 1);
			for (std::size_t i = 0; i < scaled.size(); ++i) assert(scaled[i] == (expected[i] ? P - expected[i] : 0));
		}
	}
	for (auto [n, m] : {std::pair{33, 1000}, {255, 2041}, {256, 2048}, {257, 20000}, {512, 512}, {513, 512}, {1000, 777}, {2048, 40000}}) {
		std::vector<std::uint32_t> a(n), b(m);
		for (auto &x : a) x = random() % P;
		for (auto &x : b) x = random() % P;
		const std::size_t length = std::bit_ceil(unsigned(m / n >= 8 ? 2 * n - 1 : n + m - 1));
		const auto words = ntt_mod_detail::blocked_workspace_words(length);
		auto storage = fps_detail::allocate_fps_words(words + 16);
		std::fill_n(storage.get(), words + 16, 0xdeadbeefu);
		const auto actual = ntt_mod_detail::multiply_blocked<P, Root>(a, b, 1, storage.get() + 8);
		assert((actual == reference<P, Root>(a, b)));
		for (int i = 0; i < 8; ++i) assert(storage[i] == 0xdeadbeefu && storage[words + 8 + i] == 0xdeadbeefu);
	}
}
#endif

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	check<167772161, 3>();
	check<469762049, 3>();
	check<754974721, 11>();
	check<998244353, 3>();
#endif
}
