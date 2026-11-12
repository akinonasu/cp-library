#line 1 "test/range_lis_queries.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/static_range_lis_query"
#line 1 "data_structure/range_lis_query.hpp"



#line 1 "components/quad_bit_vector.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

struct QuadBitVector {
	struct Block {
		std::uint64_t low = 0, high = 0;
		std::array<int, 4> prefix{};
	};
	int n;
	std::vector<Block> blocks;

#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void pack_avx2(const std::uint32_t *a, int n, int shift, Block *blocks) {
		auto low_shift = _mm_cvtsi32_si128(31 - shift), high_shift = _mm_cvtsi32_si128(30 - shift);
		for (int w = 0; w < (n >> 6) + bool(n & 63); ++w) {
			int start = w * 64;
			std::uint64_t low = 0, high = 0;
			int length = std::min(64, n - start), j = 0;
			for (; j + 8 <= length; j += 8) {
				auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
				low |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, low_shift)))) << j;
				high |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, high_shift)))) << j;
			}
			for (; j < length; ++j) {
				low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
				high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
			}
			blocks[w].low = low;
			blocks[w].high = high;
		}
		blocks[n >> 6].low = n & 63 ? blocks[n >> 6].low : 0;
		blocks[n >> 6].high = n & 63 ? blocks[n >> 6].high : 0;
	}
