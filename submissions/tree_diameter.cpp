#line 1 "test/rerooting_diameter.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/tree_diameter"
#line 1 "tree/rerooting.hpp"



#line 1 "components/undirected_csr.hpp"



#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace undirected_csr_detail {
template <class Arc>
void build(int n, std::span<const std::pair<int, int>> edges, std::vector<int>& start, std::unique_ptr<Arc[]>& adj) {
	assert(0 <= n && n < std::numeric_limits<int>::max());
	assert(edges.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
	start.assign(std::size_t(n) + 1, 0);
	for (auto [u, v] : edges) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		++start[u + 1], ++start[v + 1];
	}
	for (int v = 0; v < n; ++v) start[v + 1] += start[v];
	adj.reset(new Arc[2 * edges.size()]);
	std::vector<int> cursor(start.begin(), start.end() - 1);
	for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
		auto [u, v] = edges[i];
		if constexpr (std::is_same_v<Arc, int>) adj[cursor[u]++] = v, adj[cursor[v]++] = u;
		else adj[cursor[u]++] = {v, i}, adj[cursor[v]++] = {u, i};
	}
}
}

struct UndirectedCSR {
	struct Arc { int to, id; };
	int n;
	std::vector<int> start;
	std::unique_ptr<Arc[]> adj;
	UndirectedCSR(int n, std::span<const std::pair<int, int>> edges) : n(n) {
		undirected_csr_detail::build(n, edges, start, adj);
	}
	std::span<const Arc> operator[](int v) const {
		assert(0 <= v && v < n);
		return {adj.get() + start[v], std::size_t(start[v + 1] - start[v])};
	}
};

struct UndirectedVertexCSR {
	int n;
	std::vector<int> start;
	std::unique_ptr<int[]> adj;
	UndirectedVertexCSR(int n, std::span<const std::pair<int, int>> edges) : n(n) {
		undirected_csr_detail::build(n, edges, start, adj);
	}
	std::span<const int> operator[](int v) const {
		assert(0 <= v && v < n);
		return {adj.get() + start[v], std::size_t(start[v + 1] - start[v])};
	}
};


#line 5 "tree/rerooting.hpp"
#include <algorithm>
#line 11 "tree/rerooting.hpp"

template<class T, class E = T>
class Rerooting {
	int n_;
	bool built_ = false;
	std::vector<std::pair<int, int>> edges_;
	std::vector<T> values_;
	std::vector<E> messages_;

