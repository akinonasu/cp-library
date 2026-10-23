#line 1 "test/dynamic_segtree_composite.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#line 1 "data_structure/segment_tree/dynamic_segtree.hpp"


#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class Coord = std::int64_t>
class DynamicSegtree {
	static_assert(std::is_integral_v<Coord> && !std::is_same_v<Coord, bool> && sizeof(Coord) <= 8);
	static constexpr unsigned B = 8, limit = (1u << 26) - 1;
	using U = std::make_unsigned_t<Coord>;
	static constexpr unsigned tag = 1u << 31;
	static constexpr U sign = std::is_signed_v<Coord> ? U(1) << (std::numeric_limits<U>::digits - 1) : U(0);
	template <std::size_t... I>
	static auto units(std::index_sequence<I...>) { return std::array<S, sizeof...(I)>{(void(I), e())...}; }
	struct Leaf {
		std::array<U, B> keys{};
		std::array<S, B> values = units(std::make_index_sequence<B>{});
		S product = e();
		unsigned count = 0;
	};
	struct Node {
		U middle = 0;
		unsigned left = 0, right = 0;
		S product = e();
	};
	std::vector<Leaf> leaves;
	std::vector<Node> nodes{Node{}};
	unsigned root = 0, points = 0;
	Coord first, last;
	static U encode(Coord x) { return U(x) ^ sign; }
	static Coord decode(U x) { return Coord(x ^ sign); }
	Leaf &leaf(unsigned id) { return leaves[id & ~tag]; }
	const Leaf &leaf(unsigned id) const { return leaves[id & ~tag]; }
	const S &product(unsigned id) const { return id & tag ? leaf(id).product : nodes[id].product; }
	unsigned make_leaf() {
		leaves.emplace_back();
		return unsigned(leaves.size() - 1) | tag;
	}
	unsigned make_node(U middle, unsigned left, unsigned right) {
		S value = op(product(left), product(right));
		nodes.push_back({middle, left, right, std::move(value)});
		return unsigned(nodes.size() - 1);
	}
	template <unsigned N>
	[[gnu::always_inline]] static inline S fold(const S *values, unsigned count) {
		if constexpr (N == 1) return values[0];
		else {
			if (count <= N / 2) return fold<N / 2>(values, count);
			return op(fold<N / 2>(values, N / 2), fold<N / 2>(values + N / 2, count - N / 2));
		}
	}
	static void pull(Leaf &node) { node.product = fold<B>(node.values.data(), node.count); }

