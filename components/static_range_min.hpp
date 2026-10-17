#ifndef STATIC_RANGE_MIN_HPP
#define STATIC_RANGE_MIN_HPP

#include "default_init_allocator.hpp"
#include "../data_structure/sparse_table.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = int> class StaticRangeMin {
	static T minimum(T a, T b) { return b < a ? b : a; }
	std::vector<T> values_;
	std::vector<std::uint32_t, DefaultInitAllocator<std::uint32_t>> masks_;
	SparseTable<T, minimum> blocks_;
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
		blocks_.build(n ? (n - 1) / 32 + 1 : 0, [&](int i) {
			int last = i * 32 + std::min(32, n - i * 32);
			return within(i * 32, last);
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
		T value = minimum(within(l, (a + 1) * 32), within(b * 32, r));
		if (a + 1 < b) value = minimum(value, blocks_.prod_unchecked(a + 1, b));
		return value;
	}
	T all_prod() const { return prod(0, size()); }
};

#endif
