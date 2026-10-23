#line 1 "test/eulerian_trail_undirected.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/eulerian_trail_undirected"
#line 1 "graph/eulerian_trail.hpp"



#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

template<bool Directed> class EulerianTrail {
	struct Arc { int to, id; };
	int n_;
	std::vector<std::pair<int, int>> edges_;
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }

  public:
	struct Result { std::vector<int> vertices, edges; };
	explicit EulerianTrail(int n = 0) : n_(n) {
		assert(0 <= n && n < std::numeric_limits<int>::max());
	}
	EulerianTrail(int n, std::span<const std::pair<int, int>> edges) : EulerianTrail(n) {
		reserve(edges.size());
		for (auto [u, v] : edges) add_edge(u, v);
	}
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(m);
	}
	int add_edge(int u, int v) {
		vertex(u), vertex(v);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		int id = edge_count();
		edges_.emplace_back(u, v);
		return id;
	}
	std::pair<int, int> get_edge(int id) const {
		assert(0 <= id && id < edge_count());
		return edges_[id];
	}
	const std::vector<std::pair<int, int>> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	std::optional<Result> solve(int start = -1) const {
		assert(start == -1 || (0 <= start && start < n_));
		const int m = edge_count();
		if (!m) {
			Result result;
			if (n_) result.vertices.push_back(start == -1 ? 0 : start);
			return result;
		}
		std::vector<int> offset(std::size_t(n_) + 1), balance;
		if constexpr (Directed) balance.resize(n_);
		for (auto [u, v] : edges_) {
			++offset[u + 1];
			if constexpr (Directed) ++balance[u], --balance[v];
			else ++offset[v + 1];
		}
		int required = -1, finish = -1, active = -1;
		for (int v = 0; v < n_; ++v) {
			if (active == -1 && offset[v + 1]) active = v;
			if constexpr (Directed) {
				if (balance[v] == 1 && required == -1) required = v;
				else if (balance[v] == -1 && finish == -1) finish = v;
				else if (balance[v] != 0) return std::nullopt;
			} else if (offset[v + 1] & 1) {
				if (required == -1) required = v;
				else if (finish == -1) finish = v;
				else return std::nullopt;
			}
		}
		if ((required == -1) != (finish == -1)) return std::nullopt;
		if (start == -1) start = required == -1 ? active : required;
		if (required != -1 && start != required) {
			if constexpr (Directed) return std::nullopt;
			else if (start != finish) return std::nullopt;
		}
		if (!offset[start + 1]) return std::nullopt;
		for (int v = 0; v < n_; ++v) offset[v + 1] += offset[v];
		std::unique_ptr<Arc[]> adjacency(new Arc[offset.back()]);
		std::vector<int> cursor(offset.begin(), offset.end() - 1);
		for (int id = 0; id < m; ++id) {
			auto [u, v] = edges_[id];
			adjacency[cursor[u]++] = {v, id};
			if constexpr (!Directed) adjacency[cursor[v]++] = {u, id};
		}
		std::vector<unsigned char> used;
		if constexpr (!Directed) used.resize(m);
		Result result{std::vector<int>(std::size_t(m) + 1), std::vector<int>(m)};
		result.vertices[0] = start;
		int top = 1, vertex_out = m + 1, edge_out = m;
		while (top) {
			int v = result.vertices[top - 1];
			if constexpr (!Directed)
				while (cursor[v] > offset[v] && used[adjacency[cursor[v] - 1].id]) --cursor[v];
			if (cursor[v] > offset[v]) {
				auto [to, id] = adjacency[--cursor[v]];
				if constexpr (!Directed) used[id] = 1;
				result.edges[top - 1] = id;
				result.vertices[top++] = to;
			} else {
				int edge = top > 1 ? result.edges[top - 2] : -1;
				--top;
				result.vertices[--vertex_out] = v;
				if (edge != -1) result.edges[--edge_out] = edge;
			}
		}
		if (edge_out) return std::nullopt;
		return result;
	}
};

using DirectedEulerianTrail = EulerianTrail<true>;
using UndirectedEulerianTrail = EulerianTrail<false>;


#line 1 "components/fast_io.hpp"



#include <algorithm>
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


#line 4 "test/eulerian_trail_undirected.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int cases = 0;
	input.read(cases);
	while (cases--) {
		int n = 0, m = 0;
		input.read(n), input.read(m);
		UndirectedEulerianTrail graph(n);
		graph.reserve(m);
		for (int i = 0; i < m; ++i) {
			int u = 0, v = 0;
			input.read(u), input.read(v);
			graph.add_edge(u, v);
		}
		auto result = graph.solve();
		if (!result) { output.println("No"); continue; }
		output.println("Yes");
		for (int i = 0; i <= m; ++i) {
			output.write(result->vertices[i]);
			output.write(i == m ? '\n' : ' ');
		}
		for (int i = 0; i < m; ++i) {
			output.write(result->edges[i]);
			output.write(i + 1 == m ? '\n' : ' ');
		}
		if (!m) output.write('\n');
	}
}
