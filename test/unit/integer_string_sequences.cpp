#include "string/suffix_array.hpp"
#include "string/lcp_array.hpp"
#include "string/z_algorithm.hpp"
#include "string/run_enumerate.hpp"
#include "string/manacher.hpp"
#include <array>
#include <memory_resource>
#include <optional>
#include <random>

struct Expected {
	std::vector<int> sa, lcp, z, odd, even, lengths;
	std::vector<Run> runs;
	std::pair<int, int> longest{0, 0};
	std::int64_t count = 0;
};

template <class Sequence>
bool palindrome(const Sequence &s, int l, int r) {
	while (l < r) if (s[l++] != s[--r]) return false;
	return true;
}

template <class Sequence>
std::vector<Run> run_oracle(const Sequence &s) {
	int n = int(s.size());
	std::vector<Run> result;
	for (int l = 0; l < n; ++l) for (int r = l + 2; r <= n; ++r) {
		int period = 1;
		for (; period <= (r - l) / 2; ++period) {
			bool valid = true;
			for (int i = l + period; i < r; ++i) if (s[i] != s[i - period]) { valid = false; break; }
			if (valid) break;
		}
		if (period > (r - l) / 2) continue;
		if (l && s[l - 1] == s[l + period - 1]) continue;
		if (r < n && s[r] == s[r - period]) continue;
		result.push_back({period, l, r});
	}
	std::sort(result.begin(), result.end());
	return result;
}

template <class Sequence>
Expected oracle(const Sequence &s, std::optional<std::vector<Run>> known_runs = std::nullopt) {
	int n = int(s.size());
	Expected result;
	result.sa.resize(n);
	std::iota(result.sa.begin(), result.sa.end(), 0);
	std::sort(result.sa.begin(), result.sa.end(), [&](int a, int b) {
		while (a < n && b < n && s[a] == s[b]) ++a, ++b;
		return b < n && (a == n || s[a] < s[b]);
	});
	if (n) result.lcp.resize(n - 1);
	for (int i = 0; i + 1 < n; ++i) {
		int a = result.sa[i], b = result.sa[i + 1], h = 0;
		while (h < n - std::max(a, b) && s[a + h] == s[b + h]) ++h;
		result.lcp[i] = h;
	}
	result.z.resize(n);
	for (int i = 0; i < n; ++i) while (i + result.z[i] < n && s[result.z[i]] == s[i + result.z[i]]) ++result.z[i];
	result.odd.resize(n); result.even.resize(n);
	if (n) result.lengths.resize(2 * std::size_t(n) - 1);
	for (int i = 0; i < n; ++i) {
		int odd = 1, even = 0;
		while (odd <= i && i + odd < n && s[i - odd] == s[i + odd]) ++odd;
		while (even < i && i + even < n && s[i - even - 1] == s[i + even]) ++even;
		result.odd[i] = odd; result.even[i] = even;
		result.count += odd + even;
		result.lengths[2 * i] = 2 * odd - 1;
		if (i) result.lengths[2 * i - 1] = 2 * even;
		for (auto interval : {std::pair(i - odd + 1, i + odd), std::pair(i - even, i + even)}) {
			int length = interval.second - interval.first, best = result.longest.second - result.longest.first;
			if (length > best || (length == best && interval.first < result.longest.first)) result.longest = interval;
		}
	}
	result.runs = known_runs ? *known_runs : run_oracle(s);
	return result;
}

template <class Sequence>
void check_view(const Sequence &s, const Expected &expected) {
	assert(suffix_array(s) == expected.sa);
	assert(lcp_array(s, expected.sa) == expected.lcp);
	assert(z_algorithm(s) == expected.z);
	assert(run_enumerate(s) == expected.runs);
	Manacher actual(s), rebuilt;
	rebuilt.build(s);
	assert(actual.odd == expected.odd && actual.even == expected.even);
	assert(rebuilt.odd == expected.odd && rebuilt.even == expected.even);
	assert(actual.size() == int(s.size()) && actual.empty() == s.empty());
	assert(actual.lengths() == expected.lengths && palindrome_lengths(s) == expected.lengths);
	assert(actual.palindrome_count() == expected.count && actual.longest_palindrome() == expected.longest);
	for (int l = 0; l <= int(s.size()); ++l) for (int r = l; r <= int(s.size()); ++r)
		assert(actual.is_palindrome(l, r) == palindrome(s, l, r));
}

