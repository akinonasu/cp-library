#include "math/fixed_size_subset_sum.hpp"
#include "convolution/max_min_convolution.hpp"
#include "convolution/max_min_convolution_light.hpp"
#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <limits>
#include <memory>
#include <memory_resource>
#include <random>
#include <set>
#include <span>
#include <string>
#include <tuple>

template <class T>
std::set<__int128_t> subset_oracle(std::vector<T> values, int k) {
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	std::vector<std::set<__int128_t>> sums(k + 1);
	sums[0].insert(0);
	for (T x : values)
		for (int j = k; j; --j)
			for (__int128_t value : sums[j - 1]) sums[j].insert(value + __int128_t(x));
	return sums[k];
}

template <class T>
void check_subset(const std::vector<T> &values, int k) {
	auto expected = subset_oracle(values, k);
	bool overflow = !expected.empty() && (*expected.begin() < LLONG_MIN || *expected.rbegin() > LLONG_MAX);
	try {
		auto actual = fixed_size_subset_sums(values, k);
		assert(!overflow);
		assert(actual.size() == expected.size());
		assert(std::equal(actual.begin(), actual.end(), expected.begin()));
		assert(fixed_size_subset_sums(std::span<const T>(values), k) == actual);
	} catch (const std::overflow_error &) { assert(overflow); }
}

template <class T>
void integer_widths() {
	T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	std::vector<T> values{low, high, T(0), T(1), T(-1), T(2), T(2)};
	for (int k = 0; k <= 8; ++k) check_subset(values, k);
}

void subsets() {
	integer_widths<char>(); integer_widths<signed char>(); integer_widths<unsigned char>();
	integer_widths<short>(); integer_widths<unsigned short>();
	integer_widths<int>(); integer_widths<unsigned>();
	integer_widths<long>(); integer_widths<unsigned long>();
	integer_widths<long long>(); integer_widths<unsigned long long>();
	integer_widths<char8_t>(); integer_widths<char16_t>(); integer_widths<char32_t>(); integer_widths<wchar_t>();
	std::mt19937 random(315815);
	for (int trial = 0; trial < 60; ++trial) {
		std::vector<long long> values(5 + random() % 12);
		for (auto &x : values) x = int(random() % 31) - 15;
		for (int k : {0, 1, 2, 4, int(values.size()) - 1, int(values.size()), int(values.size()) + 1}) check_subset(values, k);
	}
	std::vector<int> integers{INT_MIN, INT_MAX, 0, -1};
	for (int k = 0; k <= 5; ++k) check_subset(integers, k);
	std::vector<long long> large{LLONG_MAX / 3 - 6, LLONG_MAX / 3 - 5, LLONG_MAX / 3 - 4, LLONG_MAX / 3 - 3, LLONG_MAX / 3 - 2};
	check_subset(large, 3);
	for (auto &x : large) x = -x;
	check_subset(large, 3);
	check_subset<long long>({LLONG_MIN, LLONG_MAX}, 2);
	check_subset<long long>({-LLONG_MAX, -LLONG_MAX + 1, LLONG_MAX - 1, LLONG_MAX}, 3);
	std::vector<long long> normalized{0};
	for (int i = 0; i < 55; ++i) normalized.push_back(2 * i + (i % 7 == 0));
	for (auto &x : normalized) x = -100000000000000000LL + 1000000000000LL * x;
	for (int k : {6, int(normalized.size()) - 6}) check_subset(normalized, k);
	std::vector<long long> progression(60);
	for (int i = 0; i < 60; ++i) progression[i] = -10000000000000000LL + 1234567890123LL * i;
	check_subset(progression, 6);
	check_subset(progression, 54);
	std::array<long long, 4> array{-5, 2, 8, 8};
	auto copy = array;
	assert((fixed_size_subset_sums(std::span(array), 2) == std::vector<long long>{-3, 3, 10}));
	const auto &const_array = array;
	assert((fixed_size_subset_sums(std::span(const_array), 2) == std::vector<long long>{-3, 3, 10}));
	std::pmr::vector<long long> pooled(array.begin(), array.end());
	assert((fixed_size_subset_sums(pooled, 2) == std::vector<long long>{-3, 3, 10}));
	assert(array == copy);
	std::vector<long long> out_of_range(25);
	std::iota(out_of_range.begin(), out_of_range.end(), 0LL);
	out_of_range.back() = LLONG_MAX;
	bool rejected = false;
	try { fixed_size_subset_sums(out_of_range, 6); } catch (const std::overflow_error &) { rejected = true; }
	assert(rejected);
	out_of_range.back() = 1000000000000LL;
	rejected = false;
	try { fixed_size_subset_sums(out_of_range, 6); } catch (const std::length_error &) { rejected = true; }
	assert(rejected);
}

