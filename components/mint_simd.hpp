#ifndef MINT_SIMD_HPP
#define MINT_SIMD_HPP

#include "../math/mint.hpp"
#if CP_X86
#include <immintrin.h>
#endif

namespace mint_detail {
struct SimdOps {
	typedef unsigned int U __attribute__((may_alias));
	static constexpr U mod = 998244353, twice = 2 * mod;
#if CP_X86
	static bool vectorized() {
#if defined(__AVX2__)
		return true;
#else
		static const bool available = __builtin_cpu_supports("avx2");
		return available;
#endif
	}
	using I8 = __m256i;
	[[gnu::target("avx2")]] static I8 load8(const U *a) {
		return _mm256_loadu_si256(reinterpret_cast<const I8 *>(a));
	}
	[[gnu::target("avx2")]] static void store8(U *a, I8 x) {
		_mm256_storeu_si256(reinterpret_cast<I8 *>(a), x);
	}
	__attribute__((target("avx2"), always_inline)) static inline I8 reduce8(I8 x, I8 y) {
		I8 r = _mm256_set1_epi32(998244351), p = _mm256_set1_epi32(mod);
		x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, r), p));
		y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, r), p));
		return _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
	}
	[[gnu::target("avx2")]] static I8 mul8(I8 a, I8 b) {
		return reduce8(_mm256_mul_epu32(a, b), _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
	}
	[[gnu::target("avx2")]] static I8 add8(I8 a, I8 b) {
		I8 x = _mm256_add_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(twice)));
	}
#endif
};
}

#endif
