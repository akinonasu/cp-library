#ifndef SUBSET_CONVOLUTION_MODULAR_HPP
#define SUBSET_CONVOLUTION_MODULAR_HPP

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace subset_convolution_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<const T &>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 30) && (T::mod() & 1))
		&& std::is_convertible_v<decltype(std::declval<const T &>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(SUBSET_CONVOLUTION_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template <std::uint32_t mod>
struct Modular {
	using V = std::uint32_t;
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V sub(V a, V b) { return a >= b ? a - b : a + mod - b; }
	static constexpr int width = 24;
	__attribute__((target("avx2"))) static void transform(V *a, int n) {
		const __m256i p = _mm256_set1_epi32(mod);
		if (n >= 128) {
			int q = n / 4;
			for (int i = 0; i < q; ++i)
				for (int rank = 0; rank < width; rank += 8) {
					auto *p0 = reinterpret_cast<__m256i *>(a + std::size_t(i) * width + rank);
					auto *p1 = reinterpret_cast<__m256i *>(a + std::size_t(i + q) * width + rank);
					auto *p2 = reinterpret_cast<__m256i *>(a + std::size_t(i + q * 2) * width + rank);
					auto *p3 = reinterpret_cast<__m256i *>(a + std::size_t(i + q * 3) * width + rank);
					__m256i x0 = _mm256_loadu_si256(p0), x1 = _mm256_loadu_si256(p1), x2 = _mm256_loadu_si256(p2), x3 = _mm256_loadu_si256(p3);
					__m256i u = _mm256_add_epi32(x0, x2), v = _mm256_add_epi32(_mm256_sub_epi32(x0, x2), p);
					x0 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x2 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x1, x3);
					v = _mm256_add_epi32(_mm256_sub_epi32(x1, x3), p);
					x1 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x3 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x0, x1);
					v = _mm256_add_epi32(_mm256_sub_epi32(x0, x1), p);
					x0 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x1 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x2, x3);
					v = _mm256_add_epi32(_mm256_sub_epi32(x2, x3), p);
					x2 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x3 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					_mm256_storeu_si256(p0, x0);
					_mm256_storeu_si256(p1, x1);
					_mm256_storeu_si256(p2, x2);
					_mm256_storeu_si256(p3, x3);
				}
			for (int i = 0; i < 4; ++i) transform(a + std::size_t(i) * q * width, q);
			return;
		}
		for (int half = 1; half < n; half *= 2)
			for (int start = 0; start < n; start += 2 * half)
				for (int j = 0; j < half; ++j)
					for (int rank = 0; rank < width; rank += 8) {
						auto *left = reinterpret_cast<__m256i *>(a + std::size_t(start + j) * width + rank);
						auto *right = reinterpret_cast<__m256i *>(a + std::size_t(start + half + j) * width + rank);
						__m256i x = _mm256_loadu_si256(left), y = _mm256_loadu_si256(right);
						__m256i u = _mm256_add_epi32(x, y), v = _mm256_add_epi32(_mm256_sub_epi32(x, y), p);
						_mm256_storeu_si256(left, _mm256_min_epu32(u, _mm256_sub_epi32(u, p)));
						_mm256_storeu_si256(right, _mm256_min_epu32(v, _mm256_sub_epi32(v, p)));
					}
	}
	static constexpr V inverse = [] {
		V x = 1;
		for (int i = 0; i < 5; ++i) x *= 2 - mod * x;
		return V(0) - x;
	}();
	__attribute__((target("avx2"))) static __m256i reduce(__m256i x) {
		const __m256i p = _mm256_set1_epi64x(mod), inv = _mm256_set1_epi64x(inverse);
		__m256i q = _mm256_mul_epu32(x, inv);
		x = _mm256_srli_epi64(_mm256_add_epi64(x, _mm256_mul_epu32(q, p)), 32);
		x = _mm256_min_epu32(x, _mm256_sub_epi64(x, p));
		return _mm256_min_epu32(x, _mm256_sub_epi64(x, p));
	}
	__attribute__((target("avx2"))) static void product(V *a, const V *b, int n) {
		constexpr V r2 = V((~std::uint64_t(0) % mod + 1) % mod);
		const __m256i p = _mm256_set1_epi64x(mod), scale = _mm256_set1_epi64x(r2);
		for (int pos = 0; pos < n; pos += 4) {
			__m256i x[20], y[20], out[20];
			for (int rank = 0; rank < 20; rank += 4) {
				for (int side = 0; side < 2; ++side) {
					const V *v = (side ? b : a) + std::size_t(pos) * width + rank;
					__m128i a0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v));
					__m128i a1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width));
					__m128i a2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width * 2));
					__m128i a3 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width * 3));
					__m128i u = _mm_unpacklo_epi32(a0, a1), v0 = _mm_unpackhi_epi32(a0, a1), w = _mm_unpacklo_epi32(a2, a3), z = _mm_unpackhi_epi32(a2, a3);
					auto *dest = side ? y : x;
					dest[rank] = _mm256_cvtepu32_epi64(_mm_unpacklo_epi64(u, w));
					dest[rank + 1] = _mm256_cvtepu32_epi64(_mm_unpackhi_epi64(u, w));
					dest[rank + 2] = _mm256_cvtepu32_epi64(_mm_unpacklo_epi64(v0, z));
					dest[rank + 3] = _mm256_cvtepu32_epi64(_mm_unpackhi_epi64(v0, z));
				}
			}
			out[0] = _mm256_setzero_si256();
