#include "math/fixed_size_subset_sum.hpp"
#include <cassert>
#include <random>

std::vector<long long> naive(std::vector<int> values, int k) {
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	if (k > int(values.size())) return {};
	int high = std::accumulate(values.begin(), values.end(), 0);
	std::vector<std::vector<unsigned char>> dp(k + 1, std::vector<unsigned char>(high + 1));
	dp[0][0] = 1;
	for (int x : values)
		for (int j = k; j; --j)
			for (int s = high; s >= x; --s) dp[j][s] |= dp[j - 1][s - x];
	std::vector<long long> result;
	for (int s = 0; s <= high; ++s) if (dp[k][s]) result.push_back(s);
	return result;
}

template<std::uint32_t Mod, std::uint32_t Root>
void check_modular(const std::vector<int>& values, int k) {
	int high = 0;
	for (int i = 0; i < k; ++i) high += values[values.size() - 1 - i];
	std::vector<unsigned char> present(high + 1);
	fixed_size_subset_sum_detail::include_modular<Mod, Root>(values, k, 0, present);
	auto expected = naive(values, k);
	for (long long s : expected) assert(present[s]);
	assert(std::count(present.begin(), present.end(), 1) == int(expected.size()));
}

int main() {
	assert(fixed_size_subset_sums({}, 0) == std::vector<long long>{0});
	assert(fixed_size_subset_sums({}, 1).empty());
	assert(fixed_size_subset_sums({0, 0, 0}, 2).empty());
	assert(fixed_size_subset_sums({0, 2, 5, 8}, 2) == (std::vector<long long>{2, 5, 7, 8, 10, 13}));
	assert(fixed_size_subset_sums({2000000000, 2000000001, 2000000002}, 2)
		== (std::vector<long long>{4000000001LL, 4000000002LL, 4000000003LL}));
	for (int mask = 0; mask < (1 << 9); ++mask) {
		std::vector<int> values;
		for (int i = 0; i < 9; ++i) if (mask >> i & 1) values.push_back(i);
		for (int k = 0; k <= int(values.size()) + 1; ++k) assert(fixed_size_subset_sums(values, k) == naive(values, k));
	}
	std::mt19937 random(812965);
	for (int trial = 0; trial < 120; ++trial) {
		std::vector<int> values;
		int count = 20 + random() % 50, k = 2 + random() % 9;
		for (int i = 0; i < count; ++i) values.push_back(random() % 180);
		auto expected = naive(values, k);
		assert(fixed_size_subset_sums(values, k) == expected);
		std::sort(values.begin(), values.end());
		values.erase(std::unique(values.begin(), values.end()), values.end());
		long long total = std::accumulate(values.begin(), values.end(), 0LL);
		for (auto& x : expected) x = total - x;
		std::reverse(expected.begin(), expected.end());
		assert(fixed_size_subset_sums(values, int(values.size()) - k) == expected);
	}
	std::vector<int> values{1};
	for (int i = 0; i < 80; ++i) values.push_back(2 * i);
	for (int k : {2, 5, 10}) assert(fixed_size_subset_sums(values, k) == naive(values, k));
	for (int& x : values) x = 1000000000 + 1000 * x;
	auto shifted = fixed_size_subset_sums(values, 10);
	for (auto& x : shifted) x = (x - 10000000000LL) / 1000;
	values = {1};
	for (int i = 0; i < 80; ++i) values.push_back(2 * i);
	assert(shifted == naive(values, 10));
	values = {0, 1, 2, 4, 7, 9, 10, 12};
	check_modular<998244353, 3>(values, 3);
	check_modular<2130706433, 3>(values, 3);
	check_modular<2113929217, 5>(values, 3);
	check_modular<2088763393, 5>(values, 3);
	check_modular<2013265921, 31>(values, 3);
	check_modular<1811939329, 13>(values, 3);
	check_modular<1711276033, 29>(values, 3);
	check_modular<1484783617, 5>(values, 3);
	check_modular<1300234241, 3>(values, 3);
	check_modular<1224736769, 3>(values, 3);
	values = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 15, 16, 18, 19, 20, 21, 22, 24, 26, 32, 36, 37, 38};
	std::vector<unsigned char> present(194);
	auto expected = naive(values, 6);
	auto first = fixed_size_subset_sum_detail::include_modular<257, 3>(values, 6, 0, present);
	assert(first < expected.size() && !present[113]);
	auto second = fixed_size_subset_sum_detail::include_modular<65537, 3>(values, 6, 0, present);
	assert(first + second == expected.size());
	for (auto s : expected) assert(present[s]);
	using namespace fixed_size_subset_sum_detail;
	assert(coefficient_bound(10, 2) == Digits{5});
	assert(coefficient_bound(10, 3) == Digits{15});
	assert(coefficient_bound(2, 3) == Digits{0});
	bool rejected = false;
	try { (void)coefficient_bound(200001, 100); }
	catch (const std::length_error&) { rejected = true; }
	assert(rejected);
	values.resize(24);
	std::iota(values.begin(), values.end(), 0);
	values.back() = 1000000000;
	rejected = false;
	try { (void)fixed_size_subset_sums(values, 6); }
	catch (const std::length_error&) { rejected = true; }
	assert(rejected);
	values.resize(200001);
	std::iota(values.begin(), values.end(), 0);
	auto dense = fixed_size_subset_sums(values, 10);
	assert(dense.size() == 1999911 && dense.front() == 45 && dense.back() == 1999955);
	for (std::size_t i = 0; i < dense.size(); ++i) assert(dense[i] == 45 + static_cast<long long>(i));
}
