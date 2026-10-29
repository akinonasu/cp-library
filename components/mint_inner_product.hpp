#ifndef MINT_INNER_PRODUCT_HPP
#define MINT_INNER_PRODUCT_HPP

#include "../math/mint.hpp"
#include <cstdint>
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#endif

namespace mint_detail {
template<bool Reverse> inline __uint128_t inner_product_scalar(const mint* a, const mint* b, int size) {
	__uint128_t sum = 0;
	int i = 0;
	for (; i <= size - 4; i += 4) {
		std::uint64_t part = std::uint64_t(a[i].v) * b[Reverse ? -i : i].v
			+ std::uint64_t(a[i + 1].v) * b[Reverse ? -i - 1 : i + 1].v
			+ std::uint64_t(a[i + 2].v) * b[Reverse ? -i - 2 : i + 2].v
			+ std::uint64_t(a[i + 3].v) * b[Reverse ? -i - 3 : i + 3].v;
		sum += part;
	}
	for (; i < size; ++i) sum += std::uint64_t(a[i].v) * b[Reverse ? -i : i].v;
	return sum;
}

#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
inline bool inner_product_has_avx2() {
#if defined(__AVX2__)
	return true;
#else
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#endif
}

inline bool inner_product_has_avx512() {
#if defined(__AVX512F__)
	return true;
#else
	static const bool enabled = __builtin_cpu_supports("avx512f");
	return enabled;
#endif
}

template<bool Reverse> __attribute__((target("avx2")))
__uint128_t inner_product_avx2(const mint* a, const mint* b, int size) {
	const auto order = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	const auto zero = _mm256_setzero_si256();
	auto total = zero, carry = zero;
	int i = 0;
	for (; i <= size - 8; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + (Reverse ? -i - 7 : i)));
		if constexpr (Reverse) y = _mm256_permutevar8x32_epi32(y, order);
		auto even = _mm256_mul_epu32(x, y);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
		auto next = _mm256_add_epi64(total, _mm256_add_epi64(even, odd));
		carry = _mm256_sub_epi64(carry, _mm256_cmpgt_epi64(zero, _mm256_andnot_si256(next, total)));
		total = next;
	}
	alignas(32) std::uint64_t low[4], high[4];
	_mm256_store_si256(reinterpret_cast<__m256i*>(low), total);
	_mm256_store_si256(reinterpret_cast<__m256i*>(high), carry);
	__uint128_t sum = 0;
	if (i < size) sum = inner_product_scalar<Reverse>(a + i, b + (Reverse ? -i : i), size - i);
	for (int j = 0; j < 4; ++j) sum += (__uint128_t(high[j]) << 64) + low[j];
	return sum;
}

template<bool Reverse> __attribute__((target("avx512f")))
__uint128_t inner_product_avx512(const mint* a, const mint* b, int size) {
	const auto order = _mm512_setr_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	const auto zero = _mm512_setzero_si512();
	const auto one = _mm512_set1_epi64(1);
	auto total = zero, carry = zero;
	int i = 0;
	for (; i <= size - 16; i += 16) {
		auto x = _mm512_loadu_si512(a + i);
		auto y = _mm512_loadu_si512(b + (Reverse ? -i - 15 : i));
		if constexpr (Reverse) y = _mm512_permutexvar_epi32(order, y);
		auto even = _mm512_mul_epu32(x, y);
		auto odd = _mm512_mul_epu32(_mm512_srli_epi64(x, 32), _mm512_srli_epi64(y, 32));
		auto next = _mm512_add_epi64(total, _mm512_add_epi64(even, odd));
		carry = _mm512_mask_add_epi64(carry, _mm512_cmplt_epu64_mask(next, total), carry, one);
		total = next;
	}
	alignas(64) std::uint64_t low[8], high[8];
	_mm512_store_si512(low, total);
	_mm512_store_si512(high, carry);
	__uint128_t sum = 0;
	if (i < size) sum = inner_product_scalar<Reverse>(a + i, b + (Reverse ? -i : i), size - i);
	for (int j = 0; j < 8; ++j) sum += (__uint128_t(high[j]) << 64) + low[j];
	return sum;
}
#endif

template<bool Reverse = false> mint inner_product(const mint* a, const mint* b, int size) {
	assert(size >= 0);
	__uint128_t sum;
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
	if (size >= 64 && inner_product_has_avx512()) sum = inner_product_avx512<Reverse>(a, b, size);
	else if (size >= 64 && inner_product_has_avx2()) sum = inner_product_avx2<Reverse>(a, b, size);
	else
#endif
		sum = inner_product_scalar<Reverse>(a, b, size);
	return mint::word(mint::reduce(std::uint64_t(sum % mint::mod())));
}
}

#endif
