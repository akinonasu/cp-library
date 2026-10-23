#line 1 "test/link_cut_tree_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum"
#line 1 "tree/link_cut_tree.hpp"



#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, bool commutative = false>
class LinkCutTree {
	static constexpr bool scalar = commutative && std::is_arithmetic_v<S>;
	struct Empty {};
	using Back = std::conditional_t<commutative, Empty, S>;
	struct Node {
		int ch[2] = {}, parent = 0;
		std::uint32_t count : 31 = 1;
		std::uint32_t rev : 1 = 0;
		S value, prod;
		[[no_unique_address]] Back back;
		static Back make_back(const S &v) {
			if constexpr (commutative) return {};
			else return v;
		}
		explicit Node(const S &v) : value(v), prod(v), back(make_back(v)) {}
	};
	std::vector<Node> t;
	void init(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		t.reserve(std::size_t(n) + 1);
		t.emplace_back(e());
		t[0].count = 0;
	}
	int node(int v) const {
		assert(0 <= v && v < size());
		return v + 1;
	}
	bool is_root(int x) const { return t[x].parent <= 0; }
	void pull(int x) {
		auto &v = t[x];
		int l = v.ch[0], r = v.ch[1];
		v.count = t[l].count + t[r].count + 1;
		if constexpr (scalar) v.prod = op(op(t[l].prod, v.value), t[r].prod);
		else {
			v.prod = l ? op(t[l].prod, v.value) : v.value;
			if (r) v.prod = op(std::move(v.prod), t[r].prod);
		}
		if constexpr (!commutative) {
			v.back = r ? op(t[r].back, v.value) : v.value;
			if (l) v.back = op(std::move(v.back), t[l].back);
		}
	}
	void flip(int x) {
		if constexpr (!scalar) if (!x) return;
		std::swap(t[x].ch[0], t[x].ch[1]);
		if constexpr (!commutative) std::ranges::swap(t[x].prod, t[x].back);
		t[x].rev ^= 1;
	}
	void push(int x) {
		if (!t[x].rev) return;
		flip(t[x].ch[0]);
		flip(t[x].ch[1]);
		t[x].rev = 0;
	}
	void rotate(int x) {
		int p = t[x].parent, g = t[p].parent;
		int d = t[p].ch[1] == x, c = t[x].ch[d ^ 1];
		if (g > 0) t[g].ch[t[g].ch[1] == p] = x;
		t[x].parent = g;
		t[p].ch[d] = c;
		t[c].parent = p;
		t[x].ch[d ^ 1] = p;
		t[p].parent = x;
		pull(p);
	}
	void splay(int x) {
		if (is_root(x)) {
			push(x);
			return;
		}
		do {
			int p = t[x].parent, g = t[p].parent;
			bool top = is_root(p);
			if (!top) push(g);
			push(p);
			push(x);
			if (!top) rotate((t[p].ch[0] == x) == (t[g].ch[0] == p) ? p : x);
			rotate(x);
		} while (!is_root(x));
	}

	int access(int x) {
		splay(x);
		if (t[x].ch[1]) t[t[x].ch[1]].parent = -x;
		t[x].ch[1] = 0;
		int last = x;
		while (t[x].parent) {
			int p = -t[x].parent;
			splay(p);
			if (t[p].ch[1]) t[t[p].ch[1]].parent = -p;
			t[p].ch[1] = x;
			t[x].parent = p;
			rotate(x);
			last = p;
		}
		return last;
	}
	void reroot(int x) {
		access(x);
		flip(x);
	}
	int expose(int u, int v) {
		u = node(u), v = node(v);
		reroot(u);
		access(v);
		assert(u == v || t[u].parent != 0);
		return v;
	}