#endif
	explicit QuadBitVector(int n = 0) : n(n), blocks((n >> 6) + 1) {}
	void build(const std::vector<std::uint32_t> &a, int shift) {
		assert(a.size() == std::size_t(n) && 0 <= shift && shift <= 30);
		[[maybe_unused]] bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
		vectorized = n >= 64 && __builtin_cpu_supports("avx2");
		if (vectorized) pack_avx2(a.data(), n, shift, blocks.data());
#endif
		std::array<int, 4> counts{};
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint64_t low = 0, high = 0;
			int start = w * 64, length = std::min(64, n - start);
			if (vectorized) low = blocks[w].low, high = blocks[w].high;
			else
				for (int j = 0; j < length; ++j) {
					low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
					high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
				}
			blocks[w] = {low, high, counts};
			int c1 = std::popcount(low & ~high), c2 = std::popcount(~low & high), c3 = std::popcount(low & high);
			counts[0] += length - c1 - c2 - c3;
			counts[1] += c1;
			counts[2] += c2;
			counts[3] += c3;
		}
	}
	std::uint64_t mask(const Block &b, int digit) const {
		return ((digit & 1) ? b.low : ~b.low) & ((digit & 2) ? b.high : ~b.high);
	}

	int rank(int r, int digit) const {
		const auto &b = blocks[r >> 6];
		return b.prefix[digit] + std::popcount(mask(b, digit) & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	int below(int r, int digit) const {
		if (!digit) return 0;
		if (digit == 1) return rank(r, 0);
		if (digit == 3) return r - rank(r, 3);
		const auto &b = blocks[r >> 6];
		return b.prefix[0] + b.prefix[1] + std::popcount(~b.high & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	std::array<int, 4> ranks(int r) const {
		const auto &b = blocks[r >> 6];
		std::uint64_t m = (std::uint64_t(1) << (r & 63)) - 1;
		std::array<int, 4> a;
		a[1] = b.prefix[1] + std::popcount(b.low & ~b.high & m);
		a[2] = b.prefix[2] + std::popcount(~b.low & b.high & m);
		a[3] = b.prefix[3] + std::popcount(b.low & b.high & m);
		a[0] = r - a[1] - a[2] - a[3];
		return a;
	}
};


#line 1 "components/range_lis.hpp"



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


#line 7 "components/range_lis.hpp"
#include <numeric>
#include <limits>
#include <stdexcept>
#include <span>
#include <utility>
#line 13 "components/range_lis.hpp"

namespace range_lis_detail {
inline void multiply(int n, const int *a, const int *b, int *result, int *work) {
	if (n == 1) {
		result[0] = 0;
		return;
	}
	int *inverse = work;
	work += n;
	int middle = n / 2;
	for (int side = 0; side < 2; ++side) {
		int length = side ? n - middle : middle;
		int *left = work, *rows = left + length, *right = rows + length;
		int *columns = right + length, *child = columns + length;
		int x = 0, y = 0;
		for (int i = 0; i < n; ++i) {
			if ((a[i] >= middle) == bool(side)) {
				left[x] = a[i] - (side ? middle : 0);
				rows[x++] = i;
			}
			if ((b[i] >= middle) == bool(side)) {
				right[y] = b[i] - (side ? middle : 0);
				columns[y++] = i;
			}
		}
		multiply(length, left, right, child, child + length);
		for (int i = 0; i < length; ++i) {
			int column = columns[child[i]];
			result[rows[i]] = column;
			inverse[column] = rows[i];
		}
	}
	int row = n, low = 0, high = 0, low_delta = 0, high_delta = 0;
	auto step = [&](int column) {
		return b[column] < middle ? inverse[column] >= row : inverse[column] < row;
	};
	while (row) {
		while (high < n && high_delta + step(high) == 0) {
			high_delta = 0;
			++high;
		}
		--row;
		if (a[row] < middle) {
			low_delta -= result[row] >= low;
			high_delta -= result[row] >= high;
		} else {
			low_delta -= result[row] < low;
			high_delta -= result[row] < high;
		}
		while (low_delta) low_delta += step(low++);
		if (low > high) result[row] = high;
	}
}

inline std::size_t product_space(int n) {
	std::size_t space = 0;
	for (int length = n; length > 1;) {
		int half = length / 2 + length % 2;
		space += std::size_t(length) + 5 * std::size_t(half);
		length = half;
	}
	return space;
}

inline void compose(int n, const int *a, const int *b, int *result, int *work) {
	int *left = work, *right = left + n, *inverse = right + n;
	int *rows = inverse + n, *columns = rows + n, *product = columns + n;
	std::fill_n(inverse, n, -1);
	int row_count = 0, column_count = 0;
	for (int i = 0; i < n; ++i) if (a[i] >= 0) {
		rows[row_count++] = i;
		inverse[a[i]] = i;
	}
	int cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] < 0) left[cursor++] = i;
	for (int i = 0; i < row_count; ++i) left[cursor++] = a[rows[i]];
	std::fill_n(inverse, n, -1);
	for (int i = 0; i < n; ++i) if (b[i] >= 0) inverse[b[i]] = i;
	cursor = 0;
	for (int i = 0; i < n; ++i) if (inverse[i] >= 0) {
		right[cursor++] = inverse[i];
		columns[column_count++] = i;
	}
	for (int i = 0; i < n; ++i) if (b[i] < 0) right[cursor++] = i;
	multiply(n, left, right, product, product + n);
	std::fill_n(result, n, -1);
	for (int i = 0; i < row_count; ++i) {
		int column = product[n - row_count + i];
		if (column < column_count) result[rows[i]] = columns[column];
	}
}

inline std::vector<int> compose(const std::vector<int> &a, const std::vector<int> &b) {
	int n = int(a.size());
	if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / 20) throw std::length_error("Range LIS workspace");
	std::vector<int> result(n);
	std::vector<int, DefaultInitAllocator<int>> work(6 * std::size_t(n) + product_space(n));
	if (n) compose(n, a.data(), b.data(), result.data(), work.data());
	return result;
}

inline std::size_t seaweed_space(int n) {
	if (n <= 32) return n;
	return std::max(2 * std::size_t(n) + seaweed_space(n - n / 2), 9 * std::size_t(n) + product_space(n));
}

inline void seaweed(int n, const int *permutation, int *result, int *work) {
	if (n <= 32) {
		std::fill_n(work, n, -1);
		std::fill_n(result, n, -1);
		for (int i = 0; i < n; ++i) {
			int value = permutation[i], carry = -1;
			work[value] = i;
			for (int j = value + 1; j < n; ++j)
				if (work[j] > carry) std::swap(work[j], carry);
			if (carry >= 0) result[carry] = i;
		}
		return;
	}
	int middle = n / 2;
	int *split = work, *positions = split + n, *child = positions + n;
	int low = 0, high = middle;
	for (int i = 0; i < n; ++i) {
		int value = permutation[i];
		int &cursor = value < middle ? low : high;
		split[cursor] = value < middle ? value : value - middle;
		positions[cursor++] = i;
	}
	seaweed(middle, split, result, child);
	seaweed(n - middle, split + middle, result + middle, child);
	int *a = split, *b = child;
	std::iota(a, a + n, 0);
	std::iota(b, b + n, 0);
	for (int i = 0; i < middle; ++i) a[positions[i]] = result[i] < 0 ? -1 : positions[result[i]];
	for (int i = middle; i < n; ++i) b[positions[i]] = result[i] < 0 ? -1 : positions[middle + result[i]];
	compose(n, a, b, result, b + n);
}

inline std::vector<int> seaweed(std::span<const int> permutation) {
	int n = int(permutation.size());
	if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / 20) throw std::length_error("Range LIS workspace");
	std::vector<int> result(n);
	if (!n) return result;
	if (std::is_sorted(permutation.begin(), permutation.end())) {
		std::fill(result.begin(), result.end(), -1);
		return result;
	}
	if (std::is_sorted(permutation.rbegin(), permutation.rend())) {
		std::iota(result.begin(), result.end(), 1);
		result.back() = -1;
		return result;
	}
	std::vector<int, DefaultInitAllocator<int>> work(seaweed_space(n));
	seaweed(n, permutation.data(), result.data(), work.data());
	return result;
}
}


