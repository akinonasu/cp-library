#include "string/manacher.hpp"
#include <array>
#include <deque>
#include <random>
#include <span>
#include <string>

template <class Sequence>
void check(const Sequence &s) {
	int n = int(s.size());
	Manacher result(s), rebuilt("stale");
	rebuilt.build(s);
	assert(result.size() == n && result.empty() == (n == 0));
	assert(result.odd == rebuilt.odd && result.even == rebuilt.even);
	std::vector<std::vector<bool>> palindrome(n + 1, std::vector<bool>(n + 1));
	std::int64_t count = 0;
	std::pair<int, int> longest{0, 0};
	for (int l = n; l >= 0; --l) {
		for (int r = l; r <= n; ++r) {
			palindrome[l][r] = r - l <= 1 || (s[l] == s[r - 1] && palindrome[l + 1][r - 1]);
			assert(result.is_palindrome(l, r) == palindrome[l][r]);
			if (l < r && palindrome[l][r]) {
				++count;
				if (r - l >= longest.second - longest.first) longest = {l, r};
			}
		}
	}
	assert(result.palindrome_count() == count);
	assert(result.longest_palindrome() == longest);
	auto lengths = result.lengths();
	assert(lengths.size() == (n ? 2 * s.size() - 1 : 0));
	for (int i = 0; i < n; ++i) {
		int a = 1, b = 0;
		while (a <= i && a < n - i && s[i - a] == s[i + a]) ++a;
		while (b < i && b < n - i && s[i - b - 1] == s[i + b]) ++b;
		assert(result.odd[i] == a && result.even[i] == b);
		assert(lengths[2 * i] == 2 * a - 1);
		if (i) assert(lengths[2 * i - 1] == 2 * b);
	}
}

struct Counted {
	int value;
	static inline int comparisons = 0;
	bool operator==(const Counted &other) const {
		++comparisons;
		return value == other.value;
	}
};

int main() {
	Manacher empty;
	assert(empty.empty() && empty.size() == 0);
	assert(empty.is_palindrome(0, 0) && empty.palindrome_count() == 0);
	assert((empty.longest_palindrome() == std::pair<int, int>{0, 0}));
	assert(empty.lengths().empty());
	assert(Manacher("aba").size() == 3);
	empty.build("abba");
	assert(empty.even[2] == 2);
	empty.build("");
	assert(empty.empty());
	for (int n = 0, total = 1; n <= 8; ++n, total *= 3) {
		for (int code = 0; code < total; ++code) {
			std::string s(n, '\0');
			for (int i = 0, value = code; i < n; ++i, value /= 3) s[i] = char(value % 3);
			check(s);
		}
	}
	std::mt19937 rng(250181);
	for (int alphabet : {2, 4, 26, 256}) for (int trial = 0; trial < 200; ++trial) {
		std::string s(rng() % 101, '\0');
		for (char &c : s) c = char(rng() % alphabet);
		check(std::string_view(s));
	}
	for (int prefix : {1, 3, 16, 33, 65}) {
		std::string s(prefix, 'a');
		for (int i = 0; i < 35; ++i) s.push_back(char('b' + rng() % 4));
		check(s);
		s += std::string(prefix, 'a');
		check(s);
	}
	check(std::string("\0$#\xff#\0", 6));
	check(std::vector<int>{-1, 500, 500, -1, 500, -1});
	check(std::vector<bool>{true, false, true, true});
	std::array<int, 6> values{1, -5, 1000, 1000, -5, 1};
	check(values);
	check(std::span<const int>(values));
	check(std::deque<int>(values.begin(), values.end()));
	Manacher preserved;
	{
		std::string s = "abacabba";
		preserved.build(s);
		s.assign(s.size(), 'x');
	}
	assert((preserved.longest_palindrome() == std::pair<int, int>{1, 6}));
	assert(preserved.palindrome_count() == 13);
	const int n = 500000;
	for (bool alternating : {false, true}) {
		std::vector<Counted> s(n);
		for (int i = 0; i < n; ++i) s[i].value = alternating ? i & 1 : 0;
		Counted::comparisons = 0;
		Manacher result(s);
		assert(Counted::comparisons <= 4 * n);
		std::int64_t count = 0;
		for (int i = 0; i < n; ++i) {
			assert(result.odd[i] == std::min(i + 1, n - i));
			assert(result.even[i] == (alternating ? 0 : std::min(i, n - i)));
			count += std::int64_t(result.odd[i]) + result.even[i];
		}
		assert(result.palindrome_count() == count);
		if (!alternating) assert(count == std::int64_t(n) * (n + 1) / 2);
		assert((result.longest_palindrome() == std::pair<int, int>{0, n - int(alternating)}));
	}
}
