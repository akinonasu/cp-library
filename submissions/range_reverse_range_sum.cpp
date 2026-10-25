#line 1 "test/lazy_reversible_treap_reverse.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_reverse_range_sum"
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
	template <bool resize = true>
	void pull(int x) {
		auto &v = t[x];
		int l = v.left, r = v.right;
		if constexpr (resize) v.count = count(l) + count(r) + 1;
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
		pull<false>(x);
	}
	S fold(int x, int l, int r, bool reversed = false) const {
		for (;;) {
			const auto &v = t[x];
			if (!l && r == count(x)) {
				if constexpr (!commutative) if (reversed) return v.back;
				return v.product;
			}
			int a = reversed ? v.right : v.left, b = reversed ? v.left : v.right;
			int left = count(a);
			reversed ^= v.reversed;
			if (r <= left) { x = a; continue; }
			if (left < l) { x = b; l -= left + 1; r -= left + 1; continue; }
			S result = v.value;
			if (l < left) result = op(fold(a, l, left, reversed), std::move(result));
			if (left + 1 < r) result = op(std::move(result), fold(b, 0, r - left - 1, reversed));
			return result;
		}
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
		pull<false>(x);
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
			flip(b);
			flip(c);
			t[x].left = merge(a, c);
			t[x].right = merge(b, d);
		}
		if constexpr (!commutative) pull<false>(x);
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


#line 1 "components/fast_io_bulk.hpp"



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


#line 5 "components/fast_io_bulk.hpp"
#include <limits>
#include <span>
#line 8 "components/fast_io_bulk.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		if (a < 1 || a > 10 || b <= a + 1 || b - a - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		const auto window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	static std::uint32_t one(const char *&p, bool &bad) {
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		bad |= value > 0xffffffffULL;
		p += n + 1;
		return std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				const auto g0 = two(p[0], invalid), g1 = two(p[1], invalid), g2 = two(p[2], invalid), g3 = two(p[3], invalid);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, _mm256_set1_epi32(42)),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, _mm256_set1_epi32(42)), _mm256_cmpgt_epi32(low, _mm256_set1_epi32(94967295))));
				invalid = _mm256_or_si256(invalid, overflow);
				const auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

namespace fast_io_detail {
template <class Emit, std::size_t... I>
void emit_record(Emit &emit, const std::uint32_t *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 5 "test/lazy_reversible_treap_reverse.test.cpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(long long f, long long x, int n) { return x + f * n; }
long long id() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<long long> a(n);
	std::size_t position = 0;
	read_unsigned_records<1>(input, n, [&](std::uint32_t value) { a[position++] = value; });
	LazyReversibleTreap<long long, op, e, long long, mapping, op, id, true> tree(a);
	read_unsigned_records<3>(input, q, [&](std::uint32_t type, std::uint32_t l, std::uint32_t r) {
		if (type == 0) tree.reverse(l, r);
		else { output.write(tree.prod(l, r)); output.write('\n'); }
	});
}
