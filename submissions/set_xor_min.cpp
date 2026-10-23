#line 1 "test/set_xor_min.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#line 1 "string/trie.hpp"



#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace trie_detail {
struct Empty {};
}

template<int Alphabet = 26, int First = 'a', class Data = trie_detail::Empty>
class Trie {
	static_assert(0 < Alphabet && 0 <= First && First <= 255 && Alphabet <= 256 - First);
	struct Node {
		std::array<int, Alphabet> next;
		int parent, depth;
		char letter;
		std::int64_t count = 0, terminal = 0;
		[[no_unique_address]] Data data{};
		Node(int p = -1, int d = 0, char c = 0) : parent(p), depth(d), letter(c) { next.fill(-1); }
	};
	std::vector<Node> nodes_;
	int distinct_ = 0;
	static int symbol_(char c) { return int(static_cast<unsigned char>(c)) - First; }
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }
	void check_count_(std::int64_t copies) const {
		assert(copies > 0);
		if (copies > std::numeric_limits<std::int64_t>::max() - size()) throw std::overflow_error("Trie count");
	}
	int add_(int v, std::int64_t copies) {
		if (!nodes_[v].terminal) ++distinct_;
		nodes_[v].terminal += copies;
		for (int u = v; u != -1; u = nodes_[u].parent) nodes_[u].count += copies;
		return v;
	}

  public:
	static constexpr int root = 0;
	Trie() { nodes_.emplace_back(); }
	std::int64_t size() const { return nodes_[root].count; }
	bool empty() const { return size() == 0; }
	int distinct_size() const { return distinct_; }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	void reserve(int total_length) {
		assert(0 <= total_length && total_length < std::numeric_limits<int>::max());
		nodes_.reserve(std::size_t(total_length) + 1);
	}
	void clear() { nodes_.clear(), nodes_.emplace_back(); distinct_ = 0; }
	int ensure(std::string_view s) {
		assert(s.size() < std::size_t(std::numeric_limits<int>::max()));
		int v = root;
		for (char c : s) {
			int x = symbol_(c);
			assert(0 <= x && x < Alphabet);
			int u = nodes_[v].next[x];
			if (u == -1) {
				assert(node_count() < std::numeric_limits<int>::max());
				u = node_count();
				nodes_.emplace_back(v, nodes_[v].depth + 1, c);
				nodes_[v].next[x] = u;
			}
			v = u;
		}
		return v;
	}
	int insert(std::string_view s, std::int64_t copies = 1) {
		check_count_(copies);
		return add_(ensure(s), copies);
	}
	int insert_node(int v, std::int64_t copies = 1) {
		check_(v), check_count_(copies);
		return add_(v, copies);
	}
	bool erase_node(int v, std::int64_t copies = 1) {
		check_(v), assert(copies > 0);
		if (nodes_[v].terminal < copies) return false;
		nodes_[v].terminal -= copies;
		if (!nodes_[v].terminal) --distinct_;
		for (int u = v; u != -1; u = nodes_[u].parent) nodes_[u].count -= copies;
		return true;
	}
	bool erase(std::string_view s, std::int64_t copies = 1) {
		assert(copies > 0);
		int v = find(s);
		return v != -1 && erase_node(v, copies);
	}
	int parent(int v) const { check_(v); return nodes_[v].parent; }
	int depth(int v) const { check_(v); return nodes_[v].depth; }
	char letter(int v) const { check_(v); assert(v != root); return nodes_[v].letter; }
	const std::array<int, Alphabet> &children(int v) const { check_(v); return nodes_[v].next; }
	int child(int v, char c) const {
		check_(v);
		int x = symbol_(c);
		return 0 <= x && x < Alphabet ? nodes_[v].next[x] : -1;
	}
	std::int64_t terminal_count(int v) const { check_(v); return nodes_[v].terminal; }
	std::int64_t prefix_count(int v) const { check_(v); return nodes_[v].count; }
	Data &data(int v) { check_(v); return nodes_[v].data; }
	const Data &data(int v) const { check_(v); return nodes_[v].data; }
	int walk(int v, std::string_view s) const {
		check_(v);
		for (char c : s) { v = child(v, c); if (v == -1) break; }
		return v;
	}
	int find(std::string_view s) const { return walk(root, s); }
	std::int64_t count(std::string_view s) const { int v = find(s); return v == -1 ? 0 : nodes_[v].terminal; }
	bool contains(std::string_view s) const { return count(s) != 0; }
	std::int64_t prefix_count(std::string_view s) const { int v = find(s); return v == -1 ? 0 : nodes_[v].count; }
	std::string restore(int v) const {
		check_(v);
		std::string s(nodes_[v].depth, '\0');
		for (int i = int(s.size()); i > 0; v = nodes_[v].parent) s[--i] = nodes_[v].letter;
		return s;
	}
	template<class F>
	void for_each_ancestor(int v, F &&visit) const {
		check_(v);
		for (; v != -1; v = nodes_[v].parent) visit(v);
	}
	template<class F>
	void for_each_prefix(std::string_view s, F &&visit) const {
		int v = root;
		if (nodes_[v].terminal) visit(v);
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			if (nodes_[v].terminal) visit(v);
		}
	}
	int longest_prefix(std::string_view s) const {
		int v = -1;
		for_each_prefix(s, [&](int u) { v = u; });
		return v;
	}
	int lcp(std::string_view s) const {
		int v = root, length = 0;
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			++length;
		}
		return length;
	}
	std::int64_t lcp_sum(std::string_view s) const {
		int v = root;
		std::int64_t sum = 0;
		for (char c : s) {
			v = child(v, c);
			if (v == -1 || !nodes_[v].count) break;
			if (nodes_[v].count > std::numeric_limits<std::int64_t>::max() - sum) throw std::overflow_error("Trie LCP sum");
			sum += nodes_[v].count;
		}
		return sum;
	}
	std::int64_t order_of_key(std::string_view s) const {
		int v = root;
		std::int64_t rank = 0;
		for (char c : s) {
			rank += nodes_[v].terminal;
			int x = symbol_(c);
			for (int j = 0; j < Alphabet && j < x; ++j) {
				int u = nodes_[v].next[j];
				if (u != -1) rank += nodes_[u].count;
			}
			if (x < 0 || x >= Alphabet) break;
			v = nodes_[v].next[x];
			if (v == -1) break;
		}
		return rank;
	}
	int kth_node(std::int64_t k) const {
		if (k < 0 || k >= size()) return -1;
		int v = root;
		while (true) {
			if (k < nodes_[v].terminal) return v;
			k -= nodes_[v].terminal;
			for (int u : nodes_[v].next) if (u != -1) {
				if (k < nodes_[u].count) { v = u; break; }
				k -= nodes_[u].count;
			}
		}
	}
	std::optional<std::string> kth(std::int64_t k) const {
		int v = kth_node(k);
		return v == -1 ? std::nullopt : std::optional<std::string>(restore(v));
	}
	std::optional<std::string> lower_bound(std::string_view s) const { return kth(order_of_key(s)); }
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
#line 6 "components/fast_io.hpp"
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#include <cstddef>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 17 "components/fast_io.hpp"
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


#line 6 "test/set_xor_min.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	Trie<2, '0'> trie;
	while (q--) {
		int type = 0;
		unsigned x = 0;
		input.read(type), input.read(x);
		std::array<char, 30> digits;
		for (int i = 0; i < 30; ++i) digits[i] = '0' + ((x >> (29 - i)) & 1);
		const std::string_view key(digits.data(), digits.size());
		if (type == 0) {
			if (!trie.contains(key)) trie.insert(key);
		} else if (type == 1) trie.erase(key);
		else {
			int node = trie.root;
			unsigned answer = 0;
			for (int i = 0; i < 30; ++i) {
				int next = trie.child(node, digits[i]);
				if (next == -1 || !trie.prefix_count(next)) {
					next = trie.child(node, char(digits[i] ^ 1));
					answer |= 1U << (29 - i);
				}
				node = next;
			}
			output.println(answer);
		}
	}
}
