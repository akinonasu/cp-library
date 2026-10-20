#ifndef OFFLINE_TIME_SEGMENTS_HPP
#define OFFLINE_TIME_SEGMENTS_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class OfflineTimeSegments {
	int n_;
	std::size_t base_;
	std::vector<std::size_t> offset_;
	std::vector<int> items_;

  public:
	struct Interval { int left, right, item; };
	OfflineTimeSegments(int n, const std::vector<Interval> &intervals) : n_(n), base_(1) {
		assert(n >= 0);
		if (!n || intervals.empty()) return;
		base_ = std::bit_ceil(static_cast<std::size_t>(n));
		offset_.resize(base_ * 2 + 1);
		auto cover = [&](const Interval &interval, auto &&visit) {
			assert(0 <= interval.left && interval.left <= interval.right && interval.right <= n);
			for (std::size_t l = base_ + interval.left, r = base_ + interval.right; l < r; l >>= 1, r >>= 1) {
				if (l & 1) visit(l++);
				if (r & 1) visit(--r);
			}
		};
		for (const auto &interval : intervals) cover(interval, [&](std::size_t v) { ++offset_[v]; });
		for (std::size_t v = 1; v < offset_.size(); ++v) offset_[v] += offset_[v - 1];
		items_.resize(offset_.back());
		for (const auto &interval : intervals)
			cover(interval, [&](std::size_t v) { items_[--offset_[v]] = interval.item; });
	}
	int size() const { return n_; }
	std::size_t entry_count() const { return items_.size(); }
	template <class State, class Apply, class Visit>
	void run(State &state, Apply &&apply, Visit &&visit) const {
		if (!n_) return;
		if (items_.empty()) {
			for (int t = 0; t < n_; ++t) visit(t, std::as_const(state));
			return;
		}
		auto dfs = [&](auto &&self, std::size_t v, std::size_t l, std::size_t r) -> void {
			auto saved = state.snapshot();
			for (std::size_t i = offset_[v]; i < offset_[v + 1]; ++i) apply(state, items_[i]);
			if (r - l == 1) visit(static_cast<int>(l), std::as_const(state));
			else {
				std::size_t m = (l + r) / 2;
				self(self, v * 2, l, m);
				if (m < static_cast<std::size_t>(n_)) self(self, v * 2 + 1, m, r);
			}
			state.rollback(saved);
		};
		dfs(dfs, 1, 0, base_);
	}
};

#endif
