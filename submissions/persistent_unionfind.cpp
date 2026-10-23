#line 1 "test/persistent_union_find.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#line 1 "data_structure/persistent/persistent_union_find.hpp"



#line 1 "data_structure/persistent/persistent_array.hpp"



#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class T, int Log = 3>
class PersistentArray {
	static_assert(1 <= Log && Log <= 5);
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { T value; std::array<root_type, 1 << Log> child{}; };
	int n_;
	root_type initial_ = 0;
	std::vector<Node> nodes;
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type clone(root_type root) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(nodes[root]);
		return result;
	}
	root_type update(root_type root, unsigned p, T value) {
		if (!p) return make({std::move(value), nodes[root].child});
		auto result = clone(root), current = result;
		while (p) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log;
			auto old = nodes[current].child[d];
			auto next = p ? clone(old) : make({std::move(value), nodes[old].child});
			nodes[current].child[d] = next;
			current = next;
		}
		return result;
	}
	root_type update_pair(root_type root, unsigned p, T a, unsigned q, T b) {
		if (p == q) return update(root, q, std::move(b));
		auto result = clone(root), current = result;
		while (p && q && (p & ((1 << Log) - 1)) == (q & ((1 << Log) - 1))) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log, q >>= Log;
			auto next = clone(nodes[current].child[d]);
			nodes[current].child[d] = next;
			current = next;
		}
		if (!p) nodes[current].value = std::move(a);
		else {
			unsigned d = p & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], p >> Log, std::move(a));
			nodes[current].child[d] = next;
		}
		if (!q) nodes[current].value = std::move(b);
		else {
			unsigned d = q & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], q >> Log, std::move(b));
			nodes[current].child[d] = next;
		}
		return result;
	}
	root_type build(const std::vector<T> &a, std::size_t p, std::size_t step) {
		Node node{a[p], {}};
		for (int d = 0; d < (1 << Log); ++d) {
			std::size_t next = p + step * d;
			if (next < a.size() && (d || step <= (a.size() - 1 - p) / (1 << Log)))
				node.child[d] = build(a, next, step * (1 << Log));
		}
		return make(std::move(node));
	}
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
  public:
	explicit PersistentArray(int n = 0, T value = T{}) : n_(n), nodes{Node{std::move(value), {}}} { assert(n >= 0); }
	explicit PersistentArray(const std::vector<T> &a, T default_value = T{}) : PersistentArray(0, std::move(default_value)) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) {
			std::size_t count = a.size();
			for (auto rest = (a.size() - 1) >> Log; rest; rest >>= Log) count += rest;
			reserve(count);
			initial_ = build(a, 0, 1);
		}
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	T get(root_type root, int p) const {
		check(root); assert(0 <= p && p < n_);
		while (root && p) { root = nodes[root].child[p & ((1 << Log) - 1)]; p >>= Log; }
		return nodes[root].value;
	}
	root_type set(root_type root, int p, T value) {
		check(root); assert(0 <= p && p < n_);
		return update(root, static_cast<unsigned>(p), std::move(value));
	}
	root_type set_pair(root_type root, int p, T a, int q, T b) {
		check(root); assert(0 <= p && p < n_ && 0 <= q && q < n_);
		return update_pair(root, static_cast<unsigned>(p), std::move(a), static_cast<unsigned>(q), std::move(b));
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 9 "data_structure/persistent/persistent_union_find.hpp"

class PersistentUnionFind {
	PersistentArray<int> parent;

  public:
	struct root_type {
		PersistentArray<int>::root_type data = 0;
		int components = 0;
		bool operator==(const root_type &) const = default;
	};
  private:
	std::pair<int, int> leader_size(root_type root, int x) const {
		for (;;) {
			int p = parent.get(root.data, x);
			if (p < 0) return {x, p};
			x = p;
		}
	}
  public:
	explicit PersistentUnionFind(int n = 0) : parent(n, -1) {}
	int vertex_count() const { return parent.size(); }
	root_type initial_root() const { return {parent.initial_root(), vertex_count()}; }
	int leader(root_type root, int x) const { return leader_size(root, x).first; }
	bool same(root_type root, int a, int b) const { return leader(root, a) == leader(root, b); }
	int size(root_type root, int x) const { return -leader_size(root, x).second; }
	int component_count(root_type root) const { return root.components; }
	root_type merge(root_type root, int a, int b) {
		auto [u, x] = leader_size(root, a);
		auto [v, y] = leader_size(root, b);
		if (u == v) return root;
		if (x > y) { std::swap(u, v); std::swap(x, y); }
		root.data = parent.set_pair(root.data, u, x + y, v, u);
		--root.components;
		return root;
	}
	std::vector<std::vector<int>> groups(root_type root) const {
		std::vector<std::vector<int>> result(vertex_count());
		for (int i = 0; i < vertex_count(); ++i) result[leader(root, i)].push_back(i);
		std::vector<std::vector<int>> groups;
		for (auto &g : result) if (!g.empty()) groups.push_back(std::move(g));
		return groups;
	}
	void reserve(std::size_t count) { parent.reserve(count); }
	std::size_t node_count() const { return parent.node_count(); }
	std::size_t memory_usage() const { return parent.memory_usage(); }
};


#line 3 "test/persistent_union_find.test.cpp"
#include <iostream>
#line 5 "test/persistent_union_find.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	PersistentUnionFind uf(n);
	std::vector<PersistentUnionFind::root_type> roots(q + 1, uf.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type, k, a, b;
		std::cin >> type >> k >> a >> b;
		roots[i] = roots[k + 1];
		if (type == 0) roots[i] = uf.merge(roots[i], a, b);
		else std::cout << uf.same(roots[i], a, b) << '\n';
	}
}