#line 1 "components/range_lis_batch.hpp"



#line 6 "components/range_lis_batch.hpp"
#include <functional>
#line 10 "components/range_lis_batch.hpp"

namespace range_lis_detail {
inline void replace_max(std::uint32_t *heap, int size, std::uint32_t value) {
	int at = 0;
	while (at * 2 + 1 < size) {
		int child = at * 2 + 1;
		if (child + 1 < size && heap[child] < heap[child + 1]) ++child;
		if (value >= heap[child]) break;
		heap[at] = heap[child];
		at = child;
	}
	heap[at] = value;
}
inline void replace_min(std::uint32_t *heap, int size, std::uint32_t value) {
	int at = 0;
	while (at * 2 + 1 < size) {
		int child = at * 2 + 1;
		if (child + 1 < size && heap[child] > heap[child + 1]) ++child;
		if (value <= heap[child]) break;
		heap[at] = heap[child];
		at = child;
	}
	heap[at] = value;
}
inline void batch_sqrt(std::span<const int> permutation, std::span<const std::pair<int, int>> intervals, std::vector<int> &answers) {
	int n = int(permutation.size());
	std::vector<int> offset(std::size_t(n) + 2);
	for (auto [l, r] : intervals) if (r > l) ++offset[std::size_t(r) + 1];
	for (std::size_t i = 1; i < offset.size(); ++i) offset[i] += offset[i - 1];
	std::vector<std::pair<int, int>> ordered(offset.back());
	auto cursor = offset;
	for (int i = 0; i < int(intervals.size()); ++i) {
		auto [l, r] = intervals[i];
		if (r > l) ordered[cursor[r]++] = {l, i};
	}
	int block = 1;
	while (block < n / block) ++block;
	int count = n ? (n - 1) / block + 1 : 0;
	std::vector<int> sizes(count), fenwick(std::size_t(n) + 1);
	std::vector<std::uint32_t> values(n), heap(n), pending(block);
	std::vector<bool> dirty(count);
	auto update = [&](unsigned at, int delta) {
		for (; at; at &= at - 1) fenwick[at] += delta;
	};
	for (int i = 0; i < n; ++i) {
		int x = permutation[i], part = x / block;
		int left = part * block, right = left + std::min(block, n - left);
		auto *local = heap.data() + left;
		if (dirty[part]) {
			int used = 0;
			for (int j = 0; j < sizes[part]; ++j) if (local[j] & 1) pending[used++] = local[j] >> 1;
			std::make_heap(pending.begin(), pending.begin() + used, std::greater<>{});
			for (int j = left; j < right; ++j) if (used && values[j] > pending[0]) {
				auto value = values[j];
				values[j] = pending[0];
				replace_min(pending.data(), used, value);
			}
			dirty[part] = false;
		}
		values[x] = unsigned(i) + 1;
		update(unsigned(i) + 1, 1);
		std::uint32_t carry = 0;
		for (int j = x + 1; j < right; ++j) if (values[j] > carry) std::swap(values[j], carry);
		for (int j = part + 1; j < count; ++j) {
			auto *other = heap.data() + j * block;
			if (sizes[j] && carry < (other[0] >> 1)) {
				dirty[j] = true;
				auto next = other[0] >> 1;
				replace_max(other, sizes[j], (carry << 1) | 1);
				carry = next;
			}
		}
		update(carry, -1);
		int used = 0;
		for (int j = left; j < right; ++j) if (values[j]) local[used++] = values[j] << 1;
		sizes[part] = used;
		std::make_heap(local, local + used);
		for (int j = offset[std::size_t(i) + 1]; j < offset[std::size_t(i) + 2]; ++j) {
			auto [l, id] = ordered[j];
			int answer = 0;
			for (unsigned at = unsigned(l) + 1; at <= unsigned(n); at += at & -at) answer += fenwick[at];
			answers[id] = answer;
		}
	}
}
}


