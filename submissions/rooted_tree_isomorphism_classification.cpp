#line 1 "test/rooted_tree_hash.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification"
#line 1 "tree/rooted_tree_hash.hpp"



#line 1 "components/tree_hash_core.hpp"



#line 1 "components/splitmix64.hpp"



#include <cstdint>

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 5 "components/tree_hash_core.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <compare>
#include <cstddef>
#line 12 "components/tree_hash_core.hpp"
#include <limits>
#include <vector>

struct TreeHashValue {
	std::array<std::uint64_t, 2> value{};
	auto operator<=>(const TreeHashValue &) const = default;
	struct Hasher {
		std::size_t operator()(const TreeHashValue &hash) const noexcept {
			return splitmix64(hash.value[0] ^ splitmix64(hash.value[1]));
		}
	};
};

namespace tree_hash_detail {

inline constexpr std::uint64_t mod = (std::uint64_t(1) << 61) - 1;

inline std::uint64_t default_seed() {
	static const std::uint64_t seed = splitmix64(std::uint64_t(std::chrono::steady_clock::now().time_since_epoch().count()) ^
		std::uint64_t(reinterpret_cast<std::uintptr_t>(&mod)));
	return seed;
}

inline std::uint64_t mul(std::uint64_t a, std::uint64_t b) {
	__uint128_t product = __uint128_t(a) * b;
	std::uint64_t value = (std::uint64_t(product) & mod) + std::uint64_t(product >> 61);
	return value >= mod ? value - mod : value;
}

struct Rooting {
	std::vector<int> parent, order, height, size;
};

inline Rooting root_tree(const std::vector<std::vector<int>> &graph, int root) {
	assert(graph.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(graph.size());
	assert(n ? 0 <= root && root < n : root == 0 || root == -1);
	Rooting result;
	if (!n) return result;
	result.parent.assign(n, -2);
	result.height.assign(n, 0);
	result.size.assign(n, 1);
	result.order.reserve(n);
	result.parent[root] = -1;
	result.order.push_back(root);
	[[maybe_unused]] std::size_t edges = 0;
	for (std::size_t i = 0; i < result.order.size(); ++i) {
		int v = result.order[i];
		edges += graph[v].size();
		for (int u : graph[v]) {
			assert(0 <= u && u < n && u != v);
			if (u == result.parent[v]) continue;
			assert(result.parent[u] == -2);
			result.parent[u] = v;
			result.order.push_back(u);
		}
	}
	assert(result.order.size() == graph.size() && edges == std::size_t(n - 1) * 2);
	for (auto it = result.order.rbegin(); it != result.order.rend(); ++it) {
		int v = *it, p = result.parent[v];
		if (p == -1) continue;
		result.height[p] = std::max(result.height[p], result.height[v] + 1);
		result.size[p] += result.size[v];
	}
	return result;
}

inline TreeHashValue salt_at(int height, std::uint64_t seed) {
	SplitMix64 random(seed + std::uint64_t(height) * 2 * 0x9e3779b97f4a7c15ULL);
	return { {random() % (mod - 1) + 1, random() % (mod - 1) + 1} };
}

inline void append(TreeHashValue &hash, const TreeHashValue &child, const TreeHashValue &salt) {
	for (int k = 0; k < 2; ++k) {
		std::uint64_t factor = salt.value[k] + child.value[k];
		if (factor >= mod) factor -= mod;
		hash.value[k] = mul(hash.value[k], factor);
	}
}

inline std::vector<TreeHashValue> hashes(const std::vector<std::vector<int>> &graph, const Rooting &tree, std::uint64_t seed) {
	std::vector<TreeHashValue> result(graph.size(), TreeHashValue{ {1, 1} });
	if (graph.empty()) return result;
	std::vector<TreeHashValue> salt(tree.height[tree.order[0]] + 1);
	SplitMix64 random(seed);
	for (auto &value : salt) value = { {random() % (mod - 1) + 1, random() % (mod - 1) + 1} };
	for (auto it = tree.order.rbegin(); it + 1 != tree.order.rend(); ++it) {
		int v = *it, p = tree.parent[v];
		append(result[p], result[v], salt[tree.height[p]]);
	}
	return result;
}

}


#line 5 "tree/rooted_tree_hash.hpp"
#include <utility>

class RootedTreeHash {
	std::uint64_t seed_ = default_seed();

  public:
	using Hash = TreeHashValue;
	int root = -1;
	std::vector<Hash> hash;
	std::vector<int> parent, height, subtree_size;
	static std::uint64_t default_seed() { return tree_hash_detail::default_seed(); }
	RootedTreeHash() = default;
	explicit RootedTreeHash(const std::vector<std::vector<int>> &graph, int root = 0, std::uint64_t seed = default_seed()) {
		build(graph, root, seed);
	}
	void build(const std::vector<std::vector<int>> &graph, int root = 0, std::uint64_t seed = default_seed()) {
		auto tree = tree_hash_detail::root_tree(graph, root);
		hash = tree_hash_detail::hashes(graph, tree, seed);
		parent = std::move(tree.parent);
		height = std::move(tree.height);
		subtree_size = std::move(tree.size);
		this->root = graph.empty() ? -1 : root;
		seed_ = seed;
	}
	int size() const { return static_cast<int>(hash.size()); }
	bool empty() const { return hash.empty(); }
	std::uint64_t seed() const { return seed_; }
	Hash get() const { return empty() ? Hash{} : hash[root]; }
	Hash get(int v) const {
		assert(0 <= v && v < size());
		return hash[v];
	}
	Hash operator[](int v) const { return get(v); }
};


#line 1 "components/fast_io.hpp"



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


#line 4 "test/rooted_tree_hash.test.cpp"
#include <unordered_map>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<std::vector<int>> graph(n);
	for (int v = 1; v < n; ++v) {
		int p = 0;
		input.read(p);
		graph[p].push_back(v);
		graph[v].push_back(p);
	}
	RootedTreeHash tree(graph);
	std::unordered_map<TreeHashValue, int, TreeHashValue::Hasher> ids;
	ids.reserve(n);
	std::vector<int> answer(n);
	for (int v = 0; v < n; ++v) answer[v] = ids.try_emplace(tree[v], static_cast<int>(ids.size())).first->second;
	output.write(static_cast<int>(ids.size()));
	output.write('\n');
	for (int v = 0; v < n; ++v) {
		output.write(answer[v]);
		output.write(v + 1 == n ? '\n' : ' ');
	}
}
