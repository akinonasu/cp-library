#ifndef STATIC_RANGE_MIN_HPP
#define STATIC_RANGE_MIN_HPP

#include "default_init_allocator.hpp"
#include "../data_structure/range_query/sparse_table.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

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

#endif
