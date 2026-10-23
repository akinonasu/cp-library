#ifndef LINK_CUT_TREE_HPP
#define LINK_CUT_TREE_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, bool commutative = false>
class LinkCutTree {
	static constexpr bool scalar = commutative && std::is_arithmetic_v<S>;
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int ch[2] = {}, parent = 0;
		std::uint32_t count : 31 = 1;
		std::uint32_t rev : 1 = 0;
		S value, prod;
		[[no_unique_address]] Back back;
		static Back make_back(const S &v) {
			if constexpr (commutative) return {};
			else return v;
		}
		explicit Node(const S &v) : value(v), prod(v), back(make_back(v)) {}
	};
	std::vector<Node> t;
	void init(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		t.reserve(std::size_t(n) + 1);
		t.emplace_back(e());
		t[0].count = 0;
	}
	int node(int v) const {
		assert(0 <= v && v < size());
		return v + 1;
	}
	bool is_root(int x) const { return t[x].parent <= 0; }
	void pull(int x) {
		auto &v = t[x];
		int l = v.ch[0], r = v.ch[1];
		v.count = t[l].count + t[r].count + 1;
		if constexpr (scalar) v.prod = op(op(t[l].prod, v.value), t[r].prod);
		else {
			v.prod = l ? op(t[l].prod, v.value) : v.value;
			if (r) v.prod = op(std::move(v.prod), t[r].prod);
		}
		if constexpr (!commutative) {
			v.back = r ? op(t[r].back, v.value) : v.value;
			if (l) v.back = op(std::move(v.back), t[l].back);
		}
	}
	void flip(int x) {
		if constexpr (!scalar) if (!x) return;
		std::swap(t[x].ch[0], t[x].ch[1]);
		if constexpr (!commutative) std::ranges::swap(t[x].prod, t[x].back);
		t[x].rev ^= 1;
	}
	void push(int x) {
		if (!t[x].rev) return;
		flip(t[x].ch[0]);
		flip(t[x].ch[1]);
		t[x].rev = 0;
	}
	void rotate(int x) {
		int p = t[x].parent, g = t[p].parent;
		int d = t[p].ch[1] == x, c = t[x].ch[d ^ 1];
		if (g > 0) t[g].ch[t[g].ch[1] == p] = x;
		t[x].parent = g;
		t[p].ch[d] = c;
		t[c].parent = p;
		t[x].ch[d ^ 1] = p;
		t[p].parent = x;
		pull(p);
	}
	void splay(int x) {
		if (is_root(x)) {
			push(x);
			return;
		}
		do {
			int p = t[x].parent, g = t[p].parent;
			bool top = is_root(p);
			if (!top) push(g);
			push(p);
			push(x);
			if (!top) rotate((t[p].ch[0] == x) == (t[g].ch[0] == p) ? p : x);
			rotate(x);
		} while (!is_root(x));
	}

	int access(int x) {
		splay(x);
		if (t[x].ch[1]) t[t[x].ch[1]].parent = -x;
		t[x].ch[1] = 0;
		int last = x;
		while (t[x].parent) {
			int p = -t[x].parent;
			splay(p);
			if (t[p].ch[1]) t[t[p].ch[1]].parent = -p;
			t[p].ch[1] = x;
			t[x].parent = p;
			rotate(x);
			last = p;
		}
		return last;
	}
	void reroot(int x) {
		access(x);
		flip(x);
	}
	int expose(int u, int v) {
		u = node(u), v = node(v);
		reroot(u);
		access(v);
		assert(u == v || t[u].parent != 0);
		return v;
	}

  public:
	explicit LinkCutTree(int n = 0) {
		init(n);
		for (int repeat_index = 0; repeat_index < n; ++repeat_index) t.emplace_back(t[0].value);
	}
	explicit LinkCutTree(const std::vector<S> &values) {
		assert(values.size() < std::size_t(std::numeric_limits<int>::max()));
		init(static_cast<int>(values.size()));
		for (const S &v : values) t.emplace_back(v);
	}
	int size() const { return t.empty() ? 0 : static_cast<int>(t.size()) - 1; }
	void build_forest(std::span<const std::pair<int, int>> edges) {
		assert(edges.size() < std::size_t(size()) || (edges.empty() && size() == 0));
		std::vector<int> degree(size()), neighbors(size()), parent(size(), -1), order;
		order.reserve(size());
		for (auto [u, v] : edges) {
			assert(0 <= u && u < size() && 0 <= v && v < size() && u != v);
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		for (int v = 0; v < size(); ++v) if (degree[v] <= 1) order.push_back(v);
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i];
			if (!degree[v]) continue;
			int p = neighbors[v];
			parent[v] = p;
			degree[v] = 0;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order.push_back(p);
		}
		assert(order.size() == std::size_t(size()));
		for (int v = 0; v < size(); ++v) {
			auto &x = t[v + 1];
			x.ch[0] = x.ch[1] = 0;
			x.parent = -parent[v] - 1;
			x.count = 1, x.rev = 0;
			x.prod = x.value;
			if constexpr (!commutative) x.back = x.value;
		}
	}
	S get(int v) const { return t[node(v)].value; }
	void set(int v, S value) {
		int x = node(v);
		splay(x);
		t[x].value = std::move(value);
	}
	void make_root(int v) { reroot(node(v)); }
	int root(int v) {
		int x = node(v);
		access(x);
		push(x);
		while (t[x].ch[0]) push(x = t[x].ch[0]);
		splay(x);
		return x - 1;
	}
	bool same(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return true;
		access(u);
		access(v);
		return t[u].parent != 0;
	}
	bool link(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return false;
		reroot(u);
		access(v);
		if (t[u].parent) return false;
		t[u].parent = -v;
		return true;
	}
	bool cut(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return false;
		reroot(u);
		access(v);
		if (t[v].ch[0] != u || t[u].ch[1]) return false;
		t[v].ch[0] = t[u].parent = 0;
		return true;
	}
	S prod(int u, int v) {
		int x = expose(u, v), l = t[x].ch[0];
		return l ? op(t[l].prod, t[x].value) : t[x].value;
	}
	int path_size(int u, int v) {
		int x = expose(u, v);
		return t[t[x].ch[0]].count + 1;
	}
	int distance(int u, int v) { return path_size(u, v) - 1; }
	int jump(int u, int v, int k) {
		int x = expose(u, v);
		if (k < 0 || k > int(t[t[x].ch[0]].count)) return -1;
		for (;;) {
			push(x);
			int l = t[x].ch[0], count = t[l].count;
			if (k == count) break;
			if (k < count) x = l;
			else k -= count + 1, x = t[x].ch[1];
		}
		splay(x);
		return x - 1;
	}
	int lca(int r, int u, int v) {
		r = node(r), u = node(u), v = node(v);
		reroot(r);
		access(u);
		assert(r == u || t[r].parent != 0);
		int w = access(v);
		assert(u == v || t[u].parent != 0);
		return w - 1;
	}
};

#endif
