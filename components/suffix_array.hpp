#ifndef SUFFIX_ARRAY_HPP
#define SUFFIX_ARRAY_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cassert>
#include <limits>
#include <numeric>
#include <string_view>
#include <vector>
#include <type_traits>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace suffix_array_detail {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2"))) inline void byte_seeds(const unsigned char *s, int n, std::vector<int> &seeds, std::vector<unsigned char> &marked) {
	seeds.push_back(n), marked[n] = 1;
	bool right = true;
	int end = n;
	const auto sign = _mm256_set1_epi8(static_cast<char>(0x80));
	while (end >= 33) {
		int begin = end - 32;
		auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s + begin));
		auto next = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s + begin + 1));
		auto previous = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s + begin - 1));
		auto biased = _mm256_xor_si256(a, sign);
		std::uint32_t equal = _mm256_movemask_epi8(_mm256_cmpeq_epi8(a, next));
		std::uint32_t small = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_xor_si256(next, sign), biased));
		if (right) small |= equal & 0x80000000U;
		for (int shift : {1, 2, 4, 8, 16}) {
			small |= equal & (small >> shift);
			equal &= equal >> shift;
		}
		std::uint32_t selected = small & std::uint32_t(_mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_xor_si256(previous, sign), biased)));
		while (selected) {
			int bit = 31 - std::countl_zero(selected), p = begin + bit;
			seeds.push_back(p), marked[p] = 1;
			selected ^= std::uint32_t(1) << bit;
		}
		right = small & 1;
		end = begin;
	}
	for (int i = end - 1; i > 0; --i) {
		bool current = s[i] < s[i + 1] || (s[i] == s[i + 1] && right);
		if (current && s[i - 1] > s[i]) seeds.push_back(i), marked[i] = 1;
		right = current;
	}
}
#endif
template<class Sequence>
bool prefix_order(const Sequence &s, int n, int alphabet, const std::vector<int> &seeds, std::vector<int> &ordered) {
	if (seeds.size() <= 64 || alphabet > 257) return false;
	int bits = std::bit_width(unsigned(alphabet - 1)), position_bits = std::bit_width(unsigned(n));
	int length = (64 - position_bits) / bits;
	std::uint64_t position_mask = (std::uint64_t(1) << position_bits) - 1;
	auto key_at = [&](int p) {
		std::uint64_t key = 0;
		for (int k = 0; k < length; ++k) key = (key << bits) | (k < n - p ? unsigned(s[p + k]) : 0);
		return key;
	};
	std::array<std::uint64_t, 128> sample;
	int sample_size = std::min(128, static_cast<int>(seeds.size()));
	for (int i = 0; i < sample_size; ++i) sample[i] = key_at(seeds[i]);
	std::sort(sample.begin(), sample.begin() + sample_size);
	for (int i = 1; i < sample_size; ++i) if (sample[i] == sample[i - 1]) return false;
	std::vector<std::uint64_t> entries(seeds.size()), scratch(seeds.size());
	for (std::size_t i = 0; i < seeds.size(); ++i) entries[i] = (key_at(seeds[i]) << position_bits) | unsigned(n - seeds[i]);
	int first_shift = std::max(position_bits, bits * length + position_bits - 33);
	for (int shift = first_shift; shift < bits * length + position_bits; shift += 11) {
		std::array<int, 2048> count{};
		for (auto entry : entries) ++count[(entry >> shift) & 2047];
		int total = 0;
		for (int &value : count) { int next = total + value; value = total; total = next; }
		for (auto entry : entries) scratch[count[(entry >> shift) & 2047]++] = entry;
		entries.swap(scratch);
	}
	for (std::size_t first = 0; first < entries.size();) {
		std::size_t last = first + 1;
		while (last < entries.size() && (entries[last] >> first_shift) == (entries[first] >> first_shift)) ++last;
		if (last - first > 64) return false;
		std::sort(entries.begin() + first, entries.begin() + last);
		for (std::size_t i = first + 1; i < last; ++i) if ((entries[i] >> position_bits) == (entries[i - 1] >> position_bits)) return false;
		first = last;
	}
	ordered.resize(seeds.size());
	for (std::size_t i = 0; i < entries.size(); ++i) ordered[i] = n - static_cast<int>(entries[i] & position_mask);
	return true;
}

