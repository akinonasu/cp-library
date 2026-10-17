#ifndef SORTABLE_SEGTREE_HPP
#define SORTABLE_SEGTREE_HPP

#include "../components/predecessor_set.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class Key, class S, auto op, auto e, bool commutative = false>
class SortableSegtree {
	static_assert(std::is_integral_v<Key> && !std::is_same_v<Key, bool> && sizeof(Key) <= 8);
	using U = std::make_unsigned_t<Key>;
	static constexpr U sign = std::is_signed_v<Key> ? U(1) << (std::numeric_limits<U>::digits - 1) : 0;
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int left = 0, right = 0, count = 1, bit = -1, leaf;
		S forward;
		[[no_unique_address]] Back backward;
		static Back back(const S &value) { if constexpr (commutative) return {}; else return value; }
		Node(S value, int leaf) : leaf(leaf), forward(std::move(value)), backward(back(forward)) {}
	};
	int n = 0, base = 1, free = 0;
	std::vector<U> keys;
	std::vector<Node> pool;
	std::vector<int> roots;
	std::vector<unsigned char> reversed;
	std::vector<S> aggregate;
	PredecessorSet boundaries;
	int difference(int a, int b) const {
		U x = keys[pool[a].leaf] ^ keys[pool[b].leaf];
		return x ? 30 + int(std::bit_width(x)) : int(std::bit_width(unsigned(pool[a].leaf ^ pool[b].leaf))) - 1;
	}
	bool side(int x, int bit) const {
		return bit >= 31 ? keys[pool[x].leaf] >> (bit - 31) & 1 : unsigned(pool[x].leaf) >> bit & 1;
	}
	S product(int x, bool reverse) const {
		if constexpr (commutative) { (void)reverse; return pool[x].forward; }
		else return reverse ? pool[x].backward : pool[x].forward;
	}
	void pull(int x) {
		auto &node = pool[x];
		const auto &a = pool[node.left], &b = pool[node.right];
		node.leaf = a.leaf;
		node.count = a.count + b.count;
		node.forward = op(a.forward, b.forward);
		if constexpr (!commutative) node.backward = op(b.backward, a.backward);
	}
	void release(int x) { pool[x].left = free; free = x; }
	int branch(int bit, int a, int b) {
		if (side(a, bit)) std::swap(a, b);
		int x;
		if (free) { x = free; free = pool[x].left; }
		else { x = pool.size(); pool.emplace_back(e(), 0); }
		pool[x].left = a; pool[x].right = b; pool[x].bit = bit;
		pull(x); return x;
	}
	int meld(int a, int b) {
		if (!a || !b) return a | b;
		int bit = difference(a, b), ab = pool[a].bit, bb = pool[b].bit;
		if (bit > std::max(ab, bb)) return branch(bit, a, b);
		assert(ab >= 0 || bb >= 0);
		if (ab == bb) {
			pool[a].left = meld(pool[a].left, pool[b].left);
			pool[a].right = meld(pool[a].right, pool[b].right);
			release(b);
		} else {
			if (ab < bb) std::swap(a, b), std::swap(ab, bb);
			if (side(b, ab)) pool[a].right = meld(pool[a].right, b);
			else pool[a].left = meld(pool[a].left, b);
		}
		pull(a); return a;
	}
	std::pair<int, int> split(int x, int k) {
		if (!k) return {0, x};
		if (k == pool[x].count) return {x, 0};
		int a = pool[x].left, b = pool[x].right, count = pool[a].count;
		if (k == count) { release(x); return {a, b}; }
		if (k < count) {
			auto [first, second] = split(a, k);
			pool[x].left = second; pull(x); return {first, x};
		}
		auto [first, second] = split(b, k - count);
		pool[x].right = first; pull(x); return {x, second};
	}
	int at(int x, int k) const {
		while (pool[x].bit >= 0) {
			int count = pool[pool[x].left].count;
			if (k < count) x = pool[x].left;
			else k -= count, x = pool[x].right;
		}
		return x;
	}
	S fold(int x, int l, int r, bool reverse) const {
		if (!l && r == pool[x].count) return product(x, reverse);
		int a = reverse ? pool[x].right : pool[x].left, b = reverse ? pool[x].left : pool[x].right;
		int count = pool[a].count;
		if (r <= count) return fold(a, l, r, reverse);
		if (l >= count) return fold(b, l - count, r - count, reverse);
		return op(fold(a, l, count, reverse), fold(b, 0, r - count, reverse));
	}
	void update(int p, S value) {
		aggregate[p += base] = std::move(value);
		while (p >>= 1) aggregate[p] = op(aggregate[p * 2], aggregate[p * 2 + 1]);
	}
	S middle_prod(int l, int r) const {
		S a = e(), b = e();
		for (l += base, r += base; l < r; l >>= 1, r >>= 1) {
			if (l & 1) a = op(std::move(a), aggregate[l++]);
			if (r & 1) b = op(aggregate[--r], std::move(b));
		}
		return op(std::move(a), std::move(b));
	}
	void cut(int p) {
		if (boundaries.contains(p)) return;
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		if (reversed[l]) k = pool[x].count - k;
		auto [a, b] = split(x, k);
		if (reversed[l]) std::swap(a, b);
		roots[l] = a; roots[p] = b; reversed[p] = reversed[l];
		boundaries.insert(p);
		update(l, product(a, reversed[l])); update(p, product(b, reversed[p]));
	}
	void set_value(int x, int k, S value) {
		if (pool[x].bit < 0) {
			pool[x].forward = std::move(value);
			if constexpr (!commutative) pool[x].backward = pool[x].forward;
			return;
		}
		int count = pool[pool[x].left].count;
		if (k < count) set_value(pool[x].left, k, std::move(value));
		else set_value(pool[x].right, k - count, std::move(value));
		pull(x);
	}
	void collect(int x, bool reverse, std::vector<std::pair<Key, S>> &out) const {
		if (pool[x].bit < 0) { out.emplace_back(Key(keys[pool[x].leaf] ^ sign), pool[x].forward); return; }
		collect(reverse ? pool[x].right : pool[x].left, reverse, out);
		collect(reverse ? pool[x].left : pool[x].right, reverse, out);
	}
