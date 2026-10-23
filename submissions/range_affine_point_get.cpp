#line 1 "test/dual_segtree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_point_get"
#line 1 "data_structure/segment_tree/dual_segtree.hpp"



#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template<class S, class F, auto mapping, auto composition, auto id, bool Commutative = false>
class DualSegtree {
	int n_ = 0, base_ = 1, log_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void check([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void all_apply(int k, const F &f) {
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = 1;
		} else data_[k - base_] = mapping(f, std::move(data_[k - base_]));
	}
	void push(int k) {
		if (!dirty_[k]) return;
		all_apply(2 * k, lazy_[k]);
		all_apply(2 * k + 1, lazy_[k]);
		dirty_[k] = 0;
	}
	void push_path(int k) {
		for (int h = log_; h; --h) push(k >> h);
	}
	void push_range(int l, int r) {
		int left = std::countr_zero(static_cast<unsigned>(l));
		int right = std::countr_zero(static_cast<unsigned>(r));
		for (int h = log_; h > left; --h) push(l >> h);
		for (int h = log_; h > right; --h) push((r - 1) >> h);
	}

  public:
	using value_type = S;
	using update_type = F;
	DualSegtree() = default;
	explicit DualSegtree(int n, const S &value = S{}) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 4);
		build(std::vector<S>(n, value));
	}
	explicit DualSegtree(std::vector<S> a) { build(std::move(a)); }
	void build(std::vector<S> a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		n_ = static_cast<int>(a.size());
		base_ = static_cast<int>(std::bit_ceil(static_cast<unsigned>(std::max(1, n_))));
		log_ = std::countr_zero(static_cast<unsigned>(base_));
		data_ = std::move(a);
		lazy_.assign(base_, id());
		dirty_.assign(base_, 0);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	S get(int p) const {
		check(p);
		S value = data_[p];
		for (int k = (p + base_) >> 1; k; k >>= 1)
			if (dirty_[k]) value = mapping(lazy_[k], std::move(value));
		return value;
	}
	void set(int p, const S &value) {
		check(p);
		push_path(p + base_);
		data_[p] = value;
	}
	void apply(int p, const F &f) {
		check(p);
		if constexpr (!Commutative) push_path(p + base_);
		data_[p] = mapping(f, std::move(data_[p]));
	}
	void apply(int l, int r, const F &f) {
		assert(0 <= l && l <= r && r <= n_);
		if (l == r) return;
		if (r - l == 1) { apply(l, f); return; }
		l += base_, r += base_;
		if constexpr (!Commutative) push_range(l, r);
		while (l < r) {
			if (l & 1) all_apply(l++, f);
			if (r & 1) all_apply(--r, f);
			l >>= 1, r >>= 1;
		}
	}
	std::vector<S> values() {
		for (int k = 1; k < base_; ++k) push(k);
		return data_;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 10 "components/fast_io.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
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


#line 5 "test/dual_segtree.test.cpp"

struct Affine { mint a, b; };
mint mapping(Affine f, mint x) { return f.a * x + f.b; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	DualSegtree<mint, Affine, mapping, composition, id> seg(std::move(a));
	while (q--) {
		int type = 0, l = 0;
		in.read(type); in.read(l);
		if (type == 0) {
			int r = 0;
			unsigned b = 0, c = 0;
			in.read(r); in.read(b); in.read(c);
			seg.apply(l, r, Affine{mint::raw(b), mint::raw(c)});
		} else out.println(seg.get(l).val());
	}
}
