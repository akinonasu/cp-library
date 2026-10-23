#include "data_structure/range_query/offline_range_frequency.hpp"
#include "data_structure/range_query/offline_range_count_distinct.hpp"
#include "data_structure/range_query/offline_range_sum.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <vector>

struct Key {
	int value;
	explicit Key(int x) : value(x) {}
	bool operator==(const Key &) const = default;
	bool operator<(const Key &other) const { return value < other.value; }
};
struct Hash { std::size_t operator()(const Key &key) const { return std::size_t(key.value); } };
struct Group {
	long long value = 0;
	Group() = default;
	explicit Group(Key key) : value(key.value) {}
	Group &operator+=(const Group &other) { value += other.value; return *this; }
	Group &operator-=(const Group &other) { value -= other.value; return *this; }
};

template <class T, class Sum>
void check_sum(const std::vector<T> &a, const std::vector<T> &bounds) {
	OfflineRangeSum<T, Sum> queries(a);
	std::vector<std::pair<Sum, int>> expected;
	for (int l = 0; l <= int(a.size()); ++l) for (int r = l; r <= int(a.size()); ++r) for (T upper : bounds) {
		Sum sum{}; int count = 0;
		for (int i = l; i < r; ++i) if (a[i] < upper) { sum += Sum(a[i]); ++count; }
		assert(queries.add_query(l, r, upper) == int(expected.size()));
		expected.push_back({sum, count});
	}
	assert(queries.solve() == expected);
	const auto copy = queries;
	assert(copy.solve() == expected);
	queries.build({});
	assert(queries.solve().empty());
	queries.add_query(0, 0, T{});
	assert((queries.solve() == std::vector<std::pair<Sum, int>>{{Sum{}, 0}}));
}

