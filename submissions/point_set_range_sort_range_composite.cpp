#line 1 "test/sortable_segtree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_sort_range_composite"
#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
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


#line 1 "data_structure/segment_tree/sortable_segtree.hpp"



#line 1 "components/predecessor_set.hpp"



#line 11 "components/predecessor_set.hpp"
#include <limits>
#include <span>
#line 14 "components/predecessor_set.hpp"
#include <utility>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace predecessor_set_detail {
inline void pack_scalar(const char *src, std::uint64_t *dst, std::size_t words) {
	for (std::size_t i = 0; i < words; ++i) {
		std::uint64_t word = 0;
		if constexpr (std::endian::native == std::endian::little) {
			for (int j = 0; j < 8; ++j) {
				std::uint64_t x;
				std::memcpy(&x, src + i * 64 + j * 8, 8);
				assert((x & 0xfefefefefefefefeULL) == 0x3030303030303030ULL);
				word |= ((x & 0x0101010101010101ULL) * 0x0102040810204080ULL >> 56) << (j * 8);
			}
		} else {
			for (int j = 0; j < 64; ++j) {
				assert(src[i * 64 + j] == '0' || src[i * 64 + j] == '1');
				word |= std::uint64_t(src[i * 64 + j] - '0') << j;
			}
		}
		dst[i] = word;
	}
}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2"))) inline void pack_avx2(const char *src, std::uint64_t *dst, std::size_t words) {
	const auto ones = _mm256_set1_epi8('1');
	for (std::size_t i = 0; i < words; ++i) {
		auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64));
		auto b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64 + 32));
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(a, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(b, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		auto low = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(a, ones)));
		auto high = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(b, ones)));
		dst[i] = low | (std::uint64_t(high) << 32);
	}
}
#endif
inline void pack(const char *src, std::uint64_t *dst, std::size_t words) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (words >= 4 && __builtin_cpu_supports("avx2")) return pack_avx2(src, dst, words);
#endif
	pack_scalar(src, dst, words);
}
}

