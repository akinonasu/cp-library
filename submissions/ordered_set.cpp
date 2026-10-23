#line 1 "test/ordered_set.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
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


#line 1 "data_structure/top_k_sum.hpp"



#line 8 "data_structure/top_k_sum.hpp"
#include <functional>
#include <initializer_list>
#include <limits>
#include <optional>
#line 13 "data_structure/top_k_sum.hpp"
#include <utility>
#include <vector>
#line 1 "components/ordered_multiset_btree.hpp"



#line 15 "components/ordered_multiset_btree.hpp"
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif
#line 1 "components/radix_sort.hpp"



#line 11 "components/radix_sort.hpp"

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	for (int shift = 0; shift < 64; shift += 8) {
		if (!((varying >> shift) & 255)) continue;
		std::array<std::size_t, 256> count{};
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255]++] = std::move(value);
		values.swap(scratch);
	}
}


#line 19 "components/ordered_multiset_btree.hpp"

namespace ordered_multiset_detail {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
template<bool Strict, class Key>
__attribute__((target("avx2"))) int find_avx2(const Key *data, int n, Key value) {
	unsigned less = 0;
	constexpr int lanes = 32 / sizeof(Key);
	for (int i = 0; i < n; i += lanes) {
		auto keys = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(data + i));
		__m256i limit, match;
		if constexpr (sizeof(Key) == 4) {
			limit = _mm256_set1_epi32(static_cast<int>(value));
			if constexpr (std::is_unsigned_v<Key>) { auto sign = _mm256_set1_epi32(-2147483647 - 1); keys = _mm256_xor_si256(keys, sign); limit = _mm256_xor_si256(limit, sign); }
			match = _mm256_cmpgt_epi32(limit, keys);
			if constexpr (Strict) match = _mm256_or_si256(match, _mm256_cmpeq_epi32(limit, keys));
			less |= static_cast<unsigned>(_mm256_movemask_ps(_mm256_castsi256_ps(match))) << i;
		} else {
			limit = _mm256_set1_epi64x(static_cast<long long>(value));
			if constexpr (std::is_unsigned_v<Key>) { auto sign = _mm256_set1_epi64x(-9223372036854775807LL - 1); keys = _mm256_xor_si256(keys, sign); limit = _mm256_xor_si256(limit, sign); }
			match = _mm256_cmpgt_epi64(limit, keys);
			if constexpr (Strict) match = _mm256_or_si256(match, _mm256_cmpeq_epi64(limit, keys));
			less |= static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(match))) << i;
		}
	}
	if (n < 32) less &= (1U << n) - 1;
	return std::countr_one(less);
}
template<bool Strict, class Key>
__attribute__((target("avx512f"))) int find_avx512(const Key *data, int n, Key value) {
	unsigned less = 0;
	constexpr int lanes = 64 / sizeof(Key);
	for (int i = 0; i < n; i += lanes) {
		auto keys = _mm512_loadu_si512(data + i);
		unsigned match;
		if constexpr (sizeof(Key) == 4) {
			auto limit = _mm512_set1_epi32(static_cast<int>(value));
			if constexpr (std::is_unsigned_v<Key>) match = _mm512_cmp_epu32_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
			else match = _mm512_cmp_epi32_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
		} else {
			auto limit = _mm512_set1_epi64(static_cast<long long>(value));
			if constexpr (std::is_unsigned_v<Key>) match = _mm512_cmp_epu64_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
			else match = _mm512_cmp_epi64_mask(keys, limit, Strict ? _MM_CMPINT_LE : _MM_CMPINT_LT);
		}
		less |= match << i;
	}
	if (n < 32) less &= (1U << n) - 1;
	return std::countr_one(less);
}
#endif
template<bool Strict, class Key>
int find(const Key *data, int n, Key value) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if constexpr (sizeof(Key) == 4 || sizeof(Key) == 8) {
		static const int kernel = __builtin_cpu_supports("avx512f") ? 2 : __builtin_cpu_supports("avx2") ? 1 : 0;
		if (n >= 8) {
			if (kernel == 2) return find_avx512<Strict>(data, n, value);
			if (kernel == 1) return find_avx2<Strict>(data, n, value);
		}
	}
