#line 1 "test/persistent_queue.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#line 1 "data_structure/persistent/persistent_queue.hpp"



#line 1 "data_structure/persistent/persistent_stack.hpp"



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
	struct Node { std::optional<T> value; root_type next, jump; int size, jump_length; };
	std::vector<Node> nodes{{std::nullopt, 0, 0, 0, 0}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
  public:
	root_type initial_root() const { return 0; }
	int size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type push(root_type root, T value) {
		check(root);
		assert(size(root) < std::numeric_limits<int>::max() && nodes.size() < std::numeric_limits<root_type>::max());
		auto a = nodes[root].jump;
		int length = nodes[root].jump_length;
		bool combine = length == nodes[a].jump_length;
		auto jump = combine ? nodes[a].jump : root;
		Node node{std::move(value), root, jump, size(root) + 1, combine ? 2 * length + 1 : 1};
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type pop(root_type root) const { check(root); assert(root); return nodes[root].next; }
	T top(root_type root) const { check(root); assert(root); return *nodes[root].value; }
	root_type ancestor(root_type root, int count) const {
		check(root); assert(0 <= count && count <= size(root));
		if (count == size(root)) return 0;
		while (count) {
			int length = nodes[root].jump_length;
			if (length <= count) count -= length, root = nodes[root].jump;
			else --count, root = nodes[root].next;
		}
		return root;
	}
	T get(root_type root, int k) const { assert(0 <= k && k < size(root)); return top(ancestor(root, k)); }
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 8 "data_structure/persistent/persistent_queue.hpp"

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


#line 3 "test/persistent_queue.test.cpp"
#include <iostream>
#line 5 "test/persistent_queue.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	PersistentQueue<int> queue;
	queue.reserve(q);
	std::vector<decltype(queue)::root_type> roots(q + 1);
	for (int i = 1; i <= q; ++i) {
		int type, k;
		std::cin >> type >> k;
		if (type == 0) { int x; std::cin >> x; roots[i] = queue.push(roots[k + 1], x); }
		else { std::cout << queue.front(roots[k + 1]) << '\n'; roots[i] = queue.pop(roots[k + 1]); }
	}
}
