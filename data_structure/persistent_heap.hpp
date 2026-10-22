#ifndef PERSISTENT_HEAP_HPP
#define PERSISTENT_HEAP_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

template <class T, class Compare = std::less<T>>
class PersistentHeap {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { std::optional<T> value; root_type left, right; int rank; std::int64_t size; };
	std::vector<Node> nodes{{std::nullopt, 0, 0, 0, 0}};
	Compare comp;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
  public:
	explicit PersistentHeap(Compare compare = Compare{}) : comp(std::move(compare)) {}
	root_type initial_root() const { return 0; }
	std::int64_t size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	T top(root_type root) const { check(root); assert(root); return *nodes[root].value; }
	root_type meld(root_type a, root_type b) {
		check(a); check(b);
		if (!a || !b) return a ? a : b;
		assert(size(a) <= std::numeric_limits<std::int64_t>::max() - size(b));
		if (comp(*nodes[b].value, *nodes[a].value)) std::swap(a, b);
		Node next = nodes[a];
		next.right = meld(next.right, b);
		if (nodes[next.left].rank < nodes[next.right].rank) std::swap(next.left, next.right);
		next.rank = nodes[next.right].rank + 1;
		next.size = 1 + size(next.left) + size(next.right);
		return make(std::move(next));
	}
	root_type push(root_type root, T value) { check(root); return meld(root, make({std::move(value), 0, 0, 1, 1})); }
	root_type pop(root_type root) { check(root); assert(root); return meld(nodes[root].left, nodes[root].right); }
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
