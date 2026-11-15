#include "string/concatenation_order.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <list>
#include <limits>
#include <memory_resource>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

std::string join(const std::vector<std::string> &s, const std::vector<int> &order) {
	std::string result;
	for (int i : order) result += s[i];
	return result;
}

void check(const std::vector<std::string> &s, bool brute = false) {
	auto original = s;
	std::vector<int> reference(s.size());
	std::iota(reference.begin(), reference.end(), 0);
	for (bool descending : {false, true}) {
		std::stable_sort(reference.begin(), reference.end(), [&](int i, int j) {
			if (s[i].empty() || s[j].empty()) return s[i].empty() && !s[j].empty();
			return descending ? s[i] + s[j] > s[j] + s[i] : s[i] + s[j] < s[j] + s[i];
		});
		auto actual = concatenation_order(s, descending);
		assert(actual == reference);
		auto value = descending ? max_concatenation(s) : min_concatenation(s);
		assert(value == join(s, reference));
		if (brute) {
			std::vector<int> permutation(s.size());
			std::iota(permutation.begin(), permutation.end(), 0);
			do {
				auto other = join(s, permutation);
				assert(descending ? value >= other : value <= other);
			} while (std::next_permutation(permutation.begin(), permutation.end()));
		}
	}
	assert(s == original);
}

template <class T>
std::vector<T> join_sequences(const std::vector<std::vector<T>> &sequences, const std::vector<int> &order) {
	std::vector<T> result;
	for (int i : order) result.insert(result.end(), sequences[i].begin(), sequences[i].end());
	return result;
}

template <class T>
void check_sequences(const std::vector<std::vector<T>> &sequences, bool brute = false) {
	auto original = sequences;
	for (bool descending : {false, true}) {
		std::vector<int> reference(sequences.size());
		std::iota(reference.begin(), reference.end(), 0);
		std::stable_sort(reference.begin(), reference.end(), [&](int i, int j) {
			if (sequences[i].empty() || sequences[j].empty()) return sequences[i].empty() && !sequences[j].empty();
			auto a = join_sequences(sequences, {i, j}), b = join_sequences(sequences, {j, i});
			return descending ? a > b : a < b;
		});
		auto actual = concatenation_order(sequences, descending);
		assert(actual == reference);
		auto result = descending ? max_concatenation(sequences) : min_concatenation(sequences);
		static_assert(std::is_same_v<decltype(result), std::vector<T>>);
		assert(result == join_sequences(sequences, reference));
		std::vector<std::span<const T>> views;
		for (const auto &s : sequences) views.emplace_back(s);
		assert(concatenation_order(std::span(views), descending) == reference);
		assert((descending ? max_concatenation(views) : min_concatenation(views)) == result);
		if (brute) {
			std::vector<int> permutation(sequences.size());
			std::iota(permutation.begin(), permutation.end(), 0);
			do {
				auto other = join_sequences(sequences, permutation);
				assert(descending ? result >= other : result <= other);
			} while (std::next_permutation(permutation.begin(), permutation.end()));
		}
	}
	for (const auto &a : sequences) for (const auto &b : sequences) {
		auto ab = a, ba = b;
		ab.insert(ab.end(), b.begin(), b.end());
		ba.insert(ba.end(), a.begin(), a.end());
		int expected = (ab > ba) - (ab < ba);
		assert(concatenation_compare(a, b) == expected);
		assert(concatenation_compare(std::span(a), std::span(b)) == expected);
	}
	assert(sequences == original);
}

template <class T>
void check_integer_type() {
	T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	check_sequences<T>({}, true);
	check_sequences<T>({{}, {}, {}}, true);
	check_sequences<T>({{low, high}, {high}, {low}, {}, {low, high, low, high}, {T(0)}}, true);
	check_sequences<T>({{low, high}, {low, high, low, high}, {}, {low, high}, {}}, true);
	std::mt19937_64 rng(18732 + sizeof(T));
	for (int repeat = 0; repeat < 60; ++repeat) {
		std::vector<std::vector<T>> sequences(rng() % 12);
		for (auto &s : sequences) {
			s.resize(rng() % 25);
			for (T &x : s) x = T(rng());
		}
		check_sequences(sequences, sequences.size() <= 5);
	}
	std::pmr::vector<T> pooled{low, high, low};
	std::vector<T> ordinary{high, low};
	assert(concatenation_compare(pooled, ordinary) == (low < high ? -1 : 0));
	std::array<std::pmr::vector<T>, 3> allocated{pooled, {}, pooled};
	assert((concatenation_order(allocated) == std::vector<int>{1, 0, 2}));
	assert((min_concatenation(allocated) == std::vector<T>{low, high, low, low, high, low}));
}

