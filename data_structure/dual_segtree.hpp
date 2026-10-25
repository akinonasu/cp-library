#ifndef DUAL_SEGTREE_HPP
#define DUAL_SEGTREE_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template<class S, class F, auto mapping, auto composition, auto id, bool Commutative = false>
class DualSegtree {
	int n_ = 0, base_ = 1, log_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void check([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void all_apply(int k, const F &f) {
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = 1;
		} else data_[k - base_] = mapping(f, std::move(data_[k - base_]));
	}
	void push(int k) {
		if (!dirty_[k]) return;
		all_apply(2 * k, lazy_[k]);
		all_apply(2 * k + 1, lazy_[k]);
		dirty_[k] = 0;
	}
	void push_path(int k) {
		for (int h = log_; h; --h) push(k >> h);
	}
	void push_range(int l, int r) {
		int left = std::countr_zero(static_cast<unsigned>(l));
		int right = std::countr_zero(static_cast<unsigned>(r));
		for (int h = log_; h > left; --h) push(l >> h);
		for (int h = log_; h > right; --h) push((r - 1) >> h);
	}

  public:
	using value_type = S;
	using update_type = F;
	DualSegtree() = default;
	explicit DualSegtree(int n, const S &value = S{}) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 4);
		build(std::vector<S>(n, value));
	}
	explicit DualSegtree(std::vector<S> a) { build(std::move(a)); }
	void build(std::vector<S> a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		n_ = static_cast<int>(a.size());
		base_ = static_cast<int>(std::bit_ceil(static_cast<unsigned>(std::max(1, n_))));
		log_ = std::countr_zero(static_cast<unsigned>(base_));
		data_ = std::move(a);
		lazy_.assign(base_, id());
		dirty_.assign(base_, 0);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	S get(int p) const {
		check(p);
		S value = data_[p];
		for (int k = (p + base_) >> 1; k; k >>= 1)
			if (dirty_[k]) value = mapping(lazy_[k], std::move(value));
		return value;
	}
	void set(int p, const S &value) {
		check(p);
		push_path(p + base_);
		data_[p] = value;
	}
	void apply(int p, const F &f) {
		check(p);
		if constexpr (!Commutative) push_path(p + base_);
		data_[p] = mapping(f, std::move(data_[p]));
	}
	void apply(int l, int r, const F &f) {
		assert(0 <= l && l <= r && r <= n_);
		if (l == r) return;
		if (r - l == 1) { apply(l, f); return; }
		l += base_, r += base_;
		if constexpr (!Commutative) push_range(l, r);
		while (l < r) {
			if (l & 1) all_apply(l++, f);
			if (r & 1) all_apply(--r, f);
			l >>= 1, r >>= 1;
		}
	}
	std::vector<S> values() {
		for (int k = 1; k < base_; ++k) push(k);
		return data_;
	}
};

#endif
