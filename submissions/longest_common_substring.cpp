#line 1 "test/longest_common_substring.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#line 1 "string/longest_common_substring.hpp"



#line 1 "components/sequence_compare.hpp"



#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace sequence_compare_detail {
#if defined(__x86_64__)
__attribute__((target("avx2"))) inline std::size_t forward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countr_zero(difference);
	}
	while (i < size && a[i] == b[i]) ++i;
	return i;
}

__attribute__((target("avx2"))) inline std::size_t backward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a - i - 32));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b - i - 32));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countl_zero(difference);
	}
	while (i < size && a[-std::ptrdiff_t(i) - 1] == b[-std::ptrdiff_t(i) - 1]) ++i;
	return i;
}
#endif

template<bool Vectorize = true, class T>
inline int prefix(const T *first, const T *second, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first);
		const auto *b = reinterpret_cast<const unsigned char *>(second);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a + offset, 8);
			std::memcpy(&y, b + offset, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countr_zero(difference) : std::countl_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + forward_avx2(a + offset, b + offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first[matched] == second[matched]) ++matched;
	return matched;
}

template<bool Vectorize = true, class T>
inline int suffix(const T *first_end, const T *second_end, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first_end);
		const auto *b = reinterpret_cast<const unsigned char *>(second_end);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a - offset - 8, 8);
			std::memcpy(&y, b - offset - 8, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countl_zero(difference) : std::countr_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + backward_avx2(a - offset, b - offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first_end[-matched - 1] == second_end[-matched - 1]) ++matched;
	return matched;
}
}


#line 1 "components/suffix_array.hpp"



#include <algorithm>
#include <array>
#line 8 "components/suffix_array.hpp"
#include <cassert>
#include <limits>
#include <numeric>
#include <string_view>
#include <vector>
#line 14 "components/suffix_array.hpp"
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


#line 1 "components/substring_seed.hpp"



#line 9 "components/substring_seed.hpp"
#include <optional>
#line 12 "components/substring_seed.hpp"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace substring_seed_detail {
struct Match { int first, second, length; };

