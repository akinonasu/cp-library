#line 1 "test/disjoint_sparse_table_affine.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/queue_operate_all_composite"
#line 1 "data_structure/range_query/disjoint_sparse_table.hpp"



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


#line 5 "data_structure/range_query/disjoint_sparse_table.hpp"
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
class DisjointSparseTable {
	using Value = std::conditional_t<std::is_same_v<S, bool>, std::uint8_t, S>;
	using Reference = std::conditional_t<std::is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	std::vector<std::vector<Value, DefaultInitAllocator<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (std::is_same_v<S, bool>) return bool(op(bool(a), bool(b)));
		else return op(a, b);
	}

  public:
	DisjointSparseTable() = default;
	DisjointSparseTable(const DisjointSparseTable &) = default;
	DisjointSparseTable &operator=(const DisjointSparseTable &) = default;
	DisjointSparseTable(DisjointSparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	DisjointSparseTable &operator=(DisjointSparseTable &&other) noexcept {
		if (this != &other) {
			n = std::exchange(other.n, 0);
			data = std::move(other.data);
		}
		return *this;
	}
	explicit DisjointSparseTable(const std::vector<S> &a) { build(a); }
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = static_cast<int>(a.size());
		int levels = std::max(1, int(std::bit_width(unsigned(std::max(0, n - 1)))));
		data.resize(levels);
		for (int k = 0; k < levels; ++k) {
			std::size_t length = n, half = std::size_t(1) << k;
			if (k && (length & (half * 2 - 1)) <= half) length &= ~(half * 2 - 1);
			if constexpr (std::is_default_constructible_v<Value>) data[k].resize(length);
			else data[k].assign(a.begin(), a.begin() + length);
		}
		for (int i = 0; i < n; ++i) data[0][i] = a[i];
		for (int k = 1; k < levels; ++k) {
			Value *row = data[k].data();
			std::size_t half = std::size_t(1) << k, block = half * 2;
			for (std::size_t start = 0; start < data[k].size(); start += block) {
				std::size_t mid = start + half, end = std::min(start + block, data[k].size());
				row[mid - 1] = a[mid - 1];
				row[mid] = a[mid];
				for (std::size_t i = mid - 1; i > start; --i) row[i - 1] = combine(a[i - 1], row[i]);
				for (std::size_t i = mid + 1; i < end; ++i) row[i] = combine(row[i - 1], a[i]);
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
		if (l == --r) return data[0][l];
		const auto &row = data[std::bit_width(unsigned(l ^ r)) - 1];
		return combine(row[l], row[r]);
	}
	S all_prod() const { return prod(0, n); }
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


#line 4 "test/disjoint_sparse_table_affine.test.cpp"
#include <iostream>
#include <tuple>
#line 8 "test/disjoint_sparse_table_affine.test.cpp"

using namespace std;

using S = pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }
int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	struct Query {
		int l, r;
		mint x;
	};
	vector<S> a;
	vector<Query> queries;
	a.reserve(q);
	queries.reserve(q);
	int l = 0, r = 0;
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type;
		cin >> type;
		if (type == 0) {
			S f;
			cin >> f.first >> f.second;
			a.push_back(f);
			++r;
		} else if (type == 1) ++l;
		else {
			mint x;
			cin >> x;
			queries.push_back({l, r, x});
		}
	}
	DisjointSparseTable<S, op, e> table(a);
	for (auto query : queries) {
		auto [p, q] = table.prod(query.l, query.r);
		cout << p * query.x + q << '\n';
	}
}
