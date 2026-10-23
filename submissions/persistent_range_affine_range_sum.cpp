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
	template <bool Prefix>
	S fold(root_type root, int l, int r, int end, F carry, bool active) const {
		S result = e();
		while (Prefix ? end < r : l < end) {
			inherit(root, carry, active);
			const auto &node = nodes[root];
			int m = l + (r - l) / 2;
			bool right = Prefix ? m < end : m <= end;
			if (Prefix ? right : !right) {
				auto child = right ? node.left : node.right;
				S part = active ? mapping(carry, nodes[child].value, right ? m - l : r - m) : nodes[child].value;
				result = Prefix ? op(std::move(result), std::move(part)) : op(std::move(part), std::move(result));
			}
			if (right) { root = node.right; l = m; }
			else { root = node.left; r = m; }
		}
		S part = active ? mapping(carry, nodes[root].value, r - l) : nodes[root].value;
		return Prefix ? op(std::move(result), std::move(part)) : op(std::move(part), std::move(result));
	}
	S query(root_type root, int l, int r, int ql, int qr, F carry, bool active) const {
		const auto &node = nodes[root];
		if (ql <= l && r <= qr) return active ? mapping(carry, node.value, r - l) : node.value;
		inherit(root, carry, active);
		int m = l + (r - l) / 2;
		if (qr <= m) return query(node.left, l, m, ql, qr, std::move(carry), active);
		if (m <= ql) return query(node.right, m, r, ql, qr, std::move(carry), active);
		return op(fold<false>(node.left, l, m, ql, carry, active), fold<true>(node.right, m, r, qr, carry, active));
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


#line 1 "components/fast_io.hpp"



#include <algorithm>
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


#line 7 "test/persistent_lazy_segtree.test.cpp"

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	PersistentLazySegtree<mint, op, e, F, mapping, composition, id> seg(a);
	seg.reserve(2 * a.size() + std::size_t(q) * 3 * std::bit_width(unsigned(n)));
	std::vector<decltype(seg)::root_type> roots(q + 1, seg.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0, l = 0, r = 0;
		input.read(type), input.read(k);
		if (type == 0) {
			unsigned b = 0, c = 0;
			input.read(l), input.read(r), input.read(b), input.read(c);
			roots[i] = seg.apply(roots[k + 1], l, r, {mint::raw(b), mint::raw(c)});
		} else if (type == 1) {
			int s = 0;
			input.read(s), input.read(l), input.read(r);
			roots[i] = seg.copy_range(roots[k + 1], roots[s + 1], l, r);
		} else {
			input.read(l), input.read(r);
			output.println(seg.prod(roots[k + 1], l, r).val());
		}
	}
}
