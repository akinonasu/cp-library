#include "components/floor_quotients.hpp"
#include <algorithm>
#include <cassert>
#include <cfenv>
#include <random>
#include <ranges>
#include <limits>
#include <utility>

static_assert(std::ranges::forward_range<FloorQuotientRange>);
static_assert(std::ranges::sized_range<FloorQuotientRange>);
static_assert(std::same_as<std::ranges::range_value_t<FloorQuotientRange>, std::uint64_t>);

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
		FloorQuotientRange range(n);
		assert(range.max_n() == n && range.empty() == (n == 0));
		assert(range.size() == table.size() && range.root() == table.root() && range.large_size() == table.large_size());
		std::vector<std::uint64_t> ascending(range.begin(), range.end());
		assert(ascending.size() == expected.size() && std::equal(ascending.begin(), ascending.end(), expected.rbegin()));
		for (std::uint64_t i = 0; i < range.size(); ++i) assert(range[i] == ascending[i]);
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
	for (std::uint64_t r : {UINT64_C(1), UINT64_C(31), UINT64_C(65535), UINT64_C(1) << 26, UINT64_C(4294967295)}) {
		for (std::uint64_t n : {r * r, r * (r + 1) - 1, r * (r + 1)}) {
			FloorQuotientRange range(n);
			assert(range.root() == r && range.large_size() == n / (r + 1));
		}
	}
	for (auto n : {UINT64_MAX, UINT64_MAX - 1, UINT64_C(1) << 63, UINT64_C(1) << 53}) {
		FloorQuotientRange range(n);
		assert(__uint128_t(range.root()) * range.root() <= n && __uint128_t(range.root() + 1) * (range.root() + 1) > n);
		assert(range.size() == range.root() + range.large_size());
		for (std::uint64_t i = 0; i < 32; ++i) {
			assert(range[i] == i + 1);
			assert(range[range.size() - 1 - i] == n / (i + 1));
		}
		assert(range[range.root()] == n / range.large_size());
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
	for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
		if (std::fesetround(mode)) continue;
		for (int trial = 0; trial < 100000; ++trial) {
			const auto n = trial % 3 == 0 ? (std::uint64_t(1) << 53) - random() % 1000 : random();
			const auto d = trial % 3 == 1 ? 1 + random() % 100000 : random() | 1;
			assert(floor_quotient_detail::divide(n, d) == n / d);
		}
		for (auto n : {std::uint64_t(1) << 53, (std::uint64_t(1) << 53) - 1, (std::uint64_t(1) << 53) + 1}) {
			for (std::uint64_t d = 1; d <= 10000; ++d) {
				const auto value = n / d * d;
				for (auto x : {value - 1, value, value + 1}) assert(floor_quotient_detail::divide(x, d) == x / d);
			}
		}
	}
	std::fesetround(FE_TONEAREST);
}
