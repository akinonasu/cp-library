#include "data_structure/top_k_sum.hpp"
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <type_traits>

struct Direction {
	bool descending;
	bool operator()(int a, int b) const { return descending ? a > b : a < b; }
};

struct AdditiveSum {
	long long value;
	explicit AdditiveSum(long long value) : value(value) {}
	AdditiveSum &operator+=(const AdditiveSum &x) { value += x.value; return *this; }
	friend AdditiveSum operator*(AdditiveSum x, std::int64_t copies) { return AdditiveSum(x.value * copies); }
};

template<class Set, class Compare>
void check(const Set &set, std::vector<typename Set::value_type> values, Compare compare) {
	std::sort(values.begin(), values.end(), compare);
	assert(set.size() == static_cast<typename Set::size_type>(values.size()) && set.empty() == values.empty());
	typename Set::sum_type prefix = 0;
	for (std::size_t k = 0; k <= values.size(); ++k) {
		assert(set.sum(k) == prefix);
		if (k < values.size()) {
			assert(set.kth(k) == values[k]);
			prefix += static_cast<typename Set::sum_type>(values[k]);
		}
	}
	assert(set.sum((std::numeric_limits<typename Set::size_type>::max)()) == prefix);
	assert(set.sum_all() == prefix);
	auto ascending = values;
	if constexpr (std::is_same_v<Compare, std::greater<typename Set::value_type>>) std::reverse(ascending.begin(), ascending.end());
	typename Set::sum_type small_sum = 0, large_sum = 0;
	for (std::size_t k = 0; k <= ascending.size(); ++k) {
		assert(set.sum_smallest(k) == small_sum && set.sum_largest(k) == large_sum);
		if (k < ascending.size()) {
			assert(set.kth_smallest(k) == ascending[k]);
			assert(set.kth_largest(k) == ascending[ascending.size() - 1 - k]);
			small_sum += static_cast<typename Set::sum_type>(ascending[k]);
			large_sum += static_cast<typename Set::sum_type>(ascending[ascending.size() - 1 - k]);
		}
	}
	assert(set.sum_smallest((std::numeric_limits<typename Set::size_type>::max)()) == small_sum);
	assert(set.sum_largest((std::numeric_limits<typename Set::size_type>::max)()) == large_sum);
	for (int value : {-20, -3, 0, 3, 20}) {
		auto occurrences = std::count(values.begin(), values.end(), value);
		assert(set.order_of_key(value) == std::lower_bound(values.begin(), values.end(), value, compare) - values.begin());
		assert(set.count(value) == occurrences && set.contains(value) == (occurrences != 0));
		auto ascending_compare = [&](const auto &a, const auto &b) {
			if constexpr (std::is_same_v<Compare, std::greater<typename Set::value_type>>) return compare(b, a);
			else return compare(a, b);
		};
		auto lower = std::lower_bound(ascending.begin(), ascending.end(), value, ascending_compare);
		auto upper = std::upper_bound(ascending.begin(), ascending.end(), value, ascending_compare);
		assert(set.lower_bound(value) == (lower == ascending.end() ? std::nullopt : std::optional(*lower)));
		assert(set.upper_bound(value) == (upper == ascending.end() ? std::nullopt : std::optional(*upper)));
	}
}

template<class Compare>
void randomized(Compare compare) {
	std::mt19937 rng(9413);
	TopKSum<int, long long, Compare> set(compare);
	std::vector<int> values;
	for (int step = 0; step < 30000; ++step) {
		const int operation = rng() % 14;
		const int value = static_cast<int>(rng() % 41) - 20;
		if (operation < 5) {
			set.insert(value);
			values.push_back(value);
		} else if (operation < 8) {
			auto it = std::find(values.begin(), values.end(), value);
			assert(set.erase(value) == (it != values.end()));
			if (it != values.end()) values.erase(it);
		} else if (operation == 8) {
			int replacement = static_cast<int>(rng() % 41) - 20;
			auto it = std::find(values.begin(), values.end(), value);
			assert(set.replace(value, replacement) == (it != values.end()));
			if (it != values.end()) *it = replacement;
		} else if (operation == 9) {
			assert(set.erase_all(value) == std::count(values.begin(), values.end(), value));
			values.erase(std::remove(values.begin(), values.end(), value), values.end());
		} else if (operation == 10) {
			values.resize(rng() % 65);
			for (auto &x : values) x = static_cast<int>(rng() % 41) - 20;
			set.assign(values);
		} else if (operation == 12) {
			const std::int64_t copies = rng() % 21;
			set.insert(value, copies);
			values.insert(values.end(), copies, value);
		} else if (operation == 13) {
			const std::int64_t copies = rng() % 21;
			std::int64_t removed = 0;
			while (removed < copies) {
				auto it = std::find(values.begin(), values.end(), value);
				if (it == values.end()) break;
				values.erase(it);
				++removed;
			}
			assert(set.erase(value, copies) == removed);
		} else if (step % 31 == 0) {
			set.clear();
			values.clear();
		}
		check(set, values, compare);
		if (step % 271 == 0) {
			auto copy = set;
			check(copy, values, compare);
			auto moved = std::move(copy);
			check(moved, values, compare);
			assert(copy.empty() && copy.sum(100) == 0);
			copy.insert(3);
			check(copy, std::vector<int>{3}, compare);
			copy = moved;
			copy = std::move(moved);
			assert(moved.empty() && moved.sum_all() == 0);
			moved.insert(4, 2);
			check(moved, std::vector<int>{4, 4}, compare);
			check(copy, values, compare);
			const auto &alias = copy;
			copy = alias;
			check(copy, values, compare);
			auto &same = copy;
			copy = std::move(same);
			check(copy, values, compare);
		}
	}
}

