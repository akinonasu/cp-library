#include "string/longest_common_substring.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

int reference(std::string_view s, std::string_view t) {
	std::vector<int> dp(t.size() + 1);
	int best = 0;
	for (char c : s) {
		for (int j = int(t.size()); j > 0; --j) {
			dp[j] = c == t[j - 1] ? dp[j - 1] + 1 : 0;
			best = std::max(best, dp[j]);
		}
	}
	return best;
}

void check(std::string_view s, std::string_view t, int expected = -1) {
	std::string old_s(s), old_t(t);
	auto [a, b, length] = longest_common_substring(s, t);
	assert(a >= 0 && b >= 0 && length >= 0);
	assert(std::size_t(a) + length <= s.size() && std::size_t(b) + length <= t.size());
	assert(s.substr(a, length) == t.substr(b, length));
	assert(length == (expected < 0 ? reference(s, t) : expected));
	if (!length) assert(a == 0 && b == 0);
	assert(s == old_s && t == old_t);
}

int main() {
	LongestCommonSubstringResult empty;
	assert(empty.s_pos == 0 && empty.t_pos == 0 && empty.length == 0);
	check({}, {}, 0);
	check({}, "abc", 0);
	check("abc", {}, 0);
	check("ABABC", "BABCA", 4);
	check("abc", "ac", 1);
	check("abc", "XYZ", 0);
	check("ababa", "babab", 4);
	check("abcd", "xxabcdyy", 4);
	check("xxabcdyy", "abcd", 4);
	std::string subview = "__abcdefgh__";
	check(std::string_view(subview).substr(2, 5), std::string_view(subview).substr(4, 6), 3);
	std::vector<std::string> words;
	for (int n = 0; n <= 6; ++n) {
		for (int mask = 0; mask < (1 << n); ++mask) {
			std::string s(n, '\0');
			for (int i = 0; i < n; ++i) s[i] = (mask >> i) & 1 ? char(255) : '\0';
			words.push_back(s);
		}
	}
	for (const auto &s : words) for (const auto &t : words) check(s, t);
	std::mt19937 rng(9320841);
	for (int alphabet : {2, 3, 26, 256}) {
		for (int trial = 0; trial < 500; ++trial) {
			std::string s(rng() % 101, '\0'), t(rng() % 101, '\0');
			for (char &c : s) c = char(rng() % alphabet);
			for (char &c : t) c = char(rng() % alphabet);
			check(s, t);
			check(t, s);
		}
	}
	std::string bytes;
	for (int c = 0; c < 256; ++c) bytes.push_back(char(c));
	check(bytes, bytes.substr(120) + bytes.substr(0, 120), 136);
	check(bytes, std::string(bytes.rbegin(), bytes.rend()), 1);
	std::string binary("\0#\xff$\0\xff", 6);
	check(binary, std::string("!\xff$\0\xff?", 6), 4);
	for (int length : {7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 511, 512, 513}) {
		std::string block(length, '\xff');
		check("a" + block + "b", "c" + block + "d", length);
		check(block + "a", block + "b", length);
		check("a" + block, "b" + block, length);
	}
	std::string repeated(500000, 'a');
	check(repeated, repeated, 500000);
	check(repeated, std::string(400000, 'a'), 400000);
	check(repeated, std::string(500000, 'b'), 0);
	check(repeated + 'b', 'c' + repeated, 500000);
	std::string periodic;
	for (int i = 0; i < 100000; ++i) periodic += "abcde";
	check("x" + periodic + "y", "z" + periodic + "w", 500000);
	check(periodic, periodic.substr(1) + 'a', 499999);
}