#endif
	int l = 0, r = n;
	while (l < r) {
		int m = (l + r) / 2;
		if (Strict ? data[m] <= value : data[m] < value) l = m + 1;
		else r = m;
	}
	return l;
}

template<class T, class Sum, class Aggregate>
class BTree {
	static constexpr int branch = 16, capacity = 32;
	using Count = std::int64_t;
	struct Leaf {
		std::array<T, capacity + 1> keys{};
		std::array<Count, capacity + 1> copies{};
		Aggregate total = 0;
		Count size = 0;
		int used = 0;
	};
	struct Node {
		std::array<T, branch + 1> high{};
		std::array<int, branch + 1> child{};
		std::array<Count, branch + 1> sizes{};
		std::array<Aggregate, branch + 1> sums{};
		int used = 0;
	};
	std::vector<Leaf> leaves;
	std::vector<Node> nodes;
	int root = 0, first = -1, last = -1, free_leaf = -1, free_node = 0;
	static Aggregate weight(T value, Count count) { return static_cast<Aggregate>(static_cast<Sum>(value)) * count; }
	Count length(int p) const { return p < 0 ? leaves[-p - 1].size : nodes[p - 1].sizes[nodes[p - 1].used - 1]; }
	Aggregate total(int p) const { return p < 0 ? leaves[-p - 1].total : nodes[p - 1].sums[nodes[p - 1].used - 1]; }
	T maximum(int p) const { return p < 0 ? leaves[-p - 1].keys[leaves[-p - 1].used - 1] : nodes[p - 1].high[nodes[p - 1].used - 1]; }
	int used(int p) const { return p < 0 ? leaves[-p - 1].used : nodes[p - 1].used; }
	int make_leaf() {
		if (free_leaf >= 0) {
			int p = free_leaf;
			free_leaf = leaves[p].used;
			leaves[p].used = 0;
			leaves[p].size = 0;
			leaves[p].total = 0;
			return -p - 1;
		}
		assert(leaves.size() < std::size_t((std::numeric_limits<int>::max)()));
		leaves.emplace_back();
		return -static_cast<int>(leaves.size());
	}
	int make_node() {
		if (free_node) {
			int p = free_node;
			free_node = nodes[p - 1].used;
			nodes[p - 1].used = 0;
			return p;
		}
		assert(nodes.size() < std::size_t((std::numeric_limits<int>::max)()));
		nodes.emplace_back();
		return static_cast<int>(nodes.size());
	}
	void release(int p) {
		if (p < 0) {
			int i = -p - 1;
			leaves[i].used = free_leaf;
			free_leaf = i;
		} else {
			nodes[p - 1].used = free_node;
			free_node = p;
		}
	}
	void pull(int p) {
		auto &node = nodes[p - 1];
		Count size = 0;
		Aggregate sum = 0;
		for (int i = 0; i < node.used; ++i) {
			node.high[i] = maximum(node.child[i]);
			node.sizes[i] = size += length(node.child[i]);
			node.sums[i] = sum += total(node.child[i]);
		}
	}
	void pull_leaf(int p) {
		auto &leaf = leaves[-p - 1];
		leaf.size = 0; leaf.total = 0;
		for (int i = 0; i < leaf.used; ++i) {
			leaf.size += leaf.copies[i];
			leaf.total += weight(leaf.keys[i], leaf.copies[i]);
		}
	}
	int child(int p, T value) const {
		const auto &node = nodes[p - 1];
		if (value <= node.high[0]) return 0;
		if (node.high[node.used - 2] < value) return node.used - 1;
		return find<false>(node.high.data(), node.used - 1, value);
	}
	static int position(const Leaf &leaf, T value) {
		if (!leaf.used || value <= leaf.keys[0]) return 0;
		if (leaf.keys[leaf.used - 1] <= value) return leaf.used - 1 + (leaf.keys[leaf.used - 1] < value);
		return find<false>(leaf.keys.data(), leaf.used, value);
	}
	void add(int p, int j, Count delta, Aggregate difference) {
		auto &node = nodes[p - 1];
		node.high[j] = maximum(node.child[j]);
		for (int i = j; i < node.used; ++i) {
			node.sizes[i] += delta;
			node.sums[i] += difference;
		}
	}
	void prepare_split(const std::array<int, 32> &path, int depth) {
		constexpr std::size_t limit = (std::numeric_limits<int>::max)();
		if (free_leaf < 0 && leaves.size() == leaves.capacity()) {
			assert(leaves.size() < limit);
			leaves.reserve((std::min)(limit, leaves.capacity() + leaves.capacity() / 2 + 1));
		}
		std::size_t needed = 0;
		while (depth && nodes[path[depth - 1] - 1].used == branch) { ++needed; --depth; }
		if (!depth) ++needed;
		for (int p = free_node; p && needed; p = nodes[p - 1].used) --needed;
		if (needed > nodes.capacity() - nodes.size()) {
			assert(needed <= limit - nodes.size());
			nodes.reserve((std::max)(nodes.size() + needed, (std::min)(limit, nodes.capacity() + nodes.capacity() / 2 + 1)));
		}
	}
	bool rebalance(int p, int slot) {
		auto &parent = nodes[p - 1];
		int j = slot ? slot - 1 : 0;
		int a = parent.child[j], b = parent.child[j + 1];
		int n = used(a) + used(b);
		bool merge;
		if (a < 0) {
			std::array<T, capacity * 2> keys;
			std::array<Count, capacity * 2> copies;
			auto &left = leaves[-a - 1], &right = leaves[-b - 1];
			std::copy_n(left.keys.begin(), left.used, keys.begin());
			std::copy_n(right.keys.begin(), right.used, keys.begin() + left.used);
			std::copy_n(left.copies.begin(), left.used, copies.begin());
			std::copy_n(right.copies.begin(), right.used, copies.begin() + left.used);
			merge = n <= capacity;
			left.used = merge ? n : n / 2;
			std::copy_n(keys.begin(), left.used, left.keys.begin());
			std::copy_n(copies.begin(), left.used, left.copies.begin());
			pull_leaf(a);
			if (!merge) {
				right.used = n - left.used;
				std::copy_n(keys.begin() + left.used, right.used, right.keys.begin());
				std::copy_n(copies.begin() + left.used, right.used, right.copies.begin());
				pull_leaf(b);
			} else if (last == -b - 1) last = -a - 1;
		} else {
			std::array<int, branch * 2> children;
			auto &left = nodes[a - 1], &right = nodes[b - 1];
			std::copy_n(left.child.begin(), left.used, children.begin());
			std::copy_n(right.child.begin(), right.used, children.begin() + left.used);
			merge = n <= branch;
			left.used = merge ? n : n / 2;
			std::copy_n(children.begin(), left.used, left.child.begin());
			pull(a);
			if (!merge) {
				right.used = n - left.used;
				std::copy_n(children.begin() + left.used, right.used, right.child.begin());
				pull(b);
			}
		}
		if (merge) {
			release(b);
			std::move(parent.child.begin() + j + 2, parent.child.begin() + parent.used, parent.child.begin() + j + 1);
			--parent.used;
		}
		pull(p);
		return merge;
	}
public:
	BTree() = default;
	BTree(const BTree &) = default;
	BTree(BTree &&other) noexcept { swap(other); }
	BTree &operator=(BTree other) noexcept { swap(other); return *this; }
	Count size() const { return root ? length(root) : 0; }
	void clear() { leaves.clear(); nodes.clear(); root = free_node = 0; first = last = free_leaf = -1; }
	void reserve(std::size_t distinct) {
		std::size_t count = distinct / (capacity / 2) + 2;
		leaves.reserve(count); nodes.reserve(count / (branch / 2 - 1) + 2);
	}
	void assign(std::vector<T> values) {
		clear();
		if (values.empty()) return;
		stable_radix_sort(values, [](T value) { using U = std::make_unsigned_t<T>; constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : 0; return U(U(value) ^ sign); });
		std::size_t distinct = 0;
		std::vector<Count> counts;
		for (T value : values) {
			if (!distinct || values[distinct - 1] != value) { values[distinct++] = value; counts.push_back(1); }
			else ++counts.back();
		}
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		int count = static_cast<int>((distinct + capacity * 3 / 4 - 1) / (capacity * 3 / 4));
		leaves.reserve(count);
		std::vector<int> layer;
		for (int i = 0, offset = 0; i < count; ++i) {
			int id = make_leaf();
			auto &leaf = leaves[-id - 1];
			leaf.used = static_cast<int>(distinct / count + (std::size_t(i) < distinct % count));
			std::copy_n(values.begin() + offset, leaf.used, leaf.keys.begin());
			std::copy_n(counts.begin() + offset, leaf.used, leaf.copies.begin());
			offset += leaf.used;
			pull_leaf(id); layer.push_back(id);
		}
		first = 0; last = count - 1;
		while (layer.size() > 1) {
			int count = static_cast<int>((layer.size() + branch - 1) / branch);
			std::vector<int> next;
			for (int i = 0, offset = 0; i < count; ++i) {
				int id = make_node();
				auto &node = nodes[id - 1];
				node.used = static_cast<int>(layer.size() / count + (std::size_t(i) < layer.size() % count));
				std::copy_n(layer.begin() + offset, node.used, node.child.begin());
				offset += node.used; pull(id); next.push_back(id);
			}
			layer.swap(next);
		}
		root = layer[0];
	}
	void insert(T value, Count count) {
		std::array<int, 32> path, slots;
		int p = root, depth = 0;
		if (!p) { root = p = make_leaf(); first = last = -p - 1; }
		while (p > 0) {
			int j = child(p, value);
			path[depth] = p; slots[depth++] = j; p = nodes[p - 1].child[j];
		}
		int index = -p - 1;
		int j = position(leaves[index], value), split = 0;
		bool present = j < leaves[index].used && leaves[index].keys[j] == value;
		if (!present && leaves[index].used == capacity) prepare_split(path, depth);
		auto &leaf = leaves[index];
		Aggregate difference = weight(value, count);
		leaf.size += count; leaf.total += difference;
		if (present) leaf.copies[j] += count;
		else {
			std::move_backward(leaf.keys.begin() + j, leaf.keys.begin() + leaf.used, leaf.keys.begin() + leaf.used + 1);
			std::move_backward(leaf.copies.begin() + j, leaf.copies.begin() + leaf.used, leaf.copies.begin() + leaf.used + 1);
			leaf.keys[j] = value; leaf.copies[j] = count; ++leaf.used;
			if (leaf.used > capacity) {
				split = make_leaf();
				auto &left = leaves[-p - 1], &right = leaves[-split - 1];
				left.used = capacity / 2; right.used = capacity / 2 + 1;
				std::copy_n(left.keys.begin() + left.used, right.used, right.keys.begin());
				std::copy_n(left.copies.begin() + left.used, right.used, right.copies.begin());
				pull_leaf(p); pull_leaf(split);
				if (last == -p - 1) last = -split - 1;
			}
		}
		while (depth) {
			p = path[--depth]; j = slots[depth];
			if (!split) { add(p, j, count, difference); continue; }
			auto &node = nodes[p - 1];
			std::move_backward(node.child.begin() + j + 1, node.child.begin() + node.used, node.child.begin() + node.used + 1);
			node.child[j + 1] = split; ++node.used;
			split = 0;
			if (node.used > branch) {
				split = make_node();
				auto &left = nodes[p - 1], &right = nodes[split - 1];
				left.used = branch / 2; right.used = branch / 2 + 1;
				std::copy_n(left.child.begin() + left.used, right.used, right.child.begin());
				pull(split);
			}
			pull(p);
		}
		if (split) {
			int next = make_node();
			nodes[next - 1].used = 2; nodes[next - 1].child[0] = root; nodes[next - 1].child[1] = split;
			pull(next); root = next;
		}
	}
	Count erase(T value, Count count) {
		if (!root) return 0;
		std::array<int, 32> path, slots;
		int p = root, depth = 0;
		while (p > 0) {
			int j = child(p, value);
			path[depth] = p; slots[depth++] = j; p = nodes[p - 1].child[j];
		}
		auto &leaf = leaves[-p - 1];
		int j = position(leaf, value);
		if (j == leaf.used || leaf.keys[j] != value) return 0;
		count = (std::min)(count, leaf.copies[j]);
		Aggregate difference = -weight(value, count);
		leaf.size -= count; leaf.total += difference; leaf.copies[j] -= count;
		if (!leaf.copies[j]) {
			std::move(leaf.keys.begin() + j + 1, leaf.keys.begin() + leaf.used, leaf.keys.begin() + j);
			std::move(leaf.copies.begin() + j + 1, leaf.copies.begin() + leaf.used, leaf.copies.begin() + j);
			--leaf.used;
		}
		if (!depth) {
			if (!leaf.used) { release(root); root = 0; first = last = -1; }
			return count;
		}
		bool small = leaf.used < capacity / 2;
		while (depth) {
			p = path[--depth]; j = slots[depth];
			if (small) { rebalance(p, j); small = nodes[p - 1].used < branch / 2; }
			else add(p, j, -count, difference);
		}
		if (root > 0 && nodes[root - 1].used == 1) { int old = root; root = nodes[old - 1].child[0]; release(old); }
		return count;
	}
	Count count(T value) const {
		int p = root;
		if (!p) return 0;
		while (p > 0) p = nodes[p - 1].child[child(p, value)];
		const auto &leaf = leaves[-p - 1];
		int j = position(leaf, value);
		return j < leaf.used && leaf.keys[j] == value ? leaf.copies[j] : 0;
	}
	template<bool Strict = false> Count order_of_key(T value) const {
		Count result = 0;
		int p = root;
		if (!p) return 0;
		while (p > 0) {
			const auto &node = nodes[p - 1]; int j = child(p, value);
			if (j) result += node.sizes[j - 1];
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = find<Strict>(leaf.keys.data(), leaf.used, value);
		for (int i = 0; i < j; ++i) result += leaf.copies[i];
		return result;
	}
	T kth(Count k) const {
		if (!k) return leaves[first].keys[0];
		if (k == size() - 1) return leaves[last].keys[leaves[last].used - 1];
		int p = root;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<true>(node.sizes.data(), node.used, k);
			if (j) k -= node.sizes[j - 1];
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = 0;
		while (k >= leaf.copies[j]) k -= leaf.copies[j++];
		return leaf.keys[j];
	}
	Aggregate sum(Count k) const {
		if (!root || !k) return 0;
		if (k >= size()) return total(root);
		Aggregate answer = 0;
		int p = root;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<false>(node.sizes.data(), node.used, k);
			if (j) { k -= node.sizes[j - 1]; answer += node.sums[j - 1]; } p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		for (int i = 0; k; ++i) { Count take = (std::min)(k, leaf.copies[i]); answer += weight(leaf.keys[i], take); k -= take; }
		return answer;
	}
	Aggregate sum_all() const { return root ? total(root) : Aggregate(0); }
	template<bool Strict> std::optional<T> bound(T value) const {
		int p = root;
		if (!p) return std::nullopt;
		while (p > 0) {
			const auto &node = nodes[p - 1];
			int j = find<Strict>(node.high.data(), node.used, value);
			if (j == node.used) return std::nullopt;
			p = node.child[j];
		}
		const auto &leaf = leaves[-p - 1];
		int j = find<Strict>(leaf.keys.data(), leaf.used, value);
		return j == leaf.used ? std::nullopt : std::optional<T>(leaf.keys[j]);
	}
	void swap(BTree &other) noexcept {
		leaves.swap(other.leaves); nodes.swap(other.nodes);
		std::swap(root, other.root); std::swap(first, other.first); std::swap(last, other.last);
		std::swap(free_leaf, other.free_leaf); std::swap(free_node, other.free_node);
	}
};
}


