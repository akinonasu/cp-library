#include "components/dynamic_quad_vector.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

using Counts = std::array<int, 4>;

Counts prefix(const std::vector<std::uint8_t>& values, int end) {
	Counts result{};
	for (int i = 0; i < end; ++i) ++result[values[i]];
	return result;
}

template<class Vector>
void check(const Vector& actual, const std::vector<std::uint8_t>& expected, bool all = true) {
	assert(actual.size() == int(expected.size()) && actual.empty() == expected.empty());
	Counts counts{};
	for (int i = 0; i <= int(expected.size()); ++i) {
		if (all || i == 0 || i == int(expected.size()) || i % 113 == 0) {
			assert(actual.ranks(i) == counts);
			for (int digit = 0; digit < 4; ++digit) assert(actual.rank(i, digit) == counts[digit]);
			if (i < int(expected.size())) assert(actual.access_rank(i) == std::pair(counts[expected[i]], int(expected[i])));
		}
		if (i < int(expected.size())) ++counts[expected[i]];
	}
	int uniform = -1;
	for (int digit = 0; digit < 4; ++digit) if (!expected.empty() && counts[digit] == int(expected.size())) uniform = digit;
	assert(actual.uniform_digit() == uniform);
	for (int digit = 0; digit < 4; ++digit) {
		assert(actual.select(digit, -1) == -1 && actual.select(digit, counts[digit]) == -1);
		int rank = 0;
		for (int i = 0; i < int(expected.size()); ++i) if (expected[i] == digit) {
			if (all || rank % 47 == 0) assert(actual.select(digit, rank) == i);
			++rank;
		}
	}
	std::mt19937 random(713 + int(expected.size()));
	for (int i = 0; i < 60; ++i) {
		int l = int(random() % (expected.size() + 1)), r = int(random() % (expected.size() + 1));
		if (l > r) std::swap(l, r);
		auto left = prefix(expected, l), right = prefix(expected, r);
		assert(actual.ranks(l, r) == std::pair(left, right));
		for (int digit = 0; digit < 4; ++digit) assert(actual.rank(l, r, digit) == std::pair(left[digit], right[digit]));
	}
}

template<int Block, int Branch>
void random_test() {
	using Vector = dynamic_wavelet_detail::QuadVector<Block, Branch>;
	std::mt19937 random(20261020 + Block + Branch);
	Vector actual;
	std::vector<std::uint8_t> expected;
	actual.reserve(1000);
	check(actual, expected);
	for (int i = 0; i < 5000; ++i) {
		int operation = random() % 5, digit = random() % 4;
		if (expected.empty() || operation == 0) {
			int p = int(random() % (expected.size() + 1));
			assert(actual.insert(p, digit) == prefix(expected, p)[digit]);
			expected.insert(expected.begin() + p, std::uint8_t(digit));
		} else if (operation == 1) {
			int p = int(random() % expected.size()), old = expected[p];
			assert(actual.erase_rank(p) == std::pair(prefix(expected, p)[old], old));
			expected.erase(expected.begin() + p);
		} else if (operation == 2 || operation == 3) {
			int p = int(random() % expected.size()), old = expected[p];
			auto ranks = prefix(expected, p);
			assert(actual.set_rank(p, digit) == std::tuple(ranks[old], old, ranks[digit]));
			expected[p] = std::uint8_t(digit);
		} else {
			int from = int(random() % expected.size()), to = int(random() % expected.size()), old = expected[from];
			int rank = prefix(expected, from)[old];
			expected.erase(expected.begin() + from);
			int next = prefix(expected, to)[digit];
			expected.insert(expected.begin() + to, std::uint8_t(digit));
			assert(actual.move_rank(from, to, digit) == std::tuple(rank, old, next));
		}
		if (i % 41 == 0) check(actual, expected);
	}
	check(actual, expected);
	Vector copied(actual), moved(std::move(copied));
	assert(copied.empty());
	check(moved, expected);
	copied = std::move(moved);
	assert(moved.empty());
	check(copied, expected);
	actual.clear();
	check(actual, {});
	actual.build(expected);
	check(actual, expected);
	actual.swap(moved);
	assert(actual.empty());
	check(moved, expected);
}

template<int Block, int Branch>
void structure() {
	using Vector = dynamic_wavelet_detail::QuadVector<Block, Branch>;
	constexpr int limit = Block * Branch + 1;
	for (int mode = 0; mode < 3; ++mode) {
		Vector actual;
		actual.reserve(limit);
		std::vector<std::uint8_t> expected;
		for (int i = 0; i < limit; ++i) {
			int p = mode == 0 ? 0 : mode == 1 ? int(expected.size()) : int(expected.size()) / 2;
			int digit = i % 4;
			assert(actual.insert(p, digit) == prefix(expected, p)[digit]);
			expected.insert(expected.begin() + p, std::uint8_t(digit));
			if (i % 131 == 0) check(actual, expected, false);
		}
		check(actual, expected, false);
		for (int i = 0; i < limit; ++i) {
			int p = mode == 0 ? 0 : mode == 1 ? int(expected.size()) - 1 : int(expected.size()) / 2;
			int old = expected[p];
			assert(actual.erase_rank(p) == std::pair(prefix(expected, p)[old], old));
			expected.erase(expected.begin() + p);
			if (i % 131 == 0) check(actual, expected, false);
		}
		check(actual, expected);
	}
}

