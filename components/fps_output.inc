#ifndef CP_COMPONENTS_FPS_OUTPUT_HPP
#define CP_COMPONENTS_FPS_OUTPUT_HPP

#include "../math/mint.hpp"
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
namespace fps_output_detail {
inline __m256i montgomery(__m256i a) {
	const auto b = _mm256_set1_epi32(932051910u), ni = _mm256_set1_epi32(998244351u), p = _mm256_set1_epi32(Mint::mod());
	auto even = _mm256_mul_epu32(a, b), odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), b);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}
}

inline void prepare_fps_output(std::vector<Mint> &out, std::size_t n) {
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
inline void append_fps_words(std::vector<Mint> &out, const unsigned *in, std::size_t n) {
	static_assert(sizeof(Mint) == sizeof(unsigned) && std::is_trivially_copyable_v<Mint>);
	if (n == 0) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + n);
	std::array<Mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if constexpr (montgomery) std::memcpy(static_cast<void *>(batch.data()), in + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (int j = 0; j < 64; j += 8) {
				const auto x = fps_output_detail::montgomery(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i + j)));
				std::memcpy(static_cast<void *>(batch.data() + j), &x, sizeof(x));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	for (; i < n; ++i) {
		if constexpr (montgomery) out.emplace_back(Mint::word(in[i]));
		else out.emplace_back(Mint::raw(in[i]));
	}
}
}
#pragma GCC pop_options
#endif
#endif
