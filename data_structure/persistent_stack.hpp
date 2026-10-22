#ifndef PERSISTENT_STACK_HPP
#define PERSISTENT_STACK_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

template <class T>
class PersistentStack {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { std::optional<T> value; root_type next, jump; int size; };
	std::vector<Node> nodes{{std::nullopt, 0, 0, 0}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
  public:
	root_type initial_root() const { return 0; }
	int size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type push(root_type root, T value) {
		check(root);
		assert(size(root) < std::numeric_limits<int>::max() && nodes.size() < std::numeric_limits<root_type>::max());
		auto a = nodes[root].jump, b = nodes[a].jump;
		auto jump = size(root) - size(a) == size(a) - size(b) ? b : root;
		Node node{std::move(value), root, jump, size(root) + 1};
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type pop(root_type root) const { check(root); assert(root); return nodes[root].next; }
	T top(root_type root) const { check(root); assert(root); return *nodes[root].value; }
	root_type ancestor(root_type root, int count) const {
		check(root); assert(0 <= count && count <= size(root));
		int target = size(root) - count;
		while (size(root) > target) {
			auto jump = nodes[root].jump;
			root = size(jump) >= target ? jump : nodes[root].next;
		}
		return root;
	}
	T get(root_type root, int k) const { assert(0 <= k && k < size(root)); return top(ancestor(root, k)); }
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
