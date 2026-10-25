#ifndef WARSHALL_FLOYD_KERNEL_HPP
#define WARSHALL_FLOYD_KERNEL_HPP

#include "aligned_allocator.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace warshall_floyd_detail {
template<class T> using Storage = std::vector<T, AlignedAllocator<T>>;

template<class T> struct Unsigned { using type = std::make_unsigned_t<T>; };
template<> struct Unsigned<__int128_t> { using type = __uint128_t; };

inline int backend() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(WARSHALL_FLOYD_DISABLE_SIMD)
#ifndef WARSHALL_FLOYD_DISABLE_AVX512
	if (__builtin_cpu_supports("avx512f")) return 2;
#endif
	if (__builtin_cpu_supports("avx2")) return 1;
#endif
	return 0;
}

template<class T, int B, bool Negative, bool Diagonal>
bool scalar(T* out, const T* a, const T* b, T inf) {
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if (x == inf) continue;
			for (int j = 0; j < B; ++j) {
				if constexpr (Negative) {
					if (b[k * B + j] != inf) out[i * B + j] = std::min(out[i * B + j], T(x + b[k * B + j]));
				} else {
					using U = typename Unsigned<T>::type;
					out[i * B + j] = T(std::min(U(out[i * B + j]), U(x) + U(b[k * B + j])));
				}
			}
		}
	}
	return true;
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template<class T, int B, bool Negative, bool Diagonal>
__attribute__((target("avx512f")))
bool simple512(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 64 / sizeof(T);
	[[maybe_unused]] __m512i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm512_set1_epi32(inf);
	else infinity = _mm512_set1_epi64(inf);
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if constexpr (Negative) if (x == inf) continue;
			__m512i alpha;
			if constexpr (sizeof(T) == 4) alpha = _mm512_set1_epi32(x);
			else alpha = _mm512_set1_epi64(x);
			for (int j = 0; j < B; j += lanes) {
				auto value = _mm512_loadu_si512(out + i * B + j);
				auto beta = _mm512_loadu_si512(b + k * B + j);
				[[maybe_unused]] __mmask16 valid = 0;
				if constexpr (Negative && sizeof(T) == 4) valid = _mm512_cmpneq_epi32_mask(beta, infinity);
				if constexpr (Negative && sizeof(T) == 8) valid = _mm512_cmpneq_epi64_mask(beta, infinity);
				if constexpr (sizeof(T) == 4) {
					auto sum = _mm512_add_epi32(alpha, beta);
					if constexpr (Negative) value = _mm512_mask_min_epi32(value, valid, value, sum);
					else value = _mm512_min_epu32(value, sum);
				} else {
					auto sum = _mm512_add_epi64(alpha, beta);
					if constexpr (Negative) value = _mm512_mask_min_epi64(value, __mmask8(valid), value, sum);
					else value = _mm512_min_epu64(value, sum);
				}
				_mm512_storeu_si512(out + i * B + j, value);
			}
		}
	}
	return true;
}

