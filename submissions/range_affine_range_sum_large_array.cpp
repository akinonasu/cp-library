#line 1 "test/dynamic_lazy_segtree_large.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum_large_array"
#line 1 "data_structure/segment_tree/dynamic_lazy_segtree.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id,
	auto initial = nullptr, class Coord = std::int64_t>
class DynamicLazySegtree {
	static_assert(std::is_integral_v<Coord> && !std::is_same_v<Coord, bool> && sizeof(Coord) <= 8);
	using U = std::make_unsigned_t<Coord>;
	struct Node {
		S value;
		F lazy;
		int left = 0, right = 0;
		bool dirty = false;
	};
	std::vector<Node> nodes{Node{e(), id()}};
	int root = 0;
	Coord first, last;
	static bool leaf(Coord l, Coord r) { return U(U(r) - U(l)) == 1; }
	static S base(Coord l, Coord r) {
		if (l == r) return e();
		if constexpr (std::is_same_v<decltype(initial), std::nullptr_t>) return e();
		else return initial(l, r);
	}
	S value(int k, Coord l, Coord r) const { return k ? nodes[k].value : base(l, r); }
	int make_node(Coord l, Coord r) {
		assert(nodes.size() < std::size_t(std::numeric_limits<int>::max()));
		nodes.push_back(Node{base(l, r), id()});
		return static_cast<int>(nodes.size() - 1);
	}
	void all_apply(int k, const F &f, bool leaf) {
		nodes[k].value = mapping(f, std::move(nodes[k].value));
		if (!leaf) {
			nodes[k].lazy = nodes[k].dirty ? composition(f, nodes[k].lazy) : f;
			nodes[k].dirty = true;
		}
	}
	void push(int k, Coord l, Coord m, Coord r) {
		if (!nodes[k].dirty) return;
		F f = nodes[k].lazy;
		if (!nodes[k].left) nodes[k].left = make_node(l, m);
		if (!nodes[k].right) nodes[k].right = make_node(m, r);
		all_apply(nodes[k].left, f, leaf(l, m));
		all_apply(nodes[k].right, f, leaf(m, r));
		nodes[k].dirty = false;
	}
	void pull(int k, Coord l, Coord m, Coord r) {
		nodes[k].value = op(value(nodes[k].left, l, m), value(nodes[k].right, m, r));
	}
	int update(int k, Coord l, Coord r, Coord a, Coord b, const F &f) {
		if (!k) k = make_node(l, r);
		if (a == l && b == r) { all_apply(k, f, leaf(l, r)); return k; }
		Coord m = std::midpoint(l, r);
		push(k, l, m, r);
		if (a < m) nodes[k].left = update(nodes[k].left, l, m, a, std::min(b, m), f);
		if (m < b) nodes[k].right = update(nodes[k].right, m, r, std::max(a, m), b, f);
		pull(k, l, m, r);
		return k;
	}
	int write(int k, Coord l, Coord r, Coord p, const S &x) {
		if (!k) k = make_node(l, r);
		if (leaf(l, r)) { nodes[k].value = x; return k; }
		Coord m = std::midpoint(l, r);
		push(k, l, m, r);
		if (p < m) nodes[k].left = write(nodes[k].left, l, m, p, x);
		else nodes[k].right = write(nodes[k].right, m, r, p, x);
		pull(k, l, m, r);
		return k;
	}
	S product(int k, Coord l, Coord r, Coord a, Coord b) const {
		if (!k) return base(a, b);
		if (a == l && b == r) return nodes[k].value;
		Coord m = std::midpoint(l, r);
		S result = b <= m ? product(nodes[k].left, l, m, a, b) :
			m <= a ? product(nodes[k].right, m, r, a, b) :
			op(product(nodes[k].left, l, m, a, m), product(nodes[k].right, m, r, m, b));
		return nodes[k].dirty ? mapping(nodes[k].lazy, std::move(result)) : result;
	}
	template<bool Right, class Pred>
	bool search(int k, Coord l, Coord r, Coord bound, S &acc, const F *carry, Pred &f, Coord &answer) const {
		if ((Right ? bound <= l : r <= bound)) {
			S part = value(k, l, r);
			if (carry) part = mapping(*carry, std::move(part));
			S next = Right ? op(acc, part) : op(part, acc);
			if (f(next)) { acc = std::move(next); return false; }
			if (leaf(l, r)) { answer = Right ? l : r; return true; }
		}
		Coord m = std::midpoint(l, r);
		auto descend = [&](const F *pending) {
			int left = k ? nodes[k].left : 0, right = k ? nodes[k].right : 0;
			if constexpr (Right) {
				if (bound < m && search<Right>(left, l, m, bound, acc, pending, f, answer)) return true;
				return search<Right>(right, m, r, bound, acc, pending, f, answer);
			} else {
				if (m < bound && search<Right>(right, m, r, bound, acc, pending, f, answer)) return true;
				return search<Right>(left, l, m, bound, acc, pending, f, answer);
			}
		};
		if (k && nodes[k].dirty) {
			F pending = carry ? composition(*carry, nodes[k].lazy) : nodes[k].lazy;
			return descend(&pending);
		}
		return descend(carry);
	}
	int build_values(const std::vector<S> &a, Coord l, Coord r) {
		int k = make_node(l, r);
		if (leaf(l, r)) nodes[k].value = a[std::size_t(l)];
		else {
			Coord m = std::midpoint(l, r);
			int left = build_values(a, l, m), right = build_values(a, m, r);
			nodes[k].left = left;
			nodes[k].right = right;
			pull(k, l, m, r);
		}
		return k;
	}
	void check([[maybe_unused]] Coord l, [[maybe_unused]] Coord r) const {
		assert(first <= l && l <= r && r <= last);
	}

