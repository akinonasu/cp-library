#include "string/wildcard_pattern_matching.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <numeric>
#include <random>
#include <span>
#include <type_traits>
#include <vector>

template <class T>
std::vector<bool> brute(std::span<const T> text, std::span<const T> pattern, BasicWildcardOptions<T> options = {}) {
	if (pattern.size() > text.size()) return {};
	std::vector<bool> result(text.size() - pattern.size() + 1, true);
	for (std::size_t i = 0; i < result.size(); ++i)
		for (std::size_t j = 0; j < pattern.size(); ++j)
			if (text[i + j] != pattern[j] && options.text_wildcard != text[i + j] && options.pattern_wildcard != pattern[j]) {
				result[i] = false;
				break;
			}
	return result;
}

std::vector<int> positions(const std::vector<bool> &matches) {
	std::vector<int> result;
	for (std::size_t i = 0; i < matches.size(); ++i) if (matches[i]) result.push_back(int(i));
	return result;
}

template <class T>
void check(const std::vector<T> &text, const std::vector<T> &pattern, BasicWildcardOptions<T> options = {}) {
	auto expected = brute<T>(text, pattern, options);
	assert(wildcard_pattern_matching(text, pattern, options) == expected);
	assert(wildcard_find_all(text, pattern, options) == positions(expected));
	assert(wildcard_pattern_matching(std::span<const T>(text), std::span<const T>(pattern), options) == expected);
	assert(wildcard_find_all(std::span<const T>(text), std::span<const T>(pattern), options) == positions(expected));
	if (options.text_wildcard && options.text_wildcard == options.pattern_wildcard) {
		assert(wildcard_pattern_matching(text, pattern, *options.text_wildcard) == expected);
		assert(wildcard_find_all(text, pattern, *options.text_wildcard) == positions(expected));
	}
	BasicWildcardOptions<T> defaults;
	if (options.text_wildcard == defaults.text_wildcard && options.pattern_wildcard == defaults.pattern_wildcard) {
		assert(wildcard_pattern_matching(text, pattern) == expected);
		assert(wildcard_find_all(text, pattern) == positions(expected));
	}
}

template <class T>
void widths() {
	T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	check<T>({}, {});
	check<T>({}, {low});
	check<T>({low, high, T(0)}, {});
	check<T>({low, high, low, high}, {low, high});
	check<T>({low, high, low, high}, {low, high}, {low, high});
	check<T>({low, high, low, high}, {low, high}, {std::nullopt, low});
	check<T>({low, high, low, high}, {low, high}, {high, std::nullopt});
	std::mt19937_64 rng(49193 + sizeof(T));
	std::vector<T> alphabet{low, high, T(0), T(1), T(-1), T(42), T(257)};
	for (int trial = 0; trial < 100; ++trial) {
		std::vector<T> text(rng() % 190), pattern(rng() % 200);
		for (T &x : text) x = alphabet[rng() % alphabet.size()];
		for (T &x : pattern) x = alphabet[rng() % alphabet.size()];
		BasicWildcardOptions<T> options{trial % 3 ? std::optional<T>(low) : std::nullopt,
			trial % 5 ? std::optional<T>(high) : std::nullopt};
		check(text, pattern, options);
	}
	for (int m : {1, 2, 31, 63, 64, 65, 127, 128, 129}) {
		std::vector<T> text(2 * m + 3, low), pattern(m, low);
		for (std::size_t i = 0; i < text.size(); i += 7) text[i] = high;
		for (std::size_t i = 0; i < pattern.size(); i += 11) pattern[i] = high;
		check(text, pattern, {high, high});
		check(text, pattern, {std::nullopt, high});
		check(text, pattern, {high, std::nullopt});
	}
}

void inputs() {
	std::array<std::int64_t, 5> text{INT64_MIN, -1, 257, INT64_MAX, -1};
	const std::array<std::int64_t, 2> pattern{257, INT64_MAX};
	assert((wildcard_find_all(std::span(text), std::span(pattern)) == std::vector<int>{2}));
	assert((wildcard_find_all(std::span(text), std::span(pattern), -1) == std::vector<int>{2}));
	std::pmr::vector<std::int64_t> pooled(text.begin(), text.end());
	std::vector<std::int64_t> vector_pattern(pattern.begin(), pattern.end());
	assert((wildcard_find_all(pooled, vector_pattern) == std::vector<int>{2}));
	assert(wildcard_pattern_matching(pooled, vector_pattern) == brute<std::int64_t>(text, pattern));
	std::vector<char> bytes{char(255), '*', 0, char(255), 'x', 0}, needle{char(255), '*', 0};
	check(bytes, needle);
	assert((wildcard_find_all(bytes, needle) == std::vector<int>{0, 3}));
	check(bytes, needle, {std::nullopt, std::nullopt});
	std::vector<std::int64_t> ordinary{42, -1, 42}, marker{42};
	assert((wildcard_find_all(ordinary, marker) == std::vector<int>{0, 2}));
	assert((wildcard_find_all(ordinary, marker, 42) == std::vector<int>{0, 1, 2}));
	std::vector<long long> wide{1000, -1, 1000, 257}, wide_pattern{1000, -1};
	assert((wildcard_find_all(wide, wide_pattern, -1) == std::vector<int>{0, 1, 2}));
}

