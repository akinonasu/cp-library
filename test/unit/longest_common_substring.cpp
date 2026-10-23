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

void seed_matches() {
	std::mt19937 rng(20261023);
	int completed = 0;
	for (int alphabet : {2, 3, 26, 256}) {
		for (int trial = 0; trial < 12; ++trial) {
			std::string s(256 + trial * 11, 0), t(257 + trial * 13, 0);
			for (char &c : s) c = char(rng() % alphabet);
			for (char &c : t) c = char(rng() % alphabet);
			if (trial % 3 == 0) t.replace(70, 50, s.substr(30, 50));
			int length = reference(s, t);
			if (auto match = substring_seed_detail::search(s, t, {0, 0, 0})) {
				++completed;
				assert(match->length == length);
				assert(s.substr(match->first, length) == t.substr(match->second, length));
			}
			check(s, t, length);
			check(t, s, length);
		}
	}
	assert(completed >= 24);
	std::string first(512, 0), second(513, char(255));
	for (char &c : first) c = char(rng() % 26);
	for (std::size_t i = 1; i < second.size(); i += 2) second[i] = 0;
	assert(!substring_seed_detail::search(first, second, {0, 0, 0}));
	check(first, second, 1);
	first.resize(1024), second.resize(1025);
	for (char &c : first) c = char(rng() % 26);
	for (char &c : second) c = char(rng() % 26);
	std::string block;
	for (int i = 0; i < 40; ++i) block += char('A' + i);
	first.replace(100, block.size(), block);
	second.replace(50, block.size(), block);
	for (int i = 0; i + 3 <= int(block.size()); ++i) first.replace(850 + 4 * i, 3, block.substr(i, 3));
	auto occurrences = substring_seed_detail::search(first, second, {0, 0, 0});
	assert(occurrences && occurrences->length == reference(first, second));
	check(first, second);
	std::string period(512, 0), repeated;
	for (char &c : period) c = char(rng() % 2);
	for (int i = 0; i < 16; ++i) repeated += period;
	auto shifted = repeated.substr(1) + repeated[0];
	assert(!substring_seed_detail::search(repeated, shifted, {0, 0, 0}));
	check(repeated, shifted, int(repeated.size()) - 1);
	check(shifted, repeated, int(repeated.size()) - 1);
}

int main() {
	seed_matches();
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
