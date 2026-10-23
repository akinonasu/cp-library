#ifndef RANGE_LIS_QUERY_HPP
#define RANGE_LIS_QUERY_HPP

#include "../../components/quad_bit_vector.hpp"
#include "../../components/range_lis.hpp"
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
#include <utility>
#include <vector>

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
		auto first = std::ranges::begin(values);
		std::vector<int> order(n), permutation(n);
		std::iota(order.begin(), order.end(), 0);
		std::sort(order.begin(), order.end(), [&](int a, int b) {
			if (compare(first[a], first[b])) return true;
			if (compare(first[b], first[a])) return false;
			if constexpr (Strict) return a > b;
			else return a < b;
		});
		for (int i = 0; i < n; ++i) permutation[order[i]] = i;
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

#endif
