#line 1 "test/persistent_lazy_segtree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_range_affine_range_sum"
#line 1 "data_structure/persistent/persistent_lazy_segtree.hpp"



#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class PersistentLazySegtree {
  public:
	using value_type = S;
	using root_type = std::uint32_t;
  private:
	struct Node { S value; F lazy; root_type left, right; bool pending = false; };
	std::vector<Node> nodes{{e(), id(), 0, 0}};
	int n_ = 0;
	root_type initial_ = 0;
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type join(root_type left, root_type right) { return make({op(nodes[left].value, nodes[right].value), id(), left, right}); }
	root_type build(const std::vector<S> &a, int l, int r) {
		if (r - l == 1) return make({a[l], id(), 0, 0});
		int m = l + (r - l) / 2;
		auto left = build(a, l, m), right = build(a, m, r);
		return join(left, right);
	}
	void inherit(root_type root, F &carry, bool &active) const {
		if (nodes[root].pending) {
			carry = active ? composition(carry, nodes[root].lazy) : nodes[root].lazy;
			active = true;
		}
	}
	root_type mapped(root_type root, F f, int length) {
		const auto &node = nodes[root];
		return make({mapping(f, node.value, length), node.pending ? composition(f, node.lazy) : std::move(f), node.left, node.right, true});
	}
	root_type carried(root_type root, F carry, int length, bool active) {
		return active ? mapped(root, std::move(carry), length) : root;
	}
	root_type update(root_type root, int l, int r, int ql, int qr, F f, F carry, bool active) {
		if (qr <= l || r <= ql) return carried(root, std::move(carry), r - l, active);
		if (ql <= l && r <= qr) return mapped(root, active ? composition(f, carry) : std::move(f), r - l);
		inherit(root, carry, active);
		auto a = nodes[root].left, b = nodes[root].right;
		int m = l + (r - l) / 2;
		auto left = update(a, l, m, ql, qr, f, carry, active);
		auto right = update(b, m, r, ql, qr, std::move(f), std::move(carry), active);
		return join(left, right);
	}
	root_type assign(root_type root, int l, int r, int p, S value, F carry, bool active) {
		if (r - l == 1) return make({std::move(value), id(), 0, 0});
		inherit(root, carry, active);
		auto a = nodes[root].left, b = nodes[root].right;
		int m = l + (r - l) / 2;
		root_type left, right;
		if (p < m) {
			left = assign(a, l, m, p, std::move(value), carry, active);
			right = carried(b, std::move(carry), r - m, active);
		} else {
			left = carried(a, carry, m - l, active);
			right = assign(b, m, r, p, std::move(value), std::move(carry), active);
		}
		return join(left, right);
	}
	S query(root_type root, int l, int r, int ql, int qr, F carry, bool active) const {
		const auto &node = nodes[root];
		if (ql <= l && r <= qr) return active ? mapping(carry, node.value, r - l) : node.value;
		inherit(root, carry, active);
		int m = l + (r - l) / 2;
		if (qr <= m) return query(node.left, l, m, ql, qr, std::move(carry), active);
		if (m <= ql) return query(node.right, m, r, ql, qr, std::move(carry), active);
		return op(query(node.left, l, m, ql, qr, carry, active), query(node.right, m, r, ql, qr, carry, active));
	}
	root_type copy(root_type root, root_type source, int l, int r, int ql, int qr, F carry, F source_carry, bool active, bool source_active) {
		if (qr <= l || r <= ql || (root == source && !active && !source_active)) return carried(root, std::move(carry), r - l, active);
		if (ql <= l && r <= qr) return carried(source, std::move(source_carry), r - l, source_active);
		inherit(root, carry, active); inherit(source, source_carry, source_active);
		auto a = nodes[root].left, b = nodes[root].right, c = nodes[source].left, d = nodes[source].right;
		int m = l + (r - l) / 2;
		auto left = copy(a, c, l, m, ql, qr, carry, source_carry, active, source_active);
		auto right = copy(b, d, m, r, ql, qr, std::move(carry), std::move(source_carry), active, source_active);
		return join(left, right);
	}
	template <bool Right, class Pred>
	int search(root_type root, int l, int r, int end, F carry, bool active, S &value, Pred &f) const {
		const auto &node = nodes[root];
		if (Right ? end <= l : r <= end) {
			S part = active ? mapping(carry, node.value, r - l) : node.value;
			S next = Right ? op(value, part) : op(part, value);
			if (f(next)) { value = std::move(next); return Right ? n_ : 0; }
			if (r - l == 1) return Right ? l : r;
		}
		inherit(root, carry, active);
		int m = l + (r - l) / 2;
		if constexpr (Right) {
			if (end < m) { int found = search<Right>(node.left, l, m, end, carry, active, value, f); if (found != n_) return found; }
			return search<Right>(node.right, m, r, end, carry, active, value, f);
		} else {
			if (m < end) { int found = search<Right>(node.right, m, r, end, carry, active, value, f); if (found) return found; }
			return search<Right>(node.left, l, m, end, carry, active, value, f);
		}
	}

  public:
	explicit PersistentLazySegtree(int n = 0) : n_(n) { assert(n >= 0); }
	explicit PersistentLazySegtree(const std::vector<S> &a) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) { reserve(2 * a.size() - 1); initial_ = build(a, 0, n_); }
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	root_type set(root_type root, int p, S value) { check(root); assert(0 <= p && p < n_); return assign(root, 0, n_, p, std::move(value), id(), false); }
	root_type apply(root_type root, int l, int r, F f) {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? root : update(root, 0, n_, l, r, std::move(f), id(), false);
	}
	root_type apply(root_type root, int p, F f) { assert(0 <= p && p < n_); return apply(root, p, p + 1, std::move(f)); }
	S prod(root_type root, int l, int r) const {
		check(root); assert(0 <= l && l <= r && r <= n_);
		return l == r ? e() : query(root, 0, n_, l, r, id(), false);
	}
	S get(root_type root, int p) const { assert(0 <= p && p < n_); return prod(root, p, p + 1); }
	S all_prod(root_type root) const { check(root); return nodes[root].value; }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		check(root); check(source); assert(0 <= l && l <= r && r <= n_);
		if (l == r || root == source) return root;
		if (l == 0 && r == n_) return source;
		return copy(root, source, 0, n_, l, r, id(), id(), false, false);
	}
	template <class Pred> int max_right(root_type root, int l, Pred f) const {
		check(root); assert(0 <= l && l <= n_ && f(e()));
		S value = e();
		return l == n_ ? n_ : search<true>(root, 0, n_, l, id(), false, value, f);
	}
	template <class Pred> int min_left(root_type root, int r, Pred f) const {
		check(root); assert(0 <= r && r <= n_ && f(e()));
		S value = e();
		return r == 0 ? 0 : search<false>(root, 0, n_, r, id(), false, value, f);
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#include <type_traits>

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
	[[gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[gnu::always_inline]] unsigned val() const {
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
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
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
	[[gnu::always_inline]] Mint operator+() const { return *this; }
	[[gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
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


#line 4 "test/persistent_lazy_segtree.test.cpp"
#include <iostream>
#line 6 "test/persistent_lazy_segtree.test.cpp"

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<mint> a(n);
	for (auto &x : a) std::cin >> x;
	PersistentLazySegtree<mint, op, e, F, mapping, composition, id> seg(a);
	std::vector<decltype(seg)::root_type> roots(q + 1, seg.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type, k, l, r;
		std::cin >> type >> k;
		if (type == 0) {
			mint b, c;
			std::cin >> l >> r >> b >> c;
			roots[i] = seg.apply(roots[k + 1], l, r, {b, c});
		} else if (type == 1) {
			int s;
			std::cin >> s >> l >> r;
			roots[i] = seg.copy_range(roots[k + 1], roots[s + 1], l, r);
		} else {
			std::cin >> l >> r;
			std::cout << seg.prod(roots[k + 1], l, r) << '\n';
		}
	}
}
