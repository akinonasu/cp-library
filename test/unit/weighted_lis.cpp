#include "sequence/weighted_lis.hpp"
#include "sequence/lis.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory_resource>
#include <random>
#include <span>
#include <string>
#include <type_traits>

template <bool Strict, class Range, class Weights, class Compare>
void check(const Range &a, const Weights &weights, Compare compare) {
	using Weight = std::ranges::range_value_t<const Weights>;
	int n = int(a.size());
	assert(n < 24);
	std::vector<Weight> ending(n);
	for (int i = 0; i < n; ++i) ending[i] = weights[i];
	Weight expected{};
	for (unsigned mask = 1; mask < (1u << n); ++mask) {
		int previous = -1;
		Weight sum{};
		bool valid = true;
		for (int i = 0; i < n; ++i) if (mask >> i & 1) {
			if (previous != -1 && (Strict ? !compare(a[previous], a[i]) : compare(a[i], a[previous]))) valid = false;
			sum = sum + weights[i];
			previous = i;
		}
		if (valid) {
			if (expected < sum) expected = sum;
			if (ending[previous] < sum) ending[previous] = sum;
		}
	}
	assert(weighted_lis_value<Strict>(a, weights, compare) == expected);
	assert(weighted_lis_values<Strict>(a, weights, compare) == ending);
	auto indices = weighted_lis<Strict>(a, weights, compare);
	Weight sum{};
	int previous = -1;
	for (int i : indices) {
		assert(previous < i && 0 <= i && i < n);
		if (previous != -1) assert(Strict ? compare(a[previous], a[i]) : !compare(a[i], a[previous]));
		sum = sum + weights[i];
		previous = i;
	}
	assert(sum == expected);
}

template <class T>
void integers() {
	T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	std::vector<T> a{low, high, T(0), high, low, T(1)};
	std::vector<long long> weights{-8, 4, 9, -1, 3, 7};
	check<true>(a, weights, std::less<>{});
	check<false>(a, weights, std::less<>{});
	check<true>(a, weights, std::greater<>{});
	check<false>(a, weights, std::greater<>{});
}

int main() {
	static_assert(std::is_same_v<decltype(weighted_lis_value(std::vector<int>{}, std::vector<long long>{})), long long>);
	static_assert(std::is_same_v<decltype(weighted_lis_values(std::vector<int>{}, std::vector<long long>{})), std::vector<long long>>);
	std::mt19937_64 random(900365);
	for (int n = 0; n <= 13; ++n) for (int trial = 0; trial < 35; ++trial) {
		std::vector<int> a(n);
		std::vector<long long> weights(n);
		for (int i = 0; i < n; ++i) a[i] = int(random() % 7) - 3, weights[i] = static_cast<long long>(random() % 21) - 10;
		check<true>(a, weights, std::less<>{});
		check<false>(a, weights, std::less<>{});
		check<true>(a, weights, std::greater<>{});
		check<false>(a, weights, std::greater<>{});
		std::fill(weights.begin(), weights.end(), 1);
		assert(weighted_lis_value(a, weights) == lis_length(a));
		assert(weighted_lis_value<false>(a, weights) == lis_length<false>(a));
	}
	integers<signed char>(); integers<unsigned char>();
	integers<short>(); integers<unsigned short>();
	integers<int>(); integers<unsigned int>();
	integers<long>(); integers<unsigned long>();
	integers<long long>(); integers<unsigned long long>();
	std::array<std::string, 5> words{"b", "a", "c", "c", "d"};
	std::deque<long long> weights{4, 7, -2, 9, 3};
	check<true>(words, weights, std::less<>{});
	check<false>(words, weights, std::less<>{});
	std::pmr::vector<std::string> pooled(words.begin(), words.end());
	std::array<long long, 5> fixed{4, 7, -2, 9, 3};
	check<true>(pooled, std::span(fixed), std::less<>{});
	check<false>(std::span(words), std::span<const long long, 5>(fixed), std::less<>{});
	std::vector<int> a{2, 1, 3, 4};
	std::vector<long long> negative{-3, -1, -9, -2};
	assert(weighted_lis(a, negative).empty());
	assert(weighted_lis_value(a, negative) == 0);
	assert(weighted_lis_values(a, negative) == negative);
	std::vector<long long> zeros(4);
	assert(weighted_lis_value(a, zeros) == 0);
	check<true>(a, zeros, std::less<>{});
	std::vector<__int128_t> wide{__int128_t(INT64_MAX), __int128_t(INT64_MAX), 7, -9};
	check<false>(a, wide, std::less<>{});
	std::vector<double> halves{0.5, -2.0, 1.5, 3.0};
	check<true>(a, halves, std::less<>{});
	check<false>(a, halves, std::greater<>{});
	struct Item { int key; std::string text; };
	std::array<Item, 5> items{Item{3, "a"}, Item{1, "b"}, Item{3, "c"}, Item{4, "d"}, Item{2, "e"}};
	check<true>(items, fixed, [](const Item &x, const Item &y) { return x.key < y.key; });
	check<false>(items, fixed, [](const Item &x, const Item &y) { return x.key < y.key; });
	std::vector<int> large(100003);
	std::iota(large.begin(), large.end(), 0);
	std::vector<long long> ones(large.size(), 1);
	assert(weighted_lis_value(large, ones) == static_cast<long long>(large.size()));
	auto indices = weighted_lis(large, ones);
	assert(indices == large);
	std::fill(large.begin(), large.end(), 42);
	assert(weighted_lis_value(large, ones) == 1);
	assert(weighted_lis_value<false>(large, ones) == static_cast<long long>(large.size()));
}