#line 16 "data_structure/top_k_sum.hpp"

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>, class Compare = std::greater<T>>
class TopKSum {
public:
	using value_type = T;
	using sum_type = Sum;
	using size_type = std::int64_t;
	using value_compare = Compare;

private:
	static constexpr bool descending = std::is_same_v<Compare, std::greater<T>> || std::is_same_v<Compare, std::greater<void>>;
	static constexpr bool widen = std::is_integral_v<Sum> && sizeof(Sum) <= 8;
	using Aggregate = std::conditional_t<widen, std::conditional_t<std::is_signed_v<Sum>, __int128_t, __uint128_t>, Sum>;
	static constexpr bool blocked = std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8
		&& (std::is_integral_v<Sum> || std::is_same_v<Sum, __int128_t> || std::is_same_v<Sum, __uint128_t>)
		&& (descending || std::is_same_v<Compare, std::less<T>> || std::is_same_v<Compare, std::less<void>>);
	struct EmptyBackend {};
	using Backend = std::conditional_t<blocked, ordered_multiset_detail::BTree<T, Sum, Aggregate>, EmptyBackend>;
	static Aggregate contribution(const T &value, size_type copies) {
		auto converted = static_cast<Aggregate>(static_cast<Sum>(value));
		return copies == 1 ? converted : converted * copies;
	}
	struct Node {
		Aggregate total;
		std::optional<T> value;
		size_type copies, size;
		int left = -1, right = -1, height = 1;
		Node(const T &value, size_type copies) : total(contribution(value, copies)), value(value), copies(copies), size(copies) {}
	};
	[[no_unique_address]] Compare order;
	[[no_unique_address]] Backend backend;
	std::vector<Node> nodes;
	int root = -1, free = -1;

