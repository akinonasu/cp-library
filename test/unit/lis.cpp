#include "string/lis.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <string_view>

template<bool Strict, class Range, class Compare>
void check(const Range &a, Compare compare) {
	int n = std::ranges::size(a);
	auto first = std::ranges::begin(a);
	std::vector<int> expected(n, 1);
	int length = 0;
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < i; ++j) {
			bool before;
			if constexpr (Strict) before = compare(first[j], first[i]);
			else before = !compare(first[i], first[j]);
			if (before) expected[i] = std::max(expected[i], expected[j] + 1);
		}
		length = std::max(length, expected[i]);
	}
	assert(lis_length<Strict>(a, compare) == length);
	assert(lis_lengths<Strict>(a, compare) == expected);
	auto indices = lis<Strict>(a, compare);
	assert(int(indices.size()) == length);
	for (int i = 0; i < length; ++i) {
		assert(0 <= indices[i] && indices[i] < n);
		if (!i) continue;
		assert(indices[i - 1] < indices[i]);
		if constexpr (Strict) assert(compare(first[indices[i - 1]], first[indices[i]]));
		else assert(!compare(first[indices[i]], first[indices[i - 1]]));
	}
}

template<class Range>
void check(const Range &a) {
	check<true>(a, std::less<>{}); check<false>(a, std::less<>{});
	check<true>(a, std::greater<>{}); check<false>(a, std::greater<>{});
}

struct Key {
	int value;
	explicit Key(int value) : value(value) {}
	Key(const Key &) = delete;
	Key &operator=(const Key &) = delete;
	Key(Key &&) = default;
	Key &operator=(Key &&) = default;
};

struct Small {
	int value;
	explicit Small(int value) : value(value) {}
};

struct Large {
	int value;
	std::array<int, 32> padding{};
};

int main() {
	check(std::vector<int>{});
	for (int n = 0; n <= 9; ++n) {
		int combinations = 1;
		for (int i = 0; i < n; ++i) combinations *= 3;
		for (int mask = 0; mask < combinations; ++mask) {
			int code = mask;
			std::vector<int> a(n);
			for (auto &x : a) { x = code % 3 - 1; code /= 3; }
			check(a);
		}
	}
	std::mt19937_64 rng(20261019);
	for (int n : {1, 2, 3, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257}) {
		for (int trial = 0; trial < 30; ++trial) {
			std::vector<long long> a(n);
			for (auto &x : a) x = static_cast<long long>(rng() % 101) - 50;
			check(a);
			std::sort(a.begin(), a.end()); check(a);
			std::reverse(a.begin(), a.end()); check(a);
			std::fill(a.begin(), a.end(), 0); check(a);
		}
	}
	const auto low = std::numeric_limits<long long>::lowest(), high = std::numeric_limits<long long>::max();
	check(std::vector<long long>{low, 0, high, low, high});
	check(std::vector<__int128_t>{-(__int128_t(1) << 120), 0, __int128_t(1) << 120, 0});
	check(std::vector<bool>{true, false, false, true, true, false});
	check(std::vector<double>{-std::numeric_limits<double>::infinity(), -0.0, 0.0, 3.5, std::numeric_limits<double>::infinity()});
	check(std::vector<std::string>{"b", "a", "aa", "aa", "ab", "a"});
	check(std::deque<int>{3, 1, 2, 2, 4});
	int array[] = {3, 1, 2, 2, 4};
	check(array); check(std::span<const int>(array));
	check(std::string_view("bacac"));
	check(std::array<int, 0>{});
	std::vector<Key> keys;
	std::vector<Small> small;
	std::vector<Large> large;
	for (int x : {3, 1, 4, 2, 2, 5}) {
		keys.emplace_back(x); small.emplace_back(x); large.push_back({x, {}});
	}
	auto compare = [](const auto &a, const auto &b) { return a.value < b.value; };
	check<true>(keys, compare); check<false>(keys, compare);
	check<true>(small, compare); check<false>(small, compare);
	check<true>(large, compare); check<false>(large, compare);
	auto projected = std::views::transform(array, [](int x) { return -x; });
	check(projected);
	std::vector<int> increasing(200000);
	std::iota(increasing.begin(), increasing.end(), 0);
	assert(lis_length(increasing) == int(increasing.size()));
	assert(lis(increasing) == increasing);
	std::reverse(increasing.begin(), increasing.end());
	assert(lis_length(increasing) == 1 && lis(increasing).size() == 1);
	std::fill(increasing.begin(), increasing.end(), 7);
	assert(lis_length(increasing) == 1 && lis_length<false>(increasing) == int(increasing.size()));
}
