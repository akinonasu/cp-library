#ifndef MATRIX_ARITHMETIC_HPP
#define MATRIX_ARITHMETIC_HPP

#include "matrix_base_arithmetic.hpp"
#include "field_arithmetic.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace matrix_detail {
using field_detail::SmallModint;
using field_detail::small_modint;
using field_detail::avx2;
using field_detail::ModularField;

template <class T, class F>
auto field(std::vector<T> &a, F &&f) {
	static_assert(!integer<T>);
	if constexpr (small_modint<T>) {
		std::vector<std::uint32_t> work(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) work[i] = a[i].val();
		auto result = f.template operator()<ModularField<T>>(work);
		for (std::size_t i = 0; i < a.size(); ++i) a[i] = T::raw(work[i]);
		return result;
	} else return f.template operator()<Field<T>>(a);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <std::uint32_t mod>
void product_scalar(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; j += 32) {
		int width = std::min(32, m - j);
		for (int k = 0; k < inner; k += step) {
			std::uint64_t sums[32]{};
			for (int t = k; t < std::min(inner, k + step); ++t) {
				std::uint64_t x = a[std::size_t(i) * inner + t];
				const auto *row = b + std::size_t(t) * m + j;
				for (int s = 0; s < width; ++s) sums[s] += x * row[s];
			}
			for (int s = 0; s < width; ++s) c[std::size_t(i) * m + j + s] = (sums[s] + c[std::size_t(i) * m + j + s]) % mod;
		}
	}
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
inline bool avx512_product() {
	static const bool supported = __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512dq");
	return avx2() && supported;
}

template <std::uint32_t mod>
__attribute__((target("avx512f,avx512dq"))) __m512i reduce_product_avx512(__m512i sum) {
	const auto p = _mm512_set1_epi64(mod);
	const auto q = _mm512_cvt_roundpd_epi64(
		_mm512_mul_pd(_mm512_cvtepi64_pd(sum), _mm512_set1_pd(1.0 / mod)),
		_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
	auto r = _mm512_sub_epi64(sum, _mm512_mullo_epi64(q, p));
	r = _mm512_mask_add_epi64(r, _mm512_cmp_epi64_mask(r, _mm512_setzero_si512(), _MM_CMPINT_LT), r, p);
	return _mm512_mask_sub_epi64(r, _mm512_cmp_epi64_mask(r, p, _MM_CMPINT_NLT), r, p);
}

template <std::uint32_t mod, int rows = 8>
__attribute__((target("avx512f,avx512dq"))) void product_packed_avx512(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr std::int64_t residue = (std::uint64_t(1) << 32) % mod;
	constexpr std::int64_t folded_bound = (std::int64_t(1) << 31) * residue + 0xffffffffu;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - folded_bound) / (radius * radius)));
	int i = 0;
	for (; i + rows <= n; i += rows) {
		for (int j = 0; j < m; j += 16) {
			const int width = std::min(16, m - j);
			const __mmask16 active = (1u << width) - 1;
			__m512i sums[rows][2];
			for (int r = 0; r < rows; ++r) {
				auto previous = _mm512_maskz_loadu_epi32(active, c + std::size_t(i + r) * m + j);
				sums[r][0] = _mm512_and_si512(previous, _mm512_set1_epi64(0xffffffffu));
				sums[r][1] = _mm512_srli_epi64(previous, 32);
			}
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) {
					auto x = _mm512_maskz_loadu_epi32(active, b + std::size_t(j) * inner + std::size_t(t) * width);
					auto y = _mm512_srli_epi64(x, 32);
					for (int r = 0; r < rows; ++r) {
						auto z = _mm512_set1_epi32(a[std::size_t(i + r) * inner + t]);
						sums[r][0] = _mm512_add_epi64(sums[r][0], _mm512_mul_epi32(x, z));
						sums[r][1] = _mm512_add_epi64(sums[r][1], _mm512_mul_epi32(y, z));
					}
				}
				for (int r = 0; r < rows; ++r) for (int q = 0; q < 2; ++q) {
					auto x = sums[r][q];
					sums[r][q] = _mm512_add_epi64(_mm512_mul_epi32(_mm512_srli_epi64(x, 32), _mm512_set1_epi32(residue)), _mm512_and_si512(x, _mm512_set1_epi64(0xffffffffu)));
				}
			}
			for (int r = 0; r < rows; ++r) {
				auto even = reduce_product_avx512<mod>(sums[r][0]), odd = reduce_product_avx512<mod>(sums[r][1]);
				_mm512_mask_storeu_epi32(c + std::size_t(i + r) * m + j, active, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
			}
		}
	}
	if constexpr (rows > 1) {
		if (i < n) product_packed_avx512<mod, rows / 2>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	}
}