template <class T>
concept IntegerInput = requires(const std::vector<std::vector<T>> &sequences, const std::vector<T> &s) {
	concatenation_compare(s, s);
	concatenation_order(sequences);
	min_concatenation(sequences);
	max_concatenation(sequences);
};

void check_integers() {
	static_assert(IntegerInput<int> && IntegerInput<std::uint64_t> && !IntegerInput<bool> && !IntegerInput<double>);
	static_assert(std::is_same_v<decltype(min_concatenation(std::vector<std::string>{})), std::string>);
	check_integer_type<char>();
	check_integer_type<signed char>();
	check_integer_type<unsigned char>();
	check_integer_type<short>();
	check_integer_type<unsigned short>();
	check_integer_type<int>();
	check_integer_type<unsigned int>();
	check_integer_type<long>();
	check_integer_type<unsigned long>();
	check_integer_type<long long>();
	check_integer_type<unsigned long long>();
	check_integer_type<char8_t>();
	check_integer_type<char16_t>();
	check_integer_type<char32_t>();
	check_integer_type<wchar_t>();
	check_sequences<int>({{257, -255}, {1, 65537}, {}, {257, -255, 257, -255}, {-255}}, true);
	check_sequences<std::int64_t>({{INT64_MIN, INT64_MAX}, {INT64_MAX, INT64_MIN}, {-1}, {}, {0}, {INT64_MIN}}, true);
	check_sequences<std::uint64_t>({{UINT64_MAX, 0}, {UINT64_C(1) << 63}, {}, {0}, {UINT64_MAX - 256}}, true);
	std::array<int, 3> mutable_array = {257, -9, 1};
	const std::array<int, 2> const_array = {257, -9};
	assert(concatenation_compare(std::span(mutable_array), std::span(const_array)) == -1);
	std::array<int, 3> another = {257, -9, 257};
	std::array<std::span<int, 3>, 2> fixed{std::span(mutable_array), std::span(another)};
	assert((concatenation_order(fixed) == std::vector<int>{0, 1}));
	assert((min_concatenation(fixed) == std::vector<int>{257, -9, 1, 257, -9, 257}));
	std::list<std::vector<int>> list{{3}, {}, {3, 1}, {3, 1, 1}};
	assert((concatenation_order(list) == std::vector<int>{1, 3, 2, 0}));
	std::array<std::array<int, 0>, 3> empty;
	assert((concatenation_order(empty) == std::vector<int>{0, 1, 2}));
	assert(min_concatenation(empty).empty());
	for (bool descending : {false, true}) {
		std::vector<std::int64_t> long_word(20000, INT64_MIN);
		long_word.back() = INT64_MAX;
		std::array<std::int64_t, 1> one{INT64_MIN};
		std::vector<std::span<const std::int64_t>> sequences(4000, one);
		for (std::size_t position : {std::size_t(0), sequences.size() / 2, sequences.size() - 1}) {
			sequences[position] = long_word;
			auto actual = concatenation_order(sequences, descending);
			assert(actual[descending ? 0 : actual.size() - 1] == int(position));
			int previous = -1;
			for (int i : actual) if (i != int(position)) { assert(previous < i); previous = i; }
			sequences[position] = one;
		}
	}
	std::array<std::int64_t, 3> period{INT64_MIN, 257, INT64_MAX};
	std::vector<std::int64_t> repeated;
	for (int i = 0; i < 2000; ++i) repeated.insert(repeated.end(), period.begin(), period.end());
	std::vector<std::span<const std::int64_t>> commuting(4000, period);
	commuting[0] = repeated;
	for (bool descending : {false, true}) {
		auto actual = concatenation_order(commuting, descending);
		for (int i = 0; i < int(actual.size()); ++i) assert(actual[i] == i);
	}
}

