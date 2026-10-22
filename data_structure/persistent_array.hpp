#ifndef PERSISTENT_ARRAY_HPP
#define PERSISTENT_ARRAY_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class T, int Log = 3>
class PersistentArray {
	static_assert(1 <= Log && Log <= 5);
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { T value; std::array<root_type, 1 << Log> child{}; };
	int n_;
	root_type initial_ = 0;
	std::vector<Node> nodes;
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type clone(root_type root) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(nodes[root]);
		return result;
	}
	root_type update(root_type root, unsigned p, T value) {
		if (!p) return make({std::move(value), nodes[root].child});
		auto result = clone(root), current = result;
		while (p) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log;
			auto old = nodes[current].child[d];
			auto next = p ? clone(old) : make({std::move(value), nodes[old].child});
			nodes[current].child[d] = next;
			current = next;
		}
		return result;
	}
	root_type update_pair(root_type root, unsigned p, T a, unsigned q, T b) {
		if (p == q) return update(root, q, std::move(b));
		auto result = clone(root), current = result;
		while (p && q && (p & ((1 << Log) - 1)) == (q & ((1 << Log) - 1))) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log, q >>= Log;
			auto next = clone(nodes[current].child[d]);
			nodes[current].child[d] = next;
			current = next;
		}
		if (!p) nodes[current].value = std::move(a);
		else {
			unsigned d = p & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], p >> Log, std::move(a));
			nodes[current].child[d] = next;
		}
		if (!q) nodes[current].value = std::move(b);
		else {
			unsigned d = q & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], q >> Log, std::move(b));
			nodes[current].child[d] = next;
		}
		return result;
	}
	root_type build(const std::vector<T> &a, std::size_t p, std::size_t step) {
		Node node{a[p], {}};
		for (int d = 0; d < (1 << Log); ++d) {
			std::size_t next = p + step * d;
			if (next < a.size() && (d || step <= (a.size() - 1 - p) / (1 << Log)))
				node.child[d] = build(a, next, step * (1 << Log));
		}
		return make(std::move(node));
	}
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
  public:
	explicit PersistentArray(int n = 0, T value = T{}) : n_(n), nodes{Node{std::move(value), {}}} { assert(n >= 0); }
	explicit PersistentArray(const std::vector<T> &a, T default_value = T{}) : PersistentArray(0, std::move(default_value)) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) initial_ = build(a, 0, 1);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	T get(root_type root, int p) const {
		check(root); assert(0 <= p && p < n_);
		while (root && p) { root = nodes[root].child[p & ((1 << Log) - 1)]; p >>= Log; }
		return nodes[root].value;
	}
	root_type set(root_type root, int p, T value) {
		check(root); assert(0 <= p && p < n_);
		return update(root, static_cast<unsigned>(p), std::move(value));
	}
	root_type set_pair(root_type root, int p, T a, int q, T b) {
		check(root); assert(0 <= p && p < n_ && 0 <= q && q < n_);
		return update_pair(root, static_cast<unsigned>(p), std::move(a), static_cast<unsigned>(q), std::move(b));
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
