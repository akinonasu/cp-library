#ifndef PERSISTENT_LAZY_SEGTREE_HPP
#define PERSISTENT_LAZY_SEGTREE_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class PersistentLazySegtree {
  public:
	using value_type = S;
	using root_type = std::uint32_t;
  private:
	struct Node { S value; F lazy; root_type left, right; };
	std::vector<Node> nodes{{e(), id(), 0, 0}};
	int n_ = 0;
	root_type initial_ = 0;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type join(root_type left, root_type right) { return make({op(nodes[left].value, nodes[right].value), id(), left, right}); }
	root_type build(const std::vector<S> &a, int l, int r) {
		if (r - l == 1) return make({a[l], id(), 0, 0});
		int m = l + (r - l) / 2;
		auto left = build(a, l, m), right = build(a, m, r);
		return join(left, right);
	}
	root_type mapped(root_type root, F f, int length) {
		Node node = nodes[root];
		node.value = mapping(f, node.value, length);
		node.lazy = composition(f, node.lazy);
		return make(std::move(node));
	}
	root_type update(root_type root, int l, int r, int ql, int qr, F f, F carry) {
		if (qr <= l || r <= ql) return mapped(root, carry, r - l);
		if (ql <= l && r <= qr) return mapped(root, composition(f, carry), r - l);
		Node node = nodes[root];
		carry = composition(carry, node.lazy);
		int m = l + (r - l) / 2;
		auto left = update(node.left, l, m, ql, qr, f, carry);
		auto right = update(node.right, m, r, ql, qr, f, carry);
		return join(left, right);
	}
	root_type assign(root_type root, int l, int r, int p, S value, F carry) {
		if (r - l == 1) return make({std::move(value), id(), 0, 0});
		Node node = nodes[root];
		carry = composition(carry, node.lazy);
		int m = l + (r - l) / 2;
		root_type left, right;
		if (p < m) { left = assign(node.left, l, m, p, std::move(value), carry); right = mapped(node.right, carry, r - m); }
		else { left = mapped(node.left, carry, m - l); right = assign(node.right, m, r, p, std::move(value), carry); }
		return join(left, right);
	}
	S query(root_type root, int l, int r, int ql, int qr, F carry) const {
		if (qr <= l || r <= ql) return e();
		const auto &node = nodes[root];
		if (ql <= l && r <= qr) return mapping(carry, node.value, r - l);
		carry = composition(carry, node.lazy);
		int m = l + (r - l) / 2;
		return op(query(node.left, l, m, ql, qr, carry), query(node.right, m, r, ql, qr, carry));
	}
	root_type copy(root_type root, root_type source, int l, int r, int ql, int qr, F carry, F source_carry) {
		if (qr <= l || r <= ql) return mapped(root, carry, r - l);
		if (ql <= l && r <= qr) return mapped(source, source_carry, r - l);
		Node a = nodes[root], b = nodes[source];
		carry = composition(carry, a.lazy); source_carry = composition(source_carry, b.lazy);
		int m = l + (r - l) / 2;
		auto left = copy(a.left, b.left, l, m, ql, qr, carry, source_carry);
		auto right = copy(a.right, b.right, m, r, ql, qr, carry, source_carry);
		return join(left, right);
	}
	template <bool Right, class Pred>
	int search(root_type root, int l, int r, int end, F carry, S &value, Pred &f) const {
		const auto &node = nodes[root];
		if (Right ? end <= l : r <= end) {
			S part = mapping(carry, node.value, r - l);
			S next = Right ? op(value, part) : op(part, value);
			if (f(next)) { value = std::move(next); return Right ? n_ : 0; }
			if (r - l == 1) return Right ? l : r;
		}
		carry = composition(carry, node.lazy);
		int m = l + (r - l) / 2;
		if constexpr (Right) {
			if (end < m) { int found = search<Right>(node.left, l, m, end, carry, value, f); if (found != n_) return found; }
			return search<Right>(node.right, m, r, end, carry, value, f);
		} else {
			if (m < end) { int found = search<Right>(node.right, m, r, end, carry, value, f); if (found) return found; }
			return search<Right>(node.left, l, m, end, carry, value, f);
		}
	}
  public:
	explicit PersistentLazySegtree(int n = 0) : n_(n) { assert(n >= 0); }
	explicit PersistentLazySegtree(const std::vector<S> &a) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) { reserve(2 * a.size() - 1); initial_ = build(a, 0, n_); }
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	root_type set(root_type root, int p, S value) { check(root); assert(0 <= p && p < n_); return assign(root, 0, n_, p, std::move(value), id()); }
	root_type apply(root_type root, int l, int r, F f) {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? root : update(root, 0, n_, l, r, std::move(f), id());
	}
	root_type apply(root_type root, int p, F f) { assert(0 <= p && p < n_); return apply(root, p, p + 1, std::move(f)); }
	S prod(root_type root, int l, int r) const {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? e() : query(root, 0, n_, l, r, id());
	}
	S get(root_type root, int p) const { assert(0 <= p && p < n_); return prod(root, p, p + 1); }
	S all_prod(root_type root) const { check(root); return nodes[root].value; }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		check(root); check(source); assert(0 <= l && l <= r && r <= n_);
		if (l == r || root == source) return root;
		if (l == 0 && r == n_) return source;
		return copy(root, source, 0, n_, l, r, id(), id());
	}
	template <class Pred> int max_right(root_type root, int l, Pred f) const {
		check(root); assert(0 <= l && l <= n_ && f(e()));
		S value = e();
		return l == n_ ? n_ : search<true>(root, 0, n_, l, id(), value, f);
	}
	template <class Pred> int min_left(root_type root, int r, Pred f) const {
		check(root); assert(0 <= r && r <= n_ && f(e()));
		S value = e();
		return r == 0 ? 0 : search<false>(root, 0, n_, r, id(), value, f);
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
