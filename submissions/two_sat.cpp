#line 1 "test/two_sat.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/two_sat"
#line 1 "graph/two_sat.hpp"



#line 1 "graph/connectivity/scc.hpp"



#line 1 "components/scc_core.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace scc_detail {
inline std::pair<int, std::vector<int>> ids(int n, std::span<const std::pair<int, int>> edges) {
	assert(n >= 0 && edges.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::vector<int> answer(n, -1);
	unsigned monotone = 3;
	for (auto [from, to] : edges) {
		assert(0 <= from && from < n && 0 <= to && to < n);
		monotone &= (from <= to ? 1U : 0U) | (from >= to ? 2U : 0U);
		if (!monotone) break;
	}
	if (monotone) {
		for (int v = 0; v < n; ++v) answer[v] = monotone & 1 ? v : n - 1 - v;
		return {n, std::move(answer)};
	}
	std::vector<int> offset(std::size_t(n) + 1), degree(n);
	for (auto [from, to] : edges) {
		assert(0 <= from && from < n && 0 <= to && to < n);
		++offset[from + 1];
		++degree[to];
	}
	bool functional = true;
	for (int v = 0; v < n; ++v) {
		functional &= offset[v + 1] <= 1;
		offset[v + 1] += offset[v];
	}
	std::vector<int> adjacency(edges.size()), work(offset.begin(), offset.end() - 1);
	for (auto [from, to] : edges) adjacency[work[from]++] = to;
	work.clear();
	for (int v = 0; v < n; ++v) if (!degree[v]) work.push_back(v);
	for (std::size_t head = 0; head < work.size(); ++head) {
		int v = work[head];
		answer[v] = static_cast<int>(head);
		for (int j = offset[v]; j < offset[v + 1]; ++j)
			if (--degree[adjacency[j]] == 0) work.push_back(adjacency[j]);
	}
	int peeled = static_cast<int>(work.size());
	if (peeled == n) return {n, std::move(answer)};
	if (functional) {
		int count = peeled;
		for (int v = 0; v < n; ++v) if (degree[v]) {
			int u = v;
			do {
				answer[u] = count;
				degree[u] = 0;
				u = adjacency[offset[u]];
			} while (u != v);
			++count;
		}
		return {count, std::move(answer)};
	}
	for (int v : work) answer[v] = -answer[v] - 1;
	for (int &x : degree) x = x ? -1 : n;
	work.clear();
	struct Frame { int vertex, next, low; };
	std::vector<Frame> stack;
	stack.reserve(n - peeled);
	int clock = 0, count = 0;
	auto enter = [&](int v) {
		degree[v] = clock++;
		work.push_back(v);
		stack.push_back({v, offset[v], degree[v]});
	};
	for (int root = 0; root < n; ++root) if (degree[root] == -1) {
		enter(root);
		while (!stack.empty()) {
			auto &frame = stack.back();
			int v = frame.vertex;
			if (frame.next < offset[v + 1]) {
				int to = adjacency[frame.next++];
				if (degree[to] == -1) enter(to);
				else frame.low = std::min(frame.low, degree[to]);
			} else {
				int low = frame.low;
				stack.pop_back();
				if (low == degree[v]) {
					int u;
					do {
						u = work.back();
						work.pop_back();
						degree[u] = n;
						answer[u] = count;
					} while (u != v);
					++count;
				}
				if (!stack.empty()) stack.back().low = std::min(stack.back().low, low);
			}
		}
	}
	for (int &x : answer) x = x < 0 ? -x - 1 : peeled + count - 1 - x;
	return {peeled + count, std::move(answer)};
}
}


#line 5 "graph/connectivity/scc.hpp"

class SCCGraph {
	int n_;
	std::vector<std::pair<int, int>> edges_;

  public:
	struct Components {
		std::vector<int> id, vertices, offset;
		Components() = default;
		Components(const Components &) = default;
		Components(Components &&other) noexcept { swap(other); }
		Components &operator=(Components other) noexcept { swap(other); return *this; }
		void swap(Components &other) noexcept { id.swap(other.id), vertices.swap(other.vertices), offset.swap(other.offset); }
		int size() const { return offset.empty() ? 0 : static_cast<int>(offset.size() - 1); }
		std::span<const int> operator[](int component) const {
			assert(0 <= component && component < size());
			return {vertices.data() + offset[component], std::size_t(offset[component + 1] - offset[component])};
		}
	};
	explicit SCCGraph(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	std::size_t edge_count() const { return edges_.size(); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max()));
		edges_.reserve(m);
	}
	void add_edge(int from, int to) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		edges_.emplace_back(from, to);
	}
	std::pair<int, std::vector<int>> scc_ids() const { return scc_detail::ids(n_, edges_); }
	Components decomposition() const {
		auto [count, id] = scc_ids();
		Components result;
		result.id = std::move(id);
		result.offset.resize(std::size_t(count) + 1);
		for (int x : result.id) ++result.offset[x + 1];
		for (int i = 0; i < count; ++i) result.offset[i + 1] += result.offset[i];
		result.vertices.resize(n_);
		auto cursor = result.offset;
		for (int v = 0; v < n_; ++v) result.vertices[cursor[result.id[v]]++] = v;
		return result;
	}
	std::vector<std::vector<int>> scc() const {
		auto [count, id] = scc_ids();
		std::vector<int> sizes(count);
		for (int x : id) ++sizes[x];
		std::vector<std::vector<int>> groups(count);
		for (int i = 0; i < count; ++i) groups[i].reserve(sizes[i]);
		for (int v = 0; v < n_; ++v) groups[id[v]].push_back(v);
		return groups;
	}
};


#line 5 "graph/two_sat.hpp"

class TwoSAT {
	int n_;
	SCCGraph graph_;
	std::vector<bool> answer_;
	bool valid_ = false;
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), graph_(vertices(n)), answer_(n) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		graph_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = false;
		if (i == j && f != g) return;
		graph_.add_edge(2 * i + !f, 2 * j + g);
		if (i != j) graph_.add_edge(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		auto id = graph_.scc_ids().second;
		valid_ = false;
		for (int i = 0; i < n_; ++i) {
			if (id[2 * i] == id[2 * i + 1]) return false;
			answer_[i] = id[2 * i] < id[2 * i + 1];
		}
		return valid_ = true;
	}
	std::vector<bool> answer() const { assert(valid_); return answer_; }
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


#line 4 "test/two_sat.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string token;
	in.read(token), in.read(token);
	int n = 0, m = 0;
	in.read(n), in.read(m);
	TwoSAT solver(n);
	solver.reserve(m);
	while (m--) {
		int a = 0, b = 0, zero = 0;
		in.read(a), in.read(b), in.read(zero);
		solver.add_clause((a > 0 ? a : -a) - 1, a > 0, (b > 0 ? b : -b) - 1, b > 0);
	}
	if (!solver.satisfiable()) { out.println("s UNSATISFIABLE"); return 0; }
	out.println("s SATISFIABLE");
	out.write('v');
	auto answer = solver.answer();
	for (int i = 0; i < n; ++i) { out.write(' '); out.write(answer[i] ? i + 1 : -i - 1); }
	out.println(" 0");
}
