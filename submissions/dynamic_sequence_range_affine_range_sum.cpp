#line 1 "test/lazy_reversible_treap_affine.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum"
#line 1 "data_structure/lazy_reversible_treap.hpp"



#line 1 "components/splitmix64.hpp"



#include <cstdint>

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 5 "data_structure/lazy_reversible_treap.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#line 9 "data_structure/lazy_reversible_treap.hpp"
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id, bool commutative = false>
class LazyReversibleTreap {
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int left = 0, right = 0;
		std::uint32_t count : 30 = 1;
		std::uint32_t pending : 1 = 0;
		std::uint32_t reversed : 1 = 0;
		std::uint32_t priority;
		S value, product;
		[[no_unique_address]] Back back;
		F lazy = id();
		static Back make_back(const S &value) {
			if constexpr (commutative) return {};
			else return value;
		}
		Node(S value, std::uint32_t priority) : priority(priority), value(std::move(value)), product(this->value), back(make_back(this->value)) {}
	};
	std::vector<Node> t;
	SplitMix64 rng;
	int root = 0, free = 0;
	bool lazy_applied = false;
	int count(int x) const { return t[x].count; }
	void pull(int x) {
		auto &v = t[x];
		int l = v.left, r = v.right;
		v.count = count(l) + count(r) + 1;
		v.product = l ? op(t[l].product, v.value) : v.value;
		if (r) v.product = op(std::move(v.product), t[r].product);
		if constexpr (!commutative) {
			v.back = r ? op(t[r].back, v.value) : v.value;
			if (l) v.back = op(std::move(v.back), t[l].back);
		}
	}
	void flip(int x) {
		if (!x || count(x) == 1) return;
		auto &v = t[x];
		std::swap(v.left, v.right);
		if constexpr (!commutative) std::swap(v.product, v.back);
		v.reversed ^= 1;
	}
	void act(int x, const F &f) {
		if (!x) return;
		auto &v = t[x];
		if (count(x) == 1) {
			v.value = v.product = mapping(f, v.product, 1);
			if constexpr (!commutative) v.back = v.product;
			return;
		}
		v.product = mapping(f, v.product, count(x));
		if constexpr (!commutative) v.back = mapping(f, v.back, count(x));
		v.lazy = v.pending ? composition(f, v.lazy) : f;
		v.pending = true;
	}
	void push(int x) {
		auto &v = t[x];
		if (v.reversed) {
			flip(v.left);
			flip(v.right);
			v.reversed = 0;
		}
		if (v.pending) {
			v.value = mapping(v.lazy, v.value, 1);
			act(v.left, v.lazy);
			act(v.right, v.lazy);
			v.pending = false;
		}
	}
	int make(S value) {
		if (free) {
			int x = free;
			free = t[x].left;
			t[x] = Node(std::move(value), std::uint32_t(rng()));
			return x;
		}
		assert(t.size() <= (std::size_t(1) << 30) - 1);
		t.emplace_back(std::move(value), std::uint32_t(rng()));
		return int(t.size()) - 1;
	}
	std::pair<int, int> split(int x, int k) {
		if (!x) return {0, 0};
		if (!k) return {0, x};
		if (k == count(x)) return {x, 0};
		push(x);
		int left = count(t[x].left);
		if (k <= left) {
			auto [a, b] = split(t[x].left, k);
			t[x].left = b;
			pull(x);
			return {a, x};
		}
		auto [a, b] = split(t[x].right, k - left - 1);
		t[x].right = a;
		pull(x);
		return {x, b};
	}
	int merge(int a, int b) {
		if (!a || !b) return a | b;
		if (t[a].priority > t[b].priority) {
			push(a);
			t[a].right = merge(t[a].right, b);
			pull(a);
			return a;
		}
		push(b);
		t[b].left = merge(a, t[b].left);
		pull(b);
		return b;
	}
	int insert_node(int x, int p, int added) {
		if (!x) return added;
		if (t[added].priority >= t[x].priority) {
			auto [a, b] = split(x, p);
			t[added].left = a;
			t[added].right = b;
			pull(added);
			return added;
		}
		push(x);
		int left = count(t[x].left);
		if (p <= left) t[x].left = insert_node(t[x].left, p, added);
		else t[x].right = insert_node(t[x].right, p - left - 1, added);
		pull(x);
		return x;
	}
	int erase_node(int x, int p, S &value) {
		push(x);
		int left = count(t[x].left);
		if (p == left) {
			value = std::move(t[x].value);
			int result = merge(t[x].left, t[x].right);
			t[x].left = free;
			free = x;
			return result;
		}
		if (p < left) t[x].left = erase_node(t[x].left, p, value);
		else t[x].right = erase_node(t[x].right, p - left - 1, value);
		pull(x);
		return x;
	}
	void set_node(int x, int p, S value) {
		push(x);
		int left = count(t[x].left);
		if (p == left) t[x].value = std::move(value);
		else if (p < left) set_node(t[x].left, p, std::move(value));
		else set_node(t[x].right, p - left - 1, std::move(value));
		pull(x);
	}
	S fold(int x, int l, int r) {
		if (!l && r == count(x)) return t[x].product;
		push(x);
		int left = count(t[x].left);
		if (r <= left) return fold(t[x].left, l, r);
		if (left < l) return fold(t[x].right, l - left - 1, r - left - 1);
		S result = t[x].value;
		if (l < left) result = op(fold(t[x].left, l, left), std::move(result));
		if (left + 1 < r) result = op(std::move(result), fold(t[x].right, 0, r - left - 1));
		return result;
	}
	S fold_lazy(int x, int l, int r, const F *carry = nullptr, bool reversed = false) const {
		const auto &v = t[x];
		if (!l && r == count(x)) {
			S result = v.product;
			if constexpr (!commutative) if (reversed) result = v.back;
			return carry ? mapping(*carry, std::move(result), count(x)) : result;
		}
		F combined = id();
		if (v.pending) {
			combined = carry ? composition(*carry, v.lazy) : v.lazy;
			carry = &combined;
		}
		int a = reversed ? v.right : v.left, b = reversed ? v.left : v.right;
		int left = count(a);
		reversed ^= v.reversed;
		if (r <= left) return fold_lazy(a, l, r, carry, reversed);
		if (left < l) return fold_lazy(b, l - left - 1, r - left - 1, carry, reversed);
		S result = carry ? mapping(*carry, v.value, 1) : v.value;
		if (l < left) result = op(fold_lazy(a, l, left, carry, reversed), std::move(result));
		if (left + 1 < r) result = op(std::move(result), fold_lazy(b, 0, r - left - 1, carry, reversed));
		return result;
	}
	void apply_node(int x, int l, int r, const F &f) {
		if (!l && r == count(x)) { act(x, f); return; }
		push(x);
		int left = count(t[x].left);
		if (l < left) apply_node(t[x].left, l, std::min(r, left), f);
		if (l <= left && left < r) t[x].value = mapping(f, t[x].value, 1);
		if (left + 1 < r) apply_node(t[x].right, std::max(l - left - 1, 0), r - left - 1, f);
		pull(x);
	}
	int reverse_node(int x, int l, int r) {
		if (!l && r == count(x)) { flip(x); return x; }
		push(x);
		int left = count(t[x].left);
		if (r <= left) t[x].left = reverse_node(t[x].left, l, r);
		else if (left < l) t[x].right = reverse_node(t[x].right, l - left - 1, r - left - 1);
		else {
			auto [a, b] = split(t[x].left, l);
			auto [c, d] = split(t[x].right, r - left - 1);
			t[x].left = b;
			t[x].right = c;
			pull(x);
			flip(x);
			return merge(a, merge(x, d));
		}
		pull(x);
		return x;
	}
	void collect(int x, std::vector<S> &values) {
		if (!x) return;
		push(x);
		collect(t[x].left, values);
		values.push_back(t[x].value);
		collect(t[x].right, values);
	}
	void bounds(int l, int r) const { assert(0 <= l && l <= r && r <= size()); (void)l; (void)r; }

  public:
	using value_type = S;
	using action_type = F;
	explicit LazyReversibleTreap(std::uint64_t seed = 0) : rng(seed) { t.emplace_back(e(), 0); t[0].count = 0; }
	explicit LazyReversibleTreap(const std::vector<S> &values, std::uint64_t seed = 0) : LazyReversibleTreap(seed) { assign(values); }
	LazyReversibleTreap(const LazyReversibleTreap &) = default;
	LazyReversibleTreap &operator=(const LazyReversibleTreap &) = default;
	LazyReversibleTreap(LazyReversibleTreap &&other) : LazyReversibleTreap() { swap(other); }
	LazyReversibleTreap &operator=(LazyReversibleTreap &&other) {
		if (this != &other) { LazyReversibleTreap moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(LazyReversibleTreap &other) noexcept {
		t.swap(other.t);
		std::swap(rng, other.rng);
		std::swap(root, other.root);
		std::swap(free, other.free);
		std::swap(lazy_applied, other.lazy_applied);
	}
	int size() const { return count(root); }
	bool empty() const { return !root; }
	void reserve(std::size_t n) { assert(n < (std::size_t(1) << 30)); t.reserve(n + 1); }
	std::size_t memory_usage() const { return t.capacity() * sizeof(Node); }
	void clear() { t.erase(t.begin() + 1, t.end()); root = free = 0; lazy_applied = false; }
	void assign(const std::vector<S> &values) {
		clear();
		reserve(values.size());
		std::vector<int> path;
		for (const S &value : values) {
			int x = make(value), last = 0;
			while (!path.empty() && t[path.back()].priority <= t[x].priority) {
				last = path.back(); path.pop_back(); pull(last);
			}
			t[x].left = last;
			if (!path.empty()) t[path.back()].right = x;
			path.push_back(x);
		}
		if (!path.empty()) root = path[0];
		while (!path.empty()) { pull(path.back()); path.pop_back(); }
	}
	void insert(int p, S value) { bounds(p, p); root = insert_node(root, p, make(std::move(value))); }
	S erase(int p) { bounds(p, p + 1); S value = e(); root = erase_node(root, p, value); return value; }
	S get(int p) {
		bounds(p, p + 1);
		int x = root;
		for (;;) {
			push(x);
			int left = count(t[x].left);
			if (p == left) return t[x].value;
			if (p < left) x = t[x].left;
			else p -= left + 1, x = t[x].right;
		}
	}
	void set(int p, S value) { bounds(p, p + 1); set_node(root, p, std::move(value)); }
	S prod(int l, int r) { bounds(l, r); return l == r ? e() : lazy_applied ? fold_lazy(root, l, r) : fold(root, l, r); }
	S all_prod() const { return t[root].product; }
	void apply(int p, const F &f) { apply(p, p + 1, f); }
	void apply(int l, int r, const F &f) { bounds(l, r); if (l != r) { lazy_applied = true; apply_node(root, l, r, f); } }
	void reverse(int l, int r) { bounds(l, r); if (r - l > 1) root = reverse_node(root, l, r); }
	void rotate(int l, int m, int r) {
		bounds(l, m); bounds(m, r);
		if (l == m || m == r) return;
		auto [a, b] = split(root, l);
		auto [c, d] = split(b, m - l);
		auto [f, g] = split(d, r - m);
		root = merge(a, merge(f, merge(c, g)));
	}
	void move(int l, int r, int p) {
		bounds(l, r);
		assert(0 <= p && p <= size() - (r - l));
		if (l == r || l == p) return;
		auto [a, b] = split(root, l);
		auto [c, d] = split(b, r - l);
		auto [f, g] = split(merge(a, d), p);
		root = merge(f, merge(c, g));
	}
	std::vector<S> to_vector() { std::vector<S> values; values.reserve(size()); collect(root, values); return values; }
};

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


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


#line 6 "test/lazy_reversible_treap_affine.test.cpp"

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint s, int n) { return f.a * s + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	LazyReversibleTreap<mint, op, e, F, mapping, composition, id, true> tree(a);
	tree.reserve(n + q);
	while (q--) {
		int type = 0, l = 0;
		input.read(type); input.read(l);
		if (type == 0) { unsigned x = 0; input.read(x); tree.insert(l, mint::raw(x)); }
		else if (type == 1) tree.erase(l);
		else {
			int r = 0;
			input.read(r);
			if (type == 2) tree.reverse(l, r);
			else if (type == 3) { unsigned b = 0, c = 0; input.read(b); input.read(c); tree.apply(l, r, {mint::raw(b), mint::raw(c)}); }
			else { output.write(tree.prod(l, r).val()); output.write('\n'); }
		}
	}
}
