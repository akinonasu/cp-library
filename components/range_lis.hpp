#ifndef RANGE_LIS_COMPONENT_HPP
#define RANGE_LIS_COMPONENT_HPP

#include "default_init_allocator.hpp"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <limits>
#include <stdexcept>
#include <span>
#include <utility>
#include <vector>

namespace range_lis_detail {
inline void multiply(int n, const int *a, const int *b, int *result, int *work) {
	if (n == 1) {
		result[0] = 0;
		return;
	}
	int *inverse = work;
	work += n;
	int middle = n / 2;
	for (int side = 0; side < 2; ++side) {
		int length = side ? n - middle : middle;
		int *left = work, *rows = left + length, *right = rows + length;
		int *columns = right + length, *child = columns + length;
		int x = 0, y = 0;
		for (int i = 0; i < n; ++i) {
			if ((a[i] >= middle) == bool(side)) {
				left[x] = a[i] - (side ? middle : 0);
				rows[x++] = i;
			}
			if ((b[i] >= middle) == bool(side)) {
				right[y] = b[i] - (side ? middle : 0);
				columns[y++] = i;
			}
		}
		multiply(length, left, right, child, child + length);
		for (int i = 0; i < length; ++i) {
			int column = columns[child[i]];
			result[rows[i]] = column;
			inverse[column] = rows[i];
		}
	}
	int row = n, low = 0, high = 0, low_delta = 0, high_delta = 0;
	auto step = [&](int column) {
		return b[column] < middle ? inverse[column] >= row : inverse[column] < row;
	};
	while (row) {
		while (high < n && high_delta + step(high) == 0) {
			high_delta = 0;
			++high;
		}
		--row;
		if (a[row] < middle) {
			low_delta -= result[row] >= low;
			high_delta -= result[row] >= high;
		} else {
			low_delta -= result[row] < low;
			high_delta -= result[row] < high;
		}
		while (low_delta) low_delta += step(low++);
		if (low > high) result[row] = high;
	}
}

inline std::size_t product_space(int n) {
	std::size_t space = 0;
	for (int length = n; length > 1;) {
		int half = length / 2 + length % 2;
		space += std::size_t(length) + 5 * std::size_t(half);
		length = half;
	}
	return space;
}

inline void compose(int n, const int *a, const int *b, int *result, int *work) {
	int *left = work, *right = left + n, *inverse = right + n;
	int *rows = inverse + n, *columns = rows + n, *product = columns + n;
	std::fill_n(inverse, n, -1);
	int row_count = 0, column_count = 0;
	for (int i = 0; i < n; ++i) if (a[i] >= 0) {
		rows[row_count++] = i;
		inverse[a[i]] = i;
	}
	int cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] < 0) left[cursor++] = i;
	for (int i = 0; i < row_count; ++i) left[cursor++] = a[rows[i]];
	std::fill_n(inverse, n, -1);
	for (int i = 0; i < n; ++i) if (b[i] >= 0) inverse[b[i]] = i;
	cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] >= 0) {
		right[cursor++] = inverse[i];
		columns[column_count++] = i;
	}
	for (int i = 0; i < n; ++i) if (b[i] < 0) right[cursor++] = i;
	multiply(n, left, right, product, product + n);
	std::fill_n(result, n, -1);
	for (int i = 0; i < row_count; ++i) {
		int column = product[n - row_count + i];
		if (column < column_count) result[rows[i]] = columns[column];
	}
}

inline std::vector<int> compose(const std::vector<int> &a, const std::vector<int> &b) {
	int n = int(a.size());
	if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / 20) throw std::length_error("Range LIS workspace");
	std::vector<int> result(n);
	std::vector<int, DefaultInitAllocator<int>> work(6 * std::size_t(n) + product_space(n));
	if (n) compose(n, a.data(), b.data(), result.data(), work.data());
	return result;
}

inline std::size_t seaweed_space(int n) {
	if (n <= 32) return n;
	return std::max(2 * std::size_t(n) + seaweed_space(n - n / 2), 9 * std::size_t(n) + product_space(n));
}

inline void seaweed(int n, const int *permutation, int *result, int *work) {
	if (n <= 32) {
		std::fill_n(work, n, -1);
		std::fill_n(result, n, -1);
		for (int i = 0; i < n; ++i) {
			int value = permutation[i], carry = -1;
			work[value] = i;
			for (int j = value + 1; j < n; ++j)
				if (work[j] > carry) std::swap(work[j], carry);
			if (carry >= 0) result[carry] = i;
		}
		return;
	}
	int middle = n / 2;
	int *split = work, *positions = split + n, *child = positions + n;
	int low = 0, high = middle;
	for (int i = 0; i < n; ++i) {
		int value = permutation[i];
		int &cursor = value < middle ? low : high;
		split[cursor] = value < middle ? value : value - middle;
		positions[cursor++] = i;
	}
	seaweed(middle, split, result, child);
	seaweed(n - middle, split + middle, result + middle, child);
	int *a = split, *b = child;
	std::iota(a, a + n, 0);
	std::iota(b, b + n, 0);
	for (int i = 0; i < middle; ++i) a[positions[i]] = result[i] < 0 ? -1 : positions[result[i]];
	for (int i = middle; i < n; ++i) b[positions[i]] = result[i] < 0 ? -1 : positions[middle + result[i]];
	compose(n, a, b, result, b + n);
}

inline std::vector<int> seaweed(std::span<const int> permutation) {
	int n = int(permutation.size());
	if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / 20) throw std::length_error("Range LIS workspace");
	std::vector<int> result(n);
	if (!n) return result;
	if (std::is_sorted(permutation.begin(), permutation.end())) {
		std::fill(result.begin(), result.end(), -1);
		return result;
	}
	if (std::is_sorted(permutation.rbegin(), permutation.rend())) {
		std::iota(result.begin(), result.end(), 1);
		result.back() = -1;
		return result;
	}
	std::vector<int, DefaultInitAllocator<int>> work(seaweed_space(n));
	seaweed(n, permutation.data(), result.data(), work.data());
	return result;
}
}

#endif
