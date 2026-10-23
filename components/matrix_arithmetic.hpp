#ifndef MATRIX_ARITHMETIC_HPP
#define MATRIX_ARITHMETIC_HPP

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
template <class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;

using field_detail::SmallModint;
using field_detail::small_modint;
using field_detail::avx2;
using field_detail::Field;
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

template <class Ops>
struct Elimination {
	std::vector<int> pivots;
	typename Ops::V determinant = 1;
};

template <class Ops, bool exact = false>
Elimination<Ops> eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
	using V = typename Ops::V;
	Elimination<Ops> result;
	result.pivots.reserve(std::min(h, columns));
	int rank = 0;
	for (int col = 0; col < columns && rank < h; ++col) {
		int p = -1;
		for (int i = rank; i < h; ++i) {
			V x = a[std::size_t(i) * stride + col];
			if (!Ops::zero(x, eps) && (p < 0 || Ops::better(x, a[std::size_t(p) * stride + col]))) p = i;
		}
		if (p < 0) continue;
		V *row = a.data() + std::size_t(rank) * stride;
		if (p != rank) {
			std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
			result.determinant = Ops::neg(result.determinant);
		}
		result.determinant = Ops::mul(result.determinant, row[col]);
		Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
		row[col] = V(1);
		for (int i = rank + 1; i < h; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps)) Ops::axpy(dst + col + 1, row + col + 1, stride - col - 1, Ops::neg(x));
			dst[col] = V(0);
		}
		result.pivots.push_back(col);
		++rank;
	}
	bool full = exact && rank == columns;
	if (reduced) for (int k = rank - 1; k >= 0; --k) {
		int col = result.pivots[k], start = full ? columns : col + 1;
		const V *row = a.data() + std::size_t(k) * stride;
		for (int i = 0; i < k; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps) && (!full || start < stride)) Ops::axpy(dst + start, row + start, stride - start, Ops::neg(x));
			dst[col] = V(0);
		}
	}
	return result;
}

template <class T>
[[gnu::noinline]] T integer_determinant(const std::vector<T> &a, int n) {
	static_assert(std::numeric_limits<T>::is_signed);
	using W = __int128_t;
	std::vector<W> work(a.begin(), a.end());
	W previous = 1, sign = 1;
	if (!n) return T(1);
	for (int k = 0; k + 1 < n; ++k) {
		int pivot = k;
		while (pivot < n && work[std::size_t(pivot) * n + k] == 0) ++pivot;
		if (pivot == n) return T(0);
		if (pivot != k) { for (int j = 0; j < n; ++j) std::swap(work[std::size_t(k) * n + j], work[std::size_t(pivot) * n + j]); sign = -sign; }
		W p = work[std::size_t(k) * n + k];
		for (int i = k + 1; i < n; ++i) {
			for (int j = k + 1; j < n; ++j) {
				W value = work[std::size_t(i) * n + j] * p - work[std::size_t(i) * n + k] * work[std::size_t(k) * n + j];
				assert(value % previous == 0);
				work[std::size_t(i) * n + j] = value / previous;
			}
			work[std::size_t(i) * n + k] = 0;
		}
		previous = p;
	}
	W result = sign * work.back();
	assert(result >= std::numeric_limits<T>::lowest() && result <= std::numeric_limits<T>::max());
	return T(result);
}

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
template <std::uint32_t mod>
__attribute__((target("avx2"))) void product_avx2(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	int i = 0;
	for (; i + 2 <= n; i += 2) {
		int j = 0;
		for (; j + 8 <= m; j += 8) for (int k = 0; k < inner; k += step) {
			__m256i x0 = _mm256_setzero_si256(), x1 = x0, y0 = x0, y1 = x0;
			for (int t = k; t < std::min(inner, k + step); ++t) {
				__m256i row = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + std::size_t(t) * m + j));
				__m256i odd = _mm256_srli_epi64(row, 32);
				__m256i u = _mm256_set1_epi32(a[std::size_t(i) * inner + t]);
				__m256i v = _mm256_set1_epi32(a[std::size_t(i + 1) * inner + t]);
				x0 = _mm256_add_epi64(x0, _mm256_mul_epu32(u, row));
				x1 = _mm256_add_epi64(x1, _mm256_mul_epu32(u, odd));
				y0 = _mm256_add_epi64(y0, _mm256_mul_epu32(v, row));
				y1 = _mm256_add_epi64(y1, _mm256_mul_epu32(v, odd));
			}
			std::uint64_t even[4], odd[4], even2[4], odd2[4];
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even2), y0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd2), y1);
			for (int s = 0; s < 4; ++s) {
				auto *r = c + std::size_t(i) * m + j + s * 2;
				auto *r2 = r + m;
				r[0] = (even[s] + r[0]) % mod; r[1] = (odd[s] + r[1]) % mod;
				r2[0] = (even2[s] + r2[0]) % mod; r2[1] = (odd2[s] + r2[1]) % mod;
			}
		}
		if (j < m) for (int row = i; row < i + 2; ++row) for (int col = j; col < m; ++col) {
			std::uint64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::uint64_t(a[std::size_t(row) * inner + t]) * b[std::size_t(t) * m + col];
				sum %= mod;
			}
			c[std::size_t(row) * m + col] = sum;
		}
	}
	if (i < n) product_scalar<mod>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, 1, inner, m);
}