#pragma GCC unroll 20
			for (int rank = 1; rank < 20; ++rank) {
				__m256i value = _mm256_setzero_si256();
				for (int first = 0; first < rank; first += 8) {
					__m256i sum = _mm256_setzero_si256();
					for (int i = first; i < std::min(rank, first + 8); ++i)
						sum = _mm256_add_epi64(sum, _mm256_mul_epu32(x[i], y[rank - i - 1]));
					value = _mm256_add_epi64(value, reduce(sum));
					value = _mm256_min_epu32(value, _mm256_sub_epi64(value, p));
				}
				out[rank] = reduce(_mm256_mul_epu32(value, scale));
			}
			for (int rank = 0; rank < 20; rank += 4) {
				__m128i z[4];
				for (int j = 0; j < 4; ++j) z[j] = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(out[rank + j], _mm256_setr_epi32(0, 2, 4, 6, 0, 2, 4, 6)));
				__m128i u = _mm_unpacklo_epi32(z[0], z[1]), v = _mm_unpackhi_epi32(z[0], z[1]), w = _mm_unpacklo_epi32(z[2], z[3]), zz = _mm_unpackhi_epi32(z[2], z[3]);
				V *dest = a + std::size_t(pos) * width + rank;
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest), _mm_unpacklo_epi64(u, w));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width), _mm_unpackhi_epi64(u, w));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width * 2), _mm_unpacklo_epi64(v, zz));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width * 3), _mm_unpackhi_epi64(v, zz));
			}
		}
	}
	template <class T>
	__attribute__((target("avx2"))) static std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b, int n) {
		int levels = std::bit_width(unsigned(n)) - 1;
		int bottom = n >> (std::min(levels - 3, 2) + 1);
		std::vector<V> x(std::size_t(bottom) * width), y(x.size()), counts(n), result(n);
		std::vector<V> aa(n), bb(n);
		for (int i = 0; i < int(a.size()); ++i) aa[i] = a[i].val();
		for (int i = 0; i < int(b.size()); ++i) bb[i] = b[i].val();
		for (int i = 1; i < n; ++i) counts[i] = std::popcount(unsigned(i)) - 1;
		for (int top = 0; top < n / 2; top += bottom) {
			std::fill(x.begin(), x.end(), 0);
			std::fill(y.begin(), y.end(), 0);
			for (int start = 0; start < n; start += 2 * bottom) {
				bool negative = std::popcount(unsigned((start >> 1) & top)) & 1;
				for (int mask = start == 0; mask < 2 * bottom; ++mask) {
					std::size_t index = std::size_t(mask >> 1) * width + counts[start + mask];
					if (negative) x[index] = sub(x[index], aa[start + mask]), y[index] = sub(y[index], bb[start + mask]);
					else x[index] = add(x[index], aa[start + mask]), y[index] = add(y[index], bb[start + mask]);
				}
			}
			transform(x.data(), bottom);
			transform(y.data(), bottom);
			product(x.data(), y.data(), bottom);
			transform(x.data(), bottom);
			for (int start = 0; start < n; start += 2 * bottom) {
				bool negative = std::popcount(unsigned((start >> 1) & top)) & 1;
				for (int mask = start == 0; mask < 2 * bottom; ++mask) {
					V value = x[std::size_t(mask >> 1) * width + counts[start + mask]];
					result[start + mask] = negative ? sub(result[start + mask], value) : add(result[start + mask], value);
				}
			}
		}
		V scale = 1;
		for (int i = 1; i < levels; ++i) scale = std::uint64_t(scale) * ((mod + 1) / 2) % mod;
		std::vector<T> out(n, T(0));
		out[0] = T::raw(std::uint64_t(aa[0]) * bb[0] % mod);
		for (int i = 1; i < n; ++i) out[i] = T::raw((std::uint64_t(result[i]) * scale + std::uint64_t(aa[i]) * bb[0] + std::uint64_t(aa[0]) * bb[i]) % mod);
		return out;
	}
};
#endif
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