inline std::optional<Match> search(std::string_view first, std::string_view second, Match best) {
	bool swapped = first.size() > second.size();
	if (swapped) {
		std::swap(first, second);
		std::swap(best.first, best.second);
	}
	if (first.size() < 256 || first.size() > (std::size_t(1) << 21)) return std::nullopt;
	int n = static_cast<int>(first.size()), m = static_cast<int>(second.size());
	std::array<unsigned char, 256> present{}, codes{};
	for (unsigned char c : first) present[c] |= 1;
	for (unsigned char c : second) present[c] |= 2;
	unsigned alphabet = 0, first_symbols = 0, second_symbols = 0, uniform = 0;
	bool common = false;
	for (int c = 0; c < 256; ++c) {
		if (present[c]) codes[c] = static_cast<unsigned char>(alphabet++);
		first_symbols += bool(present[c] & 1), second_symbols += bool(present[c] & 2);
		if (present[c] == 3) common = true, uniform = c;
	}
	if (!common) return Match{0, 0, 0};
	if (first_symbols == 1 || second_symbols == 1) {
		bool uniform_first = first_symbols == 1;
		auto text = uniform_first ? second : first;
		int limit = uniform_first ? n : m, run = 0;
		best = {0, 0, 0};
		for (int i = 0; i < int(text.size()); ++i) {
			run = static_cast<unsigned char>(text[i]) == uniform ? run + 1 : 0;
			if (run > best.length) best = uniform_first ? Match{0, i + 1 - run, run} : Match{i + 1 - run, 0, run};
			if (run == limit) break;
		}
		if (swapped) std::swap(best.first, best.second);
		return best;
	}
	unsigned bits = std::bit_width(alphabet - 1), length = 1;
	std::uint64_t domain = alphabet;
	while (domain < 8 * std::uint64_t(n) && (length + 1) * bits <= 32) ++length, domain *= alphabet;
	unsigned key_bits = length * bits;
	std::uint32_t mask = key_bits == 32 ? ~std::uint32_t(0) : (std::uint32_t(1) << key_bits) - 1;
	auto key_at = [&](const char *p) {
		std::uint32_t key = 0;
		for (unsigned k = 0; k < length; ++k) key = (key << bits) | codes[static_cast<unsigned char>(p[k])];
		return key;
	};
	int positions = n - static_cast<int>(length) + 1;
	std::array<std::uint32_t, 128> sample;
	for (std::size_t i = 0; i < sample.size(); ++i)
		sample[i] = key_at(first.data() + i * (positions - 1) / (sample.size() - 1));
	std::sort(sample.begin(), sample.end());
	int duplicates = 0;
	for (std::size_t i = 1; i < sample.size(); ++i) duplicates += sample[i] == sample[i - 1];
	if (duplicates > 16) return std::nullopt;
	std::size_t capacity = std::bit_ceil(2 * std::size_t(positions)), table_mask = capacity - 1;
	unsigned shift = 32 - std::countr_zero(capacity);
	std::vector<std::uint64_t> table(capacity);
	std::vector<std::uint32_t> next(positions);
	std::uint64_t probes = 0, probe_budget = 8 * (std::uint64_t(n) + m);
	auto find = [&](std::uint32_t key) -> std::size_t {
		std::size_t slot = (key * std::uint32_t(0x9e3779b9)) >> shift;
		unsigned chain = 0;
		while (table[slot] && static_cast<std::uint32_t>(table[slot] >> 32) != key) {
			if (++probes > probe_budget || ++chain > 64) return capacity;
			slot = (slot + 1) & table_mask;
		}
		return slot;
	};
	std::uint32_t key = key_at(first.data());
	for (int i = 0; i < positions; ++i) {
		std::size_t slot = find(key);
		if (slot == capacity) return std::nullopt;
		next[i] = static_cast<std::uint32_t>(table[slot]);
		table[slot] = (std::uint64_t(key) << 32) | unsigned(i + 1);
		if (i + 1 < positions) key = ((key << bits) | codes[static_cast<unsigned char>(first[i + length])]) & mask;
	}
	std::uint64_t work = 0, budget = 4 * (std::uint64_t(n) + m);
	for (int j = 0; j + length <= unsigned(m);) {
		std::size_t slot = find(key_at(second.data() + j));
		if (slot == capacity) return std::nullopt;
		auto position = static_cast<std::uint32_t>(table[slot]);
		while (position) {
			int i = static_cast<int>(position) - 1;
			if (++work > budget) return std::nullopt;
			int left_limit = std::min(i, j), right_limit = std::min(n - i, m - j);
			if (left_limit + right_limit > best.length) {
				int right = length + sequence_compare_detail::prefix(first.data() + i + length, second.data() + j + length, right_limit - length);
				int left = sequence_compare_detail::suffix(first.data() + i, second.data() + j, left_limit);
				work += right + left;
				if (right + left > best.length) best = {i - left, j - left, right + left};
				if (best.length == n) {
					if (swapped) std::swap(best.first, best.second);
					return best;
				}
				if (work > budget) return std::nullopt;
			}
			position = next[i];
		}
		j += best.length >= int(length) ? best.length - length + 2 : 1;
	}
	if (best.length + 1 < int(length)) return std::nullopt;
	if (swapped) std::swap(best.first, best.second);
	return best;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif


#line 12 "string/longest_common_substring.hpp"

struct LongestCommonSubstringResult {
	int s_pos = 0, t_pos = 0, length = 0;
};

inline LongestCommonSubstringResult longest_common_substring(std::string_view s, std::string_view t) {
	[[maybe_unused]] constexpr std::size_t maximum = std::numeric_limits<int>::max() - 1;
	assert(s.size() <= maximum && t.size() <= maximum - s.size());
	if (s.empty() || t.empty()) return {};
	int n = int(s.size()), m = int(t.size()), bound = std::min(n, m);
	LongestCommonSubstringResult best{0, 0, sequence_compare_detail::prefix(s.data(), t.data(), bound)};
	if (best.length == bound) return best;
	if (auto match = substring_seed_detail::search(s, t, {best.s_pos, best.t_pos, best.length}))
		return {match->first, match->second, match->length};
	struct Joined {
		std::string_view s, t;
		int split;
		std::size_t size() const { return s.size() + t.size() + 1; }
		int operator[](int p) const {
			if (p < split) return static_cast<unsigned char>(s[p]) + 1;
			return p == split ? 0 : static_cast<unsigned char>(t[p - split - 1]) + 1;
		}
	};
	auto sa = suffix_array_detail::induced_sort(Joined{s, t, n}, 256);
	int size = int(sa.size());
	std::vector<int> rank(size);
	for (int i = 0; i < size; ++i) rank[sa[i]] = i;
	for (int p = 0, h = 0; p < size; ++p) {
		if (h) --h;
		if (!rank[p]) { h = 0; continue; }
		int q = sa[rank[p] - 1];
		if (q == n) { h = 0; continue; }
		bool first = p < n, other = q < n;
		int a = first ? p : p - n - 1, b = other ? q : q - n - 1;
		auto left = first ? s : t, right = other ? s : t;
		int limit = std::min(int(left.size()) - a, int(right.size()) - b);
		h += sequence_compare_detail::prefix(left.data() + a + h, right.data() + b + h, limit - h);
		if (first != other && h > best.length) {
			best = first ? LongestCommonSubstringResult{a, b, h} : LongestCommonSubstringResult{b, a, h};
			if (h == bound) return best;
		}
	}
	return best;
}


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/longest_common_substring.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s, t;
	input.read(s);
	input.read(t);
	auto [a, b, length] = longest_common_substring(s, t);
	output.write(a);
	output.write(' ');
	output.write(a + length);
	output.write(' ');
	output.write(b);
	output.write(' ');
	output.println(b + length);
}
