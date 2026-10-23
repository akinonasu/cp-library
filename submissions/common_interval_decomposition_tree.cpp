#line 1 "test/permutation_tree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/common_interval_decomposition_tree"
#line 1 "tree/permutation_tree.hpp"



#line 1 "components/static_range_min.hpp"



#line 1 "components/default_init_allocator.hpp"



#include <memory>
#include <new>

template <class T>
struct DefaultInitAllocator : std::allocator<T> {
	template <class U>
	struct rebind {
		using other = DefaultInitAllocator<U>;
	};
	DefaultInitAllocator() = default;
	template <class U>
	DefaultInitAllocator(const DefaultInitAllocator<U> &) noexcept {}
	template <class U>
	void construct(U *p) { ::new (static_cast<void *>(p)) U; }
};


#line 1 "data_structure/range_query/sparse_table.hpp"



#line 5 "data_structure/range_query/sparse_table.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e = nullptr>
class SparseTable {
	using Value = std::conditional_t<std::is_same_v<S, bool>, std::uint8_t, S>;
	using Reference = std::conditional_t<std::is_same_v<S, bool>, bool, const S &>;
	int n = 0;
	std::vector<std::vector<Value, DefaultInitAllocator<Value>>> data;
	static decltype(auto) combine(const Value &a, const Value &b) {
		if constexpr (std::is_same_v<S, bool> || !std::is_invocable_v<decltype(op), const S &, const S &>) return S(op(S(a), S(b)));
		else return op(a, b);
	}

