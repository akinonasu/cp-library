#ifndef DYNAMIC_LI_CHAO_TREE_HPP
#define DYNAMIC_LI_CHAO_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, bool maximize = false, class R = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>>
class DynamicLiChaoTree {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using B = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>;
	static constexpr std::uint32_t present = std::uint32_t(1) << 31, mask = present - 1;
	static constexpr B first = std::numeric_limits<T>::lowest(), end = B(std::numeric_limits<T>::max()) + 1;
	struct Line {
		T a = 0, b = 0;
		template <class V> V eval(T x) const { return V(R(a) * R(x) + R(b)); }
	};
	struct Node { Line line; std::uint32_t left = 0, right = 0; };
	std::vector<Node> nodes_{Node{}};
	std::uint32_t root_ = 0;
	T low_, high_;
	bool wide_ = false;
	template <class V> static bool better(V a, V b) {
		if constexpr (maximize) return a > b;
		else return a < b;
	}
	std::uint32_t make_node(Line f = {}, bool has_line = false) {
		assert(nodes_.size() < present);
		nodes_.push_back({f, has_line ? present : 0, 0});
		return std::uint32_t(nodes_.size() - 1);
	}
	template <class V> std::uint32_t insert(std::uint32_t k, T l, T r, Line f) {
		if (!k) return make_node(f, true);
		std::uint32_t result = k;
		for (;;) {
			auto& node = nodes_[k];
			if (!(node.left & present)) {
				node.line = f, node.left |= present;
				return result;
			}
			auto& g = node.line;
			if (f.a == g.a) {
				if (better(f.b, g.b)) g = f;
				return result;
			}
			T m = std::midpoint(l, r);
			if (better(f.template eval<V>(m), g.template eval<V>(m))) std::swap(f, g);
			if (l == r) return result;
			bool right = better(f.a, g.a);
			if (!better(f.template eval<V>(right ? r : l), g.template eval<V>(right ? r : l))) return result;
			std::uint32_t child = right ? node.right : node.left & mask;
			if (!child) {
				child = make_node(f, true);
				if (right) nodes_[k].right = child;
				else nodes_[k].left = (nodes_[k].left & present) | child;
				return result;
			}
			k = child;
			if (right) l = T(m + 1);
			else r = m;
		}
	}
	template <class V> std::uint32_t insert_segment(std::uint32_t k, T l, T r, T ql, T qr, Line f) {
		if (k && (nodes_[k].left & present)) {
			const auto& g = nodes_[k].line;
			if (f.a == g.a) {
				if (!better(f.b, g.b)) return k;
			} else {
				T edge = better(f.a, g.a) ? std::min(r, qr) : std::max(l, ql);
				if (!better(f.template eval<V>(edge), g.template eval<V>(edge))) return k;
			}
		}
		if (ql <= l && r <= qr) return insert<V>(k, l, r, f);
		if (!k) k = make_node();
		T m = std::midpoint(l, r);
		if (ql <= m) {
			auto child = insert_segment<V>(nodes_[k].left & mask, l, m, ql, qr, f);
			nodes_[k].left = (nodes_[k].left & present) | child;
		}
		if (m < qr) {
			auto child = insert_segment<V>(nodes_[k].right, T(m + 1), r, ql, qr, f);
			nodes_[k].right = child;
		}
		return k;
	}
	void check(Line f, T l, T r) {
		if constexpr (std::is_same_v<R, __int128_t>) if (!wide_) {
			R a = f.template eval<R>(l), b = f.template eval<R>(r);
			wide_ = a < std::numeric_limits<std::int64_t>::lowest() || a > std::numeric_limits<std::int64_t>::max()
				|| b < std::numeric_limits<std::int64_t>::lowest() || b > std::numeric_limits<std::int64_t>::max();
		}
	}
	template <class F> decltype(auto) dispatch(F f) const {
		if constexpr (std::is_same_v<R, __int128_t>) if (!wide_) return f.template operator()<std::int64_t>();
		return f.template operator()<R>();
	}
	template <class V> std::optional<R> query_impl(T x) const {
		V result = maximize ? std::numeric_limits<V>::lowest() : std::numeric_limits<V>::max();
		bool found = false;
		T l = low_, r = high_;
		for (std::uint32_t k = root_; k;) {
			const auto& node = nodes_[k];
			if (node.left & present) {
				V value = node.line.template eval<V>(x);
				if (better(value, result)) result = value;
				found = true;
			}
			if (l == r) break;
			T m = std::midpoint(l, r);
			if (x <= m) k = node.left & mask, r = m;
			else k = node.right, l = T(m + 1);
		}
		if (found) return R(result);
		return std::nullopt;
	}

  public:
	using value_type = R;
	using bound_type = B;
	explicit DynamicLiChaoTree(B lower = first, B upper = end) { build(lower, upper); }
	void build(B lower, B upper) {
		assert(first <= lower && lower < upper && upper <= end);
		low_ = T(lower), high_ = T(upper - 1);
		clear();
	}
	B lower() const { return B(low_); }
	B upper() const { return B(high_) + 1; }
	bool empty() const { return root_ == 0; }
	std::size_t node_count() const { return nodes_.size() - 1; }
	void reserve(std::size_t count) {
		assert(count < present);
		nodes_.reserve(count + 1);
	}
	void clear() { nodes_.resize(1), root_ = 0, wide_ = false; }
	void add_line(T a, T b) {
		Line f{a, b};
		check(f, low_, high_);
		dispatch([&]<class V>() { root_ = insert<V>(root_, low_, high_, f); });
	}
	void add_segment(B l, B r, T a, T b) {
		assert(l <= r);
		l = std::max(l, lower()), r = std::min(r, upper());
		if (l >= r) return;
		Line f{a, b};
		T ql = T(l), qr = T(r - 1);
		check(f, ql, qr);
		dispatch([&]<class V>() { root_ = insert_segment<V>(root_, low_, high_, ql, qr, f); });
	}
	std::optional<R> query(T x) const {
		assert(low_ <= x && x <= high_);
		return dispatch([&]<class V>() { return query_impl<V>(x); });
	}
};

#endif