template <bool Maximum, class T, class Compare>
std::vector<T> convolution_oracle(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> result(a.size() + b.size() - 1, a[0]);
	std::vector<bool> used(result.size());
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) {
			T value = Maximum ? (compare(a[i], b[j]) ? a[i] : b[j]) : (compare(a[i], b[j]) ? b[j] : a[i]);
			if (!used[i + j] || (Maximum ? compare(result[i + j], value) : compare(value, result[i + j]))) result[i + j] = value;
			used[i + j] = true;
		}
	return result;
}

template <class T, class Compare>
void check_convolution(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	auto maximum = convolution_oracle<true>(a, b, compare), minimum = convolution_oracle<false>(a, b, compare);
	auto equivalent = [&](const auto &actual, const auto &expected) {
		assert(actual.size() == expected.size());
		for (std::size_t i = 0; i < actual.size(); ++i) {
			assert(!compare(actual[i], expected[i]) && !compare(expected[i], actual[i]));
			assert(std::find(a.begin(), a.end(), actual[i]) != a.end() || std::find(b.begin(), b.end(), actual[i]) != b.end());
		}
	};
	equivalent(max_min_convolution(a, b, compare), maximum);
	equivalent(min_max_convolution(a, b, compare), minimum);
	equivalent(light_convolution::max_min_convolution(a, b, compare), maximum);
	equivalent(light_convolution::min_max_convolution(a, b, compare), minimum);
}

struct Record {
	std::string key;
	int id;
	bool operator==(const Record &) const = default;
};

struct MoveOnlyCompare {
	std::unique_ptr<int> state = std::make_unique<int>(0);
	bool operator()(long long a, long long b) const { return a < b; }
};

void convolutions() {
	std::mt19937 random(984196);
	auto compare = [](const Record &a, const Record &b) { return std::tie(a.key, a.id) < std::tie(b.key, b.id); };
	for (int trial = 0; trial < 18; ++trial) {
		std::vector<Record> a(trial ? 30 + random() % 70 : 0), b(20 + random() % 80);
		for (auto &x : a) x = {std::to_string(random() % 11), int(random() % 15)};
		for (auto &x : b) x = {std::to_string(random() % 11), int(random() % 15)};
		check_convolution(a, b, compare);
		check_convolution(a, b, [](const Record &x, const Record &y) { return x.key.size() < y.key.size(); });
	}
	std::vector<std::string> a(65), b(71);
	for (std::size_t i = 0; i < a.size(); ++i) a[i] = std::string(i % 5, char('a' + i % 3));
	for (std::size_t i = 0; i < b.size(); ++i) b[i] = std::string(i % 7, char('a' + i % 2));
	check_convolution(a, b, [](const auto &x, const auto &y) { return x.size() < y.size(); });
	std::vector<long long> x{LLONG_MIN, 5, LLONG_MAX}, y{LLONG_MAX, 0, LLONG_MIN};
	check_convolution(x, y, std::less<long long>{});
	check_convolution(x, y, std::greater<long long>{});
	assert(max_min_convolution(x, y) == min_max_convolution(x, y, std::greater<long long>{}));
	assert(min_max_convolution(x, y) == max_min_convolution(x, y, std::greater<long long>{}));
	assert(max_min_convolution(x, y, MoveOnlyCompare{}) == max_min_convolution(x, y));
	assert(min_max_convolution(x, y, MoveOnlyCompare{}) == min_max_convolution(x, y));
	assert(light_convolution::max_min_convolution(x, y, MoveOnlyCompare{}) == max_min_convolution(x, y));
	assert(light_convolution::min_max_convolution(x, y, MoveOnlyCompare{}) == min_max_convolution(x, y));
}

int main() { subsets(); convolutions(); }
