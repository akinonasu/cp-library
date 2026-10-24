#ifndef PERSISTENT_HEAP_HPP
#define PERSISTENT_HEAP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, class Compare = std::less<T>>
class PersistentHeap {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	using Stored = std::conditional_t<std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>, T, std::optional<T>>;
	struct Node { Stored value; root_type left, right; int rank; std::int64_t size; };
	std::vector<Node> nodes{{{}, 0, 0, 0, 0}};
	Compare comp;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	const T &value_at(root_type root) const {
		if constexpr (std::is_same_v<Stored, T>) return nodes[root].value;
		else return *nodes[root].value;
	}
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type rebuild(root_type root, root_type right) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(nodes[root]);
		auto& node = nodes.back();
		node.right = right;
		if (nodes[node.left].rank < nodes[node.right].rank) std::swap(node.left, node.right);
		node.rank = nodes[node.right].rank + 1;
		node.size = 1 + nodes[node.left].size + nodes[node.right].size;
		return result;
	}
  public:
	explicit PersistentHeap(Compare compare = Compare{}) : comp(std::move(compare)) {}
	root_type initial_root() const { return 0; }
	std::int64_t size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	T top(root_type root) const { check(root); assert(root); return value_at(root); }
	root_type meld(root_type a, root_type b) {
		check(a); check(b);
		if (!a || !b) return a ? a : b;
		assert(size(a) <= std::numeric_limits<std::int64_t>::max() - size(b));
		std::array<root_type, 128> path;
		int depth = 0;
		while (a && b) {
			if (comp(value_at(b), value_at(a))) std::swap(a, b);
			path[depth++] = a;
			a = nodes[a].right;
		}
		auto result = a ? a : b;
		while (depth) result = rebuild(path[--depth], result);
		return result;
	}
	root_type push(root_type root, T value) {
		check(root);
		assert(size(root) < std::numeric_limits<std::int64_t>::max());
		std::array<root_type, 64> path;
		int depth = 0;
		while (root && !comp(value, value_at(root))) {
			path[depth++] = root;
			root = nodes[root].right;
		}
		auto result = make({std::move(value), root, 0, 1, size(root) + 1});
		while (depth) result = rebuild(path[--depth], result);
		return result;
	}
	root_type pop(root_type root) { check(root); assert(root); return meld(nodes[root].left, nodes[root].right); }
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