#line 16 "data_structure/range_lis_query.hpp"
#include <ranges>
#line 18 "data_structure/range_lis_query.hpp"
#include <type_traits>
#line 21 "data_structure/range_lis_query.hpp"

namespace range_lis_query_detail {
template<bool Strict, class Range, class Compare>
std::vector<int> permutation(const Range &values, Compare compare) {
	int n = int(std::ranges::size(values));
	auto first = std::ranges::begin(values);
	std::vector<int> order(n, -1), result(n);
	if constexpr (std::is_integral_v<std::ranges::range_value_t<Range>>
		&& sizeof(std::ranges::range_value_t<Range>) <= sizeof(std::uintmax_t) && std::is_same_v<Compare, std::less<>>) {
		bool direct = true;
		for (int i = 0; i < n; ++i) {
			auto value = first[i];
			if constexpr (std::is_signed_v<std::ranges::range_value_t<Range>>) {
				if (value < 0) { direct = false; break; }
			}
			if (std::uintmax_t(value) >= std::uintmax_t(n)) { direct = false; break; }
			int rank = int(value);
			if (order[rank] != -1) { direct = false; break; }
			order[rank] = i;
			result[i] = rank;
		}
		if (direct) return result;
	}
	std::iota(order.begin(), order.end(), 0);
	std::sort(order.begin(), order.end(), [&](int a, int b) {
		if (compare(first[a], first[b])) return true;
		if (compare(first[b], first[a])) return false;
		if constexpr (Strict) return a > b;
		else return a < b;
	});
	for (int i = 0; i < n; ++i) result[order[i]] = i;
	return result;
}
}

template <bool Strict = true>
class RangeLISQuery {
	int n = 0;
	std::vector<QuadBitVector> bits;
	std::vector<std::array<int, 4>> offsets;
	[[gnu::always_inline]] int query_impl(int l, int r) const {
		int length = r - l, removed = 0;
		unsigned upper = unsigned(r);
		for (int level = 0; level < int(bits.size()) && l < r; ++level) {
			int digit = (upper >> (2 * (int(bits.size()) - 1 - level))) & 3;
			const auto &row = bits[level];
			removed += row.below(r, digit) - row.below(l, digit);
			l = offsets[level][digit] + row.rank(l, digit);
			r = offsets[level][digit] + row.rank(r, digit);
		}
		return length - removed;
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("popcnt"))) int query_popcnt(int l, int r) const { return query_impl(l, r); }
#endif

public:
	RangeLISQuery() = default;
	template <class Range, class Compare = std::less<>>
		requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
	explicit RangeLISQuery(const Range &values, Compare compare = {}) { build(values, std::move(compare)); }
	RangeLISQuery(const RangeLISQuery &) = default;
	RangeLISQuery &operator=(const RangeLISQuery &) = default;
	RangeLISQuery(RangeLISQuery &&other) noexcept { swap(other); }
	RangeLISQuery &operator=(RangeLISQuery &&other) noexcept {
		if (this != &other) {
			RangeLISQuery replacement(std::move(other));
			swap(replacement);
		}
		return *this;
	}
	void swap(RangeLISQuery &other) noexcept {
		std::swap(n, other.n);
		bits.swap(other.bits);
		offsets.swap(other.offsets);
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	void clear() { n = 0; bits.clear(); offsets.clear(); }
	template <class Range, class Compare = std::less<>>
		requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
	void build(const Range &values, Compare compare = {}) {
		assert(std::ranges::size(values) <= std::size_t(std::numeric_limits<int>::max()));
		n = int(std::ranges::size(values));
		bits.clear();
		offsets.clear();
		if (!n) return;
		auto permutation = range_lis_query_detail::permutation<Strict>(values, std::move(compare));
		auto endpoints = range_lis_detail::seaweed(permutation);
		std::vector<std::uint32_t> current(n), next(n);
		for (int i = 0; i < n; ++i) current[i] = endpoints[i] < 0 ? unsigned(n) : unsigned(endpoints[i]);
		int levels = (std::bit_width(unsigned(n)) + 1) / 2;
		bits.reserve(levels);
		offsets.resize(levels);
		for (int level = 0; level < levels; ++level) {
			int shift = 2 * (levels - 1 - level);
			auto &row = bits.emplace_back(n);
			row.build(current, shift);
			auto counts = row.ranks(n);
			for (int digit = 1; digit < 4; ++digit) offsets[level][digit] = offsets[level][digit - 1] + counts[digit - 1];
			if (level + 1 == levels) break;
			auto positions = offsets[level];
			for (auto value : current) next[positions[(value >> shift) & 3]++] = value;
			current.swap(next);
		}
	}
	int query(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if (r - l <= 1) return r - l;
#if defined(__x86_64__) || defined(__i386__)
		static const bool hardware = __builtin_cpu_supports("popcnt");
		if (hardware) return query_popcnt(l, r);
#endif
		return query_impl(l, r);
	}
};

