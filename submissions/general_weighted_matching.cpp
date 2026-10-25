#line 1 "test/general_weighted_matching.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/general_weighted_matching"
#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
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


#line 1 "graph/general_weighted_matching.hpp"



#line 1 "components/weighted_blossom.hpp"



#line 8 "components/weighted_blossom.hpp"
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace weighted_matching_detail {
template <class T> class Blossom {
	struct Edge { int u = 0, v = 0; T weight = 0; };
	int n, stride, nodes;
	std::size_t head = 0;
	std::uint32_t stamp = 0;
	std::vector<Edge> graph;
	std::vector<T> dual;
	std::vector<int> match, base, parent, slack, from, queue, adjacent;
	std::vector<std::size_t> start;
	std::vector<signed char> state;
	std::vector<std::uint32_t> seen;
	std::vector<std::vector<int>> children;
	Edge &edge(int u, int v) { return graph[std::size_t(u) * stride + v]; }
	int &child(int b, int v) { return from[std::size_t(b) * (n + 1) + v]; }
	T gap(const Edge &e) const { return dual[e.u] + dual[e.v] - e.weight * 2; }
	void relax(int u, int b) {
		if (!slack[b] || gap(edge(u, b)) < gap(edge(slack[b], b))) slack[b] = u;
	}
	void reset_slack(int b) {
		slack[b] = 0;
		for (int u = 1; u <= n; ++u)
			if (base[u] != b && state[base[u]] == 0 && edge(u, b).weight > 0) relax(u, b);
	}
	void enqueue(int b) {
		if (b <= n) queue.push_back(b);
		else for (int c : children[b]) enqueue(c);
	}
	void set_base(int b, int root) {
		base[b] = root;
		if (b > n) for (int c : children[b]) set_base(c, root);
	}
	int orient(int b, int c) {
		auto &list = children[b];
		int p = std::find(list.begin(), list.end(), c) - list.begin();
		assert(p < int(list.size()));
		if (p & 1) { std::reverse(list.begin() + 1, list.end()); p = int(list.size()) - p; }
		return p;
	}
	void set_match(int b, int v) {
		Edge e = edge(b, v);
		match[b] = e.v;
		if (b <= n) return;
		int c = child(b, e.u), p = orient(b, c);
		auto &list = children[b];
		for (int i = 0; i < p; ++i) set_match(list[i], list[i ^ 1]);
		set_match(c, v);
		std::rotate(list.begin(), list.begin() + p, list.end());
	}
	void augment(int u, int v) {
		while (u) {
			int next = base[match[u]];
			set_match(u, v);
			if (!next) break;
			u = base[parent[next]];
			set_match(next, u);
			v = next;
		}
	}
	int lca(int u, int v) {
		if (!++stamp) std::fill(seen.begin(), seen.end(), 0), ++stamp;
		for (; u || v; std::swap(u, v)) if (u) {
			if (seen[u] == stamp) return u;
			seen[u] = stamp;
			u = base[match[u]];
			if (u) u = base[parent[u]];
		}
		return 0;
	}
	void contract(int u, int v, int root) {
		int b = n + 1;
		while (b <= nodes && base[b]) ++b;
		nodes = std::max(nodes, b);
		dual[b] = 0; state[b] = 0; match[b] = match[root];
		auto &list = children[b];
		list.clear(); list.push_back(root);
		for (int x = u; x != root;) {
			int y = base[match[x]];
			list.push_back(x); list.push_back(y); enqueue(y);
			x = base[parent[y]];
		}
		std::reverse(list.begin() + 1, list.end());
		for (int x = v; x != root;) {
			int y = base[match[x]];
			list.push_back(x); list.push_back(y); enqueue(y);
			x = base[parent[y]];
		}
		set_base(b, b);
		for (int x = 1; x <= nodes; ++x) edge(b, x) = edge(x, b) = Edge{};
		std::fill_n(from.begin() + std::size_t(b) * (n + 1), n + 1, 0);
		for (int c : list) {
			for (int x = 1; x <= nodes; ++x) if (edge(c, x).weight > 0) {
				if (!edge(b, x).weight || gap(edge(c, x)) < gap(edge(b, x))) {
					edge(b, x) = edge(c, x); edge(x, b) = edge(x, c);
				}
			}
			for (int x = 1; x <= n; ++x) if (child(c, x)) child(b, x) = c;
		}
		reset_slack(b);
	}
	void expand(int b) {
		for (int c : children[b]) set_base(c, c);
		int entry = child(b, edge(b, parent[b]).u), p = orient(b, entry);
		auto &list = children[b];
		for (int i = 0; i < p; i += 2) {
			int u = list[i], v = list[i + 1];
			parent[u] = edge(v, u).u;
			state[u] = 1; state[v] = 0; slack[u] = 0;
			reset_slack(v); enqueue(v);
		}
		state[entry] = 1; parent[entry] = parent[b]; slack[entry] = 0;
		for (int i = p + 1; i < int(list.size()); ++i) {
			state[list[i]] = -1; reset_slack(list[i]);
		}
		base[b] = 0;
	}
	bool found(const Edge &e) {
		int u = base[e.u], v = base[e.v];
		if (state[v] == -1) {
			parent[v] = e.u; state[v] = 1;
			int next = base[match[v]];
			assert(next);
			slack[v] = slack[next] = 0; state[next] = 0; enqueue(next);
		} else if (state[v] == 0) {
			int root = lca(u, v);
			if (!root) { augment(u, v); augment(v, u); return true; }
			contract(u, v, root);
		}
		return false;
	}
	bool step() {
		std::fill(state.begin(), state.end(), -1);
		std::fill(slack.begin(), slack.end(), 0);
		queue.clear(); head = 0;
		for (int b = 1; b <= nodes; ++b) if (base[b] == b && !match[b]) {
			parent[b] = 0; state[b] = 0; enqueue(b);
		}
		if (queue.empty()) return false;
		for (;;) {
			while (head < queue.size()) {
				int u = queue[head++];
				if (state[base[u]] != 0) continue;
				for (std::size_t i = start[u]; i < start[u + 1]; ++i) {
					int v = adjacent[i];
					if (base[u] == base[v]) continue;
					if (!gap(edge(u, v))) { if (found(edge(u, v))) return true; }
					else relax(u, base[v]);
				}
			}
			T delta = std::numeric_limits<T>::max() / 2;
			for (int b = n + 1; b <= nodes; ++b)
				if (base[b] == b && state[b] == 1) delta = std::min(delta, dual[b] / 2);
			for (int b = 1; b <= nodes; ++b) if (base[b] == b && slack[b]) {
				T d = gap(edge(slack[b], b));
				if (state[b] == -1) delta = std::min(delta, d);
				else if (state[b] == 0) delta = std::min(delta, d / 2);
			}
			for (int u = 1; u <= n; ++u) {
				if (state[base[u]] == 0) {
					if (dual[u] <= delta) return false;
					dual[u] -= delta;
				} else if (state[base[u]] == 1) dual[u] += delta;
			}
			for (int b = n + 1; b <= nodes; ++b) if (base[b] == b) {
				if (state[b] == 0) dual[b] += delta * 2;
				else if (state[b] == 1) dual[b] -= delta * 2;
			}
			queue.clear(); head = 0;
			for (int b = 1; b <= nodes; ++b)
				if (base[b] == b && slack[b] && base[slack[b]] != b && !gap(edge(slack[b], b)))
					if (found(edge(slack[b], b))) return true;
			for (int b = n + 1; b <= nodes; ++b)
				if (base[b] == b && state[b] == 1 && !dual[b]) expand(b);
		}
	}
public:
	Blossom(int n, std::span<const std::int64_t> weights, T maximum)
		: n(n), stride(2 * n + 1), nodes(n), graph(std::size_t(stride) * stride), dual(stride),
		match(stride), base(stride), parent(stride), slack(stride), from(std::size_t(stride) * (n + 1)),
		start(n + 2), state(stride, -1), seen(stride), children(stride) {
		for (int u = 1; u <= n; ++u) {
			base[u] = u; child(u, u) = u; dual[u] = maximum;
			start[u] = adjacent.size();
			for (int v = 1; v <= n; ++v) {
				T weight = T(weights[std::size_t(u - 1) * n + v - 1]);
				edge(u, v) = Edge{u, v, weight};
				if (weight > 0) adjacent.push_back(v);
			}
		}
		start[n + 1] = adjacent.size();
		queue.reserve(n);
	}
	void solve(std::vector<int> &mate) {
		while (step()) {}
		for (int u = 1; u <= n; ++u) mate[u - 1] = match[u] ? match[u] - 1 : -1;
	}
};
}