  public:
	explicit LinkCutTree(int n = 0) {
		init(n);
		for (int repeat_index = 0; repeat_index < n; ++repeat_index) t.emplace_back(t[0].value);
	}
	explicit LinkCutTree(const std::vector<S> &values) {
		assert(values.size() < std::size_t(std::numeric_limits<int>::max()));
		init(static_cast<int>(values.size()));
		for (const S &v : values) t.emplace_back(v);
	}
	int size() const { return t.empty() ? 0 : static_cast<int>(t.size()) - 1; }
	void build_forest(std::span<const std::pair<int, int>> edges) {
		assert(edges.size() < std::size_t(size()) || (edges.empty() && size() == 0));
		std::vector<int> degree(size()), neighbors(size()), parent(size(), -1), order;
		order.reserve(size());
		for (auto [u, v] : edges) {
			assert(0 <= u && u < size() && 0 <= v && v < size() && u != v);
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		for (int v = 0; v < size(); ++v) if (degree[v] <= 1) order.push_back(v);
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i];
			if (!degree[v]) continue;
			int p = neighbors[v];
			parent[v] = p;
			degree[v] = 0;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order.push_back(p);
		}
		assert(order.size() == std::size_t(size()));
		for (int v = 0; v < size(); ++v) {
			auto &x = t[v + 1];
			x.ch[0] = x.ch[1] = 0;
			x.parent = -parent[v] - 1;
			x.count = 1, x.rev = 0;
			x.prod = x.value;
			if constexpr (!commutative) x.back = x.value;
		}
	}
	S get(int v) const { return t[node(v)].value; }
	void set(int v, S value) {
		int x = node(v);
		splay(x);
		t[x].value = std::move(value);
	}
	void make_root(int v) { reroot(node(v)); }
	int root(int v) {
		int x = node(v);
		access(x);
		push(x);
		while (t[x].ch[0]) push(x = t[x].ch[0]);
		splay(x);
		return x - 1;
	}
	bool same(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return true;
		access(u);
		access(v);
		return t[u].parent != 0;
	}
	bool link(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return false;
		reroot(u);
		access(v);
		if (t[u].parent) return false;
		t[u].parent = -v;
		return true;
	}
	bool cut(int u, int v) {
		u = node(u), v = node(v);
		if (u == v) return false;
		reroot(u);
		access(v);
		if (t[v].ch[0] != u || t[u].ch[1]) return false;
		t[v].ch[0] = t[u].parent = 0;
		return true;
	}
	S prod(int u, int v) {
		int x = expose(u, v), l = t[x].ch[0];
		return l ? op(t[l].prod, t[x].value) : t[x].value;
	}
	int path_size(int u, int v) {
		int x = expose(u, v);
		return t[t[x].ch[0]].count + 1;
	}
	int distance(int u, int v) { return path_size(u, v) - 1; }
	int jump(int u, int v, int k) {
		int x = expose(u, v);
		if (k < 0 || k > int(t[t[x].ch[0]].count)) return -1;
		for (;;) {
			push(x);
			int l = t[x].ch[0], count = t[l].count;
			if (k == count) break;
			if (k < count) x = l;
			else k -= count + 1, x = t[x].ch[1];
		}
		splay(x);
		return x - 1;
	}
	int lca(int r, int u, int v) {
		r = node(r), u = node(u), v = node(v);
		reroot(r);
		access(u);
		assert(r == u || t[r].parent != 0);
		int w = access(v);
		assert(u == v || t[u].parent != 0);
		return w - 1;
	}
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
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


#line 6 "test/link_cut_tree_sum.test.cpp"

using namespace std;

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	vector<int64_t> values(n);
	for (auto &v : values) input.read(v);
	LinkCutTree<int64_t, op, e, true> tree(values);
	std::vector<std::pair<int, int>> edges(n - 1);
	for (auto &[u, v] : edges) input.read(u), input.read(v);
	tree.build_forest(edges);

	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		int type, u;
		input.read(type), input.read(u);
		if (type == 0) {
			int v, w, x;
			input.read(v), input.read(w), input.read(x);
			tree.cut(u, v);
			tree.link(w, x);
		} else if (type == 1) {
			int64_t x;
			input.read(x);
			tree.set(u, tree.get(u) + x);
		} else {
			int v;
			input.read(v);
			output.println(tree.prod(u, v));
		}
	}
}