template<bool Strict = true, class Range, class Compare = std::less<>>
	requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
std::vector<int> range_lis_queries(const Range &values, std::span<const std::pair<int, int>> intervals, Compare compare = {}) {
	assert(std::ranges::size(values) <= std::size_t(std::numeric_limits<int>::max()));
	assert(intervals.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = int(std::ranges::size(values));
	std::vector<int> answers(intervals.size());
	bool nontrivial = false;
	for (std::size_t i = 0; i < intervals.size(); ++i) {
		auto [l, r] = intervals[i];
		assert(0 <= l && l <= r && r <= n);
		answers[i] = r - l;
		nontrivial |= r - l > 1;
	}
	if (!nontrivial) return answers;
	auto permutation = range_lis_query_detail::permutation<Strict>(values, std::move(compare));
	auto endpoints = range_lis_detail::seaweed(permutation);
	std::vector<int> inverse(n, -1), offset(std::size_t(n) + 2);
	for (int i = 0; i < n; ++i) if (endpoints[i] >= 0) {
		assert(i < endpoints[i] && endpoints[i] < n && inverse[endpoints[i]] == -1);
		inverse[endpoints[i]] = i;
	}
	for (auto [l, r] : intervals) if (r - l > 1) ++offset[r + 1];
	for (std::size_t i = 1; i < offset.size(); ++i) offset[i] += offset[i - 1];
	std::vector<std::pair<int, int>> ordered(offset.back());
	auto cursor = offset;
	for (int i = 0; i < int(intervals.size()); ++i) {
		auto [l, r] = intervals[i];
		if (r - l > 1) ordered[cursor[r]++] = {l, i};
	}
	cursor.assign(std::size_t(n) + 1, 0);
	int active = 0;
	for (std::size_t r = 0; r + 1 < offset.size(); ++r) {
		if (r && inverse[r - 1] != -1) {
			++active;
			for (unsigned i = unsigned(inverse[r - 1]) + 1; i <= unsigned(n); i += i & -i) ++cursor[i];
		}
		for (int i = offset[r]; i < offset[r + 1]; ++i) {
			auto [l, id] = ordered[i];
			int before = 0;
			for (unsigned j = unsigned(l); j; j &= j - 1) before += cursor[j];
			answers[id] = int(r) - l - active + before;
		}
	}
	return answers;
}


template<bool Strict = true, class Range, class Compare = std::less<>>
	requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
std::vector<int> range_lis_queries_sqrt(const Range &values, std::span<const std::pair<int, int>> intervals, Compare compare = {}) {
	assert(std::ranges::size(values) <= std::size_t(std::numeric_limits<int>::max()));
	assert(intervals.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = int(std::ranges::size(values));
	std::vector<int> answers(intervals.size());
	bool nontrivial = false;
	for (std::size_t i = 0; i < intervals.size(); ++i) {
		auto [l, r] = intervals[i];
		assert(0 <= l && l <= r && r <= n);
		answers[i] = r - l;
		nontrivial |= r - l > 1;
	}
	if (!nontrivial) return answers;
	auto permutation = range_lis_query_detail::permutation<Strict>(values, std::move(compare));
	if (std::is_sorted(permutation.begin(), permutation.end())) return answers;
	if (std::is_sorted(permutation.rbegin(), permutation.rend())) {
		for (int &answer : answers) answer = answer != 0;
		return answers;
	}
	range_lis_detail::batch_sqrt(permutation, intervals, answers);
	return answers;
}


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#line 15 "components/fast_io.hpp"
#include <string>
#include <string_view>
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


#line 6 "test/range_lis_queries.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> values(n);
	for (auto &value : values) input.read(value);
	std::vector<std::pair<int, int>> intervals(q);
	for (auto &[l, r] : intervals) input.read(l), input.read(r);
	for (int answer : range_lis_queries_sqrt(values, intervals)) output.println(answer);
}
