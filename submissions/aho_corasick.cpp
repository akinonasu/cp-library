#line 1 "test/aho_corasick.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/aho_corasick"
#line 1 "string/aho_corasick.hpp"



#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
namespace aho_corasick_detail {
template<int Alphabet>
__attribute__((target("avx2"))) inline std::uint32_t complete(int *edges, const int *fallback) {
	std::uint32_t mask = 0;
	int i = 0;
	for (; i + 8 <= Alphabet; i += 8) {
		auto children = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(edges + i));
		auto inherited = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(fallback + i));
		auto missing = _mm256_cmpeq_epi32(children, _mm256_setzero_si256());
		mask |= (std::uint32_t(~_mm256_movemask_ps(_mm256_castsi256_ps(missing))) & 255) << i;
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(edges + i), _mm256_blendv_epi8(children, inherited, missing));
	}
	for (; i < Alphabet; ++i) {
		if (edges[i]) mask |= std::uint32_t(1) << i;
		else edges[i] = fallback[i];
	}
	return mask;
}
}
#endif

template <int Alphabet = 26, int First = 'a'>
class AhoCorasick {
	static_assert(0 < Alphabet && 0 <= First && First + Alphabet <= 256);
	struct alignas(Alphabet >= 16 ? 64 : alignof(int)) Node {
		std::array<int, Alphabet> next{};
		int matches = 0;
		int parent = 0, depth = 0, link = 0, output = -1, head = -1;
	};
	std::vector<Node> nodes_;
	std::vector<int> terminal_, next_pattern_, order_;
	bool built_ = false;
	static unsigned symbol_(char c) { return static_cast<unsigned>(static_cast<unsigned char>(c) - First); }
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }

  public:
	static constexpr int root = 0;
	AhoCorasick() { nodes_.emplace_back(); }
	void clear() {
		nodes_.clear(), nodes_.emplace_back();
		terminal_.clear(), next_pattern_.clear(), order_.clear();
		built_ = false;
	}
	void reserve(int total_length, int patterns = 0) {
		assert(0 <= total_length && total_length < std::numeric_limits<int>::max() && 0 <= patterns);
		nodes_.reserve(std::size_t(total_length) + 1);
		terminal_.reserve(patterns), next_pattern_.reserve(patterns);
	}
	int add(std::string_view pattern) {
		assert(!built_ && size() < std::numeric_limits<int>::max());
		int v = root;
		for (char c : pattern) {
			unsigned x = symbol_(c);
			assert(x < Alphabet);
			int u = nodes_[v].next[x];
			if (!u) {
				assert(node_count() < std::numeric_limits<int>::max());
				u = node_count();
				nodes_[v].next[x] = u;
				nodes_.emplace_back();
				nodes_[u].parent = v, nodes_[u].depth = nodes_[v].depth + 1;
			}
			v = u;
		}
		int id = size();
		terminal_.push_back(v), next_pattern_.push_back(nodes_[v].head);
		nodes_[v].head = id, ++nodes_[v].matches;
		return id;
	}
	void build() {
		if (built_) return;
		order_.reserve(nodes_.size()), order_.push_back(root);
		for (std::size_t i = 0; i < order_.size(); ++i) {
			int v = order_[i];
			auto &node = nodes_[v];
			const auto &fallback = nodes_[node.link].next;
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
			if constexpr (8 <= Alphabet && Alphabet <= 32) {
				if (__builtin_cpu_supports("avx2")) {
					auto mask = aho_corasick_detail::complete<Alphabet>(node.next.data(), fallback.data());
					while (mask) {
						int c = std::countr_zero(mask), u = node.next[c], f = v == root ? root : fallback[c];
						mask &= mask - 1;
						nodes_[u].link = f;
						nodes_[u].output = nodes_[f].head != -1 ? f : nodes_[f].output;
						nodes_[u].matches += nodes_[f].matches;
						order_.push_back(u);
					}
					continue;
				}
			}
#endif
			for (int c = 0; c < Alphabet; ++c) {
				if (int u = node.next[c]) {
					int f = v == root ? root : fallback[c];
					nodes_[u].link = f;
					nodes_[u].output = nodes_[f].head != -1 ? f : nodes_[f].output;
					nodes_[u].matches += nodes_[f].matches;
					order_.push_back(u);
				} else {
					node.next[c] = fallback[c];
				}
			}
		}
		built_ = true;
	}
	int size() const { return static_cast<int>(terminal_.size()); }
	bool empty() const { return terminal_.empty(); }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	bool built() const { return built_; }
	int terminal(int id) const {
		assert(0 <= id && id < size());
		return terminal_[id];
	}
	int length(int id) const { return nodes_[terminal(id)].depth; }
	int parent(int v) const { check_(v); return nodes_[v].parent; }
	int depth(int v) const { check_(v); return nodes_[v].depth; }
	int child(int v, char c) const {
		check_(v);
		unsigned x = symbol_(c);
		if (x >= Alphabet) return -1;
		int u = nodes_[v].next[x];
		return u && nodes_[u].parent == v ? u : -1;
	}
	int suffix_link(int v) const { check_(v); assert(built_); return nodes_[v].link; }
	int output_link(int v) const { check_(v); assert(built_); return nodes_[v].output; }
	int terminal_count(int v) const {
		check_(v);
		return built_ && v ? nodes_[v].matches - nodes_[nodes_[v].link].matches : nodes_[v].matches;
	}
	int match_count(int v) const { check_(v); assert(built_); return nodes_[v].matches; }
	int transition(int v, char c) const {
		check_(v), assert(built_);
		unsigned x = symbol_(c);
		return x < Alphabet ? nodes_[v].next[x] : root;
	}
	int walk(int v, std::string_view text) const {
		check_(v), assert(built_);
		for (char c : text) v = transition(v, c);
		return v;
	}
	const std::vector<int> &bfs_order() const { assert(built_); return order_; }
	template <class F>
	void for_each_match(int v, F &&visit) const {
		check_(v), assert(built_);
		if (nodes_[v].head == -1) v = nodes_[v].output;
		for (; v != -1; v = nodes_[v].output)
			for (int id = nodes_[v].head; id != -1; id = next_pattern_[id]) visit(id);
	}
	template <class F>
	void find_all(std::string_view text, F &&visit) const {
		assert(built_);
		int v = root;
		for_each_match(v, [&](int id) { visit(std::size_t(0), id); });
		for (std::size_t i = 0; i < text.size(); ++i) {
			v = transition(v, text[i]);
			for_each_match(v, [&](int id) { visit(i + 1, id); });
		}
	}
	std::uint64_t count_matches(std::string_view text) const {
		assert(built_);
		int v = root;
		std::uint64_t count = nodes_[root].matches;
		for (char c : text) {
			v = transition(v, c);
			count += nodes_[v].matches;
		}
		return count;
	}
	template <class T>
	void propagate(std::vector<T> &values) const {
		assert(built_ && values.size() == nodes_.size());
		for (std::size_t i = order_.size(); i-- > 1;) {
			int v = order_[i];
			values[nodes_[v].link] += values[v];
		}
	}
	std::vector<std::uint64_t> occurrences(std::string_view text) const {
		assert(built_);
		std::vector<std::uint64_t> hits(nodes_.size());
		int v = root;
		++hits[root];
		for (char c : text) v = transition(v, c), ++hits[v];
		propagate(hits);
		std::vector<std::uint64_t> count(size());
		for (int id = 0; id < size(); ++id) count[id] = hits[terminal_[id]];
		return count;
	}
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
#line 8 "components/fast_io.hpp"
#include <charconv>
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


#line 6 "test/aho_corasick.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n;
	input.read(n);
	std::vector<std::string> patterns(n);
	int length = 0;
	for (auto &s : patterns) input.read(s), length += static_cast<int>(s.size());
	AhoCorasick<> ac;
	ac.reserve(length, n);
	for (const auto &s : patterns) ac.add(s);
	ac.build();
	output.println(ac.node_count());
	for (int v = 1; v < ac.node_count(); ++v) {
		output.write(ac.parent(v)), output.write(' '), output.println(ac.suffix_link(v));
	}
	for (int i = 0; i < n; ++i) {
		if (i) output.write(' ');
		output.write(ac.terminal(i));
	}
	output.write('\n');
}
