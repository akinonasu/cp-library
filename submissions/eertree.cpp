#line 1 "test/eertree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/eertree"
#line 1 "string/eertree.hpp"



#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

class Eertree {
	struct Node {
		int length, link, parent, first_end, suffix_count, hits, head, next;
	};
	std::vector<Node> nodes_;
	std::vector<std::uint16_t> text_;
	std::vector<int> prefix_;
	std::array<std::array<int, 256>, 2> roots_;
	int last_ = 1, longest_ = 1;
	std::uint64_t total_ = 0;
	int child_(int v, unsigned char c) const {
		if (v < 2) return roots_[v][c];
		for (int u = nodes_[v].head; u; u = nodes_[u].next)
			if (text_[nodes_[u].first_end] == c) return u;
		return 0;
	}
	int extendable_(int v, int position, unsigned char c) const {
		while (text_[position - nodes_[v].length - 1] != c) v = nodes_[v].link;
		return v;
	}
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }

  public:
	static constexpr int odd_root = 0, even_root = 1;
	Eertree() { clear(); }
	explicit Eertree(std::string_view s) : Eertree() { reserve(static_cast<int>(s.size())); append(s); }
	void clear() {
		nodes_.clear();
		nodes_.push_back({-1, 0, -1, 0, 0, 0, 0, 0});
		nodes_.push_back({0, 0, -1, 0, 0, 0, 0, 0});
		text_.clear(), text_.push_back(256), prefix_.clear();
		roots_[0].fill(0), roots_[1].fill(0);
		last_ = longest_ = even_root, total_ = 0;
	}
	void reserve(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() - 2);
		nodes_.reserve(std::size_t(n) + 2), text_.reserve(std::size_t(n) + 1), prefix_.reserve(n);
	}
	int push_back(char symbol) {
		assert(size() < std::numeric_limits<int>::max() - 2);
		unsigned char c = static_cast<unsigned char>(symbol);
		int position = size() + 1;
		text_.push_back(c);
		int parent = extendable_(last_, position, c), v = child_(parent, c);
		if (!v) {
			int link = parent == odd_root ? even_root : child_(extendable_(nodes_[parent].link, position, c), c);
			v = node_count();
			nodes_.push_back({nodes_[parent].length + 2, link, parent, position, nodes_[link].suffix_count + 1, 0, 0, nodes_[parent].head});
			nodes_[parent].head = v;
			if (parent < 2) roots_[parent][c] = v;
			if (nodes_[v].length > nodes_[longest_].length) longest_ = v;
		}
		last_ = v, ++nodes_[v].hits;
		total_ += nodes_[v].suffix_count;
		prefix_.push_back(v);
		return v;
	}
	void append(std::string_view s) { for (char c : s) push_back(c); }
	int size() const { return static_cast<int>(prefix_.size()); }
	bool empty() const { return prefix_.empty(); }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	int distinct_count() const { return node_count() - 2; }
	int last() const { return last_; }
	int longest_palindrome() const { return longest_; }
	std::uint64_t palindrome_count() const { return total_; }
	int length(int v) const { check_(v); return nodes_[v].length; }
	int suffix_link(int v) const { check_(v); return nodes_[v].link; }
	int parent(int v) const { check_(v); assert(v >= 2); return nodes_[v].parent; }
	int suffix_count(int v) const { check_(v); return nodes_[v].suffix_count; }
	int terminal_count(int v) const { check_(v); return nodes_[v].hits; }
	std::pair<int, int> first_occurrence(int v) const {
		check_(v), assert(v >= 2);
		return {nodes_[v].first_end - nodes_[v].length, nodes_[v].first_end};
	}
	int transition(int v, char symbol) const {
		check_(v);
		int u = child_(v, static_cast<unsigned char>(symbol));
		return u ? u : -1;
	}
	const std::vector<int> &prefix_nodes() const { return prefix_; }
	std::vector<std::uint64_t> occurrences() const {
		std::vector<std::uint64_t> count(nodes_.size());
		for (int v = node_count() - 1; v >= 2; --v) {
			count[v] += nodes_[v].hits;
			if (int link = nodes_[v].link; link >= 2) count[link] += count[v];
		}
		return count;
	}
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
#include <stdexcept>
#include <string>
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


#line 5 "test/eertree.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s;
	input.read(s);
	Eertree tree(s);
	auto number = [](int v) { return v - 1; };
	output.println(tree.distinct_count());
	for (int v = 2; v < tree.node_count(); ++v) {
		output.write(number(tree.parent(v))), output.write(' '), output.println(number(tree.suffix_link(v)));
	}
	const auto &prefix = tree.prefix_nodes();
	for (int i = 0; i < tree.size(); ++i) {
		if (i) output.write(' ');
		output.write(number(prefix[i]));
	}
	output.write('\n');
}
