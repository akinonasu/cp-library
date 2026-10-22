#ifndef PERSISTENT_DEQUE_HPP
#define PERSISTENT_DEQUE_HPP

#include "../components/persistent_rope.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template <class T>
class PersistentDeque {
	static int weight(const T &) { return 1; }
	PersistentRope<T, weight> rope;
	void check([[maybe_unused]] int root) const { assert(0 <= root && static_cast<std::size_t>(root) <= rope.storage_nodes()); }
	static std::pair<T, T> split_value(const T &value, std::uint64_t) { assert(false); return {value, value}; }
  public:
	using value_type = T;
	using root_type = int;
	root_type initial_root() const { return 0; }
	int size(root_type root) const { check(root); assert(rope.size(root) <= std::uint64_t(std::numeric_limits<int>::max())); return static_cast<int>(rope.size(root)); }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type build(const std::vector<T> &a) { assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max())); return rope.build(a); }
	T get(root_type root, int p) const { assert(0 <= p && p < size(root)); return rope.at(root, p).first; }
	T front(root_type root) const { check(root); assert(!empty(root)); return rope.front(root); }
	T back(root_type root) const { check(root); assert(!empty(root)); return rope.back(root); }
	std::pair<root_type, root_type> split(root_type root, int p) {
		assert(0 <= p && p <= size(root));
		return rope.split(root, p, split_value);
	}
	root_type concat(root_type left, root_type right) {
		assert(size(left) <= std::numeric_limits<int>::max() - size(right));
		return rope.join(left, right);
	}
	root_type push_front(root_type root, T value) { return concat(rope.make(std::move(value)), root); }
	root_type push_back(root_type root, T value) { return concat(root, rope.make(std::move(value))); }
	root_type pop_front(root_type root) { check(root); assert(!empty(root)); return rope.pop_front(root).second; }
	root_type pop_back(root_type root) { check(root); assert(!empty(root)); return rope.pop_back(root).first; }
	root_type insert(root_type root, int p, T value) {
		assert(0 <= p && p <= size(root) && size(root) < std::numeric_limits<int>::max());
		return rope.insert(root, p, std::move(value), split_value);
	}
	root_type erase(root_type root, int l, int r) {
		assert(0 <= l && l <= r && r <= size(root));
		return rope.erase(root, l, r, split_value);
	}
	root_type set(root_type root, int p, T value) {
		assert(0 <= p && p < size(root));
		return rope.replace(root, p, std::move(value));
	}
	std::vector<T> to_vector(root_type root) const {
		std::vector<T> result;
		result.reserve(size(root));
		rope.for_each(root, [&](const T &x) { result.push_back(x); });
		return result;
	}
	std::size_t node_count() const { return rope.storage_nodes(); }
};

#endif
