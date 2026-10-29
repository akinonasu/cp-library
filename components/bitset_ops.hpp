#ifndef BITSET_OPS_HPP
#define BITSET_OPS_HPP

#include <bit>
#include <cstddef>
#include <cstdint>

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace bitset_ops_detail {
enum class Operation { identity, bit_and, bit_or, bit_xor, and_not };

template <Operation Op> inline std::uint64_t combine(std::uint64_t a, std::uint64_t b) {
	if constexpr (Op == Operation::identity) return a;
	if constexpr (Op == Operation::bit_and) return a & b;
	if constexpr (Op == Operation::bit_or) return a | b;
	if constexpr (Op == Operation::bit_xor) return a ^ b;
	if constexpr (Op == Operation::and_not) return a & ~b;
}

template <Operation Op> inline std::size_t scalar(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	std::size_t result = 0;
	for (std::size_t i = 0; i < n; ++i) {
		if constexpr (Op == Operation::identity) result += std::popcount(a[i]);
		else result += std::popcount(combine<Op>(a[i], b[i]));
	}
	return result;
}

template <Operation Op>
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("O3")))
#endif
inline std::size_t scalar_large(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	return scalar<Op>(a, b, n);
}

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
template <Operation Op> __attribute__((target("avx512f,avx512vpopcntdq")))
inline std::size_t avx512(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	__m512i sum = _mm512_setzero_si512();
	std::size_t i = 0;
	for (; n - i >= 8; i += 8) {
		__m512i x = _mm512_loadu_si512(a + i);
		if constexpr (Op != Operation::identity) {
			__m512i y = _mm512_loadu_si512(b + i);
			if constexpr (Op == Operation::bit_and) x = _mm512_and_si512(x, y);
			if constexpr (Op == Operation::bit_or) x = _mm512_or_si512(x, y);
			if constexpr (Op == Operation::bit_xor) x = _mm512_xor_si512(x, y);
			if constexpr (Op == Operation::and_not) x = _mm512_andnot_si512(y, x);
		}
		sum = _mm512_add_epi64(sum, _mm512_popcnt_epi64(x));
	}
	return std::size_t(_mm512_reduce_add_epi64(sum)) + scalar<Op>(a + i, b ? b + i : nullptr, n - i);
}

template <Operation Op> __attribute__((target("avx2")))
inline std::size_t avx2(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	const __m256i lookup = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
	const __m256i mask = _mm256_set1_epi8(15), zero = _mm256_setzero_si256();
	__m256i sum = zero;
	std::size_t i = 0;
	for (; n - i >= 4; i += 4) {
		__m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		if constexpr (Op != Operation::identity) {
			__m256i y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
			if constexpr (Op == Operation::bit_and) x = _mm256_and_si256(x, y);
			if constexpr (Op == Operation::bit_or) x = _mm256_or_si256(x, y);
			if constexpr (Op == Operation::bit_xor) x = _mm256_xor_si256(x, y);
			if constexpr (Op == Operation::and_not) x = _mm256_andnot_si256(y, x);
		}
		__m256i low = _mm256_shuffle_epi8(lookup, _mm256_and_si256(x, mask));
		__m256i high = _mm256_shuffle_epi8(lookup, _mm256_and_si256(_mm256_srli_epi16(x, 4), mask));
		sum = _mm256_add_epi64(sum, _mm256_sad_epu8(_mm256_add_epi8(low, high), zero));
	}
	alignas(32) std::uint64_t lanes[4];
	_mm256_store_si256(reinterpret_cast<__m256i *>(lanes), sum);
	return std::size_t(lanes[0] + lanes[1] + lanes[2] + lanes[3]) + scalar<Op>(a + i, b ? b + i : nullptr, n - i);
}

inline bool has_avx512() {
	static const bool enabled = __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vpopcntdq");
	return enabled;
}
inline bool has_avx2() {
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
}
#endif

template <Operation Op>
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("O3")))
#endif
inline void apply_large(std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	static_assert(Op != Operation::identity);
	for (std::size_t i = 0; i < n; ++i) a[i] = combine<Op>(a[i], b[i]);
}

template <Operation Op> inline void apply(std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
	static_assert(Op != Operation::identity);
	if (n >= 32) return apply_large<Op>(a, b, n);
	for (std::size_t i = 0; i < n; ++i) a[i] = combine<Op>(a[i], b[i]);
}

template <Operation Op> inline std::size_t count(const std::uint64_t *a, const std::uint64_t *b, std::size_t n) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (n >= 64) {
		if (has_avx512()) return avx512<Op>(a, b, n);
		if (has_avx2()) return avx2<Op>(a, b, n);
	}
#endif
	if (n >= 8) return scalar_large<Op>(a, b, n);
	return scalar<Op>(a, b, n);
}
}

#endif
