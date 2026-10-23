#ifndef RANGE_LIS_COMPONENT_HPP
#define RANGE_LIS_COMPONENT_HPP

#include <algorithm>
#include <cstddef>
#include <numeric>
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

inline std::vector<int> compose(const std::vector<int> &a, const std::vector<int> &b) {
	int n = int(a.size());
	std::vector<int> left(n), right(n), inverse(n, -1), rows, columns;
	rows.reserve(n);
	columns.reserve(n);
	for (int i = 0; i < n; ++i) if (a[i] >= 0) {
		rows.push_back(i);
		inverse[a[i]] = i;
	}
	int cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] < 0) left[cursor++] = i;
	for (int row : rows) left[cursor++] = a[row];
	std::fill(inverse.begin(), inverse.end(), -1);
	for (int i = 0; i < n; ++i) if (b[i] >= 0) inverse[b[i]] = i;
	cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] >= 0) {
		right[cursor++] = inverse[i];
		columns.push_back(i);
	}
	for (int i = 0; i < n; ++i) if (b[i] < 0) right[cursor++] = i;
	std::size_t space = 0;
	for (int length = n; length > 1;) {
		int half = length / 2 + length % 2;
		space += std::size_t(length) + 5 * std::size_t(half);
		length = half;
	}
	std::vector<int> product(n), work(space);
	multiply(n, left.data(), right.data(), product.data(), work.data());
	std::vector<int> result(n, -1);
	for (std::size_t i = 0; i < rows.size(); ++i) {
		int column = product[n - rows.size() + i];
		if (std::size_t(column) < columns.size()) result[rows[i]] = columns[column];
	}
	return result;
}

inline std::vector<int> seaweed(std::span<const int> permutation) {
	int n = int(permutation.size());
	if (n <= 32) {
		std::vector<int> state(n, -1), result(n, -1);
		for (int i = 0; i < n; ++i) {
			int value = permutation[i], carry = -1;
			state[value] = i;
			for (int j = value + 1; j < n; ++j)
				if (state[j] > carry) std::swap(state[j], carry);
			if (carry >= 0) result[carry] = i;
		}
		return result;
	}
	int middle = n / 2;
	std::vector<int> low, high, low_positions, high_positions;
	low.reserve(middle);
	high.reserve(n - middle);
	low_positions.reserve(middle);
	high_positions.reserve(n - middle);
	for (int i = 0; i < n; ++i) {
		int value = permutation[i];
		if (value < middle) {
			low.push_back(value);
			low_positions.push_back(i);
		} else {
			high.push_back(value - middle);
			high_positions.push_back(i);
		}
	}
	low = seaweed(low);
	high = seaweed(high);
	std::vector<int> a(n), b(n);
	std::iota(a.begin(), a.end(), 0);
	std::iota(b.begin(), b.end(), 0);
	for (int i = 0; i < middle; ++i) a[low_positions[i]] = low[i] < 0 ? -1 : low_positions[low[i]];
	for (int i = 0; i < n - middle; ++i) b[high_positions[i]] = high[i] < 0 ? -1 : high_positions[high[i]];
	return compose(a, b);
}
}

#endif
