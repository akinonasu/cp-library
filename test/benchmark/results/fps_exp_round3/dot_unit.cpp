#include "components/fps_exp_blocked.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <vector>
#if defined(__linux__)
#include <sys/mman.h>
#endif

static void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); }
}

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	using fps_detail::Mint;
	std::mt19937 rng(20261103);
	const auto check = [&](const Mint* in, std::size_t input, const Mint* g, std::size_t n) {
		Mint a = 0, b = 0;
		for (std::size_t k = 1; k <= n && k < input; ++k) {
			const Mint value = in[k] * g[n - k];
			a += value * Mint(k);
			b += value;
		}
		require(fps_detail::blocked_poly::detail::exp_tail_coefficient<false>(in, input, g, n) == a / Mint(n));
		require(fps_detail::blocked_poly::detail::exp_tail_coefficient<true>(in, input, g, n) == b / Mint(n));
	};
	std::vector<std::size_t> sizes;
	for (std::size_t n = 1; n <= 257; ++n) sizes.push_back(n);
	for (std::size_t n : {4097u, 8193u, 16385u, 262144u}) sizes.push_back(n);
	for (const auto n : sizes)
		for (const std::size_t input : {std::size_t(0), std::size_t(1), std::size_t(2), n / 3, n, n + 1, n + 17}) {
			std::vector<Mint> in(input), g(n);
			for (auto& x : in) x = Mint::word(2 * Mint::mod() - 1 - rng() % 1000);
			for (auto& x : g) x = Mint::word(2 * Mint::mod() - 1 - rng() % 1000);
			const auto saved = in;
			check(in.data(), input, g.data(), n);
			for (std::size_t i = 0; i < input; ++i) require(in[i].v == saved[i].v);
		}
	for (const std::size_t n : {7u, 8u, 9u, 31u, 32u, 33u, 63u, 64u, 65u, 129u}) {
		void* a = mmap(nullptr, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		void* b = mmap(nullptr, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		require(a != MAP_FAILED && b != MAP_FAILED);
		require(mprotect(static_cast<char*>(a) + 4096, 4096, PROT_NONE) == 0);
		require(mprotect(static_cast<char*>(b) + 4096, 4096, PROT_NONE) == 0);
		auto* in = reinterpret_cast<Mint*>(static_cast<char*>(a) + 4096 - (n + 1) * sizeof(Mint));
		auto* g = reinterpret_cast<Mint*>(static_cast<char*>(b) + 4096 - n * sizeof(Mint));
		for (std::size_t i = 0; i <= n; ++i) std::construct_at(in + i, Mint::word(2 * Mint::mod() - 1 - rng() % 1000));
		for (std::size_t i = 0; i < n; ++i) std::construct_at(g + i, Mint::word(2 * Mint::mod() - 1 - rng() % 1000));
		check(in, n + 1, g, n);
		std::destroy_n(in, n + 1); std::destroy_n(g, n);
		require(munmap(a, 8192) == 0 && munmap(b, 8192) == 0);
	}
#endif
}