template <std::uint32_t mod, int rows = 4>
__attribute__((target("avx2"))) void product_packed_avx2(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr std::int64_t residue = (std::uint64_t(1) << 32) % mod;
	constexpr std::int64_t folded_bound = (std::int64_t(1) << 31) * residue + 0xffffffffu;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - folded_bound) / (radius * radius)));
	auto reduce = [](std::int64_t sum) {
		std::int64_t r = sum % std::int64_t(mod);
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	int i = 0;
	for (; i + rows <= n; i += rows) {
		for (int j = 0; j < m; j += 8) {
			int width = std::min(8, m - j);
			auto active = _mm256_cmpgt_epi32(_mm256_set1_epi32(width), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
			__m256i sums[rows][2];
			for (int r = 0; r < rows; ++r) {
				auto previous = _mm256_maskload_epi32(reinterpret_cast<const int *>(c + std::size_t(i + r) * m + j), active);
				sums[r][0] = _mm256_and_si256(previous, _mm256_set1_epi64x(0xffffffffu));
				sums[r][1] = _mm256_srli_epi64(previous, 32);
			}
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) {
					auto x = _mm256_maskload_epi32(b + std::size_t(j) * inner + std::size_t(t) * width, active);
					auto y = _mm256_srli_epi64(x, 32);
					for (int r = 0; r < rows; ++r) {
						auto z = _mm256_set1_epi32(a[std::size_t(i + r) * inner + t]);
						sums[r][0] = _mm256_add_epi64(sums[r][0], _mm256_mul_epi32(x, z));
						sums[r][1] = _mm256_add_epi64(sums[r][1], _mm256_mul_epi32(y, z));
					}
				}
				for (int r = 0; r < rows; ++r) for (int q = 0; q < 2; ++q) {
					auto x = sums[r][q];
					sums[r][q] = _mm256_add_epi64(_mm256_mul_epi32(_mm256_srli_epi64(x, 32), _mm256_set1_epi32(residue)), _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffu)));
				}
			}
			for (int r = 0; r < rows; ++r) {
				std::int64_t even[4], odd[4];
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), sums[r][0]);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), sums[r][1]);
				std::uint32_t result[8];
				for (int q = 0; q < 4; ++q) result[q * 2] = reduce(even[q]), result[q * 2 + 1] = reduce(odd[q]);
				_mm256_maskstore_epi32(reinterpret_cast<int *>(c + std::size_t(i + r) * m + j), active, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(result)));
			}
		}

	}
	if constexpr (rows == 4) {
		if (i < n) product_packed_avx2<mod, 1>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	}
}


#endif

template <class T>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	if (!n || !inner || !m) return c;
	auto basic = [&] {
		for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
			const T &x = a[std::size_t(i) * inner + k];
			for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
		}
	};
	if constexpr (small_modint<T>) {
		if (std::uint64_t(n) * m <= 128u / unsigned(inner)) { basic(); return c; }
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (inner >= 16 && m >= 8 && avx2()) {
			const int block = avx512_product() ? 16 : 8;
			std::vector<std::int32_t> x(a.size()), y(b.size());
			std::vector<std::uint32_t> z(c.size());
			auto center = [](std::uint32_t value) {
				std::int32_t v = value;
				return v <= std::int32_t((T::mod() - 1) / 2) ? v : v - std::int32_t(T::mod());
			};
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = center(a[i].val());
			for (int j = 0; j < m; j += block) {
				int width = std::min(block, m - j);
				for (int k = 0; k < inner; ++k) for (int s = 0; s < width; ++s)
					y[std::size_t(j) * inner + std::size_t(k) * width + s] = center(b[std::size_t(k) * m + j + s].val());
			}
			if (block == 16) product_packed_avx512<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			else product_packed_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
			return c;
		}
#endif
		std::vector<std::uint32_t> x(a.size()), y(b.size()), z(c.size());
		for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
		for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
		product_scalar<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
	} else basic();
	return c;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
}

#endif
