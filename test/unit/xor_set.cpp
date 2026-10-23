#include "data_structure/xor_set.hpp"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <set>
#include <utility>

void require(bool ok) { if (!ok) std::abort(); }

template <int Bits>
void check(const XorSet<Bits> &set, const std::set<std::uint32_t> &values, std::uint32_t x) {
	require(set.size() == std::int64_t(values.size()) && set.empty() == values.empty());
	for (auto value : values) require(set.contains(value));
	if (values.empty()) return;
	auto lo = *values.begin(), hi = lo;
	for (auto value : values) {
		if ((value ^ x) < (lo ^ x)) lo = value;
		if ((value ^ x) > (hi ^ x)) hi = value;
	}
	require(set.argmin_xor(x) == lo && set.argmax_xor(x) == hi);
	require(set.min_xor(x) == (lo ^ x) && set.max_xor(x) == (hi ^ x));
}

template <int Bits>
void random_test() {
	constexpr auto mask = (std::uint32_t(1) << Bits) - 1;
	std::mt19937 rng(72491 + Bits);
	XorSet<Bits> set;
	std::set<std::uint32_t> values;
	require(set.empty() && set.size() == 0 && set.memory_usage() == 0);
	for (int i = 0; i < 3000; ++i) {
		auto value = (i % 3 == 0 ? rng() : rng() % 2048) & mask;
		if (i & 1) require(set.insert(value) == values.insert(value).second);
		else require(set.erase(value) == bool(values.erase(value)));
		require(set.contains(value) == values.contains(value));
		check(set, values, rng());
		if (i % 100 == 0) check(set, values, UINT32_MAX);
	}
	auto copy = set;
	check(copy, values, rng());
	copy.insert(mask); require(set.contains(mask) == values.contains(mask));
	copy = set;
	auto moved = std::move(copy);
	require(copy.empty() && !copy.contains(0));
	check(moved, values, rng());
	copy.insert(mask); require(copy.contains(mask) && copy.size() == 1);
	copy = std::move(moved);
	require(moved.empty()); check(copy, values, rng());
	moved.insert(0); require(moved.contains(0));
	copy.clear(); require(copy.empty());
	copy.insert(0); copy.insert(mask); check(copy, std::set<std::uint32_t>{0, mask}, rng());
	set.swap(copy); check(copy, values, rng());
	for (auto value : values) require(copy.erase(value));
	require(copy.empty());
	for (int cycle = 0; cycle < 20; ++cycle) {
		for (std::uint32_t value = 0; value <= std::min(mask, 2047U); ++value) require(copy.insert(value));
		for (std::uint32_t value = 0; value <= std::min(mask, 2047U); ++value) require(copy.erase(value));
		require(copy.empty());
	}
}

int main() {
	for (unsigned mask = 0; mask < 256; ++mask) {
		XorSet<3> set;
		std::set<std::uint32_t> values;
		for (unsigned i = 0; i < 8; ++i) if (mask >> i & 1) set.insert(i), values.insert(i);
		for (unsigned x = 0; x < 256; ++x) check(set, values, x);
		check(set, values, UINT32_MAX);
	}
	std::mt19937_64 rng(297671);
	for (int i = 0; i < 2000; ++i) {
		auto word = rng();
		if (!word) continue;
		for (unsigned x = 0; x < 64; ++x) {
			unsigned expected = 0;
			while (!(word >> (expected ^ x) & 1)) ++expected;
			require(xor_select_detail::argmin(word, x) == (expected ^ x));
		}
	}
	random_test<1>(); random_test<2>(); random_test<6>(); random_test<7>();
	random_test<9>(); random_test<10>(); random_test<15>(); random_test<22>();
	random_test<30>(); random_test<31>();
}
