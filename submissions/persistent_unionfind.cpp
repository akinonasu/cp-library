#line 1 "test/persistent_union_find.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#line 1 "data_structure/persistent/persistent_union_find.hpp"



#line 1 "data_structure/persistent/persistent_array.hpp"



#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class T, int Log = 3>
class PersistentArray {
	static_assert(1 <= Log && Log <= 5);
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	struct Node { T value; std::array<root_type, 1 << Log> child{}; };
	int n_;
	root_type initial_ = 0;
	std::vector<Node> nodes;
	root_type make(Node node) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type clone(root_type root) {
		assert(nodes.size() < std::numeric_limits<root_type>::max());
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(nodes[root]);
		return result;
	}
	root_type update(root_type root, unsigned p, T value) {
		if (!p) return make({std::move(value), nodes[root].child});
		auto result = clone(root), current = result;
		while (p) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log;
			auto old = nodes[current].child[d];
			auto next = p ? clone(old) : make({std::move(value), nodes[old].child});
			nodes[current].child[d] = next;
			current = next;
		}
		return result;
	}
	root_type update_pair(root_type root, unsigned p, T a, unsigned q, T b) {
		if (p == q) return update(root, q, std::move(b));
		auto result = clone(root), current = result;
		while (p && q && (p & ((1 << Log) - 1)) == (q & ((1 << Log) - 1))) {
			unsigned d = p & ((1 << Log) - 1);
			p >>= Log, q >>= Log;
			auto next = clone(nodes[current].child[d]);
			nodes[current].child[d] = next;
			current = next;
		}
		if (!p) nodes[current].value = std::move(a);
		else {
			unsigned d = p & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], p >> Log, std::move(a));
			nodes[current].child[d] = next;
		}
		if (!q) nodes[current].value = std::move(b);
		else {
			unsigned d = q & ((1 << Log) - 1);
			auto next = update(nodes[current].child[d], q >> Log, std::move(b));
			nodes[current].child[d] = next;
		}
		return result;
	}
	root_type build(const std::vector<T> &a, std::size_t p, std::size_t step) {
		Node node{a[p], {}};
		for (int d = 0; d < (1 << Log); ++d) {
			std::size_t next = p + step * d;
			if (next < a.size() && (d || step <= (a.size() - 1 - p) / (1 << Log)))
				node.child[d] = build(a, next, step * (1 << Log));
		}
		return make(std::move(node));
	}
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
  public:
	explicit PersistentArray(int n = 0, T value = T{}) : n_(n), nodes{Node{std::move(value), {}}} { assert(n >= 0); }
	explicit PersistentArray(const std::vector<T> &a, T default_value = T{}) : PersistentArray(0, std::move(default_value)) {
		assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		if (n_) {
			std::size_t count = a.size();
			for (auto rest = (a.size() - 1) >> Log; rest; rest >>= Log) count += rest;
			reserve(count);
			initial_ = build(a, 0, 1);
		}
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	root_type initial_root() const { return initial_; }
	T get(root_type root, int p) const {
		check(root); assert(0 <= p && p < n_);
		while (root && p) { root = nodes[root].child[p & ((1 << Log) - 1)]; p >>= Log; }
		return nodes[root].value;
	}
	root_type set(root_type root, int p, T value) {
		check(root); assert(0 <= p && p < n_);
		return update(root, static_cast<unsigned>(p), std::move(value));
	}
	root_type set_pair(root_type root, int p, T a, int q, T b) {
		check(root); assert(0 <= p && p < n_ && 0 <= q && q < n_);
		return update_pair(root, static_cast<unsigned>(p), std::move(a), static_cast<unsigned>(q), std::move(b));
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 9 "data_structure/persistent/persistent_union_find.hpp"

class PersistentUnionFind {
	PersistentArray<int> parent;

  public:
	struct root_type {
		PersistentArray<int>::root_type data = 0;
		int components = 0;
		bool operator==(const root_type &) const = default;
	};
  private:
	std::pair<int, int> leader_size(root_type root, int x) const {
		for (;;) {
			int p = parent.get(root.data, x);
			if (p < 0) return {x, p};
			x = p;
		}
	}
  public:
	explicit PersistentUnionFind(int n = 0) : parent(n, -1) {}
	int vertex_count() const { return parent.size(); }
	root_type initial_root() const { return {parent.initial_root(), vertex_count()}; }
	int leader(root_type root, int x) const { return leader_size(root, x).first; }
	bool same(root_type root, int a, int b) const { return leader(root, a) == leader(root, b); }
	int size(root_type root, int x) const { return -leader_size(root, x).second; }
	int component_count(root_type root) const { return root.components; }
	root_type merge(root_type root, int a, int b) {
		auto [u, x] = leader_size(root, a);
		auto [v, y] = leader_size(root, b);
		if (u == v) return root;
		if (x > y) { std::swap(u, v); std::swap(x, y); }
		root.data = parent.set_pair(root.data, u, x + y, v, u);
		--root.components;
		return root;
	}
	std::vector<std::vector<int>> groups(root_type root) const {
		std::vector<std::vector<int>> result(vertex_count());
		for (int i = 0; i < vertex_count(); ++i) result[leader(root, i)].push_back(i);
		std::vector<std::vector<int>> groups;
		for (auto &g : result) if (!g.empty()) groups.push_back(std::move(g));
		return groups;
	}
	void reserve(std::size_t count) { parent.reserve(count); }
	std::size_t node_count() const { return parent.node_count(); }
	std::size_t memory_usage() const { return parent.memory_usage(); }
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
#line 6 "components/fast_io.hpp"
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


#line 5 "test/persistent_union_find.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	PersistentUnionFind uf(n);
	std::vector<PersistentUnionFind::root_type> roots(q + 1, uf.initial_root());
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0, a = 0, b = 0;
		input.read(type), input.read(k), input.read(a), input.read(b);
		roots[i] = roots[k + 1];
		if (type == 0) roots[i] = uf.merge(roots[i], a, b);
		else output.println(uf.same(roots[i], a, b));
	}
}
