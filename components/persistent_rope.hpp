#ifndef PERSISTENT_ROPE_HPP
#define PERSISTENT_ROPE_HPP

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
			auto x = nodes_[a];
			if (height(x.left) >= height(x.right)) return branch(x.left, branch(x.right, b));
			auto y = nodes_[x.right];
			return branch(branch(x.left, y.left), branch(y.right, b));
		}
		if (height(b) > height(a) + 1) {
			auto x = nodes_[b];
			if (height(x.right) >= height(x.left)) return branch(branch(a, x.left), x.right);
			auto y = nodes_[x.left];
			return branch(branch(a, y.left), branch(y.right, x.right));
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
			auto x = nodes_[a];
			return balance(x.left, join(x.right, b));
		}
		if (height(b) > height(a) + 1) {
			auto x = nodes_[b];
			return balance(join(a, x.left), x.right);
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
		auto x = nodes_[v];
		if (!x.left) {
			auto [a, b] = splitter(x.value, p);
			int l = make(std::move(a)), r = make(std::move(b));
			return {l, r};
		}
		auto k = size(x.left);
		if (p == k) return {x.left, x.right};
		if (p < k) {
			auto [a, b] = split(x.left, p, splitter);
			return {a, join(b, x.right)};
		}
		auto [a, b] = split(x.right, p - k, splitter);
		return {join(x.left, a), b};
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
		auto x = nodes_[v];
		if (!x.left) return {x.value, 0};
		auto [value, rest] = pop_front(x.left);
		return {value, join(rest, x.right)};
	}
	std::pair<int, T> pop_back(int v) {
		assert(v);
		auto x = nodes_[v];
		if (!x.right) return {0, x.value};
		auto [rest, value] = pop_back(x.right);
		return {join(x.left, rest), value};
	}
	std::pair<T, std::uint64_t> at(int v, std::uint64_t p) const {
		assert(p < size(v));
		while (!leaf(v)) {
			auto x = nodes_[v];
			if (p < size(x.left)) v = x.left;
			else p -= size(x.left), v = x.right;
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

#endif
