#include "components/predecessor_set.hpp"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <set>

void require(bool ok) { if (!ok) std::abort(); }

void check(const PredecessorSet &set, const std::set<int> &values, std::uint32_t x) {
	int lo = -1, hi = -1;
	for (int value : values) {
		if (lo == -1 || (std::uint32_t(value) ^ x) < (std::uint32_t(lo) ^ x)) lo = value;
		if (hi == -1 || (std::uint32_t(value) ^ x) > (std::uint32_t(hi) ^ x)) hi = value;
	}
	require(set.argmin_xor(x) == lo && set.argmax_xor(x) == hi);
}

int main() {
	for (int n = 0; n <= 8; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		PredecessorSet set(n);
		std::set<int> values;
		for (int i = 0; i < n; ++i) if (mask >> i & 1) set.insert(i), values.insert(i);
		for (unsigned x = 0; x < 128; ++x) check(set, values, x);
		check(set, values, UINT32_MAX);
	}
	std::mt19937 rng(8973241);
	for (int n : {1, 63, 64, 65, 4095, 4096, 4097, 262143, 262144, 262145,
		16777215, 16777216, 16777217, 1073741825}) {
		PredecessorSet set(n);
		std::set<int> values;
		for (int i = 0; i < 1000; ++i) {
			int p = i % 4 ? int(rng() % n) : (i & 4 ? 0 : n - 1);
			if (i % 5) require(set.insert(p) == values.insert(p).second);
			else require(set.erase(p) == bool(values.erase(p)));
			check(set, values, rng());
			check(set, values, p);
		}
		for (unsigned x : {0U, 63U, 64U, unsigned(n - 1), unsigned(n), unsigned(INT_MAX), UINT32_MAX}) check(set, values, x);
		set.clear(); values.clear(); check(set, values, rng());
		set.insert(n - 1); values.insert(n - 1);
		check(set, values, 0); check(set, values, UINT32_MAX);
	}
	for (int n : {1, 63, 64, 65, 4095, 4096, 4097}) {
		PredecessorSet set(n, true);
		std::set<int> values;
		for (int i = 0; i < n; ++i) values.insert(i);
		for (int i = 0; i < 200; ++i) check(set, values, rng());
		auto copy = set;
		PredecessorSet moved(std::move(copy));
		check(copy, {}, UINT32_MAX); check(moved, values, UINT32_MAX);
	}
}