public:
	SortableSegtree() : aggregate(2, e()) {}
	explicit SortableSegtree(const std::vector<std::pair<Key, S>> &a) { assign(a); }
	SortableSegtree(const SortableSegtree &) = default;
	SortableSegtree &operator=(const SortableSegtree &) = default;
	SortableSegtree(SortableSegtree &&other) : SortableSegtree() { swap(other); }
	SortableSegtree &operator=(SortableSegtree &&other) {
		if (this != &other) { SortableSegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(SortableSegtree &other) {
		using std::swap;
		swap(n, other.n); swap(base, other.base); swap(free, other.free);
		keys.swap(other.keys); pool.swap(other.pool); roots.swap(other.roots); reversed.swap(other.reversed);
		aggregate.swap(other.aggregate); swap(boundaries, other.boundaries);
	}
	void assign(const std::vector<std::pair<Key, S>> &a) {
		assert(a.size() < (std::size_t(1) << 29));
		n = a.size(); base = std::bit_ceil(unsigned(std::max(n, 1))); free = 0;
		keys.clear(); keys.reserve(n);
		pool.clear(); pool.reserve(2 * std::size_t(n) + 1);
		pool.emplace_back(e(), 0); pool[0].count = 0;
		roots.resize(n); reversed.assign(n, 0); aggregate.assign(2 * base, e());
		boundaries.assign(n + 1, true);
		for (int i = 0; i < n; ++i) {
			keys.push_back(U(a[i].first) ^ sign);
			pool.emplace_back(a[i].second, i); roots[i] = i + 1;
			aggregate[base + i] = a[i].second;
		}
		for (int i = base - 1; i; --i) aggregate[i] = op(aggregate[i * 2], aggregate[i * 2 + 1]);
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	std::pair<Key, S> get(int p) const {
		assert(0 <= p && p < n);
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		x = at(x, reversed[l] ? pool[x].count - 1 - k : k);
		return {Key(keys[pool[x].leaf] ^ sign), pool[x].forward};
	}
	Key key(int p) const {
		assert(0 <= p && p < n);
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		x = at(x, reversed[l] ? pool[x].count - 1 - k : k);
		return Key(keys[pool[x].leaf] ^ sign);
	}
	void set(int p, Key key, S value) {
		assert(0 <= p && p < n);
		cut(p); cut(p + 1);
		int x = roots[p]; keys[pool[x].leaf] = U(key) ^ sign;
		pool[x].forward = std::move(value);
		if constexpr (!commutative) pool[x].backward = pool[x].forward;
		update(p, pool[x].forward);
	}
	void set(int p, S value) {
		assert(0 <= p && p < n);
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		set_value(x, reversed[l] ? pool[x].count - 1 - k : k, std::move(value));
		update(l, product(x, reversed[l]));
	}
	S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if (l == r) return e();
		int first = boundaries.prev(l), last = boundaries.prev(r - 1);
		if (first == last) return fold(roots[first], l - first, r - first, reversed[first]);
		int end = first + pool[roots[first]].count;
		S value = fold(roots[first], l - first, end - first, reversed[first]);
		if (end < last) value = op(std::move(value), middle_prod(end, last));
		return op(std::move(value), fold(roots[last], 0, r - last, reversed[last]));
	}
	S all_prod() const { return aggregate[1]; }
	void sort(int l, int r, bool descending = false) {
		assert(0 <= l && l <= r && r <= n);
		if (r - l <= 1) return;
		cut(l); cut(r);
		for (int p = boundaries.next(l + 1); p < r; p = boundaries.next(p + 1)) {
			roots[l] = meld(roots[l], roots[p]); roots[p] = 0;
			boundaries.erase(p); update(p, e());
		}
		reversed[l] = descending; update(l, product(roots[l], descending));
	}
	std::vector<std::pair<Key, S>> to_vector() const {
		std::vector<std::pair<Key, S>> out;
		out.reserve(n);
		for (int i = 0; i < n; ++i) if (roots[i]) collect(roots[i], reversed[i], out);
		return out;
	}
	void clear() { assign({}); }
	std::size_t memory_usage() const {
		return keys.capacity() * sizeof(U) + pool.capacity() * sizeof(Node) + roots.capacity() * sizeof(int)
			+ reversed.capacity() + aggregate.capacity() * sizeof(S) + boundaries.memory_usage();
	}
};

#endif
