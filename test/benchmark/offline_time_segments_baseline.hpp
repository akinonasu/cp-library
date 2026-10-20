#ifndef OFFLINE_TIME_SEGMENTS_BASELINE_HPP
#define OFFLINE_TIME_SEGMENTS_BASELINE_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class OfflineTimeSegments {
	int n_;
	std::size_t base_, entries_ = 0;
	std::vector<std::vector<int>> buckets_;

  public:
	struct Interval { int left, right, item; };
	OfflineTimeSegments(int n, const std::vector<Interval> &intervals) : n_(n), base_(1) {
		assert(n >= 0);
		if (!n || intervals.empty()) return;
		base_ = std::bit_ceil(static_cast<std::size_t>(n));
		buckets_.resize(base_ * 2);
		for (auto it = intervals.rbegin(); it != intervals.rend(); ++it) {
			assert(0 <= it->left && it->left <= it->right && it->right <= n);
			for (std::size_t l = base_ + it->left, r = base_ + it->right; l < r; l >>= 1, r >>= 1) {
				if (l & 1) buckets_[l++].push_back(it->item), ++entries_;
				if (r & 1) buckets_[--r].push_back(it->item), ++entries_;
			}
		}
	}
	int size() const { return n_; }
	std::size_t entry_count() const { return entries_; }
	template <class State, class Apply, class Visit>
	void run(State &state, Apply &&apply, Visit &&visit) const {
		if (!n_) return;
		if (!entries_) {
			for (int t = 0; t < n_; ++t) visit(t, std::as_const(state));
			return;
		}
		auto dfs = [&](auto &&self, std::size_t v, std::size_t l, std::size_t r) -> void {
			auto saved = state.snapshot();
			for (int item : buckets_[v]) apply(state, item);
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