void default_height() {
	using Vector = dynamic_wavelet_detail::QuadVector<>;
	constexpr int limit = 1024 * 32 + 1;
	for (bool front : {false, true}) {
		Vector actual;
		std::vector<std::uint8_t> expected;
		Counts counts{};
		for (int i = 0; i < limit; ++i) {
			int digit = i % 4, p = front ? 0 : i;
			assert(actual.insert(p, digit) == (front ? 0 : counts[digit]));
			expected.insert(expected.begin() + p, std::uint8_t(digit));
			++counts[digit];
			if (i == 1024 || i == limit / 2 || i + 1 == limit) check(actual, expected, false);
		}
		for (int i = 0; i < limit; ++i) {
			int p = front ? 0 : int(expected.size()) - 1, old = expected[p];
			assert(actual.erase_rank(p) == std::pair(front ? 0 : counts[old] - 1, old));
			expected.erase(expected.begin() + p);
			--counts[old];
			if (i == limit / 2 || i == limit - 1024 || i + 1 == limit) check(actual, expected, false);
		}
	}
}

template<int Block>
void build_boundaries() {
	using Vector = dynamic_wavelet_detail::QuadVector<Block, 4>;
	for (int n : {0, 1, 63, 64, 65, Block / 2 - 1, Block / 2, Block / 2 + 1, Block - 1, Block, Block + 1, 2 * Block - 1, 2 * Block, 2 * Block + 1}) {
		std::vector<std::uint8_t> expected(n);
		for (int i = 0; i < n; ++i) expected[i] = std::uint8_t((i * 7 + i / 3) % 4);
		Vector actual(expected);
		check(actual, expected, n < 1000);
		for (int i = 0; i < std::min(300, n); ++i) {
			int p = (i * 79) % n, digit = i * 3 % 4, old = expected[p];
			auto ranks = prefix(expected, p);
			assert(actual.set_rank(p, digit) == std::tuple(ranks[old], old, ranks[digit]));
			expected[p] = std::uint8_t(digit);
		}
		check(actual, expected, n < 1000);
	}
}

template<int Block>
void uniform_test() {
	using Vector = dynamic_wavelet_detail::QuadVector<Block, 4>;
	for (int digit = 0; digit < 4; ++digit) for (int n : {1, 64, Block + 1, 2 * Block + 1}) {
		std::vector<std::uint8_t> expected(n, std::uint8_t(digit));
		Vector actual(expected);
		check(actual, expected, n < 1000);
		assert(actual.move_rank(n - 1, 0, digit) == std::tuple(n - 1, digit, 0));
		check(actual, expected, false);
		int other = (digit + 1) % 4;
		assert(actual.insert(0, other) == 0);
		expected.insert(expected.begin(), std::uint8_t(other));
		check(actual, expected, false);
		assert(actual.erase_rank(0) == std::pair(0, other));
		expected.erase(expected.begin());
		assert(actual.set_rank(n / 2, other) == std::tuple(n / 2, digit, 0));
		expected[n / 2] = std::uint8_t(other);
		check(actual, expected, false);
		assert(actual.set_rank(n / 2, digit) == std::tuple(0, other, n / 2));
		expected[n / 2] = std::uint8_t(digit);
		check(actual, expected, false);
		assert(actual.move_rank(0, n - 1, other) == std::tuple(0, digit, 0));
		expected.erase(expected.begin());
		expected.push_back(std::uint8_t(other));
		check(actual, expected, false);
		assert(actual.move_rank(n - 1, 0, digit) == std::tuple(0, other, 0));
		expected.pop_back();
		expected.insert(expected.begin(), std::uint8_t(digit));
		auto copied = actual;
		Vector moved(std::move(copied));
		assert(copied.empty());
		check(moved, expected, false);
		copied = std::move(moved);
		assert(moved.empty());
		check(copied, expected, false);
	}
}

int main() {
	uniform_test<128>(); uniform_test<1024>(); uniform_test<32768>();
	random_test<128, 4>(); random_test<192, 8>(); random_test<256, 32>(); random_test<1024, 32>(); random_test<32768, 4>();
	structure<128, 4>(); structure<192, 8>(); structure<128, 32>(); default_height();
	build_boundaries<128>(); build_boundaries<192>(); build_boundaries<1024>(); build_boundaries<32768>();
}
