#ifndef RANGE_LIS_QUERY_HPP
#define RANGE_LIS_QUERY_HPP

#include "../components/quad_bit_vector.hpp"
#include "../components/range_lis.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

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


#endif
