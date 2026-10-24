#ifndef PERSISTENT_STACK_HPP
#define PERSISTENT_STACK_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T>
class PersistentStack {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	using Stored = std::conditional_t<std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>, T, std::optional<T>>;
	struct Node { Stored value; root_type next, jump; int size, jump_length; };
	std::vector<Node> nodes{{{}, 0, 0, 0, 0}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	const T &value_at(root_type root) const {
		if constexpr (std::is_same_v<Stored, T>) return nodes[root].value;
		else return *nodes[root].value;
	}
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
	T top(root_type root) const { check(root); assert(root); return value_at(root); }
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
	std::vector<T> get_many(const std::vector<std::pair<root_type, int>> &queries) const {
		assert(queries.size() <= std::size_t(std::numeric_limits<int>::max()));
		if (queries.empty()) return {};
		std::uint64_t work = 0;
		unsigned limit = std::bit_width(nodes.size());
		for (auto [root, k] : queries) { check(root); assert(0 <= k && k < size(root)); work += unsigned(k) < limit ? unsigned(k) : limit; }
		if (work <= 3 * std::uint64_t(nodes.size())) {
			std::vector<T> result;
			result.reserve(queries.size());
			for (auto [root, k] : queries) result.push_back(get(root, k));
			return result;
		}
		std::vector<root_type> first(nodes.size()), next(nodes.size()), selected(queries.size()), path;
		std::vector<int> head(nodes.size(), -1), following(queries.size());
		for (root_type v = 1; v < nodes.size(); ++v) {
			root_type p = nodes[v].next;
			next[v] = first[p]; first[p] = v;
		}
		for (int i = 0; i < int(queries.size()); ++i) {
			root_type root = queries[i].first;
			following[i] = head[root]; head[root] = i;
		}
		for (root_type v = first[0]; v;) {
			path.push_back(v);
			for (int i = head[v]; i != -1; i = following[i]) selected[i] = path[path.size() - 1 - queries[i].second];
			if (first[v]) { v = first[v]; continue; }
			while (v && !next[v]) { path.pop_back(); v = nodes[v].next; }
			if (v) { path.pop_back(); v = next[v]; }
		}
		std::vector<T> result;
		result.reserve(queries.size());
		for (root_type v : selected) result.push_back(value_at(v));
		return result;
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};

#endif