int main() {
	check_integers();
	std::vector<std::string> words;
	for (int n = 0; n <= 6; ++n)
		for (int mask = 0; mask < (1 << n); ++mask) {
			std::string s(n, '\0');
			for (int i = 0; i < n; ++i) s[i] = (mask >> i) & 1 ? char(255) : char(0);
			words.push_back(s);
		}
	for (const auto &a : words) for (const auto &b : words) {
		auto left = a + b, right = b + a;
		int expected = (left > right) - (left < right);
		assert(concatenation_compare(a, b) == expected);
		assert(concatenation_compare(b, a) == -expected);
	}
	for (const auto &s : std::vector<std::vector<std::string>>{
		{}, {""}, {"", "", ""}, {"a", "b", "c"}, {"abc", "bc", "c"}, {"b", "ba"},
		{"b", "ba", "baa"}, {"ab", "aba", "abab"}, {"a", "b", "aa", "ab", "ba", "bb"},
		{"b", "", "a", "", "aa", "a"}, {"ab", "abab", "ab", "ababab"}}) check(s, true);
	assert(min_concatenation(std::vector<std::string>{"b", "ba", "baa"}) == "baabab");
	assert(max_concatenation(std::vector<std::string>{"b", "ba", "baa"}) == "bbabaa");
	std::array<std::string_view, 4> views{"b", "", "ba", "baa"};
	assert(concatenation_order(views) == std::vector<int>({1, 3, 2, 0}));
	assert(min_concatenation(std::span<const std::string_view>(views)) == "baabab");
	std::list<std::string> list{"b", "ba", "baa"};
	assert(concatenation_order(list) == std::vector<int>({2, 1, 0}));
	const char *pointers[]{"b", "ba", "baa"};
	assert(max_concatenation(pointers) == "bbabaa");
	std::string storage = "__baab__";
	std::array<std::string_view, 3> slices{
		std::string_view(storage).substr(2, 1), std::string_view(storage).substr(2, 2), std::string_view(storage).substr(2, 3)};
	assert(min_concatenation(slices) == "baabab");
	std::mt19937_64 random(741805);
	for (int alphabet : {1, 2, 3, 26, 256}) {
		for (int t = 0; t < 300; ++t) {
			std::vector<std::string> s(random() % 25);
			for (auto &text : s) {
				text.resize(random() % 65);
				for (char &c : text) c = char(random() % alphabet);
			}
			check(s, s.size() <= 7);
		}
	}
	for (int length : {1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129}) {
		std::string period(length, 'a');
		for (char &c : period) c = char(random() % 256);
		std::vector<std::string> s{"", period, period + period, period + period + period};
		for (int t = 0; t < 20; ++t) {
			std::string text;
			for (int i = 0, repeat = 3 + random() % 7; i < repeat; ++i) text += period;
			text[random() % text.size()] = char(random() % 256);
			s.push_back(text);
		}
		check(s);
	}
	for (bool descending : {false, true}) {
		std::string long_word(200000, 'a');
		long_word.back() = 'b';
		std::vector<std::string_view> s(20000, "a");
		for (std::size_t position : {std::size_t(0), s.size() / 2, s.size() - 1}) {
			s[position] = long_word;
			auto actual = concatenation_order(s, descending);
			assert(actual[descending ? 0 : actual.size() - 1] == int(position));
			int previous = -1;
			for (int i : actual) if (i != int(position)) { assert(previous < i); previous = i; }
			s[position] = "a";
		}
	}
	std::vector<std::string_view> empty(100000);
	auto order = concatenation_order(empty);
	for (int i = 0; i < int(order.size()); ++i) assert(order[i] == i);
	assert(min_concatenation(empty).empty());
	std::string repeated;
	for (int i = 0; i < 10000; ++i) repeated += "ab";
	std::vector<std::string_view> commuting(10000, "ab");
	commuting[0] = repeated;
	for (bool descending : {false, true}) {
		auto actual = concatenation_order(commuting, descending);
		for (int i = 0; i < int(actual.size()); ++i) assert(actual[i] == i);
	}
}
