#include "data_structure/super_multiset.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

struct Direction {
	bool descending;
	bool operator()(int a, int b) const { return descending ? a > b : a < b; }
};

struct AdditiveSum {
	std::int64_t value;
	explicit AdditiveSum(std::int64_t x) : value(x) {}
	AdditiveSum &operator+=(const AdditiveSum &other) { value += other.value; return *this; }
	friend AdditiveSum operator*(AdditiveSum x, std::int64_t copies) { return AdditiveSum(x.value * copies); }
};

struct Bucket {
	int key, tag;
	explicit operator std::int64_t() const { return key / 10; }
};

struct BucketLess {
	bool operator()(const Bucket &a, const Bucket &b) const { return a.key / 10 < b.key / 10; }
};

template<class T>
__int128_t number(const T &value) {
	if constexpr (std::is_same_v<T, AdditiveSum>) return value.value;
	else return static_cast<__int128_t>(value);
}

template<class Set, class Compare>
void check(const Set &set, const std::vector<typename Set::value_type> &values, Compare compare,
	const std::vector<typename Set::value_type> &probes) {
	using T = typename Set::value_type;
	using Sum = typename Set::sum_type;
	using Count = typename Set::size_type;
	static_assert(std::is_same_v<decltype(set.count_less(std::declval<const T &>())), Count>);
	static_assert(std::is_same_v<decltype(set.count_leq(std::declval<const T &>())), Count>);
	static_assert(std::is_same_v<decltype(set.count_greater(std::declval<const T &>())), Count>);
	static_assert(std::is_same_v<decltype(set.count_geq(std::declval<const T &>())), Count>);
	static_assert(std::is_same_v<decltype(set.sum_less(std::declval<const T &>())), Sum>);
	static_assert(std::is_same_v<decltype(set.sum_leq(std::declval<const T &>())), Sum>);
	static_assert(std::is_same_v<decltype(set.sum_greater(std::declval<const T &>())), Sum>);
	static_assert(std::is_same_v<decltype(set.sum_geq(std::declval<const T &>())), Sum>);
	static_assert(std::is_same_v<decltype(set.range_sum_with_count(std::declval<const T &>())), std::pair<Sum, Count>>);
	static_assert(std::is_same_v<decltype(set.min()), std::optional<T>>);
	static_assert(std::is_same_v<decltype(set.max()), std::optional<T>>);
	static_assert(std::is_same_v<decltype(set.next(std::declval<const T &>())), std::optional<T>>);
	static_assert(std::is_same_v<decltype(set.prev(std::declval<const T &>())), std::optional<T>>);
	auto less = [&](const T &a, const T &b) {
		if constexpr (std::is_same_v<Compare, std::greater<T>> || std::is_same_v<Compare, std::greater<void>>) return compare(b, a);
		else return compare(a, b);
	};
	auto check_value = [&](const std::optional<T> &actual, const std::optional<T> &expected) {
		assert(actual.has_value() == expected.has_value());
		if (actual) assert(!less(*actual, *expected) && !less(*expected, *actual));
	};
	std::optional<T> minimum, maximum;
	for (const T &value : values) {
		if (!minimum || less(value, *minimum)) minimum = value;
		if (!maximum || less(*maximum, value)) maximum = value;
	}
	check_value(set.min(), minimum);
	check_value(set.max(), maximum);
	assert(set.size() == Count(values.size()));
	for (const T &x : probes) {
		Count counts[4]{};
		__int128_t sums[4]{};
		std::optional<T> next, prev;
		for (const T &value : values) {
			bool lt = less(value, x), gt = less(x, value);
			if (!lt && (!next || less(value, *next))) next = value;
			if (!gt && (!prev || less(*prev, value))) prev = value;
			bool selected[] = {lt, !gt, gt, !lt};
			for (int i = 0; i < 4; ++i) if (selected[i]) {
				++counts[i];
				sums[i] += number(static_cast<Sum>(value));
			}
		}
		check_value(set.next(x), next);
		check_value(set.prev(x), prev);
		assert(set.count_less(x) == counts[0] && number(set.sum_less(x)) == sums[0]);
		assert(set.count_leq(x) == counts[1] && number(set.sum_leq(x)) == sums[1]);
		assert(set.count_greater(x) == counts[2] && number(set.sum_greater(x)) == sums[2]);
		assert(set.count_geq(x) == counts[3] && number(set.sum_geq(x)) == sums[3]);
		auto combined = set.range_sum_with_count(x);
		assert(number(combined.first) == sums[0] && combined.second == counts[0]);
		for (const T &upper : probes) {
			Count count = 0;
			__int128_t sum = 0;
			if (less(x, upper)) for (const T &value : values)
				if (!less(value, x) && less(value, upper)) {
					++count;
					sum += number(static_cast<Sum>(value));
				}
			assert(set.range_freq(x, upper) == count && number(set.range_sum(x, upper)) == sum);
			auto range = set.range_sum_with_count(x, upper);
			assert(number(range.first) == sum && range.second == count);
		}
	}
}

