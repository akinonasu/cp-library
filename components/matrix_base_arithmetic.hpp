#ifndef MATRIX_BASE_ARITHMETIC_HPP
#define MATRIX_BASE_ARITHMETIC_HPP

#include "field_base.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace matrix_detail {
template <class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;

using field_detail::Field;

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

template <class T>
std::vector<T> basic_multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
		const T &x = a[std::size_t(i) * inner + k];
		for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
	}
	return c;
}
}

#endif