	size_type length(int p) const { return p < 0 ? 0 : nodes[p].size; }
	int height(int p) const { return p < 0 ? 0 : nodes[p].height; }
	void pull(int p) {
		auto &node = nodes[p];
		Aggregate total = contribution(*node.value, node.copies);
		if (node.left >= 0) total += nodes[node.left].total;
		if (node.right >= 0) total += nodes[node.right].total;
		node.total = std::move(total);
		node.size = length(node.left) + node.copies + length(node.right);
		node.height = (std::max)(height(node.left), height(node.right)) + 1;
	}
	int rotate_left(int p) {
		int next = nodes[p].right;
		nodes[p].right = nodes[next].left;
		nodes[next].left = p;
		pull(p); pull(next);
		return next;
	}
	int rotate_right(int p) {
		int next = nodes[p].left;
		nodes[p].left = nodes[next].right;
		nodes[next].right = p;
		pull(p); pull(next);
		return next;
	}
	int balance(int p) {
		pull(p);
		int left = nodes[p].left, right = nodes[p].right;
		if (height(left) > height(right) + 1) {
			if (height(nodes[left].left) < height(nodes[left].right)) nodes[p].left = rotate_left(left);
			return rotate_right(p);
		}
		if (height(right) > height(left) + 1) {
			if (height(nodes[right].right) < height(nodes[right].left)) nodes[p].right = rotate_right(right);
			return rotate_left(p);
		}
		return p;
	}
	int make_node(const T &value, size_type copies) {
		if (free >= 0) {
			int p = free, next = nodes[p].left;
			nodes[p] = Node(value, copies);
			free = next;
			return p;
		}
		assert(nodes.size() < std::size_t((std::numeric_limits<int>::max)()));
		nodes.emplace_back(value, copies);
		return static_cast<int>(nodes.size()) - 1;
	}
	void release(int p) {
		nodes[p].value.reset();
		nodes[p].total = Aggregate(0);
		nodes[p].left = free;
		free = p;
	}
	int insert_node(int p, const T &value, size_type copies) {
		if (p < 0) return make_node(value, copies);
		if (order(value, *nodes[p].value)) {
			int next = insert_node(nodes[p].left, value, copies);
			nodes[p].left = next;
		} else if (order(*nodes[p].value, value)) {
			int next = insert_node(nodes[p].right, value, copies);
			nodes[p].right = next;
		} else nodes[p].copies += copies;
		return balance(p);
	}
	int detach_first(int p, int &first) {
		if (nodes[p].left < 0) {
			first = p;
			return nodes[p].right;
		}
		nodes[p].left = detach_first(nodes[p].left, first);
		return balance(p);
	}
	int erase_node(int p, const T &value, size_type copies, size_type &removed) {
		if (p < 0) return -1;
		if (order(value, *nodes[p].value)) nodes[p].left = erase_node(nodes[p].left, value, copies, removed);
		else if (order(*nodes[p].value, value)) nodes[p].right = erase_node(nodes[p].right, value, copies, removed);
		else {
			removed = (std::min)(copies, nodes[p].copies);
			if (removed < nodes[p].copies) nodes[p].copies -= removed;
			else {
				int left = nodes[p].left, right = nodes[p].right;
				if (left < 0 || right < 0) {
					release(p);
					return left < 0 ? right : left;
				}
				int next;
				right = detach_first(right, next);
				nodes[next].left = left; nodes[next].right = right;
				release(p);
				return balance(next);
			}
		}
		return removed ? balance(p) : p;
	}
	int build(const std::vector<T> &values, const std::vector<size_type> &counts, int first, int last) {
		if (first == last) return -1;
		int middle = first + (last - first) / 2;
		int p = make_node(values[middle], counts[middle]);
		int left = build(values, counts, first, middle), right = build(values, counts, middle + 1, last);
		nodes[p].left = left; nodes[p].right = right;
		pull(p);
		return p;
	}
	static Sum result(const Aggregate &value) {
		if constexpr (widen) assert(static_cast<Aggregate>((std::numeric_limits<Sum>::min)()) <= value && value <= static_cast<Aggregate>((std::numeric_limits<Sum>::max)()));
		return static_cast<Sum>(value);
	}
	template<bool Reverse> T kth_impl(size_type k) const {
		assert(0 <= k && k < size());
		if constexpr (blocked) return backend.kth(Reverse != descending ? size() - 1 - k : k);
		int p = root;
		for (;;) {
			const auto &node = nodes[p];
			int first = Reverse ? node.right : node.left;
			size_type before = length(first);
			if (k < before) p = first;
			else if (k < before + node.copies) return *node.value;
			else { k -= before + node.copies; p = Reverse ? node.left : node.right; }
		}
	}
	template<bool Reverse> Sum sum_impl(size_type k) const {
		assert(k >= 0);
		if constexpr (blocked) {
			k = (std::min)(k, size());
			if constexpr (Reverse != descending) return result(backend.sum_all() - backend.sum(size() - k));
			else return result(backend.sum(k));
		}
		Aggregate answer(0);
		int p = root;
		while (p >= 0 && k) {
			const auto &node = nodes[p];
			if (k >= node.size) { answer += node.total; break; }
			int first = Reverse ? node.right : node.left;
			size_type before = length(first);
			if (k <= before) p = first;
			else {
				if (first >= 0) answer += nodes[first].total;
				size_type take = (std::min)(k - before, node.copies);
				answer += contribution(*node.value, take);
				k -= before + take;
				p = Reverse ? node.left : node.right;
			}
		}
		return result(answer);
	}
	template<bool Strict> std::optional<T> bound(const T &value) const {
		if constexpr (blocked) return backend.template bound<Strict>(value);
		int p = root, candidate = -1;
		while (p >= 0) {
			const auto &node = nodes[p];
			bool found;
			if constexpr (Strict) found = descending ? order(*node.value, value) : order(value, *node.value);
			else found = descending ? !order(value, *node.value) : !order(*node.value, value);
			if (found) { candidate = p; p = descending ? node.right : node.left; }
			else p = descending ? node.left : node.right;
		}
		return candidate < 0 ? std::nullopt : nodes[candidate].value;
	}

public:
	explicit TopKSum(Compare compare = Compare{}) : order(std::move(compare)) {}
	explicit TopKSum(std::vector<T> values, Compare compare = Compare{}) : TopKSum(std::move(compare)) { assign(std::move(values)); }
	TopKSum(std::initializer_list<T> values, Compare compare = Compare{}) : TopKSum(std::vector<T>(values), std::move(compare)) {}
	TopKSum(const TopKSum &) = default;
	TopKSum(TopKSum &&other) noexcept(std::is_nothrow_copy_constructible_v<Compare>)
		: order(other.order), backend(std::move(other.backend)), nodes(std::move(other.nodes)), root(std::exchange(other.root, -1)), free(std::exchange(other.free, -1)) { other.nodes.clear(); }
	TopKSum &operator=(TopKSum other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		swap(other);
		return *this;
	}
	void assign(std::vector<T> values) {
		assert(values.size() <= std::size_t((std::numeric_limits<size_type>::max)()));
		if constexpr (blocked) { Backend next; next.assign(std::move(values)); backend.swap(next); return; }
		std::sort(values.begin(), values.end(), order);
		std::vector<size_type> counts;
		std::size_t distinct = 0;
		for (std::size_t i = 0; i < values.size(); ++i) {
			if (!distinct || order(values[distinct - 1], values[i])) {
				if (distinct != i) values[distinct] = std::move(values[i]);
				++distinct;
				counts.push_back(1);
			} else ++counts.back();
		}
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		TopKSum next(order);
		next.nodes.reserve(distinct);
		next.root = next.build(values, counts, 0, static_cast<int>(distinct));
		swap(next);
	}
	void reserve(std::size_t distinct) {
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		if constexpr (blocked) backend.reserve(distinct);
		else nodes.reserve(distinct);
	}
	void insert(const T &value, size_type copies = 1) {
		assert(copies >= 0 && copies <= (std::numeric_limits<size_type>::max)() - size());
		if constexpr (blocked) { if (copies) backend.insert(value, copies); }
		else if (copies) root = insert_node(root, value, copies);
	}
	bool erase(const T &value) { return erase(value, 1) != 0; }
	size_type erase(const T &value, size_type copies) {
		assert(copies >= 0);
		if constexpr (blocked) return copies ? backend.erase(value, copies) : 0;
		size_type removed = 0;
		if (copies) root = erase_node(root, value, copies, removed);
		return removed;
	}
	size_type erase_all(const T &value) { return erase(value, (std::numeric_limits<size_type>::max)()); }
	bool replace(const T &old_value, const T &new_value) {
		if (!erase(old_value)) return false;
		insert(new_value);
		return true;
	}
	size_type size() const { if constexpr (blocked) return backend.size(); else return length(root); }
	bool empty() const { return size() == 0; }
	T kth(size_type k) const { return kth_impl<false>(k); }
	T kth_smallest(size_type k) const { return kth_impl<descending>(k); }
	T kth_largest(size_type k) const { return kth_impl<!descending>(k); }
	Sum sum(size_type k) const { return sum_impl<false>(k); }
	Sum sum_smallest(size_type k) const { return sum_impl<descending>(k); }
	Sum sum_largest(size_type k) const { return sum_impl<!descending>(k); }
	std::optional<T> lower_bound(const T &value) const { return bound<false>(value); }
	std::optional<T> upper_bound(const T &value) const { return bound<true>(value); }
	Sum sum_all() const { if constexpr (blocked) return result(backend.sum_all()); else return root < 0 ? Sum(0) : result(nodes[root].total); }
	size_type order_of_key(const T &value) const {
		if constexpr (blocked) {
			if constexpr (descending) return size() - backend.template order_of_key<true>(value);
			else return backend.order_of_key(value);
		}
		size_type answer = 0;
		int p = root;
		while (p >= 0) {
			const auto &node = nodes[p];
			if (order(*node.value, value)) { answer += length(node.left) + node.copies; p = node.right; }
			else p = node.left;
		}
		return answer;
	}
	size_type count(const T &value) const {
		if constexpr (blocked) return backend.count(value);
		int p = root;
		while (p >= 0) {
			if (order(value, *nodes[p].value)) p = nodes[p].left;
			else if (order(*nodes[p].value, value)) p = nodes[p].right;
			else return nodes[p].copies;
		}
		return 0;
	}
	bool contains(const T &value) const { return count(value) != 0; }
	void clear() {
		if constexpr (blocked) backend.clear();
		else { nodes.clear(); root = free = -1; }
	}
	void swap(TopKSum &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		using std::swap;
		if constexpr (blocked) backend.swap(other.backend);
		swap(order, other.order); nodes.swap(other.nodes);
		swap(root, other.root); swap(free, other.free);
	}
	friend void swap(TopKSum &a, TopKSum &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using LargestKSum = TopKSum<T, Sum, std::greater<T>>;

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using SmallestKSum = TopKSum<T, Sum, std::less<T>>;


#line 5 "test/ordered_set.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<int> values(n);
	for (int &x : values) in.read(x);
	SmallestKSum<int> set(values);
	while (q--) {
		int type = 0, x = 0;
		in.read(type); in.read(x);
		if (type == 0) { if (!set.contains(x)) set.insert(x); }
		if (type == 1) set.erase_all(x);
		if (type == 2) out.println(set.size() < x ? -1 : set.kth(x - 1));
		if (type == 3) out.println(set.order_of_key(x + 1));
		if (type == 4) {
			auto k = set.order_of_key(x + 1);
			out.println(k ? set.kth(k - 1) : -1);
		}
		if (type == 5) out.println(set.lower_bound(x).value_or(-1));
	}
}
