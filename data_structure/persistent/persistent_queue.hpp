#ifndef PERSISTENT_QUEUE_HPP
#define PERSISTENT_QUEUE_HPP

#include "persistent_stack.hpp"
#include <cassert>
#include <cstddef>
#include <utility>

template <class T>
class PersistentQueue {
	PersistentStack<T> stack;
  public:
	using value_type = T;
	struct root_type {
		typename PersistentStack<T>::root_type tail = 0;
		int length = 0;
		bool operator==(const root_type &) const = default;
	};
	root_type initial_root() const { return {}; }
	int size(root_type root) const { return root.length; }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type push(root_type root, T value) { return {stack.push(root.tail, std::move(value)), root.length + 1}; }
	root_type pop(root_type root) const {
		assert(!empty(root));
		return root.length == 1 ? root_type{} : root_type{root.tail, root.length - 1};
	}
	T front(root_type root) const { assert(!empty(root)); return stack.get(root.tail, root.length - 1); }
	T back(root_type root) const { assert(!empty(root)); return stack.top(root.tail); }
	T get(root_type root, int p) const { assert(0 <= p && p < size(root)); return stack.get(root.tail, root.length - 1 - p); }
	void reserve(std::size_t count) { stack.reserve(count); }
	std::size_t node_count() const { return stack.node_count(); }
	std::size_t memory_usage() const { return stack.memory_usage(); }
};

#endif