int main() {
	std::mt19937_64 random(81238139);
	for (int trial = 0; trial < 700; ++trial) {
		int n = trial < 140 ? trial : int(random() % 200);
		std::vector<int> a(n);
		for (auto &value : a) value = int(random() % 13) - 6;
		OfflineRangeFrequency frequency(a);
		std::vector<int> expected_frequency, expected_distinct;
		for (int query = 0; query < 300; ++query) {
			int l = int(random() % (n + 1)), r = int(random() % (n + 1));
			if (l > r) std::swap(l, r);
			int value = int(random() % 21) - 10;
			if (n && random() % 2) {
				int p = int(random() % n);
				a[p] = value;
				frequency.set(p, value);
			} else {
				assert(frequency.add_query(l, r, value) == int(expected_frequency.size()));
				expected_frequency.push_back(std::count(a.begin() + l, a.begin() + r, value));
			}
		}
		assert(frequency.solve() == expected_frequency);
		assert(frequency.solve() == expected_frequency);
		auto copied = frequency;
		auto moved = std::move(copied);
		assert(moved.solve() == expected_frequency);
		frequency.build(a);
		assert(frequency.solve().empty());
		for (int value = -10; value <= 10; ++value) {
			frequency.add_query(0, n, value);
			expected_frequency.push_back(std::count(a.begin(), a.end(), value));
		}
		assert(frequency.solve() == std::vector<int>(expected_frequency.end() - 21, expected_frequency.end()));
		OfflineRangeCountDistinct other(a);
		expected_distinct.clear();
		for (int query = 0; query < 300; ++query) {
			int l = int(random() % (n + 1)), r = int(random() % (n + 1));
			if (l > r) std::swap(l, r);
			other.add_query(l, r);
			expected_distinct.push_back(int(std::set<int>(a.begin() + l, a.begin() + r).size()));
		}
		assert(other.solve() == expected_distinct);
		assert(other.solve() == expected_distinct);
		other.build({});
		other.add_query(0, 0);
		assert(other.solve() == std::vector<int>{0});
	}
	for (int n : {0, 1, 15, 16, 17, 255, 256, 257, 4095, 4096, 65536, 65537}) {
		IncrementalPrefixCount counts(n);
		std::vector<int> expected(n);
		for (int i = 0; i < 300; ++i) {
			if (n) { int position = int(random() % n); ++expected[position]; counts.increment(position); }
			int r = int(random() % (n + 1)), sum = 0;
			for (int j = 0; j < r; ++j) sum += expected[j];
			assert(counts.prefix(r) == sum);
		}
		OfflineRangeFrequency frequency(expected);
		std::vector<int> answers;
		for (int i = 0; i < 300; ++i) {
			int l = int(random() % (n + 1)), r = int(random() % (n + 1)), value = int(random() % 5);
			if (l > r) std::swap(l, r);
			frequency.add_query(l, r, value);
			answers.push_back(std::count(expected.begin() + l, expected.begin() + r, value));
		}
		assert(frequency.solve() == answers);
	}
	for (int n = 0; n <= 7; ++n) for (int trial = 0; trial < 25; ++trial) {
		std::vector<std::int8_t> a(n);
		for (auto &value : a) value = std::int8_t(int(random() % 9) - 4);
		check_sum<std::int8_t, long long>(a, {-5, -4, -1, 0, 1, 4, 5});
	}
	check_sum<std::int64_t, __int128_t>({INT64_MIN, INT64_MAX, -1, 0, 1, INT64_MAX, INT64_MIN}, {INT64_MIN, INT64_MIN + 1, -1, 0, 1, INT64_MAX});
	check_sum<std::int64_t, __int128_t>({INT64_MIN + 5, INT64_MIN + 8, INT64_MIN + 11}, {INT64_MIN, INT64_MIN + 10, INT64_MIN + 100});
	check_sum<std::uint64_t, __uint128_t>({UINT64_MAX - 100, UINT64_MAX - 10, UINT64_MAX - 50}, {0, UINT64_MAX - 55, UINT64_MAX});
	check_sum<double, double>({-1, 3, 0, -2, 5}, {-3, -2, 0, 2, 5, 6});
	OfflineRangeSum<std::int64_t, std::int64_t> narrow({INT64_MAX - 1, INT64_MAX - 1, 1 - INT64_MAX, 1 - INT64_MAX});
	narrow.add_query(0, 1, INT64_MAX);
	narrow.add_query(1, 3, INT64_MAX);
	narrow.add_query(2, 3, INT64_MAX);
	assert((narrow.solve() == std::vector<std::pair<std::int64_t, int>>{{INT64_MAX - 1, 1}, {0, 2}, {1 - INT64_MAX, 1}}));
	OfflineRangeSum<Key, Group> generic({Key(3), Key(1), Key(5), Key(3)});
	generic.add_query(0, 4, Key(4));
	generic.add_query(1, 3, Key(2));
	auto answer = generic.solve();
	assert(answer[0].first.value == 7 && answer[0].second == 3);
	assert(answer[1].first.value == 1 && answer[1].second == 1);
	OfflineRangeFrequency<int> introduced({1, 2, 3});
	introduced.reserve(10);
	introduced.add_query(0, 3, 99);
	introduced.set(1, 99);
	introduced.add_query(0, 3, 99);
	introduced.add_query(0, 3, 2);
	assert((introduced.solve() == std::vector<int>{0, 1, 0}));
	OfflineRangeFrequency<Key, Hash> frequency({Key(1), Key(3), Key(1)});
	frequency.add_query(0, 3, Key(1));
	frequency.set(1, Key(1));
	frequency.add_query(0, 3, Key(1));
	assert((frequency.solve() == std::vector<int>{2, 3}));
	OfflineRangeCountDistinct<Key, Hash> distinct({Key(1), Key(3), Key(1)});
	distinct.add_query(0, 3);
	distinct.add_query(1, 3);
	assert((distinct.solve() == std::vector<int>{2, 2}));
	for (int n : {1, 63, 64, 65, 255, 256, 257, 4095, 4096, 4097, 65535, 65536, 65537, 131073}) {
		std::vector<std::int64_t> a(n);
		for (auto &value : a) value = std::int64_t(random() % 200) - 100;
		OfflineRangeSum<std::int64_t, std::int64_t> queries(a);
		std::vector<std::pair<std::int64_t, int>> expected;
		for (int i = 0; i < 300; ++i) {
			int l = int(random() % (n + 1)), r = int(random() % (n + 1));
			if (l > r) std::swap(l, r);
			std::int64_t upper = std::int64_t(random() % 240) - 120, sum = 0; int count = 0;
			for (int j = l; j < r; ++j) if (a[j] < upper) { sum += a[j]; ++count; }
			queries.add_query(l, r, upper);
			expected.push_back({sum, count});
		}
		assert(queries.solve() == expected);
	}
}