  public:
	SparseTable() = default;
	SparseTable(const SparseTable &) = default;
	SparseTable &operator=(const SparseTable &) = default;
	SparseTable(SparseTable &&other) noexcept : n(std::exchange(other.n, 0)), data(std::move(other.data)) {}
	SparseTable &operator=(SparseTable &&other) noexcept {
		if (this != &other) {
			n = std::exchange(other.n, 0);
			data = std::move(other.data);
		}
		return *this;
	}
	explicit SparseTable(const std::vector<S> &a) { build(a); }
	void clear() {
		data.clear();
		n = 0;
	}
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		build(static_cast<int>(a.size()), [&](int i) -> S { return a[i]; });
	}
	template <class F>
	void build(int size, F value) {
		assert(size >= 0);
		n = size;
		data.resize(std::bit_width(unsigned(n)));
		if (!n) return;
		if constexpr (std::is_default_constructible_v<Value>) {
			data[0].resize(n);
			for (int i = 0; i < n; ++i) data[0][i] = value(i);
		} else {
			data[0].clear();
			data[0].reserve(n);
			for (int i = 0; i < n; ++i) data[0].push_back(value(i));
		}
		for (int k = 1; k < std::bit_width(unsigned(n)); ++k) {
			int length = n - (1U << k) + 1, half = 1U << (k - 1);
			if constexpr (std::is_default_constructible_v<Value>) {
				data[k].resize(length);
				for (int i = 0; i < length; ++i) data[k][i] = combine(data[k - 1][i], data[k - 1][i + half]);
			} else {
				data[k].clear();
				data[k].reserve(length);
				for (int i = 0; i < length; ++i) data[k].push_back(combine(data[k - 1][i], data[k - 1][i + half]));
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	Reference operator[](int i) const {
		assert(0 <= i && i < n);
		return data[0][i];
	}
	[[gnu::always_inline]] S prod(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if constexpr (std::is_same_v<decltype(e), std::nullptr_t>) assert(l < r);
		else if (l == r) return e();
		return prod_unchecked(l, r);
	}
	[[gnu::always_inline]] S prod_unchecked(int l, int r) const {
		int k = std::bit_width(unsigned(r - l)) - 1;
		return combine(data[k][l], data[k][r - (1U << k)]);
	}
	S all_prod() const { return prod(0, n); }
};


#line 14 "components/static_range_min.hpp"

template <class T = int> class StaticRangeMin {
	static T minimum(T a, T b) { return b < a ? b : a; }
	std::vector<T> values_;
	std::vector<std::uint32_t, DefaultInitAllocator<std::uint32_t>> masks_;
	SparseTable<T, minimum> blocks_;
	T within(int l, int r) const {
		auto mask = masks_[r - 1] & (~std::uint32_t(0) << (l & 31));
		return values_[(l & ~31) + std::countr_zero(mask)];
	}

  public:
	StaticRangeMin() = default;
	explicit StaticRangeMin(std::vector<T> values) { build(std::move(values)); }
	void build(std::vector<T> values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		values_ = std::move(values);
		int n = size();
		masks_.resize(n);
		for (int first = 0; first < n;) {
			int last = first + std::min(32, n - first);
			std::uint32_t mask = 0;
			for (int i = first; i < last; ++i) {
				while (mask && !(values_[first + std::bit_width(mask) - 1] < values_[i]))
					mask &= ~(std::uint32_t(1) << (std::bit_width(mask) - 1));
				mask |= std::uint32_t(1) << (i - first);
				masks_[i] = mask;
			}
			first = last;
		}
		blocks_.build(n ? (n - 1) / 32 + 1 : 0, [&](int i) {
			int last = i * 32 + std::min(32, n - i * 32);
			return within(i * 32, last);
		});
	}
	int size() const { return static_cast<int>(values_.size()); }
	bool empty() const { return values_.empty(); }
	std::conditional_t<std::is_same_v<T, bool>, bool, const T &> operator[](int i) const {
		assert(0 <= i && i < size());
		return values_[i];
	}
	T prod(int l, int r) const {
		assert(0 <= l && l < r && r <= size());
		int a = l / 32, b = (r - 1) / 32;
		if (a == b) return within(l, r);
		T value = minimum(within(l, (a + 1) * 32), within(b * 32, r));
		if (a + 1 < b) value = minimum(value, blocks_.prod_unchecked(a + 1, b));
		return value;
	}
	T all_prod() const { return prod(0, size()); }
};


#line 9 "tree/permutation_tree.hpp"
#include <span>
#line 11 "tree/permutation_tree.hpp"

class PermutationTree {
  public:
	enum class Type : unsigned char { Leaf, Increasing, Decreasing, Prime };
	struct Node {
		int parent, l, r, lo, hi;
		Type type;
	};

  private:
	int root_ = -1;
	std::int64_t count_ = 0;
	std::vector<Node> nodes_;
	std::vector<int> offsets_, children_, order_;
	int add_node(int l, int r, int lo, int hi, Type type) {
		int id = size();
#ifdef __cpp_aggregate_paren_init
		nodes_.emplace_back(-1, l, r, lo, hi, type);
#else
		nodes_.push_back({-1, l, r, lo, hi, type});
#endif
		return id;
	}
	void pack(int n) {
		int m = size();
		offsets_.assign(m + 1, 0);
		for (auto &node : nodes_) if (node.parent >= 0) ++offsets_[node.parent + 1];
		for (int i = 0; i < m; ++i) offsets_[i + 1] += offsets_[i];
		children_.resize(m - 1);
		std::vector<int> cursor = offsets_;
		for (int i = 0; i < n; ++i) {
			int v = i;
			while (nodes_[v].parent >= 0) {
				int parent = nodes_[v].parent;
				children_[cursor[parent]++] = v;
				if (nodes_[parent].l != i) break;
				v = parent;
			}
		}
		count_ = n;
		for (int i = n; i < m; ++i) {
			std::int64_t k = offsets_[i + 1] - offsets_[i];
			count_ += nodes_[i].type == Type::Prime ? 1 : k * (k - 1) / 2;
		}
		order_.clear();
		order_.reserve(m);
		order_.push_back(root_);
		for (int i = 0; i < m; ++i)
			for (int child : children(order_[i])) order_.push_back(child);
	}

  public:
	PermutationTree() = default;
	explicit PermutationTree(std::span<const int> permutation) { build(permutation); }
	void build(std::span<const int> permutation) {
		assert(permutation.size() <= std::size_t(std::numeric_limits<int>::max() / 2));
		int n = static_cast<int>(permutation.size());
		root_ = -1;
		count_ = 0;
		nodes_.clear();
		offsets_.clear();
		children_.clear();
		order_.clear();
		if (!n) return;
		nodes_.reserve(2 * std::size_t(n) - 1);
		std::vector<int> inverse(n, -1);
		for (int i = 0; i < n; ++i) {
			int x = permutation[i];
			assert(0 <= x && x < n && inverse[x] < 0);
			inverse[x] = i;
			add_node(i, i + 1, x, x + 1, Type::Leaf);
		}
		struct Frame { int l, lo, hi; };
		struct Interval { int l, r, lo, hi; };
		std::vector<Interval> intervals;
		{
			StaticRangeMin minimum(std::move(inverse));
			std::vector<Frame> stack;
			stack.reserve(n);
			intervals.reserve(n - 1);
			for (int r = 1; r <= n; ++r) {
				int x = permutation[r - 1];
				Frame current{r - 1, x, x + 1};
				while (!stack.empty()) {
					auto &top = stack.back();
					top.lo = std::min(top.lo, current.lo);
					top.hi = std::max(top.hi, current.hi);
					if (top.hi - top.lo == r - top.l) {
						current = top;
						intervals.push_back({top.l, r, top.lo, top.hi});
						stack.pop_back();
					} else if (minimum.prod(top.lo, top.hi) < top.l) {
						Frame removed = top;
						stack.pop_back();
						assert(!stack.empty());
						stack.back().lo = std::min(stack.back().lo, removed.lo);
						stack.back().hi = std::max(stack.back().hi, removed.hi);
					} else break;
				}
				stack.push_back(current);
			}
			assert(stack.size() == 1 && stack[0].l == 0);
		}
		std::vector<int> at(n), next(n);
		for (int i = 0; i < n; ++i) at[i] = i, next[i] = i + 1;
		for (auto interval : intervals) {
			int l = interval.l, r = interval.r, middle = next[l];
			int first = at[l], id;
			if (next[middle] == r) {
				int second = at[middle];
				Type type = nodes_[first].hi == nodes_[second].lo ? Type::Increasing : Type::Decreasing;
				if (nodes_[first].type == type) {
					id = first;
					nodes_[id].r = r;
					nodes_[id].lo = interval.lo;
					nodes_[id].hi = interval.hi;
					nodes_[second].parent = id;
				} else {
					id = add_node(l, r, interval.lo, interval.hi, type);
					nodes_[first].parent = nodes_[second].parent = id;
				}
			} else {
				id = add_node(l, r, interval.lo, interval.hi, Type::Prime);
				for (int i = l; i < r; i = next[i]) nodes_[at[i]].parent = id;
			}
			at[l] = id;
			next[l] = r;
		}
		root_ = at[0];
		assert(next[0] == n);
		pack(n);
	}
	int size() const { return static_cast<int>(nodes_.size()); }
	bool empty() const { return nodes_.empty(); }
	int root() const { return empty() ? -1 : root_; }
	int length() const { return empty() ? 0 : nodes_[root_].r; }
	int leaf(int i) const { assert(0 <= i && i < length()); return i; }
	const Node &operator[](int i) const {
		assert(0 <= i && i < size());
		return nodes_[i];
	}
	const std::vector<Node> &nodes() const { return nodes_; }
	std::span<const int> children(int v) const {
		assert(0 <= v && v < size());
		return std::span<const int>(children_).subspan(offsets_[v], offsets_[v + 1] - offsets_[v]);
	}
	std::span<const int> order() const { return order_; }
	std::int64_t count_intervals() const { return empty() ? 0 : count_; }
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
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


#line 4 "test/permutation_tree.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<int> p(n);
	for (int &x : p) in.read(x);
	PermutationTree tree(p);
	std::vector<int> id(tree.size());
	for (int i = 0; i < tree.size(); ++i) id[tree.order()[i]] = i;
	out.println(tree.size());
	for (int v : tree.order()) {
		const auto &node = tree[v];
		out.write(node.parent < 0 ? -1 : id[node.parent]); out.write(' ');
		out.write(node.l); out.write(' ');
		out.write(node.r - 1); out.write(' ');
		out.write(node.type == PermutationTree::Type::Prime ? "prime\n" : "linear\n");
	}
}