class PredecessorSet {
	int n = 0, count = 0, height = 0;
	std::array<int, 6> offset{}, length{};
	std::vector<std::uint64_t> data;
	void mask_tail() {
		int bits = n;
		for (int h = 0; h < height; ++h) {
			if (bits & 63) data[offset[h] + length[h] - 1] &= ~std::uint64_t(0) >> (64 - (bits & 63));
			bits = length[h];
		}
	}
	void rebuild() {
		count = 0;
		for (int i = 0; i < length[0]; ++i) count += std::popcount(data[i]);
		for (int h = 1; h < height; ++h)
			for (int i = 0; i < length[h - 1]; ++i)
				if (data[offset[h - 1] + i]) data[offset[h] + (i >> 6)] |= std::uint64_t(1) << (i & 63);
	}
public:
	explicit PredecessorSet(int n = 0, bool filled = false) { assign(n, filled); }
	explicit PredecessorSet(std::string_view bits) { assign(bits); }
	PredecessorSet(int n, std::span<const std::uint64_t> words) { assign(n, words); }
	PredecessorSet(const PredecessorSet &) = default;
	PredecessorSet &operator=(const PredecessorSet &) = default;
	PredecessorSet(PredecessorSet &&other) noexcept : PredecessorSet() { swap(other); }
	PredecessorSet &operator=(PredecessorSet &&other) noexcept {
		if (this != &other) { PredecessorSet moved(std::move(other)); swap(moved); }
		return *this;
	}
	void assign(int length, bool filled = false) {
		assert(length >= 0);
		n = length; count = filled ? n : 0; height = 0;
		offset.fill(0); this->length.fill(0);
		int total = 0;
		for (std::size_t bits = n; bits;) {
			std::size_t words = (bits + 63) / 64;
			offset[height] = total; this->length[height++] = words;
			total += words;
			if (words == 1) break;
			bits = words;
		}
		data.assign(total, filled ? ~std::uint64_t(0) : 0);
		if (filled) mask_tail();
	}
	void assign(std::string_view bits) {
		assert(bits.size() <= std::size_t(std::numeric_limits<int>::max()));
		PredecessorSet next(static_cast<int>(bits.size()));
		std::size_t words = bits.size() / 64;
		predecessor_set_detail::pack(bits.data(), next.data.data(), words);
		for (std::size_t i = words * 64; i < bits.size(); ++i) {
			assert(bits[i] == '0' || bits[i] == '1');
			next.data[i >> 6] |= std::uint64_t(bits[i] - '0') << (i & 63);
		}
		next.rebuild(); swap(next);
	}
	void assign(int n, std::span<const std::uint64_t> words) {
		assert(n >= 0 && words.size() == (std::size_t(n) + 63) / 64);
		PredecessorSet next(n);
		std::copy(words.begin(), words.end(), next.data.begin());
		if (n & 63) next.data[next.length[0] - 1] &= ~std::uint64_t(0) >> (64 - (n & 63));
		next.rebuild(); swap(next);
	}
	int universe_size() const { return n; }
	int size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(int p) const { assert(0 <= p && p < n); return data[p >> 6] >> (p & 63) & 1; }
	bool insert(int p) {
		assert(0 <= p && p < n);
		if (contains(p)) return false;
		++count;
		for (int h = 0; h < height; ++h) {
			auto &word = data[offset[h] + (p >> 6)];
			auto before = word;
			word |= std::uint64_t(1) << (p & 63);
			if (before) break;
			p >>= 6;
		}
		return true;
	}
	bool erase(int p) {
		assert(0 <= p && p < n);
		if (!contains(p)) return false;
		--count;
		for (int h = 0; h < height; ++h) {
			auto &word = data[offset[h] + (p >> 6)];
			word &= ~(std::uint64_t(1) << (p & 63));
			if (word) break;
			p >>= 6;
		}
		return true;
	}
	int next(int p) const {
		p = std::max(p, 0);
		if (p >= n || !count) return n;
		for (int h = 0; h < height; ++h) {
			if ((p >> 6) >= length[h]) return n;
			auto word = data[offset[h] + (p >> 6)] >> (p & 63);
			if (word) {
				p += std::countr_zero(word);
				while (h--) p = p * 64 + std::countr_zero(data[offset[h] + p]);
				return p;
			}
			p = (p >> 6) + 1;
		}
		return n;
	}
	int prev(int p) const {
		p = std::min(p, n - 1);
		if (p < 0 || !count) return -1;
		for (int h = 0; h < height; ++h) {
			auto word = data[offset[h] + (p >> 6)] << (63 - (p & 63));
			if (word) {
				p -= std::countl_zero(word);
				while (h--) p = p * 64 + 63 - std::countl_zero(data[offset[h] + p]);
				return p;
			}
			p = (p >> 6) - 1;
			if (p < 0) return -1;
		}
		return -1;
	}
	bool set(int p, bool present) { return present ? insert(p) : erase(p); }
	int min() const { return next(0); }
	int max() const { return prev(n - 1); }
	void clear() { std::fill(data.begin(), data.end(), 0); count = 0; }
	void fill() {
		std::fill(data.begin(), data.end(), ~std::uint64_t(0));
		mask_tail();
		count = n;
	}
	template <class F> void for_each(int l, int r, F &&f) const {
		assert(0 <= l && l <= r && r <= n);
		for (int p = next(l); p < r;) {
			int base = p & ~63;
			auto word = data[p >> 6] & (~std::uint64_t(0) << (p & 63));
			if (r - base < 64) word &= ~std::uint64_t(0) >> (64 - (r - base));
			while (word) { f(base + std::countr_zero(word)); word &= word - 1; }
			if (r - base <= 64) break;
			p = next(base + 64);
		}
	}
	template <class F> void for_each(F &&f) const { for_each(0, n, f); }
	std::vector<int> to_vector() const {
		std::vector<int> out; out.reserve(count);
		for_each([&](int x) { out.push_back(x); }); return out;
	}
	std::span<const std::uint64_t> words() const & { return {data.data(), std::size_t(length[0])}; }
	void words() const && = delete;
	void swap(PredecessorSet &other) noexcept {
		using std::swap;
		swap(n, other.n); swap(count, other.count); swap(height, other.height);
		offset.swap(other.offset); length.swap(other.length); data.swap(other.data);
	}
	std::size_t memory_usage() const { return data.capacity() * sizeof(std::uint64_t); }
};


