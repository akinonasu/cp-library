#ifndef SEQUENCE_COMPARE_HPP
#define SEQUENCE_COMPARE_HPP

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace sequence_compare_detail {
#if defined(__x86_64__)
__attribute__((target("avx2"))) inline std::size_t forward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countr_zero(difference);
	}
	while (i < size && a[i] == b[i]) ++i;
	return i;
}

__attribute__((target("avx2"))) inline std::size_t backward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a - i - 32));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b - i - 32));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countl_zero(difference);
	}
	while (i < size && a[-std::ptrdiff_t(i) - 1] == b[-std::ptrdiff_t(i) - 1]) ++i;
	return i;
}
#endif

template<bool Vectorize = true, class T>
inline int prefix(const T *first, const T *second, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first);
		const auto *b = reinterpret_cast<const unsigned char *>(second);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a + offset, 8);
			std::memcpy(&y, b + offset, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countr_zero(difference) : std::countl_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + forward_avx2(a + offset, b + offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first[matched] == second[matched]) ++matched;
	return matched;
}

template<bool Vectorize = true, class T>
inline int suffix(const T *first_end, const T *second_end, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first_end);
		const auto *b = reinterpret_cast<const unsigned char *>(second_end);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a - offset - 8, 8);
			std::memcpy(&y, b - offset - 8, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countl_zero(difference) : std::countr_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + backward_avx2(a - offset, b - offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first_end[-matched - 1] == second_end[-matched - 1]) ++matched;
	return matched;
}
}

#endif
