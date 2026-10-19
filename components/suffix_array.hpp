#ifndef SUFFIX_ARRAY_HPP
#define SUFFIX_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <string_view>
#include <vector>

namespace suffix_array_detail {
template<class Sequence>
std::vector<int> induced_sort(const Sequence &s, int upper) {
	int n = static_cast<int>(s.size());
	std::vector<int> sa(n);
	if (n < 16) {
		std::iota(sa.begin(), sa.end(), 0);
		std::sort(sa.begin(), sa.end(), [&](int a, int b) {
			while (a < n && b < n && s[a] == s[b]) ++a, ++b;
			return a != n ? b != n && s[a] < s[b] : b != n;
		});
		return sa;
	}
	std::vector<unsigned char> small(n);
	for (int i = n - 2; i >= 0; --i) small[i] = s[i] == s[i + 1] ? small[i + 1] : s[i] < s[i + 1];
	std::vector<int> left(upper + 2), right(upper + 2), cursor(upper + 2);
	for (int i = 0; i < n; ++i) {
		if (small[i]) ++left[s[i] + 1];
		else ++right[s[i]];
	}
	for (int c = 0; c <= upper; ++c) {
		right[c] += left[c];
		left[c + 1] += right[c];
	}
	auto induce = [&](const std::vector<int> &seeds) {
		std::fill(sa.begin(), sa.end(), -1);
		cursor = right;
		for (int v : seeds) sa[cursor[s[v]]++] = v;
		cursor = left;
		sa[cursor[s[n - 1]]++] = n - 1;
		for (int i = 0; i < n; ++i) {
			int v = sa[i] - 1;
			if (v >= 0 && !small[v]) sa[cursor[s[v]]++] = v;
		}
		cursor = left;
		for (int i = n - 1; i >= 0; --i) {
			int v = sa[i] - 1;
			if (v >= 0 && small[v]) sa[--cursor[s[v] + 1]] = v;
		}
	};
	std::vector<int> number(n, -1), seeds;
	for (int i = 1; i < n; ++i) if (!small[i - 1] && small[i]) {
		number[i] = static_cast<int>(seeds.size());
		seeds.push_back(i);
	}
	induce(seeds);
	int m = static_cast<int>(seeds.size());
	if (!m) return sa;
	std::vector<int> sorted;
	sorted.reserve(m);
	for (int v : sa) if (number[v] >= 0) sorted.push_back(v);
	std::vector<int> reduced(m);
	int alphabet = 0;
	for (int i = 1; i < m; ++i) {
		int a = sorted[i - 1], b = sorted[i];
		int end_a = number[a] + 1 < m ? seeds[number[a] + 1] : n;
		int end_b = number[b] + 1 < m ? seeds[number[b] + 1] : n;
		bool equal = end_a - a == end_b - b && end_a < n && end_b < n;
		if (equal) for (int k = 0; k <= end_a - a; ++k) if (s[a + k] != s[b + k]) { equal = false; break; }
		if (!equal) ++alphabet;
		reduced[number[b]] = alphabet;
	}
	std::vector<int> order(m);
	if (alphabet + 1 == m) for (int i = 0; i < m; ++i) order[reduced[i]] = i;
	else order = induced_sort(reduced, alphabet);
	for (int i = 0; i < m; ++i) sorted[i] = seeds[order[i]];
	induce(sorted);
	return sa;
}

inline std::vector<int> bytes(std::string_view s) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
	struct Bytes {
		std::string_view value;
		std::size_t size() const { return value.size(); }
		unsigned char operator[](int i) const { return static_cast<unsigned char>(value[i]); }
	};
	return induced_sort(Bytes{s}, 255);
}
}

#endif
