#include "components/fps_exp_blocked.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <vector>
#if defined(__linux__)
#include <sys/mman.h>
#endif

[[maybe_unused]] static void require(bool ok, int line = __builtin_LINE()) {
	if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); }
}

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	using Mint = mint;
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

	for (const std::size_t n : {1u, 2u, 7u, 31u, 63u, 64u, 65u, 255u, 256u, 257u, 4095u, 4096u, 4097u, 4098u, 4099u, 4100u, 4101u, 8193u, 16385u, 262143u, 262144u, 262145u, 262148u, 262149u}) {
		for (const std::size_t input : {std::size_t(0), std::size_t(1), std::size_t(7), n / 3, n, n + 17}) {
			std::vector<Mint> in(input);
			for (auto& x : in) x = Mint::word(2 * Mint::mod() - 1 - rng() % 1000);
			if (input) in[0] = 0;
			const auto saved = in;
			for (bool differentiated : {false, true}) {
				std::vector<Mint> actual{5, 7}, base;
				fps_detail::blocked_poly::native_exp(in.data(), input, actual, n, differentiated);
				require(actual.size() == n + 2 && actual[0] == Mint(5) && actual[1] == Mint(7));
				const auto prefix = std::bit_floor(n), rest = n - prefix;
				const auto direct = prefix >= 4096 && rest && rest <= 4;
				const auto known = direct ? prefix : n;
				fps_detail::blocked_poly::native_exp_regular(in.data(), input, base, known, differentiated);
				for (std::size_t i = 0; i < known; ++i) require(actual[i + 2] == base[i]);
				for (std::size_t at = known; at < n; ++at) {
					Mint sum = 0;
					for (std::size_t k = 1; k <= at && k < input; ++k) {
						Mint x = in[k] * actual[at - k + 2];
						if (!differentiated) x *= Mint(k);
						sum += x;
					}
					require(actual[at + 2] == sum / Mint(at));
				}
				for (std::size_t i = 0; i < input; ++i) require(in[i].v == saved[i].v);
			}
		}
	}
#endif
}