void large_alphabets() {
	for (int m : {513, 70001}) {
		std::vector<std::uint64_t> pattern(m), text(m + 2);
		for (int i = 0; i < m; ++i) pattern[i] = UINT64_MAX - std::uint64_t(i + 1) * 65537;
		pattern.back() = UINT64_MAX;
		std::copy(pattern.begin(), pattern.end(), text.begin());
		text[m] = 17, text[m + 1] = 19;
		BasicWildcardOptions<std::uint64_t> options{std::nullopt, UINT64_MAX};
		check(text, pattern, options);
		assert((wildcard_find_all(text, pattern, options) == std::vector<int>{0}));
		text[m == 513 ? 256 : m - 1 - 65536] = 23;
		check(text, pattern, options);
		assert(wildcard_find_all(text, pattern, options).empty());
		std::copy(pattern.begin(), pattern.end(), text.begin());
		text[m == 513 ? 511 : 65535] = 23;
		check(text, pattern, options);
		assert(wildcard_find_all(text, pattern, options).empty());
	}
}

void fallback() {
	for (int m : {65, 257, 1025}) {
		std::vector<std::int64_t> text(9 * m + 19, INT64_MIN), pattern(m, INT64_MIN);
		for (std::size_t i = 0; i < pattern.size(); i += 13) pattern[i] = INT64_MAX;
		check(text, pattern, {std::nullopt, INT64_MAX});
		for (std::size_t i = 0; i < text.size(); i += 17) text[i] = INT64_MAX - 1;
		check(text, pattern, {INT64_MAX - 1, INT64_MAX});
		pattern[m / 2] = 257;
		check(text, pattern, {INT64_MAX - 1, INT64_MAX});
		for (int start : {0, m - 1, m, 2 * m - 1, int(text.size()) - m}) text[start + m / 2] = 257;
		check(text, pattern, {INT64_MAX - 1, INT64_MAX});
	}
	constexpr int m = 257;
	std::vector<std::int64_t> text(9 * m, 257), pattern(m, INT64_MIN);
	std::fill(text.begin(), text.begin() + 4 * m, INT64_MIN);
	for (int i = 0; i < m; i += 13) pattern[i] = INT64_MAX;
	pattern.back() = 257;
	check(text, pattern, {std::nullopt, INT64_MAX});
	text.assign(9 * m, INT64_MAX - 1);
	text[m / 2] = INT64_MIN;
	text[m + 17] = text[2 * m + 33] = 257;
	pattern.assign(m, INT64_MIN);
	pattern.back() = 257;
	check(text, pattern, {INT64_MAX - 1, std::nullopt});
	for (int i = 0; i < m; i += 13) pattern[i] = INT64_MAX;
	check(text, pattern, {INT64_MAX - 1, INT64_MAX});
}

void three_primes() {
	constexpr std::uint64_t product = UINT64_C(998244353) * 469762049;
	static_assert(UINT64_C(482812391) * 482812391 + UINT64_C(485622804) * 485622804 == product);
	std::vector<std::uint32_t> text(66, 1), pattern(65, 1);
	text[0] = 482812392, text[1] = 485622805;
	std::vector<bool> matches{true, false};
	wildcard_matching_detail::filter_prime<998244353>(text, pattern, matches, false, false);
	assert(matches[0] && !matches[1]);
	wildcard_matching_detail::filter_prime<469762049>(text, pattern, matches, false, false);
	assert(matches[0] && !matches[1]);
	wildcard_matching_detail::filter_prime<167772161>(text, pattern, matches, false, false);
	assert(!matches[0] && !matches[1]);
	matches = {true, false};
	wildcard_matching_detail::filter_ntt(text, pattern, matches, __uint128_t(product), false, false);
	assert(!matches[0] && !matches[1]);
	text.assign(66, UINT32_MAX);
	pattern.assign(65, UINT32_MAX);
	matches = {true, false};
	wildcard_matching_detail::filter_ntt(text, pattern, matches, 0, false, false);
	assert(matches[0] && !matches[1]);
}

template <class T>
concept Accepted = requires(const std::vector<T> &s) { wildcard_pattern_matching(s, s); wildcard_find_all(s, s); };

template <class T, class U>
concept MixedAccepted = requires(const std::vector<T> &s, const std::vector<U> &t) { wildcard_pattern_matching(s, t); };

int main() {
	static_assert(Accepted<int> && Accepted<std::uint64_t> && !Accepted<bool> && !Accepted<double>);
	static_assert(!MixedAccepted<int, unsigned int> && !MixedAccepted<std::int64_t, std::uint64_t>);
	widths<char>(); widths<signed char>(); widths<unsigned char>();
	widths<short>(); widths<unsigned short>(); widths<int>(); widths<unsigned int>();
	widths<long>(); widths<unsigned long>(); widths<long long>(); widths<unsigned long long>();
	widths<char8_t>(); widths<char16_t>(); widths<char32_t>(); widths<wchar_t>();
	inputs(); large_alphabets(); fallback(); three_primes();
	std::vector<unsigned char> text, too_long((1 << 22) + 1);
	bool threw = false;
	try { wildcard_pattern_matching(text, too_long); } catch (const std::length_error &) { threw = true; }
	assert(threw);
}
