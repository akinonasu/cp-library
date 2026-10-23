#line 1 "test/persistent_deque.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/deque"
#line 1 "data_structure/persistent/persistent_deque.hpp"



#line 1 "components/persistent_rope.hpp"



#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template<class T, auto weight> class PersistentRope {
	struct Node {
		std::uint64_t size;
		int left, right, height;
		T value;
	};
	std::vector<Node> nodes_{{0, 0, 0, 0, T{}}};
	int height(int v) const { return nodes_[v].height; }
	int append(Node node) {
		assert(nodes_.size() < std::size_t(std::numeric_limits<int>::max()));
		nodes_.push_back(std::move(node));
		return static_cast<int>(nodes_.size()) - 1;
	}
	int branch(int a, int b) {
		if (!a) return b;
		if (!b) return a;
		assert(size(a) <= std::numeric_limits<std::uint64_t>::max() - size(b));
		return append({size(a) + size(b), a, b, std::max(height(a), height(b)) + 1, T{}});
	}
	int balance(int a, int b) {
		if (height(a) > height(b) + 1) {
			int left = nodes_[a].left, right = nodes_[a].right;
			if (height(left) >= height(right)) return branch(left, branch(right, b));
			int middle_left = nodes_[right].left, middle_right = nodes_[right].right;
			return branch(branch(left, middle_left), branch(middle_right, b));
		}
		if (height(b) > height(a) + 1) {
			int left = nodes_[b].left, right = nodes_[b].right;
			if (height(right) >= height(left)) return branch(branch(a, left), right);
			int middle_left = nodes_[left].left, middle_right = nodes_[left].right;
			return branch(branch(a, middle_left), branch(middle_right, right));
		}
		return branch(a, b);
	}

  public:
	std::uint64_t size(int v) const { return nodes_[v].size; }
	std::size_t storage_nodes() const { return nodes_.size() - 1; }
	bool leaf(int v) const { return v && !nodes_[v].left; }
	T value(int v) const { assert(leaf(v)); return nodes_[v].value; }
	int make(T value) {
		auto length = std::uint64_t(weight(value));
		assert(length);
		return append({length, 0, 0, 1, std::move(value)});
	}
	int join(int a, int b) {
		if (!a || !b) return a ? a : b;
		if (height(a) > height(b) + 1) {
			int left = nodes_[a].left, right = nodes_[a].right;
			return balance(left, join(right, b));
		}
		if (height(b) > height(a) + 1) {
			int left = nodes_[b].left, right = nodes_[b].right;
			return balance(join(a, left), right);
		}
		return branch(a, b);
	}
	int build(const std::vector<T> &values) {
		auto visit = [&](auto &&self, std::size_t l, std::size_t r) -> int {
			if (l == r) return 0;
			if (r - l == 1) return make(values[l]);
			auto m = l + (r - l) / 2;
			int a = self(self, l, m), b = self(self, m, r);
			return branch(a, b);
		};
		return visit(visit, 0, values.size());
	}
	template<class Splitter> std::pair<int, int> split(int v, std::uint64_t p, const Splitter &splitter) {
		assert(p <= size(v));
		if (!p) return {0, v};
		if (p == size(v)) return {v, 0};
		int left = nodes_[v].left, right = nodes_[v].right;
		if (!left) {
			T value = nodes_[v].value;
			auto [a, b] = splitter(value, p);
			int l = make(std::move(a)), r = make(std::move(b));
			return {l, r};
		}
		auto k = size(left);
		if (p == k) return {left, right};
		if (p < k) {
			auto [a, b] = split(left, p, splitter);
			return {a, join(b, right)};
		}
		auto [a, b] = split(right, p - k, splitter);
		return {join(left, a), b};
	}
	int replace(int v, std::uint64_t p, T value) {
		assert(p < size(v));
		int a = nodes_[v].left, b = nodes_[v].right;
		if (!a) { assert(std::uint64_t(weight(value)) == size(v)); return make(std::move(value)); }
		if (p < size(a)) a = replace(a, p, std::move(value));
		else b = replace(b, p - size(a), std::move(value));
		return branch(a, b);
	}
	template<class Splitter> int insert(int v, std::uint64_t p, T value, const Splitter &splitter) {
		assert(p <= size(v));
		if (!v) return make(std::move(value));
		int a = nodes_[v].left, b = nodes_[v].right;
		if (!a) {
			if (!p) return branch(make(std::move(value)), v);
			if (p == size(v)) return branch(v, make(std::move(value)));
			auto [left, right] = split(v, p, splitter);
			int middle = make(std::move(value));
			return join(join(left, middle), right);
		}
		if (p <= size(a)) a = insert(a, p, std::move(value), splitter);
		else b = insert(b, p - size(a), std::move(value), splitter);
		return balance(a, b);
	}
	template<class Splitter> int erase(int v, std::uint64_t l, std::uint64_t r, const Splitter &splitter) {
		assert(l <= r && r <= size(v));
		if (l == r) return v;
		if (!l && r == size(v)) return 0;
		int a = nodes_[v].left, b = nodes_[v].right;
		if (!a) {
			if (!l) return split(v, r, splitter).second;
			if (r == size(v)) return split(v, l, splitter).first;
			auto [left, rest] = split(v, l, splitter);
			int right = split(rest, r - l, splitter).second;
			return join(left, right);
		}
		auto k = size(a);
		if (r <= k) a = erase(a, l, r, splitter);
		else if (l >= k) b = erase(b, l - k, r - k, splitter);
		else {
			a = erase(a, l, k, splitter);
			b = erase(b, 0, r - k, splitter);
		}
		return join(a, b);
	}

	T front(int v) const { assert(v); while (nodes_[v].left) v = nodes_[v].left; return value(v); }
	T back(int v) const { assert(v); while (nodes_[v].right) v = nodes_[v].right; return value(v); }
	std::pair<T, int> pop_front(int v) {
		assert(v);
		int left = nodes_[v].left, right = nodes_[v].right;
		if (!left) return {value(v), 0};
		auto [value, rest] = pop_front(left);
		return {std::move(value), join(rest, right)};
	}
	std::pair<int, T> pop_back(int v) {
		assert(v);
		int left = nodes_[v].left, right = nodes_[v].right;
		if (!right) return {0, value(v)};
		auto [rest, value] = pop_back(right);
		return {join(left, rest), std::move(value)};
	}
	std::pair<T, std::uint64_t> at(int v, std::uint64_t p) const {
		assert(p < size(v));
		while (int left = nodes_[v].left) {
			int right = nodes_[v].right;
			auto length = size(left);
			if (p < length) v = left;
			else p -= length, v = right;
		}
		return {value(v), p};
	}
	template<class Visitor> void for_each(int v, const Visitor &visitor) const {
		if (!v) return;
		if (leaf(v)) { visitor(value(v)); return; }
		for_each(nodes_[v].left, visitor);
		for_each(nodes_[v].right, visitor);
	}
	int copy_from(const PersistentRope &other, int root) {
		assert(this != &other);
		std::vector<int> mapped(other.nodes_.size(), 0);
		auto visit = [&](auto &&self, int v) -> int {
			if (!v || mapped[v]) return mapped[v];
			auto x = other.nodes_[v];
			if (x.left) {
				x.left = self(self, x.left);
				x.right = self(self, x.right);
			}
			return mapped[v] = append(std::move(x));
		};
		return visit(visit, root);
	}
};


#line 6 "data_structure/persistent/persistent_deque.hpp"
#include <cstddef>
#line 10 "data_structure/persistent/persistent_deque.hpp"

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


#line 3 "test/persistent_deque.test.cpp"
#include <iostream>

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	std::cin >> q;
	PersistentDeque<int> deque;
	auto root = deque.initial_root();
	while (q--) {
		int type, x;
		std::cin >> type;
		if (type == 0) { std::cin >> x; root = deque.push_front(root, x); }
		if (type == 1) { std::cin >> x; root = deque.push_back(root, x); }
		if (type == 2) root = deque.pop_front(root);
		if (type == 3) root = deque.pop_back(root);
		if (type == 4) { std::cin >> x; std::cout << deque.get(root, x) << '\n'; }
	}
}
