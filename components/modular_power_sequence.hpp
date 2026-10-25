#ifndef MODULAR_POWER_SEQUENCE_HPP
#define MODULAR_POWER_SEQUENCE_HPP

#include "barrett32.hpp"
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace modular_power_sequence_detail {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2")))
inline void generate(std::uint32_t *out, std::size_t n, std::uint32_t root, const Barrett32 &arithmetic) {
	alignas(32) std::uint32_t first[8];
	first[0] = 1 % arithmetic.mod;
	for (int j = 1; j < 8; ++j) first[j] = arithmetic.mul(first[j - 1], root);
	const std::uint32_t step = arithmetic.mul(first[7], root);
	const auto factor = _mm256_set1_epi32(step), quotient = _mm256_set1_epi32((std::uint64_t(step) << 32) / arithmetic.mod);
	const auto mod = _mm256_set1_epi32(arithmetic.mod);
	auto x = _mm256_load_si256(reinterpret_cast<const __m256i *>(first));
	std::size_t i = 0;
	for (; i + 8 <= n; i += 8) {
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(out + i), x);
		const auto even = _mm256_srli_epi64(_mm256_mul_epu32(x, quotient), 32);
		const auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), quotient);
		const auto q = _mm256_blend_epi32(even, odd, 0xaa);
		x = _mm256_sub_epi32(_mm256_mullo_epi32(x, factor), _mm256_mullo_epi32(q, mod));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, mod));
	}
	std::uint32_t value = _mm256_extract_epi32(x, 0);
	for (; i < n; ++i) { out[i] = value; value = arithmetic.mul(value, root); }
}
#endif
inline std::vector<std::uint32_t> generate(std::size_t n, std::uint32_t root, std::uint32_t mod) {
	Barrett32 arithmetic(mod);
	std::vector<std::uint32_t> result(n);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (n >= 256 && mod < (std::uint32_t(1) << 31) && __builtin_cpu_supports("avx2")) {
		generate(result.data(), n, root, arithmetic);
		return result;
	}
#endif
	std::uint32_t value = 1 % mod;
	for (std::size_t i = 0; i < n; ++i) { result[i] = value; value = arithmetic.mul(value, root); }
	return result;
}
}

#endif
