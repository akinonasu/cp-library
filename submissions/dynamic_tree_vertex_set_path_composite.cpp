#line 1 "test/link_cut_tree_composite.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#line 1 "tree/link_cut_tree.hpp"



#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
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
		if (!x) return;
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


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	Mint operator+() const { return *this; }
	Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	Mint operator+(Mint b) const { return Mint(*this) += b; }
	Mint operator-(Mint b) const { return Mint(*this) -= b; }
	Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 4 "test/link_cut_tree_composite.test.cpp"
#include <array>
#include <iostream>
#include <set>
#include <tuple>
#line 9 "test/link_cut_tree_composite.test.cpp"

using namespace std;

using S = array<mint, 2>;
S op(const S &a, const S &b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
S e() { return {1, 0}; }

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	cin >> n >> q;
	vector<S> values(n);
	for (auto &v : values) cin >> v[0] >> v[1];
	LinkCutTree<S, op, e> tree(values);
	for (int repeat_index = 0; repeat_index < (n - 1); ++repeat_index) {
		int u, v;
		cin >> u >> v;
		tree.link(u, v);
	}
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type, u;
		cin >> type >> u;
		if (type == 0) {
			int v, w, x;
			cin >> v >> w >> x;
			tree.cut(u, v);
			tree.link(w, x);
		} else if (type == 1) {
			S f;
			cin >> f[0] >> f[1];
			tree.set(u, f);
		} else {
			int v;
			mint x;
			cin >> v >> x;
			auto f = tree.prod(u, v);
			cout << f[0] * x + f[1] << '\n';
		}
	}
}