#line 14 "data_structure/segment_tree/sortable_segtree.hpp"

template <class Key, class S, auto op, auto e, bool commutative = false>
class SortableSegtree {
	static_assert(std::is_integral_v<Key> && !std::is_same_v<Key, bool> && sizeof(Key) <= 8);
	using U = std::make_unsigned_t<Key>;
	static constexpr U sign = std::is_signed_v<Key> ? U(1) << (std::numeric_limits<U>::digits - 1) : 0;
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int left = 0, right = 0, count = 1, bit = -1, leaf;
		S forward;
		[[no_unique_address]] Back backward;
		static Back back(const S &value) { if constexpr (commutative) return {}; else return value; }
		Node(S value, int leaf) : leaf(leaf), forward(std::move(value)), backward(back(forward)) {}
	};
	int n = 0, base = 1, free = 0;
	std::vector<U> keys;
	std::vector<Node> pool;
	std::vector<int> roots;
	std::vector<unsigned char> reversed;
	std::vector<S> aggregate;
	PredecessorSet boundaries;
	int difference(int a, int b) const {
		U x = keys[pool[a].leaf] ^ keys[pool[b].leaf];
		return x ? 30 + int(std::bit_width(x)) : int(std::bit_width(unsigned(pool[a].leaf ^ pool[b].leaf))) - 1;
	}
	bool side(int x, int bit) const {
		return bit >= 31 ? keys[pool[x].leaf] >> (bit - 31) & 1 : unsigned(pool[x].leaf) >> bit & 1;
	}
	S product(int x, [[maybe_unused]] bool reverse) const {
		if constexpr (commutative) return pool[x].forward;
		else return reverse ? pool[x].backward : pool[x].forward;
	}
	void pull(int x) {
		auto &node = pool[x];
		const auto &a = pool[node.left], &b = pool[node.right];
		node.leaf = a.leaf;
		node.count = a.count + b.count;
		node.forward = op(a.forward, b.forward);
		if constexpr (!commutative) node.backward = op(b.backward, a.backward);
	}
	void release(int x) { pool[x].left = free; free = x; }
	int branch(int bit, int a, int b) {
		if (side(a, bit)) std::swap(a, b);
		int x;
		if (free) { x = free; free = pool[x].left; }
		else { x = pool.size(); pool.emplace_back(e(), 0); }
		pool[x].left = a; pool[x].right = b; pool[x].bit = bit;
		pull(x); return x;
	}
	int meld(int a, int b) {
		if (!a || !b) return a | b;
		int bit = difference(a, b), ab = pool[a].bit, bb = pool[b].bit;
		if (bit > std::max(ab, bb)) return branch(bit, a, b);
		assert(ab >= 0 || bb >= 0);
		if (ab == bb) {
			pool[a].left = meld(pool[a].left, pool[b].left);
			pool[a].right = meld(pool[a].right, pool[b].right);
			release(b);
		} else {
			if (ab < bb) std::swap(a, b), std::swap(ab, bb);
			if (side(b, ab)) pool[a].right = meld(pool[a].right, b);
			else pool[a].left = meld(pool[a].left, b);
		}
		pull(a); return a;
	}
	std::pair<int, int> split(int x, int k) {
		if (!k) return {0, x};
		if (k == pool[x].count) return {x, 0};
		int a = pool[x].left, b = pool[x].right, count = pool[a].count;
		if (k == count) { release(x); return {a, b}; }
		if (k < count) {
			auto [first, second] = split(a, k);
			pool[x].left = second; pull(x); return {first, x};
		}
		auto [first, second] = split(b, k - count);
		pool[x].right = first; pull(x); return {x, second};
	}
	int at(int x, int k) const {
		while (pool[x].bit >= 0) {
			int count = pool[pool[x].left].count;
			if (k < count) x = pool[x].left;
			else k -= count, x = pool[x].right;
		}
		return x;
	}
	[[gnu::always_inline]] int locate(int p) const {
		assert(0 <= p && p < n);
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		return at(x, reversed[l] ? pool[x].count - 1 - k : k);
	}
	S fold(int x, int l, int r, bool reverse) const {
		if (!l && r == pool[x].count) return product(x, reverse);
		int a = reverse ? pool[x].right : pool[x].left, b = reverse ? pool[x].left : pool[x].right;
		int count = pool[a].count;
		if (r <= count) return fold(a, l, r, reverse);
		if (l >= count) return fold(b, l - count, r - count, reverse);
		return op(fold(a, l, count, reverse), fold(b, 0, r - count, reverse));
	}
	void update(int p, S value) {
		aggregate[p += base] = std::move(value);
		while (p >>= 1) aggregate[p] = op(aggregate[p * 2], aggregate[p * 2 + 1]);
	}
	S middle_prod(int l, int r) const {
		S a = e(), b = e();
		for (l += base, r += base; l < r; l >>= 1, r >>= 1) {
			if (l & 1) a = op(std::move(a), aggregate[l++]);
			if (r & 1) b = op(aggregate[--r], std::move(b));
		}
		return op(std::move(a), std::move(b));
	}
	void cut(int p) {
		if (boundaries.contains(p)) return;
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		if (reversed[l]) k = pool[x].count - k;
		auto [a, b] = split(x, k);
		if (reversed[l]) std::swap(a, b);
		roots[l] = a; roots[p] = b; reversed[p] = reversed[l];
		boundaries.insert(p);
		update(l, product(a, reversed[l])); update(p, product(b, reversed[p]));
	}
	void set_value(int x, int k, S value) {
		if (pool[x].bit < 0) {
			pool[x].forward = std::move(value);
			if constexpr (!commutative) pool[x].backward = pool[x].forward;
			return;
		}
		int count = pool[pool[x].left].count;
		if (k < count) set_value(pool[x].left, k, std::move(value));
		else set_value(pool[x].right, k - count, std::move(value));
		pull(x);
	}
	void collect(int x, bool reverse, std::vector<std::pair<Key, S>> &out) const {
		if (pool[x].bit < 0) { out.emplace_back(Key(keys[pool[x].leaf] ^ sign), pool[x].forward); return; }
		collect(reverse ? pool[x].right : pool[x].left, reverse, out);
		collect(reverse ? pool[x].left : pool[x].right, reverse, out);
	}
