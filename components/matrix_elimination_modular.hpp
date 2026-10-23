#ifndef MATRIX_ELIMINATION_MODULAR_HPP
#define MATRIX_ELIMINATION_MODULAR_HPP

#include "matrix_arithmetic.hpp"
#include <array>

namespace matrix_detail {
template <class Ops>
struct ModularElimination {
	using V = typename Ops::V;
	static void hessenberg(std::vector<V> &b, int n, int k, V inverse, std::vector<V> &factors) {
		bool any = false;
		for (int i = k + 1; i < n; ++i) {
			V factor = factors[i] = Ops::mul(b[std::size_t(i) * n + k - 1], inverse);
			if (!factor) continue;
			any = true;
			Ops::axpy(b.data() + std::size_t(i) * n + k, b.data() + std::size_t(k) * n + k, n - k, Ops::neg(factor));
			b[std::size_t(i) * n + k - 1] = 0;
		}
		if (!any) return;
		for (int j = 0; j < n; ++j) {
			auto *row = b.data() + std::size_t(j) * n;
			row[k] = Ops::add(row[k], Ops::dot(row + k + 1, factors.data() + k + 1, n - k - 1));
		}
	}

	static constexpr int batch = int(std::min<std::uint64_t>(8, (std::numeric_limits<std::int64_t>::max() - (Ops::mod - 1)) / (std::uint64_t(Ops::mod - 1) * (Ops::mod - 1))));
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx512f,avx512dq"))) static __m512i reduce512(__m512i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm512_add_epi64(sum, _mm512_mul_epu32(_mm512_mul_epu32(sum, _mm512_set1_epi32(negative_inverse)), _mm512_set1_epi32(Ops::mod)));
		sum = _mm512_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm512_min_epu32(sum, _mm512_sub_epi32(sum, _mm512_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx512f,avx512dq"))) static void update512(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 16 <= length; j += 16) {
			auto x = _mm512_loadu_si512(dst + j);
			auto even = _mm512_setzero_si512(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm512_loadu_si512(src[k] + first + j), b = _mm512_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm512_add_epi64(even, _mm512_mul_epu32(a, b));
				odd = _mm512_add_epi64(odd, _mm512_mul_epu32(_mm512_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) even = reduce512(even), odd = reduce512(odd);
			else even = reduce_product_avx512<Ops::mod>(even), odd = reduce_product_avx512<Ops::mod>(odd);
			x = _mm512_add_epi32(x, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
			_mm512_storeu_si512(dst + j, _mm512_min_epu32(x, _mm512_sub_epi32(x, _mm512_set1_epi32(Ops::mod))));
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
	__attribute__((target("avx2"))) static __m256i reduce256(__m256i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm256_add_epi64(sum, _mm256_mul_epu32(_mm256_mul_epu32(sum, _mm256_set1_epi32(negative_inverse)), _mm256_set1_epi32(Ops::mod)));
		sum = _mm256_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx2"))) static void update256(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 8 <= length; j += 8) {
			auto even = _mm256_setzero_si256(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src[k] + first + j));
				auto b = _mm256_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(a, b));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) {
				even = reduce256(even), odd = reduce256(odd);
				auto value = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(dst + j)), _mm256_or_si256(even, _mm256_slli_epi64(odd, 32)));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(dst + j), _mm256_min_epu32(value, _mm256_sub_epi32(value, _mm256_set1_epi32(Ops::mod))));
			} else {
				std::uint64_t x[4], y[4];
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(x), even);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(y), odd);
				for (int k = 0; k < 4; ++k) {
					dst[j + 2 * k] = (x[k] + dst[j + 2 * k]) % Ops::mod;
					dst[j + 2 * k + 1] = (y[k] + dst[j + 2 * k + 1]) % Ops::mod;
				}
			}
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
#endif
	static void update(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		if (!count || !length) return;
		if (count == 1) { if (coeff[0]) Ops::axpy(dst, src[0] + first, length, coeff[0]); return; }
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (length >= 32 && avx512_product()) { update512(dst, src, coeff, count, first, length); return; }
		if (length >= 16 && avx2()) { update256(dst, src, coeff, count, first, length); return; }
#endif
		for (int j = 0; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
	static Elimination<Ops> run(std::vector<V> &a, int h, int stride, int columns, bool reduced) {
		Elimination<Ops> result;
		result.pivots.reserve(std::min(h, columns));
		std::vector<std::array<V, batch>> factors(h);
		std::array<const V *, batch> source;
		int rank = 0;
		for (int first = 0; first < columns && rank < h; first += batch) {
			int end = std::min(columns, first + batch), start = rank;
			for (int i = rank; i < h; ++i) factors[i].fill(0);
			for (int col = first; col < end && rank < h; ++col) {
				int p = rank;
				while (p < h && !a[std::size_t(p) * stride + col]) ++p;
				if (p == h) continue;
				V *row = a.data() + std::size_t(rank) * stride;
				if (p != rank) {
					std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
					std::swap(factors[p], factors[rank]);
					result.determinant = Ops::neg(result.determinant);
				}
				update(row + end, source.data(), factors[rank].data(), rank - start, end, stride - end);
				result.determinant = Ops::mul(result.determinant, row[col]);
				Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
				row[col] = 1;
				source[rank - start] = row;
				for (int i = rank + 1; i < h; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					V x = factors[i][rank - start] = Ops::neg(dst[col]);
					if (x) Ops::axpy(dst + col + 1, row + col + 1, end - col - 1, x);
					dst[col] = 0;
				}
				result.pivots.push_back(col);
				++rank;
			}
			for (int i = rank; i < h; ++i) update(a.data() + std::size_t(i) * stride + end, source.data(), factors[i].data(), rank - start, end, stride - end);
		}
		bool full = rank == columns;
		if (reduced) for (int end = rank; end > 0;) {
			int start = std::max(0, end - batch);
			for (int k = end - 1; k >= start; --k) {
				int col = result.pivots[k], first = full ? columns : col + 1;
				const V *row = a.data() + std::size_t(k) * stride;
				for (int i = start; i < k; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					if (V x = dst[col]) Ops::axpy(dst + first, row + first, stride - first, Ops::neg(x));
					dst[col] = 0;
				}
			}
			int first = full ? columns : result.pivots[start] + 1;
			for (int k = start; k < end; ++k) source[k - start] = a.data() + std::size_t(k) * stride;
			for (int i = 0; i < start; ++i) {
				V *dst = a.data() + std::size_t(i) * stride;
				for (int k = start; k < end; ++k) factors[i][k - start] = Ops::neg(dst[result.pivots[k]]);
				update(dst + first, source.data(), factors[i].data(), end - start, first, stride - first);
				for (int k = start; k < end; ++k) dst[result.pivots[k]] = 0;
			}
			end = start;
		}
		return result;
	}
};
}

#endif