template<class Compare>
void randomized(Compare compare) {
	SuperMultiset<int, std::int64_t, Compare> set(compare);
	std::vector<int> values, probes{-40, -20, -1, 0, 1, 20, 40};
	std::mt19937 rng(463819);
	check(set, values, compare, probes);
	for (int step = 0; step < 800; ++step) {
		int x = int(rng() % 61) - 30, copies = int(rng() % 6);
		switch (rng() % 7) {
			case 0: case 1:
				set.insert(x, copies);
				values.insert(values.end(), copies, x);
				break;
			case 2: {
				std::int64_t removed = 0;
				for (; removed < copies; ++removed) {
					auto it = std::find(values.begin(), values.end(), x);
					if (it == values.end()) break;
					values.erase(it);
				}
				assert(set.erase(x, copies) == removed);
				break;
			}
			case 3: {
				std::int64_t removed = std::count(values.begin(), values.end(), x);
				assert(set.erase_all(x) == removed);
				values.erase(std::remove(values.begin(), values.end(), x), values.end());
				break;
			}
			case 4: {
				int replacement = int(rng() % 61) - 30;
				auto it = std::find(values.begin(), values.end(), x);
				assert(set.replace(x, replacement) == (it != values.end()));
				if (it != values.end()) *it = replacement;
				break;
			}
			case 5:
				values.resize(rng() % 30);
				for (int &value : values) value = int(rng() % 61) - 30;
				set.assign(values);
				break;
			default:
				if (step % 11 == 0) { set.clear(); values.clear(); }
		}
		check(set, values, compare, probes);
		if (step % 97 == 0) {
			auto copy = set;
			check(copy, values, compare, probes);
			SuperMultiset<int, std::int64_t, Compare> moved(std::move(copy));
			check(copy, {}, compare, probes);
			check(moved, values, compare, probes);
			copy = moved;
			set.swap(copy);
			check(set, values, compare, probes);
		}
	}
}

template<class Compare>
void extremes() {
	using T = std::int64_t;
	constexpr T lo = (std::numeric_limits<T>::min)(), hi = (std::numeric_limits<T>::max)();
	std::vector<T> values{lo, lo, -1, 0, 1, hi, hi};
	SuperMultiset<T, __int128_t, Compare> wide(values);
	check(wide, values, Compare{}, {lo, lo + 1, -1, 0, 1, hi - 1, hi});
	SuperMultiset<T, T, Compare> mixed(values);
	assert(mixed.range_sum(-1, 2) == 0 && mixed.range_freq(-1, 2) == 3);
	assert(mixed.range_sum_with_count(-1, 2) == std::make_pair(T(0), T(3)));
	assert(mixed.range_sum(0, 1) == 0 && mixed.range_sum(hi, lo) == 0);
	assert(mixed.count_leq(lo) == 2 && mixed.count_geq(hi) == 2);
	SuperMultiset<T, T, Compare> lower{lo, lo, lo, 0, hi}, upper{lo, 0, hi, hi, hi};
	assert(lower.sum_geq(0) == hi && lower.sum_greater(0) == hi);
	assert(lower.sum_less(lo) == 0 && lower.range_sum(hi, hi) == 0);
	assert(upper.sum_leq(0) == lo && upper.sum_less(0) == lo);
	assert(upper.sum_greater(hi) == 0 && upper.range_sum(hi, lo) == 0);
	SuperMultiset<T, __int128_t, Compare> copies;
	copies.insert(lo, hi / 2);
	copies.insert(hi, hi - hi / 2);
	assert(copies.size() == hi && copies.count_leq(lo) == hi / 2);
	assert(copies.count_geq(hi) == hi - hi / 2 && copies.count_less(hi) == hi / 2);
	assert(copies.sum_leq(lo) == __int128_t(lo) * (hi / 2));
	assert(copies.sum_geq(hi) == __int128_t(hi) * (hi - hi / 2));
	assert(copies.range_freq(lo, hi) == hi / 2);
	assert(copies.range_sum_with_count(hi) == std::make_pair(__int128_t(lo) * (hi / 2), hi / 2));
}

