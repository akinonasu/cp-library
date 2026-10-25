#line 1 "test/suffix_tree_substrings.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#line 1 "string/suffix_tree.hpp"



#line 1 "components/suffix_array.hpp"



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


#line 1 "components/sequence_compare.hpp"



#line 5 "components/sequence_compare.hpp"
#include <cstddef>
#line 7 "components/sequence_compare.hpp"
#include <cstring>
#line 9 "components/sequence_compare.hpp"
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


#line 1 "components/static_range_min.hpp"



#line 1 "components/default_init_allocator.hpp"



#include <memory>
#include <new>

template <class T>
struct DefaultInitAllocator : std::allocator<T> {
	template <class U>
	struct rebind {
		using other = DefaultInitAllocator<U>;
	};
	DefaultInitAllocator() = default;
	template <class U>
	DefaultInitAllocator(const DefaultInitAllocator<U> &) noexcept {}
	template <class U>
	void construct(U *p) { ::new (static_cast<void *>(p)) U; }
};


#line 1 "data_structure/sparse_table.hpp"



#line 12 "data_structure/sparse_table.hpp"
#include <utility>
#line 14 "data_structure/sparse_table.hpp"

template <class S, auto op, auto e = nullptr>
class SparseTable {
	using Value = std::conditional_t<std::is_same_v<S, bool>, std::uint8_t, S>;
	using Reference = std::conditional_t<std::is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	std::vector<std::vector<Value, DefaultInitAllocator<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (std::is_same_v<S, bool> || !std::is_invocable_v<decltype(op), const S &, const S &>) return S(op(S(a), S(b)));
		else return op(a, b);
	}

  public:
	SparseTable() = default;
	SparseTable(const SparseTable &) = default;
	SparseTable &operator=(const SparseTable &) = default;
	SparseTable(SparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	SparseTable &operator=(SparseTable &&other) noexcept {
		if (this != &other) {
			n = std::exchange(other.n, 0);
			data = std::move(other.data);
		}
		return *this;
	}
	explicit SparseTable(const std::vector<S> &a) { build(a); }
	void clear() {
		data.clear();
		n = 0;
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		build(static_cast<int>(a.size()), [&](int i) -> S { return a[i]; });
	}
	template <class F>
	void build(int size, F value) {
		assert(size >= 0);
		n = size;
		data.resize(std::bit_width(unsigned(n)));
		if (!n) return;
		if constexpr (std::is_default_constructible_v<Value>) {
			data[0].resize(n);
			for (int i = 0; i < n; ++i) data[0][i] = value(i);
		} else {
			data[0].clear();
			data[0].reserve(n);
			for (int i = 0; i < n; ++i) data[0].push_back(value(i));
		}
		for (int k = 1; k < std::bit_width(unsigned(n)); ++k) {
			int length = n - (1U << k) + 1, half = 1U << (k - 1);
			if constexpr (std::is_default_constructible_v<Value>) {
				data[k].resize(length);
				for (int i = 0; i < length; ++i) data[k][i] = combine(data[k - 1][i], data[k - 1][i + half]);
			} else {
				data[k].clear();
				data[k].reserve(length);
				for (int i = 0; i < length; ++i) data[k].push_back(combine(data[k - 1][i], data[k - 1][i + half]));
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	Reference operator[](int i) const {
		assert(0 <= i && i < n);
		return data[0][i];
	}
	[[gnu::always_inline]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if constexpr (std::is_same_v<decltype(e), std::nullptr_t>) assert(l < r);
		else if (l == r) return e();
		return prod_unchecked(l, r);
	}
	[[gnu::always_inline]] S prod_unchecked(int l, int r) const {
		int k = std::bit_width(unsigned(r - l)) - 1;
		return combine(data[k][l], data[k][r - (1U << k)]);
	}
	S all_prod() const { return prod(0, n); }
};


#line 12 "components/static_range_min.hpp"
#include <span>
#line 16 "components/static_range_min.hpp"

template <class T = int, bool FastBoundaries = false> class StaticRangeMin {
	static T minimum(T a, T b) { return b < a ? b : a; }
	std::vector<T> values_;
	std::vector<std::uint32_t, DefaultInitAllocator<std::uint32_t>> masks_;
	SparseTable<T, minimum> blocks_;
	struct Boundaries { std::vector<T> prefix, suffix; };
	struct Empty {};
	[[no_unique_address]] std::conditional_t<FastBoundaries, Boundaries, Empty> boundaries_;
	T within(int l, int r) const {
		auto mask = masks_[r - 1] & (~std::uint32_t(0) << (l & 31));
		return values_[(l & ~31) + std::countr_zero(mask)];
	}

  public:
	StaticRangeMin() = default;
	explicit StaticRangeMin(std::vector<T> values) { build(std::move(values)); }
	void build(std::vector<T> values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		values_ = std::move(values);
		int n = size();
		masks_.resize(n);
		for (int first = 0; first < n;) {
			int last = first + std::min(32, n - first);
			std::uint32_t mask = 0;
			for (int i = first; i < last; ++i) {
				while (mask && !(values_[first + std::bit_width(mask) - 1] < values_[i]))
					mask &= ~(std::uint32_t(1) << (std::bit_width(mask) - 1));
				mask |= std::uint32_t(1) << (i - first);
				masks_[i] = mask;
			}
			first = last;
		}
		if constexpr (FastBoundaries) {
			boundaries_.prefix = values_, boundaries_.suffix = values_;
			for (int first = 0; first < n;) {
				int last = first + std::min(32, n - first);
				for (int i = first + 1; i < last; ++i) boundaries_.prefix[i] = minimum(boundaries_.prefix[i - 1], values_[i]);
				for (int i = last - 1; i > first; --i) boundaries_.suffix[i - 1] = minimum(values_[i - 1], boundaries_.suffix[i]);
				first = last;
			}
		}
		blocks_.build(n ? (n - 1) / 32 + 1 : 0, [&](int i) {
			int last = i * 32 + std::min(32, n - i * 32);
			if constexpr (FastBoundaries) return T(boundaries_.prefix[last - 1]);
			else return within(i * 32, last);
		});
	}
	int size() const { return static_cast<int>(values_.size()); }
	bool empty() const { return values_.empty(); }
	std::conditional_t<std::is_same_v<T, bool>, bool, const T &> operator[](int i) const {
		assert(0 <= i && i < size());
		return values_[i];
	}
	T prod(int l, int r) const {
		assert(0 <= l && l < r && r <= size());
		int a = l / 32, b = (r - 1) / 32;
		if (a == b) return within(l, r);
		T value = [&] {
			if constexpr (FastBoundaries) return minimum(boundaries_.suffix[l], boundaries_.prefix[r - 1]);
			else return minimum(within(l, (a + 1) * 32), within(b * 32, r));
		}();
		if (a + 1 < b) value = minimum(value, blocks_.prod_unchecked(a + 1, b));
		return value;
	}
	std::vector<T> prod_many(std::span<const std::pair<int, int>> ranges) const {
		std::vector<T> result;
		result.reserve(ranges.size());
		if constexpr (FastBoundaries) {
			for (auto [l, r] : ranges) result.emplace_back(prod(l, r));
		} else {
			std::array<int, 32> left, right;
			for (std::size_t first = 0; first < ranges.size(); first += 32) {
				int count = int(std::min<std::size_t>(32, ranges.size() - first));
				for (int i = 0; i < count; ++i) {
					auto [l, r] = ranges[first + i];
					assert(0 <= l && l < r && r <= size());
					int a = l & ~31, b = (r - 1) & ~31;
					auto mask = masks_[a == b ? r - 1 : a + 31] & (~std::uint32_t(0) << (l & 31));
					left[i] = a + std::countr_zero(mask);
					right[i] = a == b ? left[i] : b + std::countr_zero(masks_[r - 1]);
#if defined(__GNUC__) || defined(__clang__)
					if constexpr (!std::is_same_v<T, bool>) {
						__builtin_prefetch(values_.data() + left[i]);
						__builtin_prefetch(values_.data() + right[i]);
					}
#endif
				}
				for (int i = 0; i < count; ++i) {
					auto [l, r] = ranges[first + i];
					T value = minimum(values_[left[i]], values_[right[i]]);
					int a = l / 32 + 1, b = (r - 1) / 32;
					if (a < b) value = minimum(value, blocks_.prod_unchecked(a, b));
					result.emplace_back(std::move(value));
				}
			}
		}
		return result;
	}

	T all_prod() const { return prod(0, size()); }
};


#line 9 "string/suffix_tree.hpp"
#include <optional>
#line 11 "string/suffix_tree.hpp"
#include <string>
#line 13 "string/suffix_tree.hpp"

class SuffixTree {
	struct Node { int parent, depth, left, right; };
	std::string text_;
	mutable std::vector<Node> nodes_;
	std::vector<int> sa_;
	mutable std::vector<int> lcp_, rank_;
	mutable std::vector<int> terminals_, branches_, offset_, children_;
	mutable std::vector<unsigned char> symbols_;
	mutable std::array<int, 256> root_children_{};
	mutable bool lcp_ready_ = false, tree_ready_ = false;
	mutable std::uint64_t distinct_ = 0;
	mutable std::optional<StaticRangeMin<std::pair<int, int>>> index_;
	void check([[maybe_unused]] int v) const { prepare_tree(); assert(0 <= v && v < static_cast<int>(nodes_.size())); }
	int range_end(int v) const { int r = nodes_[v].right; return r < 0 ? nodes_[-r - 1].right : r; }
	void prepare_lcp() const {
		if (lcp_ready_) return;
		int n = size();
		rank_.resize(n + 1);
		for (int i = 0; i <= n; ++i) rank_[sa_[i]] = i;
		lcp_.assign(n, 0);
		for (int p = 0, h = 0; p < n; ++p) {
			if (h) --h;
			int i = rank_[p], q = sa_[i - 1];
			h += sequence_compare_detail::prefix(text_.data() + p + h, text_.data() + q + h, std::min(n - p, n - q) - h);
			lcp_[i - 1] = h;
		}
		distinct_ = std::uint64_t(n) * (n + 1) / 2;
		for (int value : lcp_) distinct_ -= value;
		lcp_ready_ = true;
	}
	void prepare_tree() const {
		if (tree_ready_) return;
		prepare_lcp();
		int n = size();
		nodes_.clear();
		nodes_.reserve(std::size_t(n) + 1);
		nodes_.push_back({-1, 0, 0, n + 1});
		terminals_.assign(n + 1, 0);
		branches_.resize(n);
		std::vector<int> stack{0};
		for (int i = 1; i <= n; ++i) {
			int common = lcp_[i - 1], last = -1;
			while (nodes_[stack.back()].depth > common) { last = stack.back(); nodes_[last].right = i; stack.pop_back(); }
			if (nodes_[stack.back()].depth < common) {
				int v = static_cast<int>(nodes_.size());
				nodes_.push_back({stack.back(), common, nodes_[last].left, n + 1});
				nodes_[last].parent = v;
				stack.push_back(v);
			}
			branches_[i - 1] = stack.back();
			int v = static_cast<int>(nodes_.size()), p = sa_[i];
			nodes_.push_back({stack.back(), n - p, i, n + 1});
			terminals_[p] = v;
			stack.push_back(v);
		}
		int count = static_cast<int>(nodes_.size());
		offset_.assign(nodes_.size() + 1, 0);
		for (int v = 1; v < count; ++v) ++offset_[nodes_[v].parent + 1];
		for (int v = 0; v < count; ++v) offset_[v + 1] += offset_[v];
		children_.resize(nodes_.size() - 1);
		symbols_.resize(children_.size());
		auto cursor = offset_;
		for (int v = 1; v < count; ++v) {
			int p = nodes_[v].parent, i = cursor[p]++;
			children_[i] = v;
			symbols_[i] = static_cast<unsigned char>(text_[sa_[nodes_[v].left] + nodes_[p].depth]);
		}
		root_children_.fill(-1);
		for (int i = offset_[0]; i < offset_[1]; ++i) {
			int v = children_[i];
			root_children_[static_cast<unsigned char>(text_[sa_[nodes_[v].left]])] = v;
		}
		for (int i = n; i >= 0; --i) {
			int v = terminals_[sa_[i]];
			if (offset_[v + 1] - offset_[v] == 1) {
				int u = children_[offset_[v]];
				nodes_[v].right = nodes_[u].right < 0 ? nodes_[u].right : -u - 1;
			}
		}
		tree_ready_ = true;
	}
	void prepare_index() const {
		prepare_tree();
		if (index_) return;
		std::vector<std::pair<int, int>> values(size());
		for (int i = 0; i < size(); ++i) values[i] = {lcp_[i], branches_[i]};
		index_.emplace(std::move(values));
	}
	int child(int v, unsigned char c) const {
		if (!v) return root_children_[c];
		int l = offset_[v], r = offset_[v + 1];
		if (r - l <= 4) {
			for (int i = l; i < r; ++i) if (symbols_[i] == c) return children_[i];
			return -1;
		}
		int end = r;
		while (l < r) { int m = l + (r - l) / 2; if (symbols_[m] < c) l = m + 1; else r = m; }
		return l < end && symbols_[l] == c ? children_[l] : -1;
	}

  public:
	struct Locus { int node, length; };
	static constexpr int root = 0;
	SuffixTree() { build({}); }
	explicit SuffixTree(std::string_view s) { build(s); }
	SuffixTree(const SuffixTree &) = default;
	SuffixTree &operator=(const SuffixTree &) = default;
	SuffixTree(SuffixTree &&other) : SuffixTree() { swap(other); }
	SuffixTree &operator=(SuffixTree &&other) {
		if (this != &other) { SuffixTree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(SuffixTree &other) noexcept {
		text_.swap(other.text_);
		nodes_.swap(other.nodes_);
		sa_.swap(other.sa_); lcp_.swap(other.lcp_); rank_.swap(other.rank_);
		terminals_.swap(other.terminals_); branches_.swap(other.branches_);
		offset_.swap(other.offset_); children_.swap(other.children_);
		symbols_.swap(other.symbols_);
		root_children_.swap(other.root_children_);
		std::swap(distinct_, other.distinct_);
		std::swap(lcp_ready_, other.lcp_ready_);
		std::swap(tree_ready_, other.tree_ready_);
		index_.swap(other.index_);
	}
	void build(std::string_view s) {
		assert(s.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		std::string copy(s);
		text_.swap(copy);
		int n = size();
		sa_ = suffix_array_detail::bytes(text_);
		sa_.insert(sa_.begin(), n);
		rank_.clear(); lcp_.clear();
		lcp_ready_ = false;
		nodes_.clear(); terminals_.clear(); branches_.clear(); offset_.clear(); children_.clear(); symbols_.clear();
		tree_ready_ = false;
		index_.reset();
	}
	void clear() { build({}); }
	int size() const { return static_cast<int>(text_.size()); }
	bool empty() const { return text_.empty(); }
	int node_count() const { prepare_tree(); return static_cast<int>(nodes_.size()); }
	std::string_view text() const { return text_; }
	int parent(int v) const { check(v); return nodes_[v].parent; }
	int depth(int v) const { check(v); return nodes_[v].depth; }
	std::pair<int, int> edge_range(int v) const {
		check(v);
		if (!v) return {0, 0};
		int p = sa_[nodes_[v].left];
		return {p + nodes_[nodes_[v].parent].depth, p + nodes_[v].depth};
	}
	std::pair<int, int> label_range(int v) const { check(v); int p = sa_[nodes_[v].left]; return {p, p + nodes_[v].depth}; }
	std::span<const int> children(int v) const {
		check(v);
		int count = offset_[v + 1] - offset_[v];
		return count ? std::span<const int>(children_.data() + offset_[v], count) : std::span<const int>{};
	}
	int transition(int v, char c) const { check(v); return child(v, static_cast<unsigned char>(c)); }
	int suffix_start(int v) const { check(v); int p = sa_[nodes_[v].left]; return size() - p == nodes_[v].depth ? p : -1; }
	int suffix_node(int p) const { prepare_tree(); assert(0 <= p && p <= size()); return terminals_[p]; }
	std::pair<int, int> occurrence_range(int v) const { check(v); return {nodes_[v].left, range_end(v)}; }
	int occurrence_count(int v) const { check(v); return range_end(v) - nodes_[v].left; }
	const std::vector<int> &suffix_array() const { return sa_; }
	const std::vector<int> &lcp_array() const { prepare_lcp(); return lcp_; }
	std::uint64_t distinct_substrings() const { prepare_lcp(); return distinct_; }
	std::optional<Locus> find(std::string_view pattern) const {
		prepare_tree();
		if (pattern.size() > text_.size()) return std::nullopt;
		int v = 0, matched = 0, length = static_cast<int>(pattern.size());
		while (matched < length) {
			int u = child(v, static_cast<unsigned char>(pattern[matched]));
			if (u == -1) return std::nullopt;
			if (nodes_[u].right >= 0) {
				int start = sa_[nodes_[u].left] + matched;
				int take = std::min(length - matched, nodes_[u].depth - matched);
				if (sequence_compare_detail::prefix(text_.data() + start, pattern.data() + matched, take) != take) return std::nullopt;
				matched += take;
				v = u;
				continue;
			}
			int end = -nodes_[u].right - 1;
			int start = sa_[nodes_[end].left] + nodes_[v].depth;
			int take = std::min(length - matched, nodes_[end].depth - nodes_[v].depth);
			if (sequence_compare_detail::prefix(text_.data() + start, pattern.data() + matched, take) != take) return std::nullopt;
			matched += take;
			if (matched < nodes_[end].depth && nodes_[u].depth < matched) {
				int l = nodes_[u].left + 1, r = std::min(nodes_[end].left, nodes_[u].left + matched - nodes_[u].depth);
				while (l < r) { int m = l + (r - l) / 2; if (size() - sa_[m] < matched) l = m + 1; else r = m; }
				v = l == nodes_[end].left ? end : terminals_[sa_[l]];
			} else v = matched <= nodes_[u].depth ? u : end;
		}
		return Locus{v, length};
	}
	std::optional<Locus> advance(Locus locus, char c) const {
		check(locus.node);
		int v = locus.node;
		assert((!v && locus.length == 0) || (v && nodes_[nodes_[v].parent].depth < locus.length && locus.length <= nodes_[v].depth));
		if (locus.length < nodes_[v].depth) {
			if (text_[sa_[nodes_[v].left] + locus.length] != c) return std::nullopt;
			return Locus{v, locus.length + 1};
		}
		int u = child(v, static_cast<unsigned char>(c));
		return u == -1 ? std::nullopt : std::optional<Locus>(Locus{u, locus.length + 1});
	}
	bool contains(std::string_view pattern) const { return find(pattern).has_value(); }
	int count(std::string_view pattern) const { auto locus = find(pattern); return locus ? occurrence_count(locus->node) : 0; }
	std::vector<int> occurrences(std::string_view pattern) const {
		auto locus = find(pattern);
		if (!locus) return {};
		auto [l, r] = occurrence_range(locus->node);
		return {sa_.begin() + l, sa_.begin() + r};
	}
	int lcp(int p, int q) const {
		assert(0 <= p && p <= size() && 0 <= q && q <= size());
		if (p == q) return size() - p;
		prepare_lcp();
		int a = rank_[p], b = rank_[q];
		if (a > b) std::swap(a, b);
		prepare_index();
		return index_->prod(a, b).first;
	}
	int lca(int u, int v) const {
		check(u); check(v);
		int end_u = range_end(u), end_v = range_end(v);
		if (nodes_[u].left <= nodes_[v].left && end_v <= end_u) return u;
		if (nodes_[v].left <= nodes_[u].left && end_u <= end_v) return v;
		int a = nodes_[u].left, b = nodes_[v].left;
		if (a > b) std::swap(a, b);
		prepare_index();
		return index_->prod(a, b).second;
	}
	int suffix_link(int v) const {
		check(v);
		if (!v) return 0;
		int p = suffix_start(v);
		if (p >= 0) return terminals_[p + 1];
		return lca(terminals_[sa_[nodes_[v].left] + 1], terminals_[sa_[range_end(v) - 1] + 1]);
	}
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 14 "components/fast_io.hpp"
#include <stdexcept>
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


#line 4 "test/suffix_tree_substrings.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	SuffixTree tree(s);
	out.println(tree.distinct_substrings());
}
