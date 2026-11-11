#ifndef STATIC_THRESHOLD_SEARCH_HPP
#define STATIC_THRESHOLD_SEARCH_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <std::integral T = std::int64_t, bool Smaller = false, bool Reverse = false>
requires (sizeof(T) <= 8 && !std::is_same_v<T, bool>)
class StaticThresholdSearch {
	std::vector<T> values_, minimum_, maximum_;
	unsigned base_ = 1;
	enum class Mode { ge, gt, le, lt };
	template <Mode M> static bool accepts(T a, T x) {
		if constexpr (M == Mode::ge) return a >= x;
		if constexpr (M == Mode::gt) return a > x;
		if constexpr (M == Mode::le) return a <= x;
		if constexpr (M == Mode::lt) return a < x;
	}
	template <Mode M> const std::vector<T> &extrema() const {
		if constexpr (M == Mode::ge || M == Mode::gt) return maximum_;
		else return minimum_;
	}
	template <Mode M, bool Forward> int search(int endpoint, T x) const {
		assert(0 <= endpoint && endpoint <= size());
		if constexpr (Forward) { if (endpoint == size()) return size(); }
		else if (!endpoint) return size();
		int p = Forward ? endpoint : endpoint - 1;
		if (accepts<M>(values_[p], x)) return p;
		const auto &tree = extrema<M>();
		if (!accepts<M>(tree[1], x)) return size();
		unsigned block = unsigned(p) / 64;
		if constexpr (Forward) {
			int end = int(std::min<std::size_t>(values_.size(), std::size_t(block + 1) * 64));
			for (++p; p < end; ++p) if (accepts<M>(values_[p], x)) return p;
			unsigned k = base_ + block + 1;
			if (k == base_ * 2) return size();
			do {
				while (!(k & 1)) k >>= 1;
				if (accepts<M>(tree[k], x)) {
					while (k < base_) { k *= 2; if (!accepts<M>(tree[k], x)) ++k; }
					std::size_t begin = std::size_t(k - base_) * 64;
					end = int(std::min(values_.size(), begin + 64));
					for (std::size_t i = begin; i < std::size_t(end); ++i) if (accepts<M>(values_[i], x)) return int(i);
					return size();
				}
				++k;
			} while ((k & -k) != k);
		} else {
			int begin = int(block * 64);
			for (--p; p >= begin; --p) if (accepts<M>(values_[p], x)) return p;
			unsigned k = base_ + block;
			if (k == base_) return size();
			do {
				--k;
				while (k > 1 && (k & 1)) k >>= 1;
				if (accepts<M>(tree[k], x)) {
					while (k < base_) { k = k * 2 + 1; if (!accepts<M>(tree[k], x)) --k; }
					int last = int(std::min(values_.size(), std::size_t(k - base_ + 1) * 64));
					begin = int((k - base_) * 64);
					for (int i = last; i-- > begin;) if (accepts<M>(values_[i], x)) return i;
					return size();
				}
			} while ((k & -k) != k);
		}
		return size();
	}
	void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }
  public:
	StaticThresholdSearch() = default;
	explicit StaticThresholdSearch(std::vector<T> values) { build(std::move(values)); }
	StaticThresholdSearch(const StaticThresholdSearch &) = default;
	StaticThresholdSearch &operator=(const StaticThresholdSearch &other) {
		if (this != &other) { StaticThresholdSearch copy(other); swap(copy); } return *this;
	}
	StaticThresholdSearch(StaticThresholdSearch &&other) noexcept { swap(other); }
	StaticThresholdSearch &operator=(StaticThresholdSearch &&other) noexcept {
		if (this != &other) { StaticThresholdSearch moved(std::move(other)); swap(moved); } return *this;
	}
	void swap(StaticThresholdSearch &other) noexcept {
		values_.swap(other.values_); minimum_.swap(other.minimum_); maximum_.swap(other.maximum_); std::swap(base_, other.base_);
	}
	void build(std::vector<T> values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		StaticThresholdSearch next;
		next.values_ = std::move(values);
		unsigned blocks = unsigned((next.values_.size() + 63) / 64);
		next.base_ = std::bit_ceil(std::max(1U, blocks));
		next.minimum_.assign(std::size_t(next.base_) * 2, std::numeric_limits<T>::max());
		next.maximum_.assign(std::size_t(next.base_) * 2, std::numeric_limits<T>::lowest());
		for (unsigned block = 0; block < blocks; ++block) {
			std::size_t begin = std::size_t(block) * 64, end = std::min(begin + 64, next.values_.size());
			T low = next.values_[begin], high = low;
			for (std::size_t i = begin + 1; i < end; ++i) {
				low = std::min(low, next.values_[i]);
				high = std::max(high, next.values_[i]);
			}
			next.minimum_[next.base_ + block] = low;
			next.maximum_[next.base_ + block] = high;
		}
		for (unsigned v = next.base_ - 1; v; --v) {
			next.minimum_[v] = std::min(next.minimum_[v * 2], next.minimum_[v * 2 + 1]);
			next.maximum_[v] = std::max(next.maximum_[v * 2], next.maximum_[v * 2 + 1]);
		}
		swap(next);
	}
	void clear() { StaticThresholdSearch next; swap(next); }
	int size() const { return int(values_.size()); }
	bool empty() const { return values_.empty(); }
	std::span<const T> values() const { return values_; }
	const T &operator[](int i) const { assert(0 <= i && i < size()); return values_[i]; }
	int next(int l, T x) const requires (!Reverse) { return search<Smaller ? Mode::le : Mode::ge, true>(l, x); }
	int next_strict(int l, T x) const requires (!Reverse) { return search<Smaller ? Mode::lt : Mode::gt, true>(l, x); }
	int first(int l, int r, T x) const requires (!Reverse) { check_range(l, r); if (l == r) return size(); int p = next(l, x); return p < r ? p : size(); }
	int first_strict(int l, int r, T x) const requires (!Reverse) { check_range(l, r); if (l == r) return size(); int p = next_strict(l, x); return p < r ? p : size(); }
	int next_ge(int l, T x) const requires (!Smaller && !Reverse) { return next(l, x); }
	int next_gt(int l, T x) const requires (!Smaller && !Reverse) { return next_strict(l, x); }
	int next_le(int l, T x) const requires (Smaller && !Reverse) { return next(l, x); }
	int next_lt(int l, T x) const requires (Smaller && !Reverse) { return next_strict(l, x); }
	int prev(int r, T x) const requires Reverse { return search<Smaller ? Mode::le : Mode::ge, false>(r, x); }
	int prev_strict(int r, T x) const requires Reverse { return search<Smaller ? Mode::lt : Mode::gt, false>(r, x); }
	int last(int l, int r, T x) const requires Reverse { check_range(l, r); if (l == r) return size(); int p = prev(r, x); return p != size() && p >= l ? p : size(); }
	int last_strict(int l, int r, T x) const requires Reverse { check_range(l, r); if (l == r) return size(); int p = prev_strict(r, x); return p != size() && p >= l ? p : size(); }
	int prev_ge(int r, T x) const requires (!Smaller && Reverse) { return prev(r, x); }
	int prev_gt(int r, T x) const requires (!Smaller && Reverse) { return prev_strict(r, x); }
	int prev_le(int r, T x) const requires (Smaller && Reverse) { return prev(r, x); }
	int prev_lt(int r, T x) const requires (Smaller && Reverse) { return prev_strict(r, x); }
	int next_record(int i) const {
		assert(0 <= i && i < size());
		if constexpr (Reverse) return search<Smaller ? Mode::lt : Mode::gt, false>(i, values_[i]);
		else return search<Smaller ? Mode::lt : Mode::gt, true>(i + 1, values_[i]);
	}
};

#endif