template <class T>
void check(std::vector<T> s, std::optional<std::vector<Run>> known_runs = std::nullopt) {
	auto expected = oracle(s, known_runs);
	const auto saved = s;
	check_view(s, expected);
	check_view(std::span<T>(s), expected);
	check_view(std::span<const T>(s), expected);
	std::pmr::monotonic_buffer_resource memory;
	std::pmr::vector<T> custom(&memory);
	custom.assign(s.begin(), s.end());
	check_view(custom, expected);
	assert(s == saved);
}

template <class T>
void width() {
	T minimum = std::numeric_limits<T>::min(), maximum = std::numeric_limits<T>::max();
	std::array<T, 8> values{minimum, maximum, minimum, maximum, T(0), T(0), maximum, minimum};
	auto expected = oracle(values);
	check_view(std::span(values), expected);
	check_view(std::span<const T, 8>(values), expected);
	check(std::vector<T>(values.begin(), values.end()));
	check(std::vector<T>{});
	check(std::vector<T>{maximum});
	check(std::vector<T>(20, maximum));
}

void bounded_alphabet() {
	std::vector<int> values{1024, 0, 1024, 1, 0, 1024, 1};
	auto expected = oracle(values);
	assert(suffix_array(values, 1024) == expected.sa);
	assert(suffix_array(std::span(values), 1024) == expected.sa);
	assert(suffix_array(std::span<const int>(values), 1024) == expected.sa);
	std::array<int, 7> fixed;
	std::copy(values.begin(), values.end(), fixed.begin());
	assert(suffix_array(std::span(fixed), 1024) == expected.sa);
	assert(suffix_array(std::span<const int, 7>(fixed), 1024) == expected.sa);
	std::pmr::vector<int> custom(values.begin(), values.end());
	assert(suffix_array(custom, 1024) == expected.sa);
	for (int upper : {std::numeric_limits<int>::max() - 1, std::numeric_limits<int>::max()}) {
		assert(suffix_array(values, upper) == expected.sa);
		assert(suffix_array(custom, upper) == expected.sa);
		assert(suffix_array(std::span(fixed), upper) == expected.sa);
		assert(suffix_array(std::span<const int, 7>(fixed), upper) == expected.sa);
	}
	std::array<int, 0> empty;
	assert(suffix_array(std::span(empty), 0).empty());
	assert(suffix_array(std::span<const int, 0>(empty), 0).empty());
	assert(suffix_array(std::vector<int>{}, 0).empty());
	assert(suffix_array(std::pmr::vector<int>{}, 0).empty());
}

void char_order() {
	std::string bytes{char(255), char(0), char(128), char(255)};
	std::vector<char> values(bytes.begin(), bytes.end());
	auto expected = oracle(values);
	check_view(values, expected);
	check_view(std::span(values), expected);
	check_view(std::span<const char>(values), expected);
	std::vector<unsigned char> unsigned_values(bytes.begin(), bytes.end());
	assert(suffix_array(bytes) == oracle(unsigned_values).sa);
	assert(lcp_array(bytes, suffix_array(bytes)) == oracle(unsigned_values).lcp);
	assert(z_algorithm(bytes) == expected.z && run_enumerate(bytes) == expected.runs);
}

int main() {
	width<char>(); width<signed char>(); width<unsigned char>();
	width<short>(); width<unsigned short>(); width<int>(); width<unsigned int>();
	width<long>(); width<unsigned long>(); width<long long>(); width<unsigned long long>();
	width<wchar_t>(); width<char8_t>(); width<char16_t>(); width<char32_t>();
	bounded_alphabet();
	char_order();
	for (int n = 0; n <= 7; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::vector<std::int64_t> values(n);
		for (int i = 0; i < n; ++i) values[i] = (mask >> i & 1) ? std::numeric_limits<std::int64_t>::min() : std::numeric_limits<std::int64_t>::max();
		check(values);
	}
	std::mt19937_64 random(82087123);
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<std::uint64_t> values(random() % 40);
		for (auto &x : values) x = trial % 2 ? random() : (random() % 4) * (std::uint64_t(1) << 62);
		check(values);
	}
	std::vector<std::uint64_t> alphabet(1026);
	for (int i = 0; i < 513; ++i) alphabet[i] = alphabet[i + 513] = std::numeric_limits<std::uint64_t>::max() - i;
	check(alphabet, std::vector<Run>{{513, 0, 1026}});
	alphabet.resize(513);
	check(alphabet, std::vector<Run>{});
	std::vector<bool> bits{true, false, true, true, false};
	auto expected = oracle(bits);
	assert(suffix_array(bits) == expected.sa && lcp_array(bits, expected.sa) == expected.lcp);
	assert(Manacher(bits).lengths() == expected.lengths && palindrome_lengths(bits) == expected.lengths);
}
