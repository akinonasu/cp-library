#ifndef CP_COMPONENTS_FPS_OUTPUT_HPP
#define CP_COMPONENTS_FPS_OUTPUT_HPP

#include "mint_arithmetic.hpp"
#include <array>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#include <sys/mman.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {

inline void prepare_fps_output(std::vector<mint> &out, std::size_t n) {
	if (n <= out.capacity()) return;
	out.reserve(n);
	if (n - out.size() < (1u << 17)) return;
	const auto first = reinterpret_cast<std::uintptr_t>(out.data() + out.size());
	const auto end = reinterpret_cast<std::uintptr_t>(out.data() + n);
	[[maybe_unused]] const auto advise = [first, end](std::uintptr_t alignment, int advice) {
		const auto lo = (first + alignment - 1) & ~(alignment - 1), hi = end & ~(alignment - 1);
		if (lo < hi) static_cast<void>(madvise(reinterpret_cast<void *>(lo), hi - lo, advice));
	};
#if defined(MADV_HUGEPAGE)
	advise(2u * 1024 * 1024, MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
	advise(4096, MADV_POPULATE_WRITE);
#endif
}

template <bool montgomery>
inline void append_fps_words(std::vector<mint> &out, const unsigned *in, std::size_t n) {
	static_assert(sizeof(mint) == sizeof(unsigned) && std::is_trivially_copyable_v<mint>);
	if (n == 0) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + n);
	std::array<mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if constexpr (montgomery) std::memcpy(static_cast<void *>(batch.data()), in + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (int j = 0; j < 64; j += 8) {
				const auto x = mint_detail::Ops::mul8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i + j)), _mm256_set1_epi32(932051910u));
				std::memcpy(static_cast<void *>(batch.data() + j), &x, sizeof(x));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	for (; i < n; ++i) {
		if constexpr (montgomery) out.emplace_back(mint::word(in[i]));
		else out.emplace_back(mint::raw(in[i]));
	}
}
}
#pragma GCC pop_options
#endif
#endif