  public:
	using value_type = S;
	using update_type = F;
	using coordinate_type = Coord;
	explicit DynamicLazySegtree(Coord n = 0) : DynamicLazySegtree(0, n) {}
	DynamicLazySegtree(Coord l, Coord r) : first(l), last(r) { assert(l <= r); }
	explicit DynamicLazySegtree(const std::vector<S> &a) : first(0), last(0) { build(a); }
	DynamicLazySegtree(const DynamicLazySegtree &) = default;
	DynamicLazySegtree &operator=(const DynamicLazySegtree &) = default;
	DynamicLazySegtree(DynamicLazySegtree &&other) : DynamicLazySegtree(other.first, other.last) { swap(other); }
	DynamicLazySegtree &operator=(DynamicLazySegtree &&other) {
		if (this != &other) { DynamicLazySegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(DynamicLazySegtree &other) noexcept {
		nodes.swap(other.nodes);
		std::swap(root, other.root);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::uintmax_t(std::numeric_limits<Coord>::max()));
		assert(a.size() < std::size_t(std::numeric_limits<int>::max()) / 2);
		clear();
		first = 0;
		last = static_cast<Coord>(a.size());
		reserve(a.empty() ? 0 : 2 * a.size() - 1);
		if (!a.empty()) root = build_values(a, first, last);
	}
	void reserve(std::size_t count) {
		assert(count < std::size_t(std::numeric_limits<int>::max()));
		nodes.reserve(count + 1);
	}
	void clear() { nodes.erase(nodes.begin() + 1, nodes.end()); root = 0; }
	U size() const { return U(last) - U(first); }
	bool empty() const { return first == last; }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	S all_prod() const { return value(root, first, last); }
	S prod(Coord l, Coord r) const { check(l, r); return l == r ? e() : product(root, first, last, l, r); }
	S get(Coord p) const { assert(first <= p && p < last); return prod(p, Coord(p + 1)); }
	void set(Coord p, const S &x) { assert(first <= p && p < last); root = write(root, first, last, p, x); }
	void apply(Coord l, Coord r, const F &f) {
		check(l, r);
		if (l != r) root = update(root, first, last, l, r, f);
	}
	void apply(Coord p, const F &f) { assert(first <= p && p < last); apply(p, Coord(p + 1), f); }
	template<class Pred>
	Coord max_right(Coord l, Pred f) const {
		assert(first <= l && l <= last && f(e()));
		Coord answer = last;
		if (l != last) { S acc = e(); search<true>(root, first, last, l, acc, nullptr, f, answer); }
		return answer;
	}
	template<class Pred>
	Coord min_left(Coord r, Pred f) const {
		assert(first <= r && r <= last && f(e()));
		Coord answer = first;
		if (r != first) { S acc = e(); search<false>(root, first, last, r, acc, nullptr, f, answer); }
		return answer;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
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


#line 5 "test/dynamic_lazy_segtree_large.test.cpp"

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }
Sum initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }

int main() {
	FastInput in;
	FastOutput out;
	std::int64_t n = 0;
	int q = 0;
	in.read(n); in.read(q);
	DynamicLazySegtree<Sum, op, e, Affine, mapping, composition, id, initial> seg(n);
	while (q--) {
		int type = 0;
		std::int64_t l = 0, r = 0;
		in.read(type); in.read(l); in.read(r);
		if (type == 0) {
			unsigned b = 0, c = 0;
			in.read(b); in.read(c);
			seg.apply(l, r, Affine{mint::raw(b), mint::raw(c)});
		} else out.println(seg.prod(l, r).sum.val());
	}
}
