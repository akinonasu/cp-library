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
	AdditiveSum &operator-=(const AdditiveSum &x) { value -= x.value; return *this; }
};

template<class Set, class Compare>
void check(const Set &set, std::vector<typename Set::value_type> values, Compare compare) {
	std::sort(values.begin(), values.end(), compare);
	const auto count = (std::min)(set.k(), values.size());
	typename Set::sum_type sum = 0;
	for (std::size_t i = 0; i < count; ++i) sum += static_cast<typename Set::sum_type>(values[i]);
	assert(set.size() == values.size() && set.empty() == values.empty());
	assert(set.selected_size() == count && set.sum() == sum);
	for (int value : {-20, -3, 0, 3, 20}) {
		auto occurrences = static_cast<std::size_t>(std::count(values.begin(), values.end(), value));
		assert(set.count(value) == occurrences && set.contains(value) == (occurrences != 0));
	}
}

template<class Compare>
void randomized(Compare compare) {
	std::mt19937 rng(9413);
	TopKSum<int, long long, Compare> set(0, compare);
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
			set.set_k(rng() % (values.size() + 20));
		} else if (operation == 10) {
			values.resize(rng() % 65);
			for (auto &x : values) x = static_cast<int>(rng() % 41) - 20;
			set.assign(values);
		} else if (operation == 12) {
			const std::size_t copies = rng() % 21;
			set.insert(value, copies);
			values.insert(values.end(), copies, value);
		} else if (operation == 13) {
			const std::size_t copies = rng() % 21;
			std::size_t removed = 0;
			while (removed < copies) {
				auto it = std::find(values.begin(), values.end(), value);
				if (it == values.end()) break;
				values.erase(it);
				++removed;
			}
			assert(set.erase(value, copies) == removed);
		} else if (step % 31 == 0) {
			const auto k = set.k();
			set.clear();
			values.clear();
			assert(set.k() == k);
		}
		check(set, values, compare);
		if (step % 271 == 0) {
			auto copy = set;
			check(copy, values, compare);
			auto moved = std::move(copy);
			check(moved, values, compare);
			assert(copy.empty() && copy.sum() == 0 && copy.k() == set.k());
			copy.insert(3);
			check(copy, std::vector<int>{3}, compare);
			copy = moved;
			copy = std::move(moved);
			assert(moved.empty() && moved.sum() == 0);
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
	static_assert(std::is_same_v<decltype(std::declval<TopKSum<>&>().erase(0, 0)), TopKSum<>::size_type>);
	TopKSum<> largest(2, {3, 1, 5, 5});
	assert(largest.sum() == 10 && largest.count(5) == 2);
	assert(largest.erase(5) && largest.sum() == 8);
	assert(largest.replace(1, 8) && largest.sum() == 13);
	assert(!largest.replace(99, 1) && largest.sum() == 13);
	largest.set_k(20);
	assert(largest.sum() == 16 && largest.k() == 20 && largest.selected_size() == 3);
	largest.set_k(0);
	assert(largest.sum() == 0 && largest.size() == 3);
	largest.set_k((std::numeric_limits<std::size_t>::max)());
	assert(largest.sum() == 16);
	SmallestKSum<> smallest(2, {3, 1, 5, 5});
	assert(smallest.sum() == 4);
	SmallestKSum<> empty(8);
	swap(smallest, empty);
	assert(smallest.empty() && smallest.k() == 8 && empty.sum() == 4 && empty.k() == 2);
	randomized(std::less<int>{});
	randomized(std::greater<int>{});
	randomized(Direction{false});
	randomized(Direction{true});
	TopKSum<int, long long, Direction> a(1, {2, 3}, Direction{true});
	TopKSum<int, long long, Direction> b(1, {8, 9}, Direction{false});
	swap(a, b);
	a.insert(0);
	b.insert(10);
	assert(a.sum() == 0 && b.sum() == 10);

	const auto lo = (std::numeric_limits<std::int64_t>::min)();
	const auto hi = (std::numeric_limits<std::int64_t>::max)();
	TopKSum<std::int64_t, __int128_t> wide(3, {lo, lo, hi, hi});
	assert(wide.sum() == __int128_t(hi) * 2 + lo);
	wide.set_k(4);
	assert(wide.sum() == -2);
	assert(wide.replace(lo, hi));
	assert(wide.sum() == __int128_t(hi) * 3 + lo);
	TopKSum<int> widened(3, {2000000000, 2000000000, 2000000000});
	assert(widened.sum() == 6000000000LL);
	TopKSum<unsigned, std::uint64_t> unsigned_values(2, {0, 3, 5, 7});
	assert(unsigned_values.sum() == 12 && unsigned_values.erase(7) && unsigned_values.sum() == 8);
	TopKSum<double> decimals(2, {1.25, -0.5, 3.5});
	assert(decimals.sum() == 4.75 && decimals.replace(3.5, -4.0) && decimals.sum() == 0.75);
	TopKSum<> single(1, {5, 4, 3});
	assert(single.replace(5, 0) && single.sum() == 4);
	assert(single.replace(4, 8) && single.sum() == 8);
	TopKSum<> batch(3, {7, 5, 5, 1});
	batch.insert(100, 0);
	assert(batch.erase(5, 0) == 0 && batch.size() == 4 && batch.sum() == 17);
	batch.insert(5, 4);
	assert(batch.count(5) == 6 && batch.sum() == 17);
	assert(batch.erase(5, 3) == 3 && batch.count(5) == 3 && batch.sum() == 17);
	assert(batch.erase(7, 4) == 1 && batch.sum() == 15);
	assert(batch.erase(5, (std::numeric_limits<std::size_t>::max)()) == 3 && batch.sum() == 1);
	assert(batch.erase(99, 100) == 0 && batch.sum() == 1);
	batch.insert(-3, 100000);
	assert(batch.size() == 100001 && batch.sum() == -5);
	assert(batch.erase(-3, 100000) == 100000 && batch.sum() == 1);
	batch.set_k(0);
	batch.insert(8, 4);
	assert(batch.sum() == 0 && batch.count(8) == 4);
	assert(batch.erase(8, 8) == 4 && batch.sum() == 0);
	batch.set_k(20);
	batch.insert(9, 4);
	assert(batch.sum() == 37 && batch.k() == 20 && batch.selected_size() == 5);
	assert(batch.erase(9, 8) == 4 && batch.sum() == 1);
	TopKSum<int, AdditiveSum> additive(2);
	additive.insert(3, 4);
	assert(additive.sum().value == 6);
	assert(additive.erase(3, 3) == 3 && additive.sum().value == 3);
	auto bucket = [](int a, int b) { return a / 10 < b / 10; };
	TopKSum<int, long long, decltype(bucket)> equivalent(1, bucket);
	equivalent.insert(11);
	equivalent.insert(12);
	equivalent.insert(31);
	assert(equivalent.erase(19, 2) == 2 && equivalent.sum() == 31);
	TopKSum<std::int64_t, __int128_t> wide_batch(2);
	wide_batch.insert(hi, 4);
	assert(wide_batch.sum() == __int128_t(hi) * 2);
	assert(wide_batch.erase(hi, 3) == 3 && wide_batch.sum() == hi);
}