  public:
	using Vertex = T;
	using Edge = E;
	explicit Rerooting(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
		edges_.reserve(std::max(0, n - 1));
	}
	int size() const { return n_; }
	bool empty() const { return n_ == 0; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	int add_edge(int u, int v) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		edges_.emplace_back(u, v);
		return edge_count() - 1;
	}
	const std::pair<int, int>& edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const T& operator[](int v) const {
		assert(built_ && 0 <= v && v < n_);
		return values_[v];
	}
	const std::vector<T>& values() const {
		assert(built_);
		return values_;
	}
	const E& edge_dp(int id, bool reverse = false) const {
		assert(built_ && 0 <= id && id < edge_count());
		return messages_[2 * id + reverse];
	}
	template<class Merge, class AddVertex, class AddEdge>
	const std::vector<T>& build(E unit, Merge&& merge, AddVertex&& add_vertex, AddEdge&& add_edge, int root = 0) {
		assert(n_ == 0 || (0 <= root && root < n_));
		built_ = false;
		if constexpr (std::is_default_constructible_v<E>) messages_.resize(2 * edges_.size());
		else messages_.assign(2 * edges_.size(), unit);
		if (n_ == 0) {
			values_.clear();
			built_ = true;
			return values_;
		}
		UndirectedCSR graph(n_, edges_);
		for (int v = 0; v < n_; ++v)
			for (int i = graph.start[v]; i < graph.start[v + 1]; ++i) {
				int id = graph.adj[i].id;
				graph.adj[i].id = 2 * id + (edges_[id].second == v);
			}
		std::vector<int> parent(n_, -2), parent_edge(n_, -1), order;
		order.reserve(n_);
		auto visit = [&](int s) {
			if (parent[s] != -2) return;
			parent[s] = -1;
			order.push_back(s);
			for (std::size_t i = order.size() - 1; i < order.size(); ++i) {
				int v = order[i];
				for (auto [to, id] : graph[v]) {
					if (id == parent_edge[v]) continue;
					assert(parent[to] == -2);
					if (parent[to] != -2) continue;
					parent[to] = v, parent_edge[to] = id ^ 1;
					order.push_back(to);
				}
			}
		};
		visit(root);
		for (int v = 0; v < n_; ++v) visit(v);
		for (auto it = order.rbegin(); it != order.rend(); ++it) {
			int v = *it, id = parent_edge[v];
			if (id == -1) continue;
			E value = unit;
			for (auto arc : graph[v])
				if (arc.id != id) value = merge(value, messages_[arc.id ^ 1]);
			messages_[id] = add_edge(add_vertex(value, v), v, parent[v], id >> 1);
		}
		if constexpr (std::is_default_constructible_v<T>) values_.resize(n_);
		else values_.assign(n_, unit);
		int maximum = 0;
		for (int v = 0; v < n_; ++v) maximum = std::max(maximum, graph.start[v + 1] - graph.start[v]);
		std::vector<E> prefix(maximum, unit);
		for (int v : order) {
			auto arcs = graph[v];
			int degree = static_cast<int>(arcs.size());
			if (degree == 0) {
				values_[v] = add_vertex(unit, v);
				continue;
			}
			auto incoming = [&](int i) -> const E& { return messages_[arcs[i].id ^ 1]; };
			if (degree > 1) prefix[1] = incoming(0);
			for (int i = 1; i + 1 < degree; ++i) prefix[i + 1] = merge(prefix[i], incoming(i));
			values_[v] = add_vertex(degree == 1 ? incoming(0) : merge(prefix[degree - 1], incoming(degree - 1)), v);
			E suffix = unit;
			for (int i = degree - 1; i >= 0; --i) {
				auto [to, id] = arcs[i];
				if (id != parent_edge[v]) {
					E value = i == degree - 1 ? prefix[i] : i == 0 ? suffix : merge(prefix[i], suffix);
					messages_[id] = add_edge(add_vertex(value, v), v, to, id >> 1);
				}
				if (i > 0) suffix = i == degree - 1 ? incoming(i) : merge(incoming(i), suffix);
			}
		}
		built_ = true;
		return values_;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 10 "components/fast_io.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#line 14 "components/fast_io.hpp"
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


#line 4 "test/rerooting_diameter.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	using Data = std::pair<long long, int>;
	Rerooting<Data> tree(n);
	std::vector<long long> weights(n - 1);
	std::vector<std::pair<int, int>> edges;
	for (int id = 0; id < n - 1; ++id) {
		int u = 0, v = 0;
		input.read(u), input.read(v), input.read(weights[id]);
		tree.add_edge(u, v), edges.emplace_back(u, v);
	}
	tree.build(Data{-1, -1}, [](Data x, Data y) { return std::max(x, y); },
		[](Data x, int v) { return std::max(x, Data{0, v}); },
		[&](Data x, int, int, int id) { return Data{x.first + weights[id], x.second}; });
	int u = 0;
	for (int v = 1; v < n; ++v) if (tree[v].first > tree[u].first) u = v;
	int target = tree[u].second;
	UndirectedCSR graph(n, edges);
	std::vector<int> previous(n, -1), order{u};
	previous[u] = u;
	for (std::size_t i = 0; i < order.size(); ++i)
		for (auto arc : graph[order[i]]) if (previous[arc.to] == -1) previous[arc.to] = order[i], order.push_back(arc.to);
	std::vector<int> path{target};
	while (path.back() != u) path.push_back(previous[path.back()]);
	output.write(tree[u].first), output.write(' '), output.println(path.size());
	for (int v : path) output.write(v), output.write(' ');
	output.write('\n');
}
