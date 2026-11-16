#include "data_structure/xor_set.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

template <int Bits>
std::uint64_t mask() {
	if constexpr (Bits == 64) return UINT64_MAX;
	else return (UINT64_C(1) << Bits) - 1;
}

template <int Bits>
void check(XorSet<Bits> &actual, const std::set<std::uint64_t> &expected, std::uint64_t query) {
	assert(actual.size() == std::int64_t(expected.size()));
	assert(actual.empty() == expected.empty());
	if (expected.empty()) return;
	auto small = *std::min_element(expected.begin(), expected.end(), [&](auto a, auto b) { return (a ^ query) < (b ^ query); });
	auto large = *std::max_element(expected.begin(), expected.end(), [&](auto a, auto b) { return (a ^ query) < (b ^ query); });
	assert(actual.argmin_xor(query) == small);
	assert(actual.argmax_xor(query) == large);
	assert(actual.min_xor(query) == (small ^ query));
	assert(actual.max_xor(query) == (large ^ query));
}

template <int Bits>
void widths() {
	static_assert(std::is_same_v<typename XorSet<Bits>::value_type, std::uint64_t>);
	XorSet<Bits> actual;
	std::set<std::uint64_t> expected;
	std::array<std::uint64_t, 9> special{0, 1, 2, mask<Bits>(), mask<Bits>() - 1,
		UINT64_C(1) << (Bits - 1), (UINT64_C(1) << (Bits - 1)) - 1, UINT32_MAX, UINT64_C(1) << 31};
	for (auto value : special) {
		assert(actual.insert(value) == expected.insert(value).second);
		assert(actual.contains(value));
		assert(!actual.insert(value));
		for (auto query : {UINT64_C(0), UINT64_MAX, UINT64_C(1) << 63, value}) check(actual, expected, query);
	}
	std::mt19937_64 rng(827294 + Bits);
	for (int iteration = 0; iteration < 5000; ++iteration) {
		std::uint64_t value = rng() & mask<Bits>();
		if (iteration % 3 == 0) value = special[rng() % special.size()];
		if (iteration % 5 == 0 && !expected.empty()) value = *std::next(expected.begin(), rng() % expected.size());
		switch (rng() % 4) {
			case 0: case 1: assert(actual.insert(value) == expected.insert(value).second); break;
			case 2: assert(actual.erase(value) == (expected.erase(value) != 0)); break;
			default: assert(actual.contains(value) == expected.contains(value)); break;
		}
		check(actual, expected, rng());
	}
	auto copy = actual;
	XorSet<Bits> assigned;
	assigned = actual;
	for (auto query : {UINT64_C(0), UINT64_MAX, UINT64_C(1) << 63}) { check(copy, expected, query); check(assigned, expected, query); }
	copy.clear();
	assert(copy.empty() && actual.size() == std::int64_t(expected.size()));
	assert(copy.insert(0) && copy.contains(0));
	auto moved = std::move(assigned);
	assert(assigned.empty() && assigned.insert(1));
	check(moved, expected, rng());
	assigned = std::move(moved);
	assert(moved.empty() && moved.insert(2));
	check(assigned, expected, rng());
	auto *same = &assigned;
	assigned = *same;
	assigned = std::move(*same);
	check(assigned, expected, rng());
	while (!expected.empty()) {
		auto value = *expected.begin();
		assert(actual.erase(value));
		expected.erase(value);
		assert(!actual.erase(value));
		check(actual, expected, rng());
	}
	assert(actual.empty());
	assert(actual.insert(mask<Bits>()));
	assert(actual.argmin_xor(0) == mask<Bits>());
	assert(actual.erase(mask<Bits>()));
	XorSet<Bits> one;
	one.insert(1);
	actual.swap(one);
	assert(actual.contains(1) && one.empty());
}

int main() {
	static_assert(std::is_same_v<XorSet<30>::value_type, std::uint32_t>);
	static_assert(std::is_same_v<XorSet<31>::value_type, std::uint32_t>);
	widths<32>(); widths<33>(); widths<40>(); widths<63>(); widths<64>();
	XorSet<64> values;
	std::size_t capacity = 0;
	for (int round = 0; round < 1000; ++round) {
		for (std::uint64_t i = 0; i < 128; ++i) assert(values.insert(i * UINT64_C(0x123456789abcdef)));
		if (!round) capacity = values.memory_usage();
		assert(values.memory_usage() == capacity);
		for (std::uint64_t i = 0; i < 128; ++i) assert(values.erase(i * UINT64_C(0x123456789abcdef)));
		assert(values.empty());
	}
	assert(capacity < 65536);
	std::vector<XorSet<64>> stored(2);
	stored[0].insert(UINT64_MAX);
	assert(stored[0].argmax_xor(0) == UINT64_MAX && stored[1].empty());
}
