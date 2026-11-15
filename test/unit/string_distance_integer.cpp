#include "string/edit_distance.hpp"
#include "string/lcs.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

template <class T>
int brute_lcs(std::span<const T> s, std::span<const T> t) {
	std::vector<int> row(t.size() + 1);
	for (T x : s) {
		int diagonal = 0;
		for (std::size_t j = 0; j < t.size(); ++j) {
			int previous = row[j + 1];
			row[j + 1] = x == t[j] ? diagonal + 1 : std::max(row[j], previous);
			diagonal = previous;
		}
	}
	return row.back();
}

template <class T>
int brute_edit(std::span<const T> s, std::span<const T> t) {
	std::vector<int> row(t.size() + 1);
	std::iota(row.begin(), row.end(), 0);
	for (T x : s) {
		int diagonal = row[0]++;
		for (std::size_t j = 0; j < t.size(); ++j) {
			int previous = row[j + 1];
			row[j + 1] = std::min({previous + 1, row[j] + 1, diagonal + (x != t[j])});
			diagonal = previous;
		}
	}
	return row.back();
}

template <class T>
bool is_subsequence(const std::vector<T> &sequence, std::span<const T> input) {
	std::size_t matched = 0;
	for (T x : input)
		if (matched < sequence.size() && sequence[matched] == x) ++matched;
	return matched == sequence.size();
}

template <class T>
void check_views(std::span<const T> s, std::span<const T> t, int common = -1, int distance = -1) {
	if (common < 0) common = brute_lcs(s, t);
	if (distance < 0) distance = brute_edit(s, t);
	assert(lcs_length(s, t) == common);
	assert(lcs_length(t, s) == common);
	assert(edit_distance(s, t) == distance);
	assert(edit_distance(t, s) == distance);
	for (auto result : {lcs(s, t), lcs(t, s)}) {
		static_assert(std::is_same_v<decltype(result), std::vector<T>>);
		assert(int(result.size()) == common);
		assert(is_subsequence(result, s) && is_subsequence(result, t));
	}
}

template <class T>
void check(const std::vector<T> &s, const std::vector<T> &t, int common = -1, int distance = -1) {
	if (common < 0) common = brute_lcs<T>(s, t);
	if (distance < 0) distance = brute_edit<T>(s, t);
	assert(lcs_length(s, t) == common && edit_distance(s, t) == distance);
	auto result = lcs(s, t);
	assert(int(result.size()) == common && is_subsequence<T>(result, s) && is_subsequence<T>(result, t));
	check_views<T>(s, t, common, distance);
}

template <class T>
void check_type() {
	const T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	check<T>({}, {});
	check<T>({low, high, T(0)}, {});
	check<T>({}, {low, high, T(0)});
	check<T>({low, high, low, high}, {high, low, high});
	check<T>({T(0), T(1), high, low, T(0)}, {T(1), low, T(0), high});
	std::vector<T> s(133, high), t(131, high);
	t[65] = low;
	check(s, t);
	std::mt19937_64 rng(163912 + sizeof(T));
	for (int repeat = 0; repeat < 150; ++repeat) {
		s.resize(rng() % 150);
		t.resize(rng() % 150);
		for (T &x : s) x = T(rng());
		for (T &x : t) x = rng() % 3 && !s.empty() ? s[rng() % s.size()] : T(rng());
		check(s, t);
	}
	std::pmr::vector<T> pooled(s.begin(), s.end());
	assert(lcs_length(pooled, t) == lcs_length(s, t));
	assert(edit_distance(pooled, t) == edit_distance(s, t));
	assert(lcs(pooled, t) == lcs(s, t));
}

template <class T>
concept Accepted = requires(const std::vector<T> &s) {
	lcs_length(s, s);
	lcs(s, s);
	edit_distance(s, s);
};

int main() {
	static_assert(Accepted<int> && Accepted<std::uint64_t> && !Accepted<bool> && !Accepted<double>);
	std::array<int, 4> mutable_array = {256, -7, 3, 9};
	const std::array<int, 2> const_array = {-7, 9};
	assert(lcs_length(std::span(mutable_array), std::span(const_array)) == 2);
	assert((lcs(std::span(mutable_array), std::span(const_array)) == std::vector<int>{-7, 9}));
	assert(edit_distance(std::span(mutable_array), std::span(const_array)) == 2);
	check_type<char>();
	check_type<signed char>();
	check_type<unsigned char>();
	check_type<short>();
	check_type<unsigned short>();
	check_type<int>();
	check_type<unsigned int>();
	check_type<long>();
	check_type<unsigned long>();
	check_type<long long>();
	check_type<unsigned long long>();
	check_type<char8_t>();
	check_type<char16_t>();
	check_type<char32_t>();
	check_type<wchar_t>();
	check<int>({1, 257, -255, 65537}, {257, -255, 1, 65537});
	check<std::int64_t>({INT64_MIN, -1, 0, INT64_MAX, INT64_MIN}, {INT64_MAX, INT64_MIN, -1, 0});
	check<std::uint64_t>({UINT64_MAX, 0, UINT64_C(1) << 63, UINT64_MAX - 256}, {UINT64_MAX - 256, UINT64_C(1) << 63, 0, UINT64_MAX});
	std::vector<int> wide(1025);
	std::iota(wide.begin(), wide.end(), -512);
	auto backward = wide;
	std::reverse(backward.begin(), backward.end());
	check(wide, backward);
	std::mt19937 rng(7239);
	for (int n : {1, 2, 31, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023}) {
		std::vector<int> s(n), t(n + 3);
		for (int &x : s) x = int(rng() % 701) - 350;
		for (int &x : t) x = int(rng() % 701) - 350;
		check(s, t);
		check_views<int>(std::span<const int>(s).subspan(n / 3), std::span<const int>(t).first(n / 2));
	}
	std::vector<unsigned char> bytes(256);
	std::iota(bytes.begin(), bytes.end(), 0);
	auto reversed_bytes = bytes;
	std::reverse(reversed_bytes.begin(), reversed_bytes.end());
	check(bytes, reversed_bytes, 1, 256);
	std::string a(bytes.begin(), bytes.end()), b(reversed_bytes.begin(), reversed_bytes.end());
	assert(lcs_length(a, b) == lcs_length(bytes, reversed_bytes));
	assert(edit_distance(a, b) == edit_distance(bytes, reversed_bytes));
	std::vector<std::int64_t> s(50000), t(s.size());
	for (int i = 0; i < int(s.size()); ++i) s[i] = INT64_MIN + 2 * i, t[i] = INT64_MIN + 2 * i + 1;
	t[s.size() / 3] = s[s.size() / 3];
	check(s, t, 1, int(s.size()) - 1);
}
