#line 1 "test/min_plus_convex.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_arbitrary"
#line 1 "convolution/min_plus_convolution.hpp"



#line 1 "components/smawk.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <vector>

template <class Better>
std::vector<int> smawk(int length, const std::vector<int> &columns, Better better) {
	assert(length >= 0 && (!length || !columns.empty()));
	if (!length) return {};
	std::vector<int> best(length);
	auto solve = [&](auto &&self, std::size_t first, std::size_t stride, const std::vector<int> &cols) -> void {
		int rows = (length - 1 - first) / stride + 1;
		std::vector<int> reduced;
		reduced.reserve(std::min<std::size_t>(rows, cols.size()));
		for (int col : cols) {
			while (!reduced.empty() && better(first + (int(reduced.size()) - 1) * stride, reduced.back(), col)) reduced.pop_back();
			if (int(reduced.size()) < rows) reduced.push_back(col);
		}
		if (rows > 1) self(self, first + stride, stride * 2, reduced);
		std::size_t left = 0;
		for (std::size_t row = first; row < std::size_t(length); row += 2 * stride) {
			std::size_t right = left;
			if (row + stride < std::size_t(length))
				while (reduced[right] != best[row + stride]) ++right;
			else right = reduced.size() - 1;
			best[row] = reduced[left];
			for (std::size_t i = left + 1; i <= right; ++i)
				if (better(row, best[row], reduced[i])) best[row] = reduced[i];
			left = right;
		}
	};
	solve(solve, 0, 1, columns);
	return best;
}

template <class Better>
std::vector<int> smawk(int rows, int columns, Better better) {
	assert(columns >= 0);
	std::vector<int> cols(columns);
	std::iota(cols.begin(), cols.end(), 0);
	return smawk(rows, cols, better);
}


#line 8 "convolution/min_plus_convolution.hpp"
#include <cstdint>
#include <functional>
#include <limits>
#line 13 "convolution/min_plus_convolution.hpp"

namespace min_plus_convolution_detail {

template <class T, class Compare>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = a.size(), m = b.size(), length = n + m - 1;
	std::vector<int> columns(m);
	std::iota(columns.begin(), columns.end(), 0);
	auto better = [&](int row, int x, int y) {
		if (row < y) return false;
		if (row - x >= n) return true;
		return compare(a[row - y] + b[y], a[row - x] + b[x]);
	};
	auto best = smawk(length, columns, better);
	std::vector<T> result;
	result.reserve(length);
	for (int i = 0; i < length; ++i) result.push_back(a[i - best[i]] + b[best[i]]);
	return result;
}
template <class T, class Compare>
std::vector<T> opposite(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = a.size(), m = b.size(), length = n + m - 1;
	std::vector<T> result(length, a[0] + b[0]);
	std::vector<std::uint8_t> done(length);
	auto solve = [&](auto &&self, int l, int r, int parent_l, int parent_r) -> void {
		int low = std::max(0, r - n), high = std::min(m, l + 1);
		std::vector<int> columns;
		for (int i = high - 1; i >= low;) {
			if (parent_l <= i && i < parent_r) i = parent_l - 1;
			else columns.push_back(i--);
		}
		if (!columns.empty()) {
			auto best = smawk(r - l, columns, [&](int row, int x, int y) {
				return compare(a[l + row - y] + b[y], a[l + row - x] + b[x]);
			});
			for (int i = l; i < r; ++i) {
				T value = a[i - best[i - l]] + b[best[i - l]];
				if (!done[i] || compare(value, result[i])) result[i] = value;
				done[i] = 1;
			}
		}
		if (r - l > 1) {
			int mid = l + (r - l) / 2;
			self(self, l, mid, low, high);
			self(self, mid, r, low, high);
		}
	};
	solve(solve, 0, length, 0, 0);
	return result;
}

}

template <class T>
std::vector<T> min_plus_convolution_convex(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::multiply(a, b, std::less<T>{});
}
template <class T>
std::vector<T> max_plus_convolution_concave(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::multiply(a, b, std::greater<T>{});
}

template <class T>
std::vector<T> min_plus_convolution_concave(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::opposite(a, b, std::less<T>{});
}
template <class T>
std::vector<T> max_plus_convolution_convex(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::opposite(a, b, std::greater<T>{});
}


#line 4 "test/min_plus_convex.test.cpp"
#include <iostream>
#line 6 "test/min_plus_convex.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : min_plus_convolution_convex(a, b)) std::cout << x << ' ';
	std::cout << '\n';
}
