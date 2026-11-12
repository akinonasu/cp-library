#ifndef CONVOLUTION_MOD_HPP
#define CONVOLUTION_MOD_HPP

namespace convolution_mod_detail { struct FullBackend; }
#ifndef CP_CONVOLUTION_MOD_BACKEND
#define CP_CONVOLUTION_MOD_BACKEND 1
namespace convolution_mod_detail { using DefaultBackend = FullBackend; }
#endif

#include "../components/convolution_mod_core.hpp"
#include "../components/ntt_mod.hpp"
#include "../components/ntt_mod_blocked.hpp"

namespace convolution_mod_detail {
struct FullBackend {
	static constexpr bool specialized = true;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	static constexpr bool accelerated = true;
	static bool enabled(std::size_t smaller, std::size_t n) { return smaller > 32 && n >= 1024 && __builtin_cpu_supports("avx2"); }
	template <Word P, Word Root, class T>
	static std::vector<Word> multiply(const std::vector<T> &a, const std::vector<T> &b, Word factor, Word *workspace) {
		return ntt_mod_detail::multiply_blocked<P, Root>(a, b, factor, workspace);
	}
#else
	static constexpr bool accelerated = false;
#endif
	template <Word... Mods>
	static fps_detail::FpsWords workspace(std::size_t a, std::size_t b) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (((Mods < (Word(1) << 30)) && ...)) {
			const auto smaller = std::min(a, b), larger = std::max(a, b);
			const auto n = std::bit_ceil(larger / smaller >= 8 ? 2 * smaller - 1 : a + b - 1);
			if (enabled(smaller, n)) return fps_detail::allocate_fps_words(ntt_mod_detail::blocked_workspace_words(n));
		}
#else
		(void)a; (void)b;
#endif
		return {};
	}
	template <Word P, Word PR, Word Q, Word QR>
	static std::vector<Word> pair(const std::vector<Word> &a, const std::vector<Word> &b, Word mod) {
		static_assert(P < Q && 2 * std::uint64_t(Q) < (std::uint64_t(1) << 32));
		auto storage = workspace<P, Q>(a.size(), b.size());
		auto x = multiply_as<FullBackend, P, PR>(a, b, 1, storage.get());
		auto y = multiply_as<FullBackend, Q, QR>(a, b, 1, storage.get());
		static const Word inverse = ntt_mod_detail::power<Q>(P, Q - 2);
		for (std::size_t i = 0; i < x.size(); ++i) {
			std::uint64_t digit = std::uint64_t(y[i] + Q - x[i]) * inverse % Q;
			x[i] = (x[i] + std::uint64_t(P) * digit) % mod;
		}
		return x;
	}
	static bool fits(Word mod, __uint128_t bound) {
		return mod == 167772161 || mod == 469762049 || mod == 754974721 || bound < 469762049ULL * 754974721;
	}
	static std::vector<Word> convolution(const std::vector<Word> &a, const std::vector<Word> &b, Word mod, __uint128_t bound) {
		if (mod == 167772161) return multiply_as<FullBackend, 167772161, 3>(a, b);
		if (mod == 469762049) return multiply_as<FullBackend, 469762049, 3>(a, b);
		if (mod == 754974721) return multiply_as<FullBackend, 754974721, 11>(a, b);
		if (bound < 754974721) {
			auto result = multiply_as<FullBackend, 754974721, 11>(a, b);
			for (auto &value : result) value %= mod;
			return result;
		}
		return pair<469762049, 3, 754974721, 11>(a, b, mod);
	}
};
template <Word P, Word Root, class T>
std::vector<Word> multiply(const std::vector<T> &a, const std::vector<T> &b, Word factor = 1, Word *workspace = nullptr) {
	return multiply_as<FullBackend, P, Root>(a, b, factor, workspace);
}
}

#endif
