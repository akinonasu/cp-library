#ifndef PERSISTENT_SEGTREE_HPP
#define PERSISTENT_SEGTREE_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e>
class PersistentSegtree {
  public:
	using value_type = S;
	using root_type = std::uint32_t;

  private:
	struct Node {
		S value;
		root_type left, right;
	};
	int n_ = 0;
	root_type initial_ = 0;
	std::vector<Node> nodes{{e(), 0, 0}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(S value, root_type left = 0, root_type right = 0) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		root_type result = static_cast<root_type>(nodes.size());
		nodes.push_back({std::move(value), left, right});
		return result;
	}
	root_type join(root_type left, root_type right) {
		return left || right ? make(op(nodes[left].value, nodes[right].value), left, right) : 0;
	}
	root_type build(const std::vector<S> &a, int l, int r) {
		if (r - l == 1) return make(a[l]);
		int m = l + (r - l) / 2;
		root_type left = build(a, l, m), right = build(a, m, r);
		return join(left, right);
	}
	template <bool Append>
	root_type update(root_type root, int p, S value) {
		check(root);
		assert(0 <= p && p < n_);
		std::array<root_type, 32> path;
		unsigned depth = 0;
		std::uint32_t right_path = 0;
		int l = 0, r = n_;
		while (r - l > 1) {
			path[depth] = root;
			int m = l + (r - l) / 2;
			if (p < m) { root = nodes[root].left; r = m; }
			else { right_path |= std::uint32_t(1) << depth; root = nodes[root].right; l = m; }
			++depth;
		}
		if constexpr (Append) value = op(nodes[root].value, value);
		root = make(std::move(value));
		while (depth) {
			root_type parent = path[--depth];
			root = right_path >> depth & 1 ? join(nodes[parent].left, root) : join(root, nodes[parent].right);
		}
		return root;
	}
	S prefix(root_type root, int l, int r, int end) const {
		S result = e();
		while (root && r - l > 1 && end < r) {
			int m = l + (r - l) / 2;
			if (end <= m) { root = nodes[root].left; r = m; }
			else { result = op(result, nodes[nodes[root].left].value); root = nodes[root].right; l = m; }
		}
		return root && l < end ? op(result, nodes[root].value) : result;
	}
	S suffix(root_type root, int l, int r, int start) const {
		S result = e();
		while (root && r - l > 1 && l < start) {
			int m = l + (r - l) / 2;
			if (m <= start) { root = nodes[root].right; l = m; }
			else { result = op(nodes[nodes[root].right].value, result); root = nodes[root].left; r = m; }
		}
		return root && start < r ? op(nodes[root].value, result) : result;
	}
	root_type copy_impl(root_type root, root_type source, int l, int r, int ql, int qr) {
		if (root == source || qr <= l || r <= ql) return root;
		if (ql <= l && r <= qr) return source;
		int m = l + (r - l) / 2;
		root_type old_left = nodes[root].left, old_right = nodes[root].right;
		root_type source_left = nodes[source].left, source_right = nodes[source].right;
		root_type left = copy_impl(old_left, source_left, l, m, ql, qr);
		root_type right = copy_impl(old_right, source_right, m, r, ql, qr);
		return left == old_left && right == old_right ? root : join(left, right);
	}
	template <class Pred>
	int search_right(root_type root, int l, int r, int start, S &value, Pred &f) const {
		if (!root) return n_;
		if (start <= l) {
			S next = op(value, nodes[root].value);
			if (f(next)) { value = std::move(next); return n_; }
			if (r - l == 1) return l;
		}
		int m = l + (r - l) / 2;
		if (start < m) {
			int found = search_right(nodes[root].left, l, m, start, value, f);
			if (found != n_) return found;
		}
		return search_right(nodes[root].right, m, r, start, value, f);
	}
	template <class Pred>
	int search_left(root_type root, int l, int r, int end, S &value, Pred &f) const {
		if (!root) return 0;
		if (r <= end) {
			S next = op(nodes[root].value, value);
			if (f(next)) { value = std::move(next); return 0; }
			if (r - l == 1) return r;
		}
		int m = l + (r - l) / 2;
		if (m < end) {
			int found = search_left(nodes[root].right, m, r, end, value, f);
			if (found) return found;
		}
		return search_left(nodes[root].left, l, m, end, value, f);
	}

  public:
	explicit PersistentSegtree(int n = 0) : n_(n) { assert(0 <= n); }
	explicit PersistentSegtree(const std::vector<S> &a) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) { reserve(2 * a.size() - 1); initial_ = build(a, 0, n_); }
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	void reserve(std::size_t count) {
		assert(count < std::numeric_limits<root_type>::max());
		nodes.reserve(count + 1);
	}
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	root_type set(root_type root, int p, S value) { return update<false>(root, p, std::move(value)); }
	root_type multiply(root_type root, int p, S value) { return update<true>(root, p, std::move(value)); }
	S get(root_type root, int p) const {
		check(root);
		assert(0 <= p && p < n_);
		int l = 0, r = n_;
		while (root && r - l > 1) {
			int m = l + (r - l) / 2;
			if (p < m) { root = nodes[root].left; r = m; }
			else { root = nodes[root].right; l = m; }
		}
		return nodes[root].value;
	}
	S prod(root_type root, int l, int r) const {
		check(root);
		assert(0 <= l && l <= r && r <= n_);
		if (l == r) return e();
		int first = 0, last = n_;
		while (root) {
			if (l == first && r == last) return nodes[root].value;
			int m = first + (last - first) / 2;
			if (r <= m) { root = nodes[root].left; last = m; }
			else if (m <= l) { root = nodes[root].right; first = m; }
			else return op(suffix(nodes[root].left, first, m, l), prefix(nodes[root].right, m, last, r));
		}
		return e();
	}
	S all_prod(root_type root) const { check(root); return nodes[root].value; }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		check(root); check(source);
		assert(0 <= l && l <= r && r <= n_);
		return l == r ? root : copy_impl(root, source, 0, n_, l, r);
	}
	template <class Pred>
	int max_right(root_type root, int l, Pred f) const {
		check(root);
		assert(0 <= l && l <= n_ && f(e()));
		S value = e();
		return l == n_ ? n_ : search_right(root, 0, n_, l, value, f);
	}
	template <class Pred>
	int min_left(root_type root, int r, Pred f) const {
		check(root);
		assert(0 <= r && r <= n_ && f(e()));
		S value = e();
		return r ? search_left(root, 0, n_, r, value, f) : 0;
	}
};

#endif
