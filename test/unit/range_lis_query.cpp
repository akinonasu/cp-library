#include "data_structure/range_lis_query.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <span>
#include <string_view>
#include <vector>

template <bool Strict, class Range, class Compare>
int brute(const Range &values, int l, int r, Compare compare) {
	std::vector<int> dp(r - l, 1);
	int best = 0;
	for (int i = l; i < r; ++i) {
		for (int j = l; j < i; ++j) {
			bool before = Strict ? compare(values[j], values[i]) : !compare(values[i], values[j]);
			if (before) dp[i - l] = std::max(dp[i - l], dp[j - l] + 1);
		}
		best = std::max(best, dp[i - l]);
	}
	return best;
}

void exhaustive() {
	for (int n = 0, count = 1; n <= 7; ++n, count *= 3) {
		for (int code = 0; code < count; ++code) {
			std::vector<int> values(n);
			for (int i = 0, x = code; i < n; ++i, x /= 3) values[i] = x % 3;
			RangeLISQuery<> strict(values);
			RangeLISQuery<false> weak(values);
			for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
				assert(strict.query(l, r) == brute<true>(values, l, r, std::less<>{}));
				assert(weak.query(l, r) == brute<false>(values, l, r, std::less<>{}));
			}
		}
	}
}

void randomized() {
	std::mt19937 random(701365);
	for (int n : {31, 32, 33, 63, 64, 65, 100, 127, 128, 129, 257, 511, 512, 513}) {
		for (int round = 0; round < 5; ++round) {
			std::vector<int> values(n);
			for (auto &value : values) value = int(random() % (round == 0 ? 5 : n * 2)) - n;
			RangeLISQuery<> strict(values);
			RangeLISQuery<false> weak(values);
			RangeLISQuery<> decreasing(values, std::greater<>{});
			RangeLISQuery<false> weak_decreasing(values, std::greater<>{});
			for (int q = 0; q < 60; ++q) {
				int l = random() % (n + 1), r = random() % (n + 1);
				if (l > r) std::swap(l, r);
				assert(strict.query(l, r) == brute<true>(values, l, r, std::less<>{}));
				assert(weak.query(l, r) == brute<false>(values, l, r, std::less<>{}));
				assert(decreasing.query(l, r) == brute<true>(values, l, r, std::greater<>{}));
				assert(weak_decreasing.query(l, r) == brute<false>(values, l, r, std::greater<>{}));
			}
			auto copy = strict;
			int answer = strict.query(0, n);
			strict.clear();
			assert(strict.empty() && strict.size() == 0 && strict.query(0, 0) == 0);
			assert(copy.query(0, n) == answer);
			RangeLISQuery<> moved(std::move(copy));
			assert(copy.empty() && copy.query(0, 0) == 0 && moved.query(0, n) == answer);
			strict = std::move(moved);
			assert(moved.empty() && strict.query(0, n) == answer);
			std::reverse(values.begin(), values.end());
			strict.build(values);
			assert(strict.query(0, n) == brute<true>(values, 0, n, std::less<>{}));
		}
	}
}

void large() {
	const int n = 100003;
	std::vector<int> a(n);
	std::iota(a.begin(), a.end(), 0);
	RangeLISQuery<> tree(a);
	assert(tree.size() == n && tree.query(0, n) == n && tree.query(123, 99999) == 99876);
	std::reverse(a.begin(), a.end());
	tree.build(a);
	assert(tree.query(0, n) == 1 && tree.query(32, 9999) == 1);
	std::fill(a.begin(), a.end(), 1);
	tree.build(a);
	RangeLISQuery<false> weak(a);
	assert(tree.query(0, n) == 1 && weak.query(3, n) == n - 3);
}

int main() {
	exhaustive();
	randomized();
	large();
	RangeLISQuery<> empty;
	assert(empty.empty() && empty.query(0, 0) == 0);
	std::array<std::int64_t, 4> extremes{INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX};
	RangeLISQuery<> tree(extremes);
	assert(tree.query(0, 4) == 2);
	RangeLISQuery<false> weak{std::span<const std::int64_t>(extremes)};
	assert(weak.query(0, 4) == 3);
	std::deque<int> deque{3, 2, 4, 1, 5};
	tree.build(deque);
	assert(tree.query(0, 5) == 3);
	tree.build(std::string_view("banana"));
	assert(tree.query(0, 6) == 2);
	std::vector<bool> booleans{true, false, true, true};
	tree.build(booleans);
	assert(tree.query(0, 4) == 2);
	std::vector<std::unique_ptr<int>> pointers;
	for (int value : {3, 1, 2, 2, 4}) pointers.push_back(std::make_unique<int>(value));
	tree.build(pointers, [](const auto &a, const auto &b) { return *a < *b; });
	assert(tree.query(0, 5) == 3 && *pointers[0] == 3);
	int a[]{2, 1, 3};
	tree.build(a);
	assert(tree.query(0, 3) == 2);
	empty.swap(tree);
	assert(empty.query(0, 3) == 2 && tree.empty());
	tree.build(std::vector<int>{});
	assert(tree.query(0, 0) == 0);
}
