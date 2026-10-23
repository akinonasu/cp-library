#include "sequence/inversion_count.hpp"
#include <array>
#include <cassert>
#include <deque>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <string>

template<class Range, class Compare>
void check(const Range &a, Compare compare) {
	auto first = std::ranges::begin(a);
	std::int64_t expected = 0;
	int n = std::ranges::size(a);
	for (int i = 0; i < n; ++i)
		for (int j = i + 1; j < n; ++j) expected += bool(compare(first[j], first[i]));
	assert(inversion_count(a, compare) == expected);
}

template<class Range>
void check(const Range &a) {
	check(a, std::less<>{});
	check(a, std::greater<>{});
}

struct Key {
	int value;
	explicit Key(int value) : value(value) {}
};

struct Large {
	int value;
	std::array<int, 32> padding{};
};

int main() {
	static_assert(std::is_same_v<decltype(inversion_count(std::vector<int>{})), std::int64_t>);
	for (int n = 0; n <= 8; ++n) {
		std::vector<int> a(n);
		while (true) {
			check(a);
			int i = 0;
			while (i < n && ++a[i] == 3) a[i++] = 0;
			if (i == n) break;
		}
	}
	for (int n = 1; n <= 8; ++n) {
		std::vector<int> a(n);
		std::iota(a.begin(), a.end(), 0);
		do { check(a); } while (std::next_permutation(a.begin(), a.end()));
	}
	std::mt19937 random(20261021);
	for (int trial = 0; trial < 2000; ++trial) {
		std::vector<int> a(random() % 101);
		for (int &x : a) x = int(random() % 31) - 15;
		const auto saved = a;
		check(a);
		assert(a == saved);
	}
	check(std::array<int, 0>{});
	check(std::array<std::int64_t, 5>{std::numeric_limits<std::int64_t>::max(), 0,
		std::numeric_limits<std::int64_t>::min(), 0, std::numeric_limits<std::int64_t>::max()});
	check(std::deque<int>{4, 1, 3, 1, 2});
	check(std::vector<bool>{true, false, true, false, false});
	check(std::string("banana"));
	check(std::views::iota(0, 40));
	const int array[]{5, 2, 4, 6, 1};
	check(array);
	check(std::span<const int>(array));
	std::vector<Key> keys;
	std::vector<Large> large;
	std::vector<std::unique_ptr<int>> pointers;
	for (int x : {4, 1, 3, 1, 2}) {
		keys.emplace_back(x);
		large.push_back({x, {}});
		pointers.push_back(std::make_unique<int>(x));
	}
	check(keys, [](const Key &a, const Key &b) { return a.value < b.value; });
	check(large, [](const Large &a, const Large &b) { return a.value < b.value; });
	check(pointers, [](const auto &a, const auto &b) { return *a < *b; });
	std::vector<int> crossed(32);
	std::iota(crossed.begin(), crossed.end(), 0);
	std::rotate(crossed.begin() + 16, crossed.begin() + 24, crossed.end());
	check(crossed);
	std::vector<Key> grouped, waves;
	for (int i = 0; i < 257; ++i) {
		grouped.emplace_back(256 - i);
		waves.emplace_back((i * 37 % 257) / 3);
	}
	check(grouped, [](const Key &a, const Key &b) { return a.value / 7 < b.value / 7; });
	check(grouped, [](const Key &a, const Key &b) { return a.value / 7 > b.value / 7; });
	check(waves, [](const Key &a, const Key &b) { return a.value < b.value; });
	std::vector<int> big(1000000);
	std::iota(big.begin(), big.end(), 0);
	const std::int64_t maximum = std::int64_t(big.size()) * (big.size() - 1) / 2;
	assert(inversion_count(big) == 0);
	assert(inversion_count(big, std::greater<>{}) == maximum);
	std::reverse(big.begin(), big.end());
	assert(inversion_count(big) == maximum);
	std::fill(big.begin(), big.end(), 7);
	assert(inversion_count(big) == 0);
	std::fill(big.begin() + big.size() / 2, big.end(), 3);
	const std::int64_t half = big.size() / 2;
	assert(inversion_count(big) == half * half);
}