template<class Sequence>
std::vector<int> encoded_sort(const Sequence &s, int n, int alphabet) {
	std::vector<int> sa(std::size_t(n) + 1);
	if (n < 16) {
		std::iota(sa.begin(), sa.end(), 0);
		std::sort(sa.begin(), sa.end(), [&](int a, int b) {
			while (a < n && b < n && s[a] == s[b]) ++a, ++b;
			return a != n ? b != n && s[a] < s[b] : b != n;
		});
		return sa;
	}
	std::vector<unsigned char> is_seed(std::size_t(n) + 1);
	std::vector<int> seeds;
	seeds.reserve(n / 2 + 1);
	bool gathered = false;
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if constexpr (std::is_same_v<Sequence, std::vector<unsigned char>>) {
		if (n >= 64 && __builtin_cpu_supports("avx2")) {
			byte_seeds(s.data(), n, seeds, is_seed);
			gathered = true;
		}
	}
#endif
	if (!gathered) {
		bool next = true;
		for (int i = n - 1; i >= 0; --i) {
			bool current = s[i] == s[i + 1] ? next : s[i] < s[i + 1];
			if (!current && next) seeds.push_back(i + 1), is_seed[i + 1] = 1;
			next = current;
		}
	}
	std::reverse(seeds.begin(), seeds.end());
	std::vector<std::size_t> ends(alphabet), work(alphabet);
	for (std::size_t i = 0; i < sa.size(); ++i) ++ends[s[i]];
	std::partial_sum(ends.begin(), ends.end(), ends.begin());
	auto induce = [&](const std::vector<int> &ordered) {
		std::fill(sa.begin(), sa.end(), 0);
		work = ends;
		for (auto i = ordered.rbegin(); i != ordered.rend(); ++i) if (*i != n) sa[--work[s[*i]]] = *i + 1;
		work[0] = 0;
		std::copy(ends.begin(), ends.end() - 1, work.begin() + 1);
		int last = n;
		if (s[n - 2] < s[n - 1]) last = -last;
		sa[work[s[n - 1]]++] = last;
		for (std::size_t i = 1; i < sa.size(); ++i) {
			int p = sa[i];
			if (p > 1) {
				int j = p - 2, c = s[j], v = j + 1;
				if (!j || s[j - 1] < c) v = -v;
				sa[work[c]++] = v;
			}
		}
		work = ends;
		for (int i = n; i > 0; --i) {
			int p = sa[i];
			sa[i] = (p < 0 ? -p : p) - 1;
			if (p < -1) {
				int j = -p - 2, c = s[j], v = j + 1;
				if (j && s[j - 1] <= c) v = -v;
				sa[--work[c]] = v;
			}
		}
		sa[0] = n;
	};
	std::vector<int> ordered;
	int used = alphabet;
	while (used > 1 && ends[used - 1] == ends[used - 2]) --used;
	if (prefix_order(s, n, used, seeds, ordered)) { induce(ordered); return sa; }
	induce(seeds);
	int m = static_cast<int>(seeds.size());
	std::vector<int> sorted;
	sorted.reserve(m);
	for (int p : sa) if (is_seed[p]) sorted.push_back(p);
	std::vector<int> names(std::size_t(n) / 2 + 1);
	for (int i = 0; i + 1 < m; ++i) names[seeds[i] >> 1] = seeds[i + 1] - seeds[i] + 1;
	names[n >> 1] = 1;
	int rank = -1, previous = -1, previous_length = 0;
	for (int p : sorted) {
		int length = names[p >> 1];
		bool different = previous < 0 || length != previous_length;
		if (!different) for (int k = 0; k < length; ++k) if (s[p + k] != s[previous + k]) { different = true; break; }
		rank += different;
		names[p >> 1] = rank;
		previous = p, previous_length = length;
	}
	std::vector<int> reduced(m), order(m);
	for (int i = 0; i < m; ++i) reduced[i] = names[seeds[i] >> 1];
	if (rank + 1 == m) for (int i = 0; i < m; ++i) order[reduced[i]] = i;
	else order = encoded_sort(reduced, m - 1, rank + 1);
	for (int i = 0; i < m; ++i) sorted[i] = seeds[order[i]];
	induce(sorted);
	return sa;
}

template<class Sequence>
std::vector<int> induced_sort(const Sequence &s, int upper) {
	struct Encoded {
		const Sequence &value;
		int n;
		int operator[](int i) const { return i == n ? 0 : value[i] + 1; }
	};
	int n = static_cast<int>(s.size());
	if (n < 2) return n ? std::vector<int>{0} : std::vector<int>{};
	int common = 1;
	while (common < n && s[common] == s[0]) ++common;
	if (common == n) {
		std::vector<int> sa(n);
		std::iota(sa.rbegin(), sa.rend(), 0);
		return sa;
	}
	auto sa = encoded_sort(Encoded{s, n}, n, upper + 2);
	sa.erase(sa.begin());
	return sa;
}

inline std::vector<int> bytes(std::string_view s) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (s.empty()) return {};
	std::array<int, 256> codes{};
	for (unsigned char c : s) codes[c] = 1;
	int alphabet = 0;
	for (int &code : codes) if (code) code = ++alphabet;
	int n = static_cast<int>(s.size());
	if (alphabet == 1) {
		std::vector<int> sa(n);
		std::iota(sa.rbegin(), sa.rend(), 0);
		return sa;
	}
	auto build = [&]<class Symbol>() {
		std::vector<Symbol> text(s.size() + 1);
		for (int i = 0; i < n; ++i) text[i] = static_cast<Symbol>(codes[static_cast<unsigned char>(s[i])]);
		auto sa = encoded_sort(text, n, alphabet + 1);
		sa.erase(sa.begin());
		return sa;
	};
	return alphabet < 256 ? build.template operator()<unsigned char>() : build.template operator()<std::uint16_t>();
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
