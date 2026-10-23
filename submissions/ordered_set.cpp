#line 1 "test/persistent_multiset.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#line 1 "data_structure/persistent/persistent_multiset.hpp"



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
class PersistentMultiset {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { std::optional<T> value; root_type left, right; std::int64_t count, size; int height; };
	std::vector<Node> nodes{{std::nullopt, 0, 0, 0, 0, 0}};
	Compare comp;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(T value, root_type left, root_type right, std::int64_t count) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		Node node{std::move(value), left, right, count, count + size(left) + size(right), std::max(nodes[left].height, nodes[right].height) + 1};
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type rebuild(root_type root, root_type left, root_type right, std::int64_t count) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(nodes[root]);
		auto &node = nodes.back();
		node.left = left; node.right = right; node.count = count;
		node.size = count + nodes[left].size + nodes[right].size;
		node.height = std::max(nodes[left].height, nodes[right].height) + 1;
		return result;
	}
	root_type balance(root_type root, root_type left, root_type right, std::int64_t count) {
		if (nodes[left].height > nodes[right].height + 1) {
			auto a = nodes[left].left, b = nodes[left].right;
			if (nodes[a].height >= nodes[b].height) {
				auto r = rebuild(root, b, right, count);
				return rebuild(left, a, r, nodes[left].count);
			}
			auto l = rebuild(left, a, nodes[b].left, nodes[left].count);
			auto r = rebuild(root, nodes[b].right, right, count);
			return rebuild(b, l, r, nodes[b].count);
		}
		if (nodes[right].height > nodes[left].height + 1) {
			auto a = nodes[right].left, b = nodes[right].right;
			if (nodes[b].height >= nodes[a].height) {
				auto l = rebuild(root, left, a, count);
				return rebuild(right, l, b, nodes[right].count);
			}
			auto l = rebuild(root, left, nodes[a].left, count);
			auto r = rebuild(right, nodes[a].right, b, nodes[right].count);
			return rebuild(a, l, r, nodes[a].count);
		}
		return rebuild(root, left, right, count);
	}
	root_type erase_first(root_type root) {
		auto left = nodes[root].left, right = nodes[root].right;
		if (!left) return right;
		left = erase_first(left);
		return balance(root, left, right, nodes[root].count);
	}
	root_type update(root_type root, const T &value, std::int64_t count, bool remove) {
		if (!root) return remove ? 0 : make(value, 0, 0, count);
		auto left = nodes[root].left, right = nodes[root].right;
		auto next_count = nodes[root].count;
		if (comp(value, *nodes[root].value)) left = update(left, value, count, remove);
		else if (comp(*nodes[root].value, value)) right = update(right, value, count, remove);
		else {
			if (!remove) next_count += count;
			else if (count < next_count) next_count -= count;
			else {
				if (!left || !right) return left ? left : right;
				auto successor = right;
				while (nodes[successor].left) successor = nodes[successor].left;
				right = erase_first(right);
				return balance(successor, left, right, nodes[successor].count);
			}
		}
		if (left == nodes[root].left && right == nodes[root].right && next_count == nodes[root].count) return root;
		return balance(root, left, right, next_count);
	}

  public:
	explicit PersistentMultiset(Compare compare = Compare{}) : comp(std::move(compare)) {}
	root_type initial_root() const { return 0; }
	root_type build(std::vector<T> values) {
		assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()));
		std::sort(values.begin(), values.end(), comp);
		std::vector<std::pair<T, std::int64_t>> distinct;
		for (const T &value : values) {
			if (distinct.empty() || comp(distinct.back().first, value)) distinct.emplace_back(value, 1);
			else ++distinct.back().second;
		}
		auto visit = [&](auto &&self, std::size_t l, std::size_t r) -> root_type {
			if (l == r) return 0;
			auto m = l + (r - l) / 2;
			auto left = self(self, l, m), right = self(self, m + 1, r);
			return make(distinct[m].first, left, right, distinct[m].second);
		};
		return visit(visit, 0, distinct.size());
	}
	std::int64_t size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	std::int64_t count(root_type root, const T &value) const {
		check(root);
		while (root) {
			const auto &node = nodes[root];
			if (comp(value, *node.value)) root = node.left;
			else if (comp(*node.value, value)) root = node.right;
			else return node.count;
		}
		return 0;
	}
	bool contains(root_type root, const T &value) const { return count(root, value) != 0; }
	root_type insert(root_type root, const T &value, std::int64_t count = 1) {
		assert(count >= 0 && size(root) <= std::numeric_limits<std::int64_t>::max() - count);
		return count ? update(root, value, count, false) : root;
	}
	root_type erase(root_type root, const T &value, std::int64_t count = 1) {
		check(root); assert(count >= 0);
		return count ? update(root, value, count, true) : root;
	}
	root_type erase_all(root_type root, const T &value) { return erase(root, value, std::numeric_limits<std::int64_t>::max()); }
	T kth(root_type root, std::int64_t k) const {
		assert(0 <= k && k < size(root));
		while (true) {
			const auto &node = nodes[root];
			auto left = size(node.left);
			if (k < left) root = node.left;
			else if (k < left + node.count) return *node.value;
			else { k -= left + node.count; root = node.right; }
		}
	}
	std::int64_t order_of_key(root_type root, const T &value) const {
		check(root);
		std::int64_t result = 0;
		while (root) {
			const auto &node = nodes[root];
			if (comp(*node.value, value)) { result += size(node.left) + node.count; root = node.right; }
			else root = node.left;
		}
		return result;
	}
	std::optional<T> lower_bound(root_type root, const T &value) const {
		check(root);
		std::optional<T> result;
		while (root) {
			const auto &node = nodes[root];
			if (!comp(*node.value, value)) { result = node.value; root = node.left; }
			else root = node.right;
		}
		return result;
	}
	std::optional<T> upper_bound(root_type root, const T &value) const {
		check(root);
		std::optional<T> result;
		while (root) {
			const auto &node = nodes[root];
			if (comp(value, *node.value)) { result = node.value; root = node.left; }
			else root = node.right;
		}
		return result;
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 3 "test/persistent_multiset.test.cpp"
#include <iostream>
#line 5 "test/persistent_multiset.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	PersistentMultiset<int> set;
	std::vector<int> a(n);
	for (int &x : a) std::cin >> x;
	auto root = set.build(a);
	while (q--) {
		int type, x;
		std::cin >> type >> x;
		if (type == 0) { if (!set.contains(root, x)) root = set.insert(root, x); }
		if (type == 1) root = set.erase_all(root, x);
		if (type == 2) std::cout << (set.size(root) < x ? -1 : set.kth(root, x - 1)) << '\n';
		if (type == 3) std::cout << set.order_of_key(root, x) + set.count(root, x) << '\n';
		if (type == 4) { auto k = set.order_of_key(root, x) + set.count(root, x); std::cout << (k ? set.kth(root, k - 1) : -1) << '\n'; }
		if (type == 5) std::cout << set.lower_bound(root, x).value_or(-1) << '\n';
	}
}
