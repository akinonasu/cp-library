#line 1 "test/biconnected_components.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/biconnected_components"
#line 1 "graph/connectivity/block_cut_tree.hpp"



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


#line 5 "graph/connectivity/block_cut_tree.hpp"
#include <algorithm>

class BlockCutTree {
	struct Frame { int v, next, low; };
	bool built_ = false;
	std::vector<int> vertices_, boundaries_;
	void check_([[maybe_unused]] int v) const { assert(built_ && 0 <= v && v < n); }

  public:
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> node_id, articulation;
	std::vector<std::vector<int>> tree;

	explicit BlockCutTree(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		assert(edges.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		int id = static_cast<int>(edges.size());
		edges.emplace_back(u, v);
		return id;
	}
	void build() {
		built_ = false;
		UndirectedVertexCSR graph(n, edges);
		std::vector<int> ord(n, -1), active;
		std::vector<Frame> stack;
		active.reserve(n), stack.reserve(n);
		vertices_.clear(), boundaries_.assign(1, 0);
		vertices_.reserve(n);
		node_id.assign(n, -1), articulation.clear(), tree.clear();
		int timer = 0;
		auto append = [&](int v, int block) {
			vertices_.push_back(v);
			node_id[v] = node_id[v] == -1 ? block : -2;
		};
		for (int root = 0; root < n; ++root) if (ord[root] == -1) {
			ord[root] = timer++;
			active.push_back(root);
			stack.push_back({root, graph.start[root], ord[root]});
			while (!stack.empty()) {
				auto& frame = stack.back();
				int v = frame.v;
				if (frame.next == graph.start[v + 1]) {
					int value = frame.low;
					stack.pop_back();
					if (stack.empty()) {
						active.pop_back();
						if (node_id[v] == -1) {
							append(v, static_cast<int>(boundaries_.size()) - 1);
							boundaries_.push_back(static_cast<int>(vertices_.size()));
						}
					} else {
						int p = stack.back().v;
						stack.back().low = std::min(stack.back().low, value);
						if (value >= ord[p]) {
							int block = static_cast<int>(boundaries_.size()) - 1, u;
							append(p, block);
							do {
								u = active.back();
								active.pop_back();
								append(u, block);
							} while (u != v);
							boundaries_.push_back(static_cast<int>(vertices_.size()));
						}
					}
					continue;
				}
				while (frame.next != graph.start[v + 1]) {
					int to = graph.adj[frame.next++];
					if (ord[to] != -1) frame.low = std::min(frame.low, ord[to]);
					else {
						ord[to] = timer++;
						active.push_back(to);
						stack.push_back({to, graph.start[to], ord[to]});
						break;
					}
				}
			}
		}
		int blocks = static_cast<int>(boundaries_.size()) - 1;
		for (int v = 0; v < n; ++v) if (node_id[v] == -2) {
			node_id[v] = blocks + static_cast<int>(articulation.size());
			articulation.push_back(v);
		}
		tree.resize(std::size_t(blocks) + articulation.size());
		std::vector<int> degree(tree.size());
		for (int c = 0; c < blocks; ++c)
			for (int i = boundaries_[c]; i < boundaries_[c + 1]; ++i) {
				int v = node_id[vertices_[i]];
				if (v >= blocks) ++degree[c], ++degree[v];
			}
		for (std::size_t v = 0; v < tree.size(); ++v) tree[v].reserve(degree[v]);
		for (int c = 0; c < blocks; ++c)
			for (int i = boundaries_[c]; i < boundaries_[c + 1]; ++i) {
				int v = node_id[vertices_[i]];
				if (v >= blocks) tree[c].push_back(v), tree[v].push_back(c);
			}
		built_ = true;
	}
	int block_count() const { assert(built_); return static_cast<int>(boundaries_.size()) - 1; }
	int node_count() const { assert(built_); return static_cast<int>(tree.size()); }
	bool is_articulation(int v) const { check_(v); return node_id[v] >= block_count(); }
	bool is_block(int node) const { assert(built_ && 0 <= node && node < node_count()); return node < block_count(); }
	int vertex(int node) const {
		assert(built_ && 0 <= node && node < node_count());
		return node < block_count() ? -1 : articulation[node - block_count()];
	}
	std::span<const int> group(int block) const {
		assert(built_ && 0 <= block && block < block_count());
		return {vertices_.data() + boundaries_[block], std::size_t(boundaries_[block + 1] - boundaries_[block])};
	}
	std::vector<std::vector<int>> groups() const {
		std::vector<std::vector<int>> result(block_count());
		for (int c = 0; c < block_count(); ++c) {
			auto members = group(c);
			result[c].assign(members.begin(), members.end());
		}
		return result;
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


#line 4 "test/biconnected_components.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	BlockCutTree graph(n);
	graph.edges.reserve(m);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		input.read(u), input.read(v);
		graph.add_edge(u, v);
	}
	graph.build();
	output.println(graph.block_count());
	for (int c = 0; c < graph.block_count(); ++c) {
		auto group = graph.group(c);
		output.write(group.size());
		for (int v : group) output.write(' '), output.write(v);
		output.write('\n');
	}
}