int main() {
	randomized(std::less<int>{});
	randomized(std::greater<int>{});
	randomized(std::less<>{});
	randomized(std::greater<>{});
	randomized(Direction{false});
	randomized(Direction{true});
	extremes<std::less<std::int64_t>>();
	extremes<std::greater<std::int64_t>>();
	constexpr std::uint64_t hi = (std::numeric_limits<std::uint64_t>::max)();
	std::vector<std::uint64_t> values{0, 1, 1, std::uint64_t(1) << 63, hi, hi};
	SmallestKSum<std::uint64_t, __uint128_t> wide_unsigned(values);
	check(wide_unsigned, values, std::less<std::uint64_t>{}, {0, 1, 2, std::uint64_t(1) << 63, hi - 1, hi});
	SuperMultiset<std::uint64_t, std::uint64_t> unsigned_copies;
	constexpr std::int64_t count = (std::numeric_limits<std::int64_t>::max)();
	unsigned_copies.insert(3, count - 1); unsigned_copies.insert(4);
	assert(unsigned_copies.count_leq(3) == count - 1 && unsigned_copies.count_geq(0) == count);
	assert(unsigned_copies.sum_greater(3) == 4 && unsigned_copies.sum_geq(4) == 4);
	assert(unsigned_copies.range_sum_with_count(4, 5) == std::make_pair(std::uint64_t(4), std::int64_t(1)));
	assert(unsigned_copies.range_sum(5, 4) == 0 && unsigned_copies.sum_less(3) == 0);
	std::vector<int> integers{-5, -5, 0, 3, 3, 9};
	SuperMultiset<int, AdditiveSum> additive(integers);
	check(additive, integers, std::greater<int>{}, {-10, -5, 0, 3, 10});
	std::vector<Bucket> buckets{ {-29, 1}, {-21, 2}, {-3, 0}, {7, 4}, {11, 7}, {19, 1}, {37, 0} };
	SuperMultiset<Bucket, std::int64_t, BucketLess> grouped(buckets);
	check(grouped, buckets, BucketLess{}, { {-40, 0}, {-20, 9}, {-1, 3}, {15, 4}, {30, 7}, {50, 2} });
	SuperMultiset<int, std::int64_t, Direction> a({-3, 0, 4}, Direction{false}), b({-2, 1, 9}, Direction{true});
	a.swap(b);
	check(a, {-2, 1, 9}, Direction{true}, {-4, 0, 2, 10});
	check(b, {-3, 0, 4}, Direction{false}, {-4, 0, 2, 10});
	b = a;
	check(b, {-2, 1, 9}, Direction{true}, {-4, 0, 2, 10});
	SuperMultiset<double> decimals{-1.5, 0.25, 2.0, 2.0};
	assert(decimals.sum_leq(0.25) == -1.25 && decimals.sum_geq(2.0) == 4.0);
	assert(decimals.count_greater(0.25) == 2 && decimals.range_sum(0.0, 3.0) == 4.25);
	assert(decimals.range_sum_with_count(0.0, 3.0) == std::make_pair(4.25, std::int64_t(3)));
}
