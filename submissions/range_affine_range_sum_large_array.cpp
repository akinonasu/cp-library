#line 1 "test/compressed_lazy_segtree_large.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum_large_array"
#line 1 "data_structure/segment_tree/lazy_segtree.hpp"



#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class LazySegtree {
	int n_ = 0;
	unsigned base_ = 1, height_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void pull(unsigned k) { data_[k] = op(data_[k * 2], data_[k * 2 + 1]); }
	void all_apply(unsigned k, const F &f) {
		data_[k] = mapping(f, data_[k]);
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = true;
		}
	}
	void push(unsigned k) {
		if (!dirty_[k]) return;
		all_apply(k * 2, lazy_[k]);
		all_apply(k * 2 + 1, lazy_[k]);
		dirty_[k] = false;
	}
	void push_path(unsigned p) {
		for (unsigned i = height_; i; --i) push(p >> i);
	}
	void push_boundaries(unsigned l, unsigned r) {
		for (unsigned i = height_; i; --i) {
			if ((l >> i) << i != l) push(l >> i);
			if ((r >> i) << i != r) push((r - 1) >> i);
		}
	}
	void allocate(int n) {
		assert(n >= 0);
		n_ = n;
		base_ = std::bit_ceil(unsigned(n) ? unsigned(n) : 1U);
		height_ = std::countr_zero(base_);
		data_.assign(std::size_t(base_) * 2, e());
		lazy_.assign(base_, id());
		dirty_.assign(base_, false);
	}
	void vertex([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	S prod_rec(unsigned k, unsigned l, unsigned r, unsigned ql, unsigned qr) const {
		if (ql <= l && r <= qr) return data_[k];
		unsigned m = l + (r - l) / 2;
		S value = qr <= m ? prod_rec(k * 2, l, m, ql, qr)
			: m <= ql ? prod_rec(k * 2 + 1, m, r, ql, qr)
			: op(prod_rec(k * 2, l, m, ql, qr), prod_rec(k * 2 + 1, m, r, ql, qr));
		return dirty_[k] ? mapping(lazy_[k], value) : value;
	}

  public:
	explicit LazySegtree(int n = 0) { allocate(n); }
	explicit LazySegtree(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		allocate(static_cast<int>(a.size()));
		for (int i = 0; i < n_; ++i) data_[base_ + unsigned(i)] = a[i];
		for (unsigned k = base_ - 1; k; --k) pull(k);
	}
	int size() const { return n_; }
	void set(int p, S value) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = std::move(value);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	S get(int p) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		return data_[k];
	}
	S all_prod() const { return data_[1]; }
	S prod(int l, int r) {
		range(l, r);
		if (l == r) return e();
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r);
		push_boundaries(a, b);
		S left = e(), right = e();
		while (a < b) {
			if (a & 1) left = op(left, data_[a++]);
			if (b & 1) right = op(data_[--b], right);
			a >>= 1, b >>= 1;
		}
		return op(left, right);
	}
	S prod(int l, int r) const {
		range(l, r);
		if (l == r) return e();
		return prod_rec(1, 0, base_, unsigned(l), unsigned(r));
	}
	void apply(int p, F f) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = mapping(f, data_[k]);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	void apply(int l, int r, F f) {
		range(l, r);
		if (l == r) return;
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r), first = a, last = b;
		push_boundaries(a, b);
		while (a < b) {
			if (a & 1) all_apply(a++, f);
			if (b & 1) all_apply(--b, f);
			a >>= 1, b >>= 1;
		}
		for (unsigned i = 1; i <= height_; ++i) {
			if ((first >> i) << i != first) pull(first >> i);
			if ((last >> i) << i != last) pull((last - 1) >> i);
		}
	}
	template<auto predicate> int max_right(int l) { return max_right(l, predicate); }
	template<class Pred> int max_right(int l, Pred predicate) {
		range(l, l);
		assert(predicate(e()));
		if (l == n_) return n_;
		unsigned k = base_ + unsigned(l);
		push_path(k);
		S acc = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(acc, data_[k]);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k *= 2;
					S part = op(acc, data_[k]);
					if (predicate(part)) acc = std::move(part), ++k;
				}
				return static_cast<int>(k - base_);
			}
			acc = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template<auto predicate> int min_left(int r) { return min_left(r, predicate); }
	template<class Pred> int min_left(int r, Pred predicate) {
		range(r, r);
		assert(predicate(e()));
		if (!r) return 0;
		unsigned k = base_ + unsigned(r);
		push_path(k - 1);
		S acc = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], acc);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k = k * 2 + 1;
					S part = op(data_[k], acc);
					if (predicate(part)) acc = std::move(part), --k;
				}
				return static_cast<int>(k + 1 - base_);
			}
			acc = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
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


#line 1 "components/radix_sort.hpp"



#line 11 "components/radix_sort.hpp"

template <int Bits, class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key &&key) {
	static_assert(1 <= Bits && Bits <= 16);
	constexpr unsigned buckets = 1U << Bits, mask = buckets - 1;
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	using Counts = std::conditional_t<(Bits <= 11), std::array<std::size_t, buckets>, std::vector<std::size_t>>;
	Counts count = [] {
		if constexpr (Bits <= 11) return Counts{};
		else return Counts(buckets);
	}();
	for (int shift = 0; shift < 64; shift += Bits) {
		if (!((varying >> shift) & mask)) continue;
		std::fill(count.begin(), count.end(), 0);
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask]++] = std::move(value);
		values.swap(scratch);
	}
}

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	stable_radix_sort<8>(values, std::move(key));
}


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


#line 8 "test/compressed_lazy_segtree_large.test.cpp"

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	std::uint32_t n = 0;
	int q = 0;
	input.read(n), input.read(q);
	struct Query { int type = 0, l = 0, r = 0; Affine f; };
	struct Endpoint { std::uint32_t value; int index; };
	std::vector<Query> queries(q);
	std::vector<Endpoint> endpoints;
	endpoints.reserve(2 * std::size_t(q) + 2);
	endpoints.push_back({0, -1}), endpoints.push_back({n, -1});
	for (int i = 0; i < q; ++i) {
		auto &query = queries[i];
		std::uint32_t l = 0, r = 0;
		input.read(query.type), input.read(l), input.read(r);
		endpoints.push_back({l, 2 * i}), endpoints.push_back({r, 2 * i + 1});
		if (!query.type) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			query.f = {mint::raw(a), mint::raw(b)};
		}
	}
	stable_radix_sort<11>(endpoints, [](const Endpoint &point) { return point.value; });
	std::vector<std::uint32_t> coordinates;
	coordinates.reserve(endpoints.size());
	for (auto point : endpoints) {
		if (coordinates.empty() || coordinates.back() != point.value) coordinates.push_back(point.value);
		if (point.index < 0) continue;
		int index = int(coordinates.size()) - 1;
		if (point.index & 1) queries[point.index / 2].r = index;
		else queries[point.index / 2].l = index;
	}
	std::vector<Sum> values(coordinates.size() - 1);
	for (std::size_t i = 0; i < values.size(); ++i) values[i] = {0, coordinates[i + 1] - coordinates[i]};
	LazySegtree<Sum, op, e, Affine, mapping, composition, id> tree(values);
	for (const auto &query : queries) {
		if (!query.type) tree.apply(query.l, query.r, query.f);
		else output.println(tree.prod(query.l, query.r).sum.val());
	}
}
