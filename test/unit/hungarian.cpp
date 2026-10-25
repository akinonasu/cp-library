#include "graph/hungarian.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

template <class Cost, class T>
void check(const Hungarian<Cost> &res, const std::vector<std::vector<T>> &a, int m, Cost expected) {
	int n = int(a.size());
	assert(res.cost == expected);
	assert(int(res.row_to_col.size()) == n && int(res.col_to_row.size()) == m);
	Cost sum = 0;
	std::vector<int> used(m);
	for (int i = 0; i < n; ++i) {
		int j = res.row_to_col[i];
		assert(0 <= j && j < m && !used[j]++ && res.col_to_row[j] == i);
		sum += Cost(a[i][j]);
	}
	assert(sum == res.cost);
	for (int j = 0; j < m; ++j) {
		if (!used[j]) assert(res.col_to_row[j] == -1);
		else assert(res.row_to_col[res.col_to_row[j]] == j);
	}
}

template <class Cost, class T>
Cost brute(const std::vector<std::vector<T>> &a, int m, bool maximize) {
	int n = int(a.size());
	std::vector<Cost> dp(1 << m, maximize ? std::numeric_limits<Cost>::lowest() : std::numeric_limits<Cost>::max());
	dp[0] = 0;
	for (int mask = 0; mask < (1 << m); ++mask) {
		int i = __builtin_popcount(unsigned(mask));
		if (i >= n) continue;
		for (int j = 0; j < m; ++j) if (!(mask >> j & 1)) {
			Cost x = dp[mask] + Cost(a[i][j]);
			if (maximize) dp[mask | (1 << j)] = std::max(dp[mask | (1 << j)], x);
			else dp[mask | (1 << j)] = std::min(dp[mask | (1 << j)], x);
		}
	}
	Cost result = maximize ? std::numeric_limits<Cost>::lowest() : std::numeric_limits<Cost>::max();
	for (int mask = 0; mask < (1 << m); ++mask) if (__builtin_popcount(unsigned(mask)) == n) {
		if (maximize) result = std::max(result, dp[mask]);
		else result = std::min(result, dp[mask]);
	}
	return result;
}

template <class Cost, class T>
void test(const std::vector<std::vector<T>> &a, int m) {
	for (bool maximize : {false, true}) {
		Cost expected = brute<Cost>(a, m, maximize);
		Hungarian<Cost> res(a, maximize);
		check(res, a, a.empty() ? 0 : m, expected);
		std::vector<T> flat(a.size() * (m + 3), T(123));
		for (std::size_t i = 0; i < a.size(); ++i) std::copy(a[i].begin(), a[i].end(), flat.begin() + i * (m + 3));
		check(Hungarian<Cost>(int(a.size()), m, flat.data(), m + 3, maximize), a, m, expected);
	}
}

int main() {
	std::mt19937 rng(612991);
	for (int m = 0; m <= 8; ++m) for (int n = 0; n <= m; ++n)
		for (int trial = 0; trial < 160; ++trial) {
			std::vector<std::vector<int>> a(n, std::vector<int>(m));
			for (auto &row : a) for (auto &x : row) x = int(rng() % (trial % 3 ? 2001 : 3)) - (trial % 3 ? 1000 : 1);
			test<long long>(a, m);
			if (trial % 16 == 0) test<int>(a, m);
		}
	for (int n = 1; n <= 8; ++n) for (int kind = 0; kind < 5; ++kind) {
		std::vector<std::vector<long long>> a(n, std::vector<long long>(n));
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			if (kind == 0) a[i][j] = 7;
			if (kind == 1) a[i][j] = (i - j) * (i - j);
			if (kind == 2) a[i][j] = -(i - j) * (i - j);
			if (kind == 3) a[i][j] = (i == j ? -1 : 1000000000000LL);
			if (kind == 4) a[i][j] = 1000000000000LL * i + 7919 * j;
		}
		test<long long>(a, n);
	}
	std::vector<std::vector<long long>> wide{{4000000000000000000LL, -4000000000000000000LL}, {-3000000000000000000LL, 4000000000000000000LL}};
	test<__int128_t>(wide, 2);
	std::array<std::array<int, 3>, 2> a{{{{2, 1, 3}}, {{1, 3, 2}}}};
	Hungarian res(a);
	assert(res.cost == 2);
	std::vector<std::vector<long long>> b{{8}};
	test<short>(std::vector<std::vector<short>>{ {3, -2, 5}, {1, 4, 0} }, 3);
	Hungarian single(b);
	assert(single.cost == 8 && single.row_to_col[0] == 0);
}