template<class T, int B, bool Negative>
__attribute__((target("avx512f")))
void product512(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 64 / sizeof(T), rows = 4, columns = 2;
	[[maybe_unused]] __m512i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm512_set1_epi32(inf);
	else infinity = _mm512_set1_epi64(inf);
	for (int i = 0; i < B; i += rows) for (int j = 0; j < B; j += lanes * columns) {
		__m512i value[rows][columns];
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			value[r][q] = _mm512_loadu_si512(out + (i + r) * B + j + q * lanes);
		for (int k = 0; k < B; ++k) {
			__m512i beta[columns];
			[[maybe_unused]] __mmask16 valid[columns]{};
			for (int q = 0; q < columns; ++q) {
				beta[q] = _mm512_loadu_si512(b + k * B + j + q * lanes);
				__mmask16 mask = 0;
				if constexpr (Negative && sizeof(T) == 4) mask = _mm512_cmpneq_epi32_mask(beta[q], infinity);
				if constexpr (Negative && sizeof(T) == 8) mask = _mm512_cmpneq_epi64_mask(beta[q], infinity);
				valid[q] = mask;
			}
			for (int r = 0; r < rows; ++r) {
				T x = a[(i + r) * B + k];
				if constexpr (Negative) if (x == inf) continue;
				__m512i alpha;
				if constexpr (sizeof(T) == 4) alpha = _mm512_set1_epi32(x);
				else alpha = _mm512_set1_epi64(x);
				for (int q = 0; q < columns; ++q) {
					if constexpr (sizeof(T) == 4) {
						auto sum = _mm512_add_epi32(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm512_mask_min_epi32(value[r][q], valid[q], value[r][q], sum);
						else value[r][q] = _mm512_min_epu32(value[r][q], sum);
					} else {
						auto sum = _mm512_add_epi64(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm512_mask_min_epi64(value[r][q], __mmask8(valid[q]), value[r][q], sum);
						else value[r][q] = _mm512_min_epu64(value[r][q], sum);
					}
				}
			}
		}
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			_mm512_storeu_si512(out + (i + r) * B + j + q * lanes, value[r][q]);
	}
}

template<class T, int B, bool Negative, bool Diagonal>
__attribute__((target("avx2")))
bool simple256(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 32 / sizeof(T);
	[[maybe_unused]] __m256i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm256_set1_epi32(inf);
	else infinity = _mm256_set1_epi64x(inf);
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if constexpr (Negative) if (x == inf) continue;
			__m256i alpha;
			if constexpr (sizeof(T) == 4) alpha = _mm256_set1_epi32(x);
			else alpha = _mm256_set1_epi64x(x);
			for (int j = 0; j < B; j += lanes) {
				auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out + i * B + j));
				auto beta = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + k * B + j));
				[[maybe_unused]] __m256i valid = _mm256_setzero_si256();
				if constexpr (Negative && sizeof(T) == 4) valid = _mm256_cmpeq_epi32(beta, infinity);
				if constexpr (Negative && sizeof(T) == 8) valid = _mm256_cmpeq_epi64(beta, infinity);
				if constexpr (sizeof(T) == 4) {
					auto sum = _mm256_add_epi32(alpha, beta);
					if constexpr (Negative) value = _mm256_min_epi32(value, _mm256_blendv_epi8(sum, infinity, valid));
					else value = _mm256_min_epu32(value, sum);
				} else {
					auto sum = _mm256_add_epi64(alpha, beta);
					if constexpr (Negative) {
						sum = _mm256_blendv_epi8(sum, infinity, valid);
						value = _mm256_blendv_epi8(value, sum, _mm256_cmpgt_epi64(value, sum));
					} else {
						auto choose = _mm256_andnot_si256(_mm256_cmpgt_epi64(_mm256_setzero_si256(), sum), _mm256_cmpgt_epi64(value, sum));
						value = _mm256_blendv_epi8(value, sum, choose);
					}
				}
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(out + i * B + j), value);
			}
		}
	}
	return true;
}

template<class T, int B, bool Negative>
__attribute__((target("avx2")))
void product256(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 32 / sizeof(T), rows = 4, columns = 2;
	[[maybe_unused]] __m256i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm256_set1_epi32(inf);
	else infinity = _mm256_set1_epi64x(inf);
	for (int i = 0; i < B; i += rows) for (int j = 0; j < B; j += lanes * columns) {
		__m256i value[rows][columns];
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			value[r][q] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out + (i + r) * B + j + q * lanes));
		for (int k = 0; k < B; ++k) {
			__m256i beta[columns];
			[[maybe_unused]] __m256i valid[columns]{};
			for (int q = 0; q < columns; ++q) {
				beta[q] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + k * B + j + q * lanes));
				__m256i mask = _mm256_setzero_si256();
				if constexpr (Negative && sizeof(T) == 4) mask = _mm256_cmpeq_epi32(beta[q], infinity);
				if constexpr (Negative && sizeof(T) == 8) mask = _mm256_cmpeq_epi64(beta[q], infinity);
				valid[q] = mask;
			}
			for (int r = 0; r < rows; ++r) {
				T x = a[(i + r) * B + k];
				if constexpr (Negative) if (x == inf) continue;
				__m256i alpha;
				if constexpr (sizeof(T) == 4) alpha = _mm256_set1_epi32(x);
				else alpha = _mm256_set1_epi64x(x);
				for (int q = 0; q < columns; ++q) {
					if constexpr (sizeof(T) == 4) {
						auto sum = _mm256_add_epi32(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm256_min_epi32(value[r][q], _mm256_blendv_epi8(sum, infinity, valid[q]));
						else value[r][q] = _mm256_min_epu32(value[r][q], sum);
					} else {
						auto sum = _mm256_add_epi64(alpha, beta[q]);
						if constexpr (Negative) {
							sum = _mm256_blendv_epi8(sum, infinity, valid[q]);
							value[r][q] = _mm256_blendv_epi8(value[r][q], sum, _mm256_cmpgt_epi64(value[r][q], sum));
						} else {
							auto choose = _mm256_andnot_si256(_mm256_cmpgt_epi64(_mm256_setzero_si256(), sum), _mm256_cmpgt_epi64(value[r][q], sum));
							value[r][q] = _mm256_blendv_epi8(value[r][q], sum, choose);
						}
					}
				}
			}
		}
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(out + (i + r) * B + j + q * lanes), value[r][q]);
	}
}