int main() {
	static_assert(std::is_same_v<TopKSum<int>::sum_type, std::int64_t>);
	static_assert(std::is_signed_v<TopKSum<>::size_type>);
	static_assert(std::is_same_v<decltype(std::declval<const TopKSum<int>&>().kth(0)), int>);
	static_assert(std::is_same_v<decltype(std::declval<TopKSum<>&>().erase(0, 0)), TopKSum<>::size_type>);
	TopKSum<> largest{3, 1, 5, 5};
	const auto &read = largest;
	assert(read.sum(0) == 0 && read.sum(1) == 5 && read.sum(2) == 10 && read.sum(4) == 14);
	assert(read.sum(100) == 14 && read.sum_all() == 14);
	assert(read.kth_smallest(0) == 1 && read.kth_largest(0) == 5);
	assert(read.sum_smallest(2) == 4 && read.sum_largest(2) == 10);
	assert(read.lower_bound(4) == 5 && read.upper_bound(3) == 5);
	assert(!read.lower_bound(6) && !read.upper_bound(5));
	assert(largest.erase(5) && read.sum(2) == 8);
	assert(largest.replace(1, 8) && read.sum(2) == 13);
	assert(!largest.replace(99, 1) && read.sum(2) == 13);
	SmallestKSum<> smallest{3, 1, 5, 5};
	assert(smallest.sum(2) == 4 && smallest.kth(2) == 5);
	SmallestKSum<> empty;
	swap(smallest, empty);
	assert(smallest.empty() && smallest.sum(10) == 0 && empty.sum(2) == 4);

	randomized(std::less<int>{});
	randomized(std::greater<int>{});
	randomized(Direction{false});
	randomized(Direction{true});
	TopKSum<int, long long, Direction> a({2, 3}, Direction{true});
	TopKSum<int, long long, Direction> b({8, 9}, Direction{false});
	swap(a, b);
	a.insert(0); b.insert(10);
	assert(a.sum(1) == 0 && b.sum(1) == 10);

	const auto lo = (std::numeric_limits<std::int64_t>::min)();
	const auto hi = (std::numeric_limits<std::int64_t>::max)();
	TopKSum<> mixed{lo, lo, hi, hi};
	assert(mixed.sum(1) == hi && mixed.sum(4) == -2);
	assert(mixed.kth(0) == hi && mixed.kth(3) == lo);
	assert(mixed.sum_smallest(1) == lo && mixed.sum_largest(1) == hi);
	assert(mixed.lower_bound(lo) == lo && mixed.upper_bound(lo) == hi);
	assert(mixed.lower_bound(hi) == hi && !mixed.upper_bound(hi));
	SmallestKSum<> negative{lo, lo, hi, hi};
	assert(negative.sum(1) == lo && negative.sum_all() == -2);
	assert(negative.sum_smallest(1) == lo && negative.sum_largest(1) == hi);
	TopKSum<std::int64_t, __int128_t> wide{lo, lo, hi, hi};
	assert(wide.sum(2) == __int128_t(hi) * 2 && wide.sum(3) == __int128_t(hi) * 2 + lo);
	wide.insert(hi, 1000000000000LL);
	assert(wide.sum(1000000000000LL) == __int128_t(hi) * 1000000000000LL);
	assert(wide.erase(hi, 1000000000001LL) == 1000000000001LL && wide.count(hi) == 1);
	TopKSum<> maximum;
	maximum.insert(1, hi);
	assert(maximum.size() == hi && maximum.count(1) == hi && maximum.sum(hi) == hi && maximum.kth(hi - 1) == 1);
	assert(maximum.erase(1));
	assert(maximum.replace(1, -1) && maximum.sum_all() == hi - 3);
	maximum.clear();
	maximum.insert(lo, 1000000000000LL);
	assert(maximum.sum(1) == lo && maximum.kth(999999999999LL) == lo);
	assert(maximum.erase_all(lo) == 1000000000000LL && maximum.empty());
	assert(maximum.sum_smallest(1) == 0 && maximum.sum_largest(1) == 0);
	assert(!maximum.lower_bound(lo) && !maximum.upper_bound(hi));
	TopKSum<unsigned, std::uint64_t> unsigned_values{0, 3, 5, 7};
	assert(unsigned_values.sum(2) == 12 && unsigned_values.erase(7) && unsigned_values.sum(2) == 8);
	assert(unsigned_values.lower_bound(0) == 0 && unsigned_values.upper_bound(0) == 3);
	assert(!unsigned_values.lower_bound((std::numeric_limits<unsigned>::max)()));
	TopKSum<double> decimals{1.25, -0.5, 3.5};
	assert(decimals.sum(2) == 4.75 && decimals.replace(3.5, -4.0) && decimals.sum(2) == 0.75);
	TopKSum<int, AdditiveSum> additive;
	additive.insert(3, 4);
	assert(additive.sum(2).value == 6 && additive.sum_all().value == 12);
	additive.insert(1, 2);
	assert(additive.sum_smallest(3).value == 5 && additive.sum_largest(3).value == 9);
	assert(additive.erase_all(1) == 2);
	assert(additive.erase(3, 3) == 3 && additive.sum(2).value == 3);

	SmallestKSum<std::int8_t> bytes{-128, -1, 0, 127};
	assert(bytes.kth(0) == -128 && bytes.kth(3) == 127 && bytes.order_of_key(0) == 2);
	SmallestKSum<std::uint16_t> shorts{0, 32768, 65535};
	assert(shorts.sum(3) == 98303 && shorts.lower_bound(32769) == 65535);
	SmallestKSum<std::uint64_t, __uint128_t> wide_unsigned{0, std::uint64_t(1) << 63, (std::numeric_limits<std::uint64_t>::max)()};
	assert(wide_unsigned.order_of_key(std::uint64_t(1) << 63) == 1);
	assert(wide_unsigned.kth(2) == (std::numeric_limits<std::uint64_t>::max)());
	assert(wide_unsigned.sum(3) == (__uint128_t(3) << 63) - 1);
	assert(wide_unsigned.erase(std::uint64_t(1) << 63));
	assert(wide_unsigned.sum(2) == (std::numeric_limits<std::uint64_t>::max)());

	std::mt19937_64 full_range_rng(783105);
	std::vector<std::uint64_t> full_range(512);
	for (auto &value : full_range) value = full_range_rng();
	std::sort(full_range.begin(), full_range.end());
	SmallestKSum<std::uint64_t, __uint128_t> full_width(full_range);
	__uint128_t full_prefix = 0;
	for (std::size_t i = 0; i < full_range.size(); ++i) {
		assert(full_width.order_of_key(full_range[i]) == static_cast<std::int64_t>(i));
		assert(full_width.kth(i) == full_range[i]);
		assert(full_width.lower_bound(full_range[i]) == full_range[i]);
		assert(full_width.upper_bound(full_range[i]) == (i + 1 == full_range.size() ? std::nullopt : std::optional(full_range[i + 1])));
		full_prefix += full_range[i];
		assert(full_width.sum(i + 1) == full_prefix);
	}
	for (std::size_t i = 0; i < full_range.size(); i += 2) assert(full_width.erase(full_range[i]));
	for (std::size_t i = 1; i < full_range.size(); i += 2) assert(full_width.kth(i / 2) == full_range[i]);

	std::vector<std::int64_t> shuffled(12000);
	std::iota(shuffled.begin(), shuffled.end(), -6000);
	std::mt19937 rng(31249);
	std::shuffle(shuffled.begin(), shuffled.end(), rng);
	SmallestKSum<std::int64_t, __int128_t> blocks;
	std::vector<std::int64_t> remaining;
	for (auto value : shuffled) { blocks.insert(value, 3); remaining.push_back(value); }
	std::sort(remaining.begin(), remaining.end());
	for (auto value : shuffled) {
		auto it = std::lower_bound(remaining.begin(), remaining.end(), value);
		assert(blocks.order_of_key(value) == 3 * (it - remaining.begin()));
		assert(blocks.erase_all(value) == 3);
		remaining.erase(it);
		if (!remaining.empty()) {
			int k = rng() % remaining.size();
			assert(blocks.kth(3 * k) == remaining[k]);
			__int128_t expected = 0;
			for (int i = 0; i < k; ++i) expected += 3 * __int128_t(remaining[i]);
			assert(blocks.sum(3 * k) == expected);
			assert(blocks.lower_bound(remaining[k]) == remaining[k]);
		}
	}
	assert(blocks.empty());
	for (auto value : shuffled) blocks.insert(value);
	assert(blocks.size() == static_cast<std::int64_t>(shuffled.size()));

	TopKSum<int> alternate;
	alternate.reserve(100000);
	std::vector<int> sequence(100000);
	std::iota(sequence.begin(), sequence.end(), 1);
	alternate.assign(sequence);
	for (int i = 0; i < 100000; ++i) {
		assert(alternate.kth(i) == 100000 - i);
		assert(alternate.sum(i % 2 ? 1 : 99999) == (i % 2 ? 100000LL : 5000049999LL));
	}
	for (int x : sequence) assert(alternate.erase(x));
	assert(alternate.empty() && alternate.sum(100) == 0);
	for (int repeat = 0; repeat < 3; ++repeat) {
		for (int x : sequence) alternate.insert(x);
		for (int x : sequence) assert(alternate.erase(x));
		assert(alternate.empty());
	}
}
