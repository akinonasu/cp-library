#line 1 "test/lowest_common_ancestor.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#line 1 "tree/lca.hpp"



#line 1 "data_structure/range_query/sparse_table.hpp"



#line 1 "components/default_init_allocator.hpp"



#include <memory>
#include <new>

template <class T>
struct DefaultInitAllocator : std::allocator<T> {
	template <class U>
	struct rebind {
		using other = DefaultInitAllocator<U>;
	};
	DefaultInitAllocator() = default;
	template <class U>
	DefaultInitAllocator(const DefaultInitAllocator<U> &) noexcept {}
	template <class U>
	void construct(U *p) { ::new (static_cast<void *>(p)) U; }
};


#line 5 "data_structure/range_query/sparse_table.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e = nullptr>
class SparseTable {
	using Value = std::conditional_t<std::is_same_v<S, bool>, std::uint8_t, S>;
	using Reference = std::conditional_t<std::is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	std::vector<std::vector<Value, DefaultInitAllocator<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (std::is_same_v<S, bool> || !std::is_invocable_v<decltype(op), const S &, const S &>) return S(op(S(a), S(b)));
		else return op(a, b);
	}

  public:
	SparseTable() = default;
	SparseTable(const SparseTable &) = default;
	SparseTable &operator=(const SparseTable &) = default;
	SparseTable(SparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	SparseTable &operator=(SparseTable &&other) noexcept {
		if (this != &other) {
			n = std::exchange(other.n, 0);
			data = std::move(other.data);
		}
		return *this;
	}
	explicit SparseTable(const std::vector<S> &a) { build(a); }
	void clear() {
		data.clear();
		n = 0;
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		build(static_cast<int>(a.size()), [&](int i) -> S { return a[i]; });
	}
	template <class F>
	void build(int size, F value) {
		assert(size >= 0);
		n = size;
		data.resize(std::bit_width(unsigned(n)));
		if (!n) return;
		if constexpr (std::is_default_constructible_v<Value>) {
			data[0].resize(n);
			for (int i = 0; i < n; ++i) data[0][i] = value(i);
		} else {
			data[0].clear();
			data[0].reserve(n);
			for (int i = 0; i < n; ++i) data[0].push_back(value(i));
		}
		for (int k = 1; k < std::bit_width(unsigned(n)); ++k) {
			int length = n - (1U << k) + 1, half = 1U << (k - 1);
			if constexpr (std::is_default_constructible_v<Value>) {
				data[k].resize(length);
				for (int i = 0; i < length; ++i) data[k][i] = combine(data[k - 1][i], data[k - 1][i + half]);
			} else {
				data[k].clear();
				data[k].reserve(length);
				for (int i = 0; i < length; ++i) data[k].push_back(combine(data[k - 1][i], data[k - 1][i + half]));
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	Reference operator[](int i) const {
		assert(0 <= i && i < n);
		return data[0][i];
	}
	[[gnu::always_inline]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if constexpr (std::is_same_v<decltype(e), std::nullptr_t>) assert(l < r);
		else if (l == r) return e();
		return prod_unchecked(l, r);
	}
	[[gnu::always_inline]] S prod_unchecked(int l, int r) const {
		int k = std::bit_width(unsigned(r - l)) - 1;
		return combine(data[k][l], data[k][r - (1U << k)]);
	}
	S all_prod() const { return prod(0, n); }
};


#line 11 "tree/lca.hpp"

class LCA {
	static int min_op(int a, int b) { return std::min(a, b); }
	std::vector<int> in, out, order;
	SparseTable<int, min_op> table;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && static_cast<int>(in.size()) == n); }

  public:
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth;
	explicit LCA(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return static_cast<int>(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		in.resize(n);
		out.resize(n);
		order.resize(n);
		parent.assign(n, -1);
		depth.assign(n, 0);
		table.clear();
		if (!n) return;
		std::vector<int> start(std::size_t(n) + 1);
		for (auto [u, v] : edges) ++start[u + 1], ++start[v + 1];
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		std::vector<int> adj(2 * edges.size());
		std::copy(start.begin(), start.end() - 1, out.begin());
		for (auto [u, v] : edges) adj[out[u]++] = v, adj[out[v]++] = u;
		std::fill(out.begin(), out.end(), 1);
		int timer = 0, top = n - 1;
		order[top] = root;
		parent[root] = root;
		while (top < n) {
			int v = order[top++];
			in[v] = timer;
			order[timer++] = v;
			for (int i = start[v]; i < start[v + 1]; ++i) {
				int to = adj[i];
				if (to == parent[v]) continue;
				assert(parent[to] == -1);
				parent[to] = v;
				depth[to] = depth[v] + 1;
				order[--top] = to;
			}
		}
		assert(timer == n);
		for (int i = n - 1; i >= 1; --i) out[parent[order[i]]] += out[order[i]];
		for (int v = 0; v < n; ++v) out[v] += in[v];
		table.build(n, [&](int i) { return i ? in[parent[order[i]]] : 0; });
		parent[root] = -1;
	}
	int lca(int u, int v) const {
		check(u);
		check(v);
		int l = in[u], r = in[v];
		if (l == r) return u;
		if (l > r) std::swap(l, r);
		++l;
		return order[table.prod_unchecked(l, r + 1)];
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	bool is_ancestor(int u, int v) const {
		check(u);
		check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	std::pair<int, int> subtree(int v) const {
		check(v);
		return {in[v], out[v]};
	}
};


#line 3 "test/lowest_common_ancestor.test.cpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	LCA tree(n);
	for (int v = 1; v < n; ++v) {
		int p;
		cin >> p;
		tree.add_edge(p, v);
	}
	tree.build();
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int u, v;
		cin >> u >> v;
		cout << tree.lca(u, v) << '\n';
	}
}