#endif

template<class T, int B, bool Negative, int Backend>
struct Kernel {
	static bool close(T* out, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) return simple512<T, B, Negative, true>(out, out, out, inf);
		if constexpr (Backend == 1) return simple256<T, B, Negative, true>(out, out, out, inf);
#endif
		return scalar<T, B, Negative, true>(out, out, out, inf);
	}
	static void edge(T* out, const T* a, const T* b, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) { simple512<T, B, Negative, false>(out, a, b, inf); return; }
		if constexpr (Backend == 1) { simple256<T, B, Negative, false>(out, a, b, inf); return; }
#endif
		scalar<T, B, Negative, false>(out, a, b, inf);
	}
	static void product(T* out, const T* a, const T* b, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) { product512<T, B, Negative>(out, a, b, inf); return; }
		if constexpr (Backend == 1) { product256<T, B, Negative>(out, a, b, inf); return; }
#endif
		scalar<T, B, Negative, false>(out, a, b, inf);
	}
};

template<class T, int B, bool Negative, int Backend>
bool solve(T* data, int blocks, T inf) {
	using K = Kernel<T, B, Negative, Backend>;
	auto tile = [&](int i, int j) { return data + (std::size_t(i) * blocks + j) * B * B; };
	auto product = [&](int row, int col, int pivot, int rows, int cols, int inner) {
		for (int k = pivot; k < pivot + inner; ++k)
			for (int i = row; i < row + rows; ++i)
				for (int j = col; j < col + cols; ++j) K::product(tile(i, j), tile(i, k), tile(k, j), inf);
	};
	constexpr int macro = 16;
	for (int base = 0; base < blocks; base += macro) {
		int size = std::min(macro, blocks - base);
		for (int k = base; k < base + size; ++k) {
			if (!K::close(tile(k, k), inf)) return false;
			for (int j = base; j < base + size; ++j) if (j != k) K::edge(tile(k, j), tile(k, k), tile(k, j), inf);
			for (int i = base; i < base + size; ++i) if (i != k) K::edge(tile(i, k), tile(i, k), tile(k, k), inf);
			for (int i = base; i < base + size; ++i) if (i != k)
				for (int j = base; j < base + size; ++j) if (j != k) K::product(tile(i, j), tile(i, k), tile(k, j), inf);
		}
		for (int j = 0; j < blocks; j += macro) if (j != base)
			product(base, j, base, size, std::min(macro, blocks - j), size);
		for (int i = 0; i < blocks; i += macro) if (i != base)
			product(i, base, base, std::min(macro, blocks - i), size, size);
		for (int i = 0; i < blocks; i += macro) if (i != base)
			for (int j = 0; j < blocks; j += macro) if (j != base)
				product(i, j, base, std::min(macro, blocks - i), std::min(macro, blocks - j), size);
	}
	return true;
}

template<class T, int B, bool Negative>
bool run(T* data, int blocks, T inf, int selected = backend()) {
	if constexpr (sizeof(T) <= 8) {
		if (selected == 2) return solve<T, B, Negative, 2>(data, blocks, inf);
		if (selected == 1) return solve<T, B, Negative, 1>(data, blocks, inf);
	}
	return solve<T, B, Negative, 0>(data, blocks, inf);
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
