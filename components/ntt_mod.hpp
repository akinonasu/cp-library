#ifndef NTT_MOD_HPP
#define NTT_MOD_HPP

#include "ntt_mod_core.hpp"

namespace ntt_mod_detail {
inline void bit_reverse(std::vector<std::uint32_t> &a) {
	for (std::size_t i = 1, j = 0; i < a.size(); ++i) {
		std::size_t bit = a.size() >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
}

template <std::uint32_t mod, std::uint32_t root>
void transform(std::vector<std::uint32_t> &a, bool inverse, bool vectorized) {
	Plan<mod, root> plan(a.size(), vectorized);
	if (inverse) {
		bit_reverse(a);
		backward(a, plan, vectorized);
		std::reverse(a.begin() + 1, a.end());
		scale<mod>(a, mod - (mod - 1) / a.size(), vectorized);
	} else {
		forward(a, plan, vectorized);
		if constexpr (mod < (std::uint32_t(1) << 30)) for (auto &x : a) if (x >= mod) x -= mod;
		bit_reverse(a);
	}
}

}

template <std::uint32_t mod, std::uint32_t root>
void ntt_mod(std::vector<std::uint32_t> &a, bool inverse = false) {
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::transform<mod, root>(a, inverse, vectorized);
}

#endif
