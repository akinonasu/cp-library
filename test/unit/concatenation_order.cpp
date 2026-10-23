#include "string/concatenation_order.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <list>
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

int main() {
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
