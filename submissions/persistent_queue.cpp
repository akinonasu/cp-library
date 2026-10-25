#line 1 "test/persistent_queue_batch.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#line 1 "data_structure/persistent/persistent_queue.hpp"



#line 1 "data_structure/persistent/persistent_stack.hpp"



#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T>
class PersistentStack {
  public:
	using value_type = T;
	using root_type = std::uint32_t;
  private:
	using Stored = std::conditional_t<std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>, T, std::optional<T>>;
	struct Node { Stored value; root_type next, jump; int size, jump_length; };
	std::vector<Node> nodes{{{}, 0, 0, 0, 0}};
	void check([[maybe_unused]] root_type root) const { assert(root < nodes.size()); }
	const T &value_at(root_type root) const {
		if constexpr (std::is_same_v<Stored, T>) return nodes[root].value;
		else return *nodes[root].value;
	}
  public:
	root_type initial_root() const { return 0; }
	int size(root_type root) const { check(root); return nodes[root].size; }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type push(root_type root, T value) {
		check(root);
		assert(size(root) < std::numeric_limits<int>::max() && nodes.size() < std::numeric_limits<root_type>::max());
		auto a = nodes[root].jump;
		int length = nodes[root].jump_length;
		bool combine = length == nodes[a].jump_length;
		auto jump = combine ? nodes[a].jump : root;
		Node node{std::move(value), root, jump, size(root) + 1, combine ? 2 * length + 1 : 1};
		auto result = static_cast<root_type>(nodes.size());
		nodes.push_back(std::move(node));
		return result;
	}
	root_type pop(root_type root) const { check(root); assert(root); return nodes[root].next; }
	T top(root_type root) const { check(root); assert(root); return value_at(root); }
	root_type ancestor(root_type root, int count) const {
		check(root); assert(0 <= count && count <= size(root));
		if (count == size(root)) return 0;
		while (count) {
			int length = nodes[root].jump_length;
			if (length <= count) count -= length, root = nodes[root].jump;
			else --count, root = nodes[root].next;
		}
		return root;
	}
	T get(root_type root, int k) const { assert(0 <= k && k < size(root)); return top(ancestor(root, k)); }
	std::vector<T> get_many(const std::vector<std::pair<root_type, int>> &queries) const {
		assert(queries.size() <= std::size_t(std::numeric_limits<int>::max()));
		if (queries.empty()) return {};
		std::uint64_t work = 0;
		unsigned limit = std::bit_width(nodes.size());
		for (auto [root, k] : queries) { check(root); assert(0 <= k && k < size(root)); work += unsigned(k) < limit ? unsigned(k) : limit; }
		if (work <= 3 * std::uint64_t(nodes.size())) {
			std::vector<T> result;
			result.reserve(queries.size());
			for (auto [root, k] : queries) result.push_back(get(root, k));
			return result;
		}
		std::vector<root_type> first(nodes.size()), next(nodes.size()), selected(queries.size()), path;
		std::vector<int> head(nodes.size(), -1), following(queries.size());
		for (root_type v = 1; v < nodes.size(); ++v) {
			root_type p = nodes[v].next;
			next[v] = first[p]; first[p] = v;
		}
		for (int i = 0; i < int(queries.size()); ++i) {
			root_type root = queries[i].first;
			following[i] = head[root]; head[root] = i;
		}
		for (root_type v = first[0]; v;) {
			path.push_back(v);
			for (int i = head[v]; i != -1; i = following[i]) selected[i] = path[path.size() - 1 - queries[i].second];
			if (first[v]) { v = first[v]; continue; }
			while (v && !next[v]) { path.pop_back(); v = nodes[v].next; }
			if (v) { path.pop_back(); v = next[v]; }
		}
		std::vector<T> result;
		result.reserve(queries.size());
		for (root_type v : selected) result.push_back(value_at(v));
		return result;
	}
	void reserve(std::size_t count) { assert(count < std::numeric_limits<root_type>::max()); nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
};


#line 9 "data_structure/persistent/persistent_queue.hpp"

template <class T>
class PersistentQueue {
	PersistentStack<T> stack;
  public:
	using value_type = T;
	struct root_type {
		typename PersistentStack<T>::root_type tail = 0;
		int length = 0;
		bool operator==(const root_type &) const = default;
	};
	root_type initial_root() const { return {}; }
	int size(root_type root) const { return root.length; }
	bool empty(root_type root) const { return size(root) == 0; }
	root_type push(root_type root, T value) { return {stack.push(root.tail, std::move(value)), root.length + 1}; }
	root_type pop(root_type root) const {
		assert(!empty(root));
		return root.length == 1 ? root_type{} : root_type{root.tail, root.length - 1};
	}
	T front(root_type root) const { assert(!empty(root)); return stack.get(root.tail, root.length - 1); }
	T back(root_type root) const { assert(!empty(root)); return stack.top(root.tail); }
	T get(root_type root, int p) const { assert(0 <= p && p < size(root)); return stack.get(root.tail, root.length - 1 - p); }
	std::vector<T> front_many(const std::vector<root_type> &roots) const {
		assert(roots.size() <= std::size_t(std::numeric_limits<int>::max()));
		std::uint64_t work = 0;
		unsigned limit = std::bit_width(stack.node_count());
		for (auto root : roots) { assert(!empty(root)); unsigned distance = unsigned(root.length - 1); work += distance < limit ? distance : limit; }
		if (work <= 3 * std::uint64_t(stack.node_count())) {
			std::vector<T> result;
			result.reserve(roots.size());
			for (auto root : roots) result.push_back(front(root));
			return result;
		}
		std::vector<std::pair<typename PersistentStack<T>::root_type, int>> queries;
		queries.reserve(roots.size());
		for (auto root : roots) { assert(!empty(root)); queries.emplace_back(root.tail, root.length - 1); }
		return stack.get_many(queries);
	}
	void reserve(std::size_t count) { stack.reserve(count); }
	std::size_t node_count() const { return stack.node_count(); }
	std::size_t memory_usage() const { return stack.memory_usage(); }
};


#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
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


#line 5 "test/persistent_queue_batch.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	PersistentQueue<int> queue;
	queue.reserve(q);
	using Root = decltype(queue)::root_type;
	std::vector<Root> roots(q + 1), queries;
	queries.reserve(q);
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		if (type == 0) { int x = 0; input.read(x); roots[i] = queue.push(roots[k + 1], x); }
		else { queries.push_back(roots[k + 1]); roots[i] = queue.pop(roots[k + 1]); }
	}
	for (int value : queue.front_many(queries)) output.println(value);
}
