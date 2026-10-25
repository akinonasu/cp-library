#ifndef PERSISTENT_QUEUE_HPP
#define PERSISTENT_QUEUE_HPP

#include "persistent_stack.hpp"
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

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
	std::vector<T> front_many(const std::vector<root_type> &roots) const {
		assert(roots.size() <= std::size_t(std::numeric_limits<int>::max()));
		std::uint64_t work = 0;
		unsigned limit = std::bit_width(stack.node_count());
		for (auto root : roots) { assert(!empty(root)); unsigned distance = unsigned(root.length - 1); work += distance < limit ? distance : limit; }
		if (work <= 3 * std::uint64_t(stack.node_count())) {
			std::vector<T> result;
			result.reserve(roots.size());
			for (auto root : roots) result.push_back(front(root));
			return result;
		}
		std::vector<std::pair<typename PersistentStack<T>::root_type, int>> queries;
		queries.reserve(roots.size());
		for (auto root : roots) { assert(!empty(root)); queries.emplace_back(root.tail, root.length - 1); }
		return stack.get_many(queries);
	}
	void reserve(std::size_t count) { stack.reserve(count); }
	std::size_t node_count() const { return stack.node_count(); }
	std::size_t memory_usage() const { return stack.memory_usage(); }
};

#endif
