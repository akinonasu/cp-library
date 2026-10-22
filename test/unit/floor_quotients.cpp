#include "components/floor_quotients.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <utility>

int main() {
	for (std::uint64_t n = 0; n <= 5000; ++n) {
		FloorQuotients table(n);
		std::vector<std::uint64_t> expected;
		for (std::uint64_t i = 1; i <= n;) {
			const auto value = n / i;
			expected.push_back(value);
			i = n / value + 1;
		}
		assert(table.values() == expected);
		assert(table.size() == table.root() + table.large_size());
		assert(table.root() * table.root() <= n && (table.root() + 1) * (table.root() + 1) > n);
		assert(!table.contains(0) && !table.contains(n + 1));
		for (std::size_t i = 0; i < table.size(); ++i) {
			const auto x = table.values()[i];
			assert(table.contains(x) && table.index(x) == i);
			for (auto d : {std::uint64_t(1), std::min<std::uint64_t>(2, x), std::min<std::uint64_t>(3, x), x})
				assert(table.values()[table.divided_index(i, d)] == x / d);
		}
		if (n <= 100) for (std::uint64_t x = 1; x <= n; ++x)
			assert(table.contains(x) == std::binary_search(expected.begin(), expected.end(), x, std::greater<>()));
	}
	FloorQuotients large(1000000000000ULL);
	std::mt19937_64 random(20261022);
	for (int trial = 0; trial < 10000; ++trial) {
		const auto i = random() % large.size(), x = large.values()[i], d = 1 + random() % x;
		assert(large.values()[large.divided_index(i, d)] == x / d);
	}
	auto copy = large;
	auto moved = std::move(copy);
	assert(copy.max_n() == 0 && copy.root() == 0 && copy.size() == 0);
	assert(moved.values() == large.values());
	copy = std::move(moved);
	assert(moved.max_n() == 0 && moved.size() == 0 && copy.values() == large.values());
}