template <std::uint32_t mod, int rows = 4>
__attribute__((target("avx2"))) void product_packed_avx2(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - (mod - 1)) / (radius * radius)));
	auto reduce = [](std::int64_t sum, std::uint32_t c = 0) {
		std::int64_t r = (sum + std::int64_t(c)) % std::int64_t(mod);
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	int i = 0;
	for (; i + rows <= n; i += rows) {
		int j = 0;
		for (; j + 8 <= m; j += 8) for (int k = 0; k < inner; k += step) {
			__m256i x0 = _mm256_setzero_si256(), x1 = x0, y0 = x0, y1 = x0, z0 = x0, z1 = x0, w0 = x0, w1 = x0;
			for (int t = k; t < std::min(inner, k + step); ++t) {
				__m256i row = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + std::size_t(j) * inner + std::size_t(t) * 8));
				__m256i odd = _mm256_srli_epi64(row, 32);
				__m256i u = _mm256_set1_epi32(a[std::size_t(i) * inner + t]);
				__m256i v = _mm256_set1_epi32(a[std::size_t(i + 1) * inner + t]);
				x0 = _mm256_add_epi64(x0, _mm256_mul_epi32(u, row));
				x1 = _mm256_add_epi64(x1, _mm256_mul_epi32(u, odd));
				y0 = _mm256_add_epi64(y0, _mm256_mul_epi32(v, row));
				y1 = _mm256_add_epi64(y1, _mm256_mul_epi32(v, odd));
				if constexpr (rows == 4) {
					__m256i p = _mm256_set1_epi32(a[std::size_t(i + 2) * inner + t]);
					__m256i q = _mm256_set1_epi32(a[std::size_t(i + 3) * inner + t]);
					z0 = _mm256_add_epi64(z0, _mm256_mul_epi32(p, row));
					z1 = _mm256_add_epi64(z1, _mm256_mul_epi32(p, odd));
					w0 = _mm256_add_epi64(w0, _mm256_mul_epi32(q, row));
					w1 = _mm256_add_epi64(w1, _mm256_mul_epi32(q, odd));
				}
			}
			std::int64_t even[4], odd[4], even2[4], odd2[4], even3[4], odd3[4], even4[4], odd4[4];
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even2), y0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd2), y1);
			if constexpr (rows == 4) {
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even3), z0);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd3), z1);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even4), w0);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd4), w1);
			}
			for (int s = 0; s < 4; ++s) {
				auto *r = c + std::size_t(i) * m + j + s * 2;
				auto *r2 = r + m;
				r[0] = reduce(even[s], r[0]); r[1] = reduce(odd[s], r[1]);
				r2[0] = reduce(even2[s], r2[0]); r2[1] = reduce(odd2[s], r2[1]);
				if constexpr (rows == 4) {
					auto *r3 = r2 + m, *r4 = r3 + m;
					r3[0] = reduce(even3[s], r3[0]); r3[1] = reduce(odd3[s], r3[1]);
					r4[0] = reduce(even4[s], r4[0]); r4[1] = reduce(odd4[s], r4[1]);
				}
			}
		}
		if (j < m) for (int row = i; row < i + rows; ++row) for (int col = j; col < m; ++col) {
			std::int64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::int64_t(a[std::size_t(row) * inner + t]) * b[std::size_t(j) * inner + std::size_t(t) * (m - j) + col - j];
				sum = reduce(sum);
			}
			c[std::size_t(row) * m + col] = sum;
		}
	}
	if constexpr (rows == 4) {
		if (i < n) product_packed_avx2<mod, 2>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	} else for (; i < n; ++i) for (int j = 0; j < m; j += 8) {
		int width = std::min(8, m - j);
		for (int s = 0; s < width; ++s) {
			std::int64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::int64_t(a[std::size_t(i) * inner + t]) * b[std::size_t(j) * inner + std::size_t(t) * width + s];
				sum = reduce(sum);
			}
			c[std::size_t(i) * m + j + s] = sum;
		}
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
		if (n >= 4 && inner >= 16 && m >= 8 && avx2()) {
			std::vector<std::int32_t> x(a.size()), y(b.size());
			std::vector<std::uint32_t> z(c.size());
			auto center = [](std::uint32_t value) {
				std::int32_t v = value;
				return v <= std::int32_t((T::mod() - 1) / 2) ? v : v - std::int32_t(T::mod());
			};
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = center(a[i].val());
			for (int j = 0; j < m; j += 8) {
				int width = std::min(8, m - j);
				for (int k = 0; k < inner; ++k) for (int s = 0; s < width; ++s)
					y[std::size_t(j) * inner + std::size_t(k) * width + s] = center(b[std::size_t(k) * m + j + s].val());
			}
			product_packed_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
			return c;
		}
#endif
		std::vector<std::uint32_t> x(a.size()), y(b.size()), z(c.size());
		for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
		for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (inner >= 16 && m >= 8 && avx2()) product_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		else
#endif
		product_scalar<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
	} else basic();
	return c;
}
}

#endif
