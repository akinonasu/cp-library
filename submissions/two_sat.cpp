#line 1 "test/two_sat.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/two_sat"
#line 1 "graph/two_sat.hpp"



#line 1 "components/two_sat_core.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace two_sat_detail {
inline bool solve(int variables, std::span<const std::pair<int, int>> edges, std::span<const int> forced, std::vector<bool> &answer) {
	assert(0 <= variables && variables <= std::numeric_limits<int>::max() / 2);
	assert(edges.size() <= std::size_t(std::numeric_limits<int>::max()) && answer.size() == std::size_t(variables));
	int n = 2 * variables;
	std::vector<signed char> values(variables, -1);
	std::vector<int> trail;
	trail.reserve(variables);
	for (int literal : forced) {
		if (values[literal >> 1] < 0) {
			values[literal >> 1] = literal & 1;
			trail.push_back(literal);
		} else if (values[literal >> 1] != (literal & 1)) return false;
	}
	if (trail.size() == std::size_t(variables)) {
		for (auto [from, to] : edges) {
			if (values[from >> 1] == (from & 1) && values[to >> 1] != (to & 1)) return false;
		}
		for (int i = 0; i < variables; ++i) answer[i] = values[i];
		return true;
	}
	std::vector<int> offset(std::size_t(n) + 1, -1), adjacency(edges.size());
	std::size_t branching = 0;
	for (std::size_t i = 0; i < edges.size(); ++i) {
		auto [from, to] = edges[i];
		branching += offset[from] >= 0;
		adjacency[i] = offset[from];
		offset[from] = static_cast<int>(i);
	}
	bool compact = false;
	auto make_compact = [&] {
		std::fill(offset.begin(), offset.end(), 0);
		for (auto [from, to] : edges) ++offset[from];
		for (int v = 1; v < n; ++v) offset[v] += offset[v - 1];
		offset[n] = static_cast<int>(edges.size());
		for (auto [from, to] : edges) adjacency[--offset[from]] = to;
		compact = true;
	};
	if (variables >= 1024) {
		auto closure = [&]<bool Adaptive>() -> int {
			std::size_t budget = std::size_t(variables) + edges.size();
			if constexpr (!Adaptive) budget = (std::min)(budget, std::numeric_limits<std::size_t>::max() / 2) * 2;
			std::size_t switch_after = std::max<std::size_t>(1024, (std::size_t(variables) + edges.size()) / 16);
			std::size_t compact_budget = budget - std::min(budget, switch_after);
			auto propagate = [&] {
				for (std::size_t i = 0; i < trail.size(); ++i) {
					if (!budget) return -1;
					--budget;
					int v = trail[i];
					if constexpr (Adaptive) if (!compact && budget < compact_budget) make_compact();
					auto visit = [&](int to) {
						if (!budget) return -1;
						--budget;
						if (values[to >> 1] < 0) {
							values[to >> 1] = to & 1;
							trail.push_back(to);
						} else if (values[to >> 1] != (to & 1)) {
							for (int literal : trail) values[literal >> 1] = -1;
							return 0;
						}
						return 1;
					};
					if constexpr (Adaptive) if (compact) {
						for (int at = offset[v]; at < offset[v + 1]; ++at) {
							int status = visit(adjacency[at]);
							if (status != 1) return status;
						}
						continue;
					}
					for (int at = offset[v]; at >= 0; at = adjacency[at]) {
						int status = visit(edges[at].second);
						if (status != 1) return status;
					}
				}
				return 1;
			};
			auto empty = [&](int v) {
				if constexpr (Adaptive) if (compact) return offset[v] == offset[v + 1];
				return offset[v] < 0;
			};
			int status = propagate();
			if (!status) return 0;
			for (int i = 0; status > 0 && i < variables; ++i) if (values[i] < 0) {
				if (empty(2 * i)) values[i] = 0;
				else if (empty(2 * i + 1)) values[i] = 1;
				else {
					trail.assign(1, 2 * i);
					values[i] = 0;
					status = propagate();
					if (!status) {
						trail.assign(1, 2 * i + 1);
						values[i] = 1;
						status = propagate();
						if (!status) return 0;
					}
				}
			}
			return status;
		};
		int status = edges.size() >= std::size_t(variables) && branching > edges.size() / 32
			? closure.template operator()<true>() : closure.template operator()<false>();
		if (status >= 0) {
			if (status) for (int i = 0; i < variables; ++i) answer[i] = values[i];
			return status;
		}
	}
	if (!compact) make_compact();
	trail.clear();
	std::vector<int> order(n, -1);
	struct Frame { int vertex, next, low; };
	std::vector<Frame> stack;
	stack.reserve(n);
	int clock = 0, component = -2;
	auto enter = [&](int v) {
		order[v] = clock++;
		trail.push_back(v);
		stack.push_back({v, offset[v], order[v]});
	};
	for (int root = 0; root < n; ++root) if (order[root] == -1) {
		if (offset[root] == offset[root + 1]) { order[root] = component--; continue; }
		enter(root);
		while (!stack.empty()) {
			auto &frame = stack.back();
			int v = frame.vertex, next = frame.next, low = frame.low;
			for (; next < offset[v + 1]; ++next) {
				int to = adjacency[next];
				if (order[to] == -1) {
					if (offset[to] == offset[to + 1]) { order[to] = component--; continue; }
					break;
				}
				if (order[to] >= 0) low = std::min(low, order[to]);
			}
			frame.low = low;
			if (next < offset[v + 1]) {
				frame.next = next + 1;
				enter(adjacency[next]);
			} else {
				stack.pop_back();
				if (low == order[v]) {
					int u;
					do {
						u = trail.back();
						trail.pop_back();
						order[u] = component;
						if (order[u ^ 1] == component) return false;
					} while (u != v);
					--component;
				}
				if (!stack.empty()) stack.back().low = std::min(stack.back().low, low);
			}
		}
	}
	for (int i = 0; i < variables; ++i) answer[i] = order[2 * i] < order[2 * i + 1];
	return true;
}
}


#line 5 "graph/two_sat.hpp"

class TwoSAT {
	int n_;
	std::vector<std::pair<int, int>> edges_;
	std::vector<int> forced_;
	std::vector<bool> answer_;
	bool valid_ = false, solved_ = false;
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), answer_(vertices(n) / 2) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = solved_ = false;
		if (i == j && f != g) return;
		assert(edges_.size() <= std::size_t(std::numeric_limits<int>::max() - 1 - int(i != j)));
		edges_.emplace_back(2 * i + !f, 2 * j + g);
		if (i == j) forced_.push_back(2 * i + f);
		if (i != j) edges_.emplace_back(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		if (solved_) return valid_;
		valid_ = false;
		if (edges_.empty()) std::fill(answer_.begin(), answer_.end(), false), valid_ = true;
		else valid_ = two_sat_detail::solve(n_, edges_, forced_, answer_);
		solved_ = true;
		return valid_;
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