	template <bool Append>
	static void write(Leaf &node, unsigned p, U key, S value) {
		if (p < node.count && node.keys[p] == key) {
			if constexpr (Append) node.values[p] = op(node.values[p], value);
			else node.values[p] = std::move(value);
		} else {
			std::move_backward(node.keys.begin() + p, node.keys.begin() + node.count, node.keys.begin() + node.count + 1);
			std::move_backward(node.values.begin() + p, node.values.begin() + node.count, node.values.begin() + node.count + 1);
			node.keys[p] = key;
			node.values[p] = std::move(value);
			++node.count;
		}
		pull(node);
	}
	template <bool Append>
	void update(U key, S value) {
		if (!root) {
			root = make_leaf();
			write<Append>(leaf(root), 0, key, std::move(value));
			++points;
			return;
		}
		std::array<unsigned, 64> path;
		unsigned depth = 0, id = root;
		bool outside = false;
		while (!(id & tag)) {
			unsigned h = std::countr_zero(nodes[id].middle);
			if (((key ^ nodes[id].middle) >> h) > 1) { outside = true; break; }
			assert(depth < path.size());
			path[depth++] = id;
			id = key < nodes[id].middle ? nodes[id].left : nodes[id].right;
		}
		unsigned replacement = id;
		if (outside) {
			assert(points < limit);
			unsigned added = make_leaf();
			write<Append>(leaf(added), 0, key, std::move(value));
			unsigned h = std::bit_width(U(key ^ nodes[id].middle)) - 1;
			U middle = ((key | nodes[id].middle) >> h) << h;
			replacement = key < middle ? make_node(middle, added, id) : make_node(middle, id, added);
			++points;
		} else {
			auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
			bool exists = p < node.count && node.keys[p] == key;
			assert(exists || points < limit);
			if (exists || node.count < B) write<Append>(node, p, key, std::move(value));
			else {
				U low = std::min(key, node.keys[0]), high = std::max(key, node.keys[B - 1]);
				unsigned h = std::bit_width(U(low ^ high)) - 1;
				U middle = ((low | high) >> h) << h;
				unsigned cut = std::lower_bound(node.keys.begin(), node.keys.end(), middle) - node.keys.begin();
				unsigned added = make_leaf();
				auto &a = leaf(id), &b = leaf(added);
				b.count = B - cut;
				for (unsigned i = 0; i < b.count; ++i) b.keys[i] = a.keys[cut + i], b.values[i] = std::move(a.values[cut + i]);
				a.count = cut;
				if (key < middle) write<Append>(a, p, key, std::move(value));
				else write<Append>(b, p - cut, key, std::move(value));
				if (key < middle) pull(b);
				else pull(a);
				replacement = make_node(middle, id, added);
			}
			points += !exists;
		}
		if (depth) {
			auto parent = path[depth - 1];
			if (key < nodes[parent].middle) nodes[parent].left = replacement;
			else nodes[parent].right = replacement;
		} else root = replacement;
		while (depth) {
			unsigned k = path[--depth];
			nodes[k].product = op(product(nodes[k].left), product(nodes[k].right));
		}
	}
	S prefix(unsigned id, U r) const {
		S result = e();
		while (!(id & tag)) {
			if (r <= nodes[id].middle) id = nodes[id].left;
			else { result = op(result, product(nodes[id].left)); id = nodes[id].right; }
		}
		const auto &node = leaf(id);
		for (unsigned i = 0; i < node.count && node.keys[i] < r; ++i) result = op(result, node.values[i]);
		return result;
	}
	S suffix(unsigned id, U l) const {
		S result = e();
		while (!(id & tag)) {
			if (nodes[id].middle <= l) id = nodes[id].right;
			else { result = op(product(nodes[id].right), result); id = nodes[id].left; }
		}
		const auto &node = leaf(id);
		for (unsigned i = node.count; i && l <= node.keys[i - 1];) result = op(node.values[--i], result);
		return result;
	}
	template <class Pred>
	bool search_right(unsigned id, U lower, U l, S &value, Pred &f, Coord &answer) const {
		if (l <= lower) {
			S next = op(value, product(id));
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, l) - node.keys.begin();
			for (; p < node.count; ++p) {
				S next = op(value, node.values[p]);
				if (!f(next)) { answer = decode(node.keys[p]); return true; }
				value = std::move(next);
			}
			return false;
		}
		U middle = nodes[id].middle;
		if (l < middle && search_right(nodes[id].left, lower, l, value, f, answer)) return true;
		return search_right(nodes[id].right, middle, l, value, f, answer);
	}
	template <class Pred>
	bool search_left(unsigned id, U upper, U r, S &value, Pred &f, Coord &answer) const {
		if (upper <= r) {
			S next = op(product(id), value);
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, r) - node.keys.begin();
			while (p) {
				S next = op(node.values[--p], value);
				if (!f(next)) { answer = decode(node.keys[p]) + 1; return true; }
				value = std::move(next);
			}
			return false;
		}
		U middle = nodes[id].middle;
		if (middle < r && search_left(nodes[id].right, upper, r, value, f, answer)) return true;
		return search_left(nodes[id].left, middle, r, value, f, answer);
	}

  public:
	using value_type = S;
	using coordinate_type = Coord;
	explicit DynamicSegtree(Coord n = 0) : DynamicSegtree(0, n) {}
	DynamicSegtree(Coord l, Coord r) : first(l), last(r) { assert(l <= r); }
	DynamicSegtree(const DynamicSegtree &) = default;
	DynamicSegtree &operator=(const DynamicSegtree &) = default;
	DynamicSegtree(DynamicSegtree &&other) : DynamicSegtree(other.first, other.last) { swap(other); }
	DynamicSegtree &operator=(DynamicSegtree &&other) {
		if (this != &other) { DynamicSegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(DynamicSegtree &other) noexcept {
		leaves.swap(other.leaves);
		nodes.swap(other.nodes);
		std::swap(root, other.root);
		std::swap(points, other.points);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void reserve(std::size_t n) {
		assert(n <= limit);
		std::size_t capacity = (3 * n + 2 * B - 1) / (2 * B);
		leaves.reserve(capacity);
		nodes.reserve(capacity + 1);
	}
	std::size_t node_count() const { return points; }
	std::size_t memory_usage() const { return leaves.capacity() * sizeof(Leaf) + nodes.capacity() * sizeof(Node); }
	U size() const { return U(last) - U(first); }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	void clear() { leaves.clear(); nodes.erase(nodes.begin() + 1, nodes.end()); root = points = 0; }
	void set(Coord p, S value) { assert(first <= p && p < last); update<false>(encode(p), std::move(value)); }
	void multiply(Coord p, S value) { assert(first <= p && p < last); update<true>(encode(p), std::move(value)); }
	S get(Coord p) const {
		assert(first <= p && p < last);
		if (!root) return e();
		U key = encode(p);
		unsigned id = root;
		while (!(id & tag)) id = key < nodes[id].middle ? nodes[id].left : nodes[id].right;
		const auto &node = leaf(id);
		unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
		return i < node.count && node.keys[i] == key ? node.values[i] : e();
	}
	S prod(Coord l, Coord r) const {
		assert(first <= l && l <= r && r <= last);
		if (!root || l == r) return e();
		if (l == first && r == last) return product(root);
		U a = encode(l), b = encode(r);
		unsigned id = root;
		while (!(id & tag)) {
			U middle = nodes[id].middle;
			if (b <= middle) id = nodes[id].left;
			else if (middle <= a) id = nodes[id].right;
			else return op(suffix(nodes[id].left, a), prefix(nodes[id].right, b));
		}
		const auto &node = leaf(id);
		unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, a) - node.keys.begin();
		S result = e();
		for (; i < node.count && node.keys[i] < b; ++i) result = op(result, node.values[i]);
		return result;
	}
	S all_prod() const { return root ? product(root) : e(); }
	template <class Pred>
	Coord max_right(Coord l, Pred f) const {
		assert(first <= l && l <= last && f(e()));
		Coord answer = last;
		if (root && l != last) { S value = e(); search_right(root, encode(first), encode(l), value, f, answer); }
		return answer;
	}
	template <class Pred>
	Coord min_left(Coord r, Pred f) const {
		assert(first <= r && r <= last && f(e()));
		Coord answer = first;
		if (root && r != first) { S value = e(); search_left(root, encode(last), encode(r), value, f, answer); }
		return answer;
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


#line 5 "test/dynamic_segtree_composite.test.cpp"
#include <iostream>
#line 7 "test/dynamic_segtree_composite.test.cpp"

using S = std::pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	constexpr std::int64_t step = std::int64_t(1) << 40;
	DynamicSegtree<S, op, e> tree(std::int64_t(n) * step);
	tree.reserve(n);
	for (int i = 0; i < n; ++i) {
		S f;
		std::cin >> f.first >> f.second;
		tree.set(std::int64_t(i) * step, f);
	}
	while (q--) {
		int type, l;
		std::cin >> type >> l;
		if (type == 0) {
			S f;
			std::cin >> f.first >> f.second;
			tree.set(std::int64_t(l) * step, f);
		} else {
			int r;
			mint x;
			std::cin >> r >> x;
			auto [a, b] = tree.prod(std::int64_t(l) * step, std::int64_t(r) * step);
			std::cout << a * x + b << '\n';
		}
	}
}
