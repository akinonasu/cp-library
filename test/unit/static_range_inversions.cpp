#include "sequence/static_range_inversions.hpp"
#include <array>
#include <cassert>
#include <deque>
#include <limits>
#include <memory>
#include <random>
#include <span>
#include <string>

template<class Range, class Compare = std::less<>>
void check(const Range &a, const std::vector<std::pair<int, int>> &queries, Compare compare = {}) {
	const auto saved = static_range_inversion_counts(a, queries, compare);
	static_assert(std::is_same_v<typename std::remove_cvref_t<decltype(saved)>::value_type, std::int64_t>);
	auto first = std::ranges::begin(a);
	assert(saved.size() == queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [l, r] = queries[i];
		std::int64_t expected = 0;
		for (int x = l; x < r; ++x)
			for (int y = x + 1; y < r; ++y) expected += bool(compare(first[y], first[x]));
		assert(saved[i] == expected);
	}
}

template<class Range>
void check_all(const Range &a) {
	int n = std::ranges::size(a);
	std::vector<std::pair<int, int>> queries;
	for (int l = 0; l <= n; ++l)
		for (int r = l; r <= n; ++r) queries.emplace_back(l, r);
	check(a, queries);
	check(a, queries, std::greater<>{});
	std::reverse(queries.begin(), queries.end());
	check(a, queries);
	check(a, queries, std::greater<>{});
}

struct Key {
	int value;
	explicit Key(int value) : value(value) {}
};

int main() {
	for (int n = 0; n <= 7; ++n) {
		std::vector<int> a(n);
		while (true) {
			check_all(a);
			int i = 0;
			while (i < n && ++a[i] == 3) a[i++] = 0;
			if (i == n) break;
		}
	}
	std::mt19937 random(20261023);
	for (int trial = 0; trial < 400; ++trial) {
		int n = 1 + random() % 150, q = 1 + random() % 150;
		std::vector<int> a(n);
		for (int &x : a) x = int(random() % 41) - 20;
		std::vector<std::pair<int, int>> queries(q);
		for (auto &[l, r] : queries) {
			l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
		}
		check(a, queries);
		check(a, queries, std::greater<>{});
	}
	check_all(std::array<std::int64_t, 6>{std::numeric_limits<std::int64_t>::min(), 0,
		std::numeric_limits<std::int64_t>::max(), 0, -1, std::numeric_limits<std::int64_t>::min()});
	check_all(std::array<std::uint64_t, 6>{std::numeric_limits<std::uint64_t>::max(), 0, 1, 0, 1, 2});
	check_all(std::vector<bool>{true, true, false, true, false});
	check_all(std::string("banana"));
	check_all(std::deque<int>{3, 1, 2, 1});
	const int array[]{3, 1, 2, 1};
	check_all(array);
	check_all(std::span<const int>(array));
	check(array, { {1, 4} });
	check(array, { {2, 2} });
	check(array, { {0, 4} }, std::greater<>{});
	check_all(std::views::iota(0, 15));
	std::vector<Key> keys;
	std::vector<std::unique_ptr<int>> pointers;
	for (int value : {4, 2, 4, 1, 3}) {
		keys.emplace_back(value);
		pointers.push_back(std::make_unique<int>(value));
	}
	const std::vector<std::pair<int, int>> queries{ {0, 5}, {1, 4}, {5, 5}, {0, 0}, {2, 3}, {0, 5} };
	check(keys, queries, [](const Key &a, const Key &b) { return a.value < b.value; });
	check(keys, queries, [](const Key &a, const Key &b) { return a.value / 2 < b.value / 2; });
	check(pointers, queries, [](const auto &a, const auto &b) { return *a < *b; });
	assert(static_range_inversion_counts(std::vector<int>(1000), {}).empty());
	std::vector<int> reversed(100000);
	std::iota(reversed.rbegin(), reversed.rend(), 0);
	const auto answer = static_range_inversion_counts(reversed, { {0, 100000}, {1, 99999}, {50000, 50000} });
	assert(answer[0] == std::int64_t(100000) * 99999 / 2);
	assert(answer[1] == std::int64_t(99998) * 99997 / 2);
	assert(answer[2] == 0);
}