public:
	SortableSegtree() : aggregate(2, e()) {}
	explicit SortableSegtree(const std::vector<std::pair<Key, S>> &a) { assign(a); }
	SortableSegtree(const SortableSegtree &) = default;
	SortableSegtree &operator=(const SortableSegtree &) = default;
	SortableSegtree(SortableSegtree &&other) : SortableSegtree() { swap(other); }
	SortableSegtree &operator=(SortableSegtree &&other) {
		if (this != &other) { SortableSegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(SortableSegtree &other) {
		using std::swap;
		swap(n, other.n); swap(base, other.base); swap(free, other.free);
		keys.swap(other.keys); pool.swap(other.pool); roots.swap(other.roots); reversed.swap(other.reversed);
		aggregate.swap(other.aggregate); swap(boundaries, other.boundaries);
	}
	void assign(const std::vector<std::pair<Key, S>> &a) {
		assert(a.size() < (std::size_t(1) << 29));
		n = a.size(); base = std::bit_ceil(unsigned(std::max(n, 1))); free = 0;
		keys.clear(); keys.reserve(n);
		pool.clear(); pool.reserve(2 * std::size_t(n) + 1);
		pool.emplace_back(e(), 0); pool[0].count = 0;
		roots.resize(n); reversed.assign(n, 0); aggregate.assign(2 * base, e());
		boundaries.assign(n + 1, true);
		for (int i = 0; i < n; ++i) {
			keys.push_back(U(a[i].first) ^ sign);
			pool.emplace_back(a[i].second, i); roots[i] = i + 1;
			aggregate[base + i] = a[i].second;
		}
		for (int i = base - 1; i; --i) aggregate[i] = op(aggregate[i * 2], aggregate[i * 2 + 1]);
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	std::pair<Key, S> get(int p) const {
		int x = locate(p);
		return {Key(keys[pool[x].leaf] ^ sign), pool[x].forward};
	}
	Key key(int p) const { return Key(keys[pool[locate(p)].leaf] ^ sign); }
	void set(int p, Key key, S value) {
		assert(0 <= p && p < n);
		cut(p); cut(p + 1);
		int x = roots[p]; keys[pool[x].leaf] = U(key) ^ sign;
		pool[x].forward = std::move(value);
		if constexpr (!commutative) pool[x].backward = pool[x].forward;
		update(p, pool[x].forward);
	}
	void set(int p, S value) {
		assert(0 <= p && p < n);
		int l = boundaries.prev(p), x = roots[l], k = p - l;
		set_value(x, reversed[l] ? pool[x].count - 1 - k : k, std::move(value));
		update(l, product(x, reversed[l]));
	}
	S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if (l == r) return e();
		int first = boundaries.prev(l), last = boundaries.prev(r - 1);
		if (first == last) return fold(roots[first], l - first, r - first, reversed[first]);
		int end = first + pool[roots[first]].count;
		S value = fold(roots[first], l - first, end - first, reversed[first]);
		if (end < last) value = op(std::move(value), middle_prod(end, last));
		return op(std::move(value), fold(roots[last], 0, r - last, reversed[last]));
	}
	S all_prod() const { return aggregate[1]; }
	void sort(int l, int r, bool descending = false) {
		assert(0 <= l && l <= r && r <= n);
		if (r - l <= 1) return;
		cut(l); cut(r);
		for (int p = boundaries.next(l + 1); p < r; p = boundaries.next(p + 1)) {
			roots[l] = meld(roots[l], roots[p]); roots[p] = 0;
			boundaries.erase(p); update(p, e());
		}
		reversed[l] = descending; update(l, product(roots[l], descending));
	}
	std::vector<std::pair<Key, S>> to_vector() const {
		std::vector<std::pair<Key, S>> out;
		out.reserve(n);
		for (int i = 0; i < n; ++i) if (roots[i]) collect(roots[i], reversed[i], out);
		return out;
	}
	void clear() { assign({}); }
	std::size_t memory_usage() const {
		return keys.capacity() * sizeof(U) + pool.capacity() * sizeof(Node) + roots.capacity() * sizeof(int)
			+ reversed.capacity() + aggregate.capacity() * sizeof(S) + boundaries.memory_usage();
	}
};


#line 4 "test/sortable_segtree.test.cpp"

struct Affine { std::uint32_t a, b; };
Affine compose(Affine f, Affine g) {
	return {std::uint32_t(std::uint64_t(g.a) * f.a % 998244353), std::uint32_t((std::uint64_t(g.a) * f.b + g.b) % 998244353)};
}
Affine identity() { return {1, 0}; }

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<std::pair<std::uint32_t, Affine>> a(n);
	for (auto &[key, f] : a) in.read(key), in.read(f.a), in.read(f.b);
	SortableSegtree<std::uint32_t, Affine, compose, identity> tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		in.read(type), in.read(l);
		if (type == 0) {
			std::uint32_t key = 0, x = 0, y = 0;
			in.read(key), in.read(x), in.read(y);
			tree.set(l, key, {x, y});
		} else {
			in.read(r);
			if (type == 1) {
				std::uint32_t x = 0;
				in.read(x);
				auto f = tree.prod(l, r);
				out.println((std::uint64_t(f.a) * x + f.b) % 998244353);
			} else tree.sort(l, r, type == 3);
		}
	}
}