#line 12 "graph/general_weighted_matching.hpp"

class GeneralWeightedMatching {
	int n;
	std::vector<std::int64_t> weights;
	std::int64_t maximum = 0;
	__int128_t total = 0;
	int cardinality = 0;
	bool built = true;
	static int checked_size(int n) { assert(0 <= n && n <= (std::numeric_limits<int>::max() - 1) / 2); return n; }
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n); }
public:
	using weight_type = std::int64_t;
	using total_type = __int128_t;
	std::vector<int> mate;
	explicit GeneralWeightedMatching(int n = 0) : n(checked_size(n)), weights(std::size_t(this->n) * this->n), mate(this->n, -1) {}
	GeneralWeightedMatching(const GeneralWeightedMatching &) = default;
	GeneralWeightedMatching &operator=(const GeneralWeightedMatching &) = default;
	GeneralWeightedMatching(GeneralWeightedMatching &&other) noexcept : GeneralWeightedMatching() { swap(other); }
	GeneralWeightedMatching &operator=(GeneralWeightedMatching &&other) noexcept {
		if (this != &other) { GeneralWeightedMatching moved(std::move(other)); swap(moved); }
		return *this;
	}
	int size() const { return n; }
	void add_edge(int u, int v, weight_type weight) {
		vertex(u); vertex(v);
		if (u == v || weight <= 0) return;
		auto &current = weights[std::size_t(u) * n + v];
		if (weight <= current) return;
		current = weights[std::size_t(v) * n + u] = weight;
		maximum = std::max(maximum, weight); built = false;
	}
	total_type build() {
		if (built) return total;
		std::int64_t bound = 4 * (std::int64_t(n) + 1);
		if (maximum <= std::numeric_limits<int>::max() / bound)
			weighted_matching_detail::Blossom<int>(n, weights, int(maximum)).solve(mate);
		else if (maximum <= std::numeric_limits<std::int64_t>::max() / bound)
			weighted_matching_detail::Blossom<std::int64_t>(n, weights, maximum).solve(mate);
		else weighted_matching_detail::Blossom<__int128_t>(n, weights, maximum).solve(mate);
		total = 0; cardinality = 0;
		for (int u = 0; u < n; ++u) if (u < mate[u]) {
			total += weights[std::size_t(u) * n + mate[u]]; ++cardinality;
		}
		built = true; return total;
	}
	total_type weight() const { assert(built); return total; }
	int matching_size() const { assert(built); return cardinality; }
	std::vector<std::pair<int, int>> edges() const {
		assert(built);
		std::vector<std::pair<int, int>> result; result.reserve(cardinality);
		for (int u = 0; u < n; ++u) if (u < mate[u]) result.emplace_back(u, mate[u]);
		return result;
	}
	void clear() {
		std::fill(weights.begin(), weights.end(), 0); std::fill(mate.begin(), mate.end(), -1);
		maximum = 0; total = 0; cardinality = 0; built = true;
	}
	void swap(GeneralWeightedMatching &other) noexcept {
		using std::swap;
		swap(n, other.n); weights.swap(other.weights); swap(maximum, other.maximum);
		swap(total, other.total); swap(cardinality, other.cardinality); swap(built, other.built); mate.swap(other.mate);
	}
};


#line 4 "test/general_weighted_matching.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	in.read(n), in.read(m);
	GeneralWeightedMatching graph(n);
	while (m--) {
		int u = 0, v = 0;
		std::int64_t weight = 0;
		in.read(u), in.read(v), in.read(weight);
		graph.add_edge(u, v, weight);
	}
	auto weight = graph.build();
	out.write(graph.matching_size()); out.write(' '); out.println(weight);
	for (auto [u, v] : graph.edges()) { out.write(u); out.write(' '); out.println(v); }
}
