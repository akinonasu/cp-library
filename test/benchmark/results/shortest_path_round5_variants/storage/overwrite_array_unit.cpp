#include "components/overwrite_array.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <random>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

struct alignas(128) Record {
	static inline int constructions = 0;
	std::uint64_t first;
	int second;
	Record(std::uint64_t a, int b) noexcept : first(a), second(b) { ++constructions; }
	Record(const Record &) noexcept = default;
};

void check(const OverwriteArray<Record> &array, std::size_t n) {
	assert(array.size() == n && array.empty() == (n == 0));
	std::span<const Record> view(array);
	assert(view.size() == n);
	for (std::size_t i = 0; i < n; ++i) {
		assert(view[i].first == i * 999983 + 7);
		assert(view[i].second == -int(i));
		assert(reinterpret_cast<std::uintptr_t>(&view[i]) % alignof(Record) == 0);
	}
}

int main() {
	static_assert(std::is_trivially_destructible_v<Record>);
	static_assert(std::is_nothrow_copy_constructible_v<Record>);
	static_assert(std::is_nothrow_move_constructible_v<OverwriteArray<Record>>);
	static_assert(std::is_nothrow_move_assignable_v<OverwriteArray<Record>>);
	OverwriteArray<Record> array, destination;
	check(array, 0);
	OverwriteArray<Record> empty_copy(array);
	destination = array;
	check(empty_copy, 0);
	check(destination, 0);
	std::mt19937 random(981721);
	for (std::size_t n : {0, 1, 2, 65, 17, 257, 4, 0, 1000}) {
		int before = Record::constructions;
		array.resize(n);
		assert(Record::constructions == before);
		std::vector<std::size_t> order(n);
		std::iota(order.begin(), order.end(), 0);
		std::shuffle(order.begin(), order.end(), random);
		for (auto i : order) std::construct_at(array.data() + i, i * 999983 + 7, -int(i));
		assert(Record::constructions == before + int(n));
		check(array, n);
		OverwriteArray<Record> copy(array);
		check(copy, n);
		assert(n == 0 || copy.data() != array.data());
		destination = array;
		check(destination, n);
		auto *self = &destination;
		destination = *self;
		destination = std::move(*self);
		check(destination, n);
		OverwriteArray<Record> moved(std::move(copy));
		check(moved, n);
		check(copy, 0);
		copy = std::move(destination);
		check(copy, n);
		check(destination, 0);
		if (n) {
			copy[0].second = 123;
			assert(array[0].second == 0 && moved[0].second == 0);
		}
		auto retained = copy.data();
		copy.clear();
		check(copy, 0);
		assert(copy.data() == retained);
	}
	auto retained = array.data();
	bool failed = false;
	try { array.resize(std::numeric_limits<std::size_t>::max()); }
	catch (const std::bad_alloc &) { failed = true; }
	assert(failed && array.data() == retained);
	check(array, 1000);
	OverwriteArray<Record> empty;
	array = empty;
	check(array, 0);
	array = std::move(empty);
	check(array, 0);
	check(empty, 0);
	OverwriteArray<std::pair<int, std::uint64_t>> pairs;
	pairs.resize(3);
	std::construct_at(pairs.data() + 2, 7, std::uint64_t(1) << 63);
	std::construct_at(pairs.data(), -4, std::uint64_t(5));
	std::construct_at(pairs.data() + 1, 2, std::uint64_t(9));
	auto copied_pairs = pairs;
	assert(copied_pairs[2].first == 7 && copied_pairs[2].second == std::uint64_t(1) << 63);
	assert(copied_pairs[0].first == -4 && copied_pairs[1].second == 9);
}
