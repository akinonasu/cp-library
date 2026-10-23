#line 1 "test/segtree_beats.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#line 1 "data_structure/advanced/segtree_beats.hpp"



#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>
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


#line 10 "data_structure/advanced/segtree_beats.hpp"

template <class T = long long, class Sum = T>
class SegtreeBeats {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
	static constexpr T inf = std::numeric_limits<T>::max();
	static constexpr T neg_inf = std::numeric_limits<T>::lowest();
	struct Node {
		Sum sum;
		T lo, hi, lo2, hi2, add;
		int nlo, nhi;
	};
	int n = 0;
	mutable std::vector<Node, DefaultInitAllocator<Node>> tree;
	static Node *left(Node *p) { return p + 1; }
	static Node *right(Node *p, int l, int m) { return p + 2 * (m - l); }
	static void fill(Node &a, T x, int len) {
		a = {Sum(x) * len, x, x, inf, neg_inf, 0, len, len};
	}
	static void add_node(Node &a, T x, int len) {
		a.sum += Sum(x) * len;
		a.lo += x;
		a.hi += x;
		if (a.lo2 != inf) a.lo2 += x;
		if (a.hi2 != neg_inf) a.hi2 += x;
		a.add += x;
	}
	static void lower(Node &a, T x) {
		a.sum += (Sum(x) - Sum(a.hi)) * a.nhi;
		if (a.lo == a.hi) a.lo = x;
		else if (a.lo2 == a.hi) a.lo2 = x;
		a.hi = x;
	}
	static void raise(Node &a, T x) {
		a.sum += (Sum(x) - Sum(a.lo)) * a.nlo;
		if (a.hi == a.lo) a.hi = x;
		else if (a.hi2 == a.lo) a.hi2 = x;
		a.lo = x;
	}
	static void pull(Node *p, Node *lc, Node *rc) {
		auto &a = *p;
		const auto &b = *lc, &c = *rc;
		a.sum = b.sum + c.sum;
		if (b.lo < c.lo) a.lo = b.lo, a.lo2 = std::min(b.lo2, c.lo), a.nlo = b.nlo;
		else if (b.lo > c.lo) a.lo = c.lo, a.lo2 = std::min(b.lo, c.lo2), a.nlo = c.nlo;
		else a.lo = b.lo, a.lo2 = std::min(b.lo2, c.lo2), a.nlo = b.nlo + c.nlo;
		if (b.hi > c.hi) a.hi = b.hi, a.hi2 = std::max(b.hi2, c.hi), a.nhi = b.nhi;
		else if (b.hi < c.hi) a.hi = c.hi, a.hi2 = std::max(b.hi, c.hi2), a.nhi = c.nhi;
		else a.hi = b.hi, a.hi2 = std::max(b.hi2, c.hi2), a.nhi = b.nhi + c.nhi;
	}
	static void push(Node *p, int l, int m, int r) {
		auto &a = *p, &b = *left(p), &c = *right(p, l, m);
		if (a.lo == a.hi) {
			fill(b, a.lo, m - l);
			fill(c, a.lo, r - m);
			a.add = 0;
			return;
		}
		if (a.add) {
			add_node(b, a.add, m - l);
			add_node(c, a.add, r - m);
			a.add = 0;
		}
		if (b.hi > a.hi) lower(b, a.hi);
		if (b.lo < a.lo) raise(b, a.lo);
		if (c.hi > a.hi) lower(c, a.hi);
		if (c.lo < a.lo) raise(c, a.lo);
	}
	template <class F>
	static void build(Node *p, int l, int r, const F &f) {
		if (r - l == 1) return fill(*p, f(l), 1);
		int m = (l + r) / 2;
		auto lc = left(p), rc = right(p, l, m);
		build(lc, l, m, f);
		build(rc, m, r, f);
		p->add = 0;
		pull(p, lc, rc);
	}
	template <int op>
	static void update(Node *p, int l, int r, int ql, int qr, T x) {
		auto &a = *p;
		if constexpr (op == 0) { if (a.hi <= x) return; }
		if constexpr (op == 1) { if (a.lo >= x) return; }
		if constexpr (op == 3) { if (a.lo == x && a.hi == x) return; }
		if (ql <= l && r <= qr) {
			if constexpr (op == 0) {
				if (x <= a.lo) return fill(a, x, r - l);
				if (a.hi2 < x) return lower(a, x);
			}
			if constexpr (op == 1) {
				if (x >= a.hi) return fill(a, x, r - l);
				if (a.lo2 > x) return raise(a, x);
			}
			if constexpr (op == 2) return add_node(a, x, r - l);
			if constexpr (op == 3) return fill(a, x, r - l);
		}
		int m = (l + r) / 2;
		auto lc = left(p), rc = right(p, l, m);
		push(p, l, m, r);
		if (ql < m) update<op>(lc, l, m, ql, qr, x);
		if (m < qr) update<op>(rc, m, r, ql, qr, x);
		pull(p, lc, rc);
	}
	template <int op>
	using Result = std::conditional_t<op == 0, Sum, T>;
	template <int op>
	static Result<op> query(Node *p, int l, int r, int ql, int qr) {
		const auto &a = *p;
		if (ql <= l && r <= qr) {
			if constexpr (op == 0) return a.sum;
			if constexpr (op == 1) return a.lo;
			if constexpr (op == 2) return a.hi;
		}
		if (a.lo == a.hi) {
			if constexpr (op == 0) return Sum(a.lo) * (std::min(r, qr) - std::max(l, ql));
			else return a.lo;
		}
		int m = (l + r) / 2;
		push(p, l, m, r);
		if (qr <= m) return query<op>(left(p), l, m, ql, qr);
		if (m <= ql) return query<op>(right(p, l, m), m, r, ql, qr);
		auto b = query<op>(left(p), l, m, ql, qr), c = query<op>(right(p, l, m), m, r, ql, qr);
		if constexpr (op == 0) return b + c;
		if constexpr (op == 1) return std::min(b, c);
		if constexpr (op == 2) return std::max(b, c);
	}
	static void collect(Node *p, int l, int r, std::vector<T> &a) {
		if (p->lo == p->hi) {
			std::fill(a.begin() + l, a.begin() + r, p->lo);
			return;
		}
		int m = (l + r) / 2;
		push(p, l, m, r);
		collect(left(p), l, m, a);
		collect(right(p, l, m), m, r, a);
	}
	void check(int l, int r) const { assert(0 <= l && l <= r && r <= n); (void)l; (void)r; }
public:
	SegtreeBeats() = default;
	explicit SegtreeBeats(int n, T x = 0) : n(n), tree(n > 0 ? 2 * std::size_t(n) - 1 : 0) {
		assert(n >= 0);
		if (n) build(tree.data(), 0, n, [x](int) { return x; });
	}
	explicit SegtreeBeats(const std::vector<T> &a) : n(int(a.size())), tree(n ? 2 * std::size_t(n) - 1 : 0) {
		if (n) build(tree.data(), 0, n, [&](int i) { return a[i]; });
	}
	int size() const { return n; }
	void chmin(int l, int r, T x) { check(l, r); if (l < r) update<0>(tree.data(), 0, n, l, r, x); }
	void chmax(int l, int r, T x) { check(l, r); if (l < r) update<1>(tree.data(), 0, n, l, r, x); }
	void add(int l, int r, T x) { check(l, r); if (l < r && x) update<2>(tree.data(), 0, n, l, r, x); }
	void assign(int l, int r, T x) { check(l, r); if (l < r) update<3>(tree.data(), 0, n, l, r, x); }
	Sum sum(int l, int r) const { check(l, r); return l < r ? query<0>(tree.data(), 0, n, l, r) : Sum(0); }
	T min(int l, int r) const { check(l, r); return l < r ? query<1>(tree.data(), 0, n, l, r) : inf; }
	T max(int l, int r) const { check(l, r); return l < r ? query<2>(tree.data(), 0, n, l, r) : neg_inf; }
	Sum all_sum() const { return n ? tree[0].sum : Sum(0); }
	T all_min() const { return n ? tree[0].lo : inf; }
	T all_max() const { return n ? tree[0].hi : neg_inf; }
	T get(int p) const { assert(0 <= p && p < n); return min(p, p + 1); }
	void set(int p, T x) { assert(0 <= p && p < n); assign(p, p + 1, x); }
	std::vector<T> values() const {
		std::vector<T> a(n);
		if (n) collect(tree.data(), 0, n, a);
		return a;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#line 14 "components/fast_io.hpp"
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


#line 5 "test/segtree_beats.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n);
	in.read(q);
	std::vector<long long> a(n);
	for (auto &x : a) in.read(x);
	SegtreeBeats seg(a);
	while (q--) {
		int t = 0, l = 0, r = 0;
		in.read(t);
		in.read(l);
		in.read(r);
		if (t == 3) out.println(seg.sum(l, r));
		else {
			long long x = 0;
			in.read(x);
			if (t == 0) seg.chmin(l, r, x);
			if (t == 1) seg.chmax(l, r, x);
			if (t == 2) seg.add(l, r, x);
		}
	}
}
