#line 1 "test/flat_hash_map.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#line 1 "data_structure/flat_hash_map.hpp"



#include <algorithm>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flat_hash_map_detail {
template<class K, class V, class Hash, class Equal>
inline constexpr bool packed = std::is_integral_v<K> && !std::is_same_v<K, bool> && sizeof(K) <= 8 &&
	std::is_same_v<Hash, std::hash<K>> && std::is_same_v<Equal, std::equal_to<K>> &&
	std::is_trivially_copyable_v<V> && std::is_nothrow_default_constructible_v<V> &&
	std::is_nothrow_copy_constructible_v<V> && std::is_nothrow_copy_assignable_v<V>;

template<class K, class V, class Hash, class Equal, bool = packed<K, V, Hash, Equal>>
class Table {
	std::unordered_map<K, V, Hash, Equal> values;
public:
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) : Table() { swap(other); }
	Table &operator=(Table other) { swap(other); return *this; }
	void swap(Table &other) noexcept(noexcept(values.swap(other.values))) { values.swap(other.values); }
	std::size_t size() const { return values.size(); }
	bool empty() const { return values.empty(); }
	V *find(const K &key) { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	const V *find(const K &key) const { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	V &operator[](const K &key) { return values[key]; }
	void set(const K &key, const V &value) { values.insert_or_assign(key, value); }
	bool erase(const K &key) { return values.erase(key); }
	void reserve(std::size_t n) { values.reserve(n); }
	void clear() { values.clear(); }
};

template<class K, class V, class Hash, class Equal>
class Table<K, V, Hash, Equal, true> {
	std::vector<K> keys;
	std::vector<V> values;
	std::vector<unsigned char> states;
	std::size_t count = 0, deleted = 0;
	unsigned shift = 64;
	std::uint64_t salt = std::chrono::steady_clock::now().time_since_epoch().count();
	std::size_t index(K key) const {
		return ((static_cast<std::uint64_t>(key) ^ salt) * UINT64_C(11400714819323198485)) >> shift;
	}
	std::size_t locate(K key) const {
		std::size_t p = index(key), mask = states.size() - 1;
		while (states[p] && (states[p] != 1 || keys[p] != key)) p = (p + 1) & mask;
		return p;
	}
	void rehash(std::size_t capacity) {
		Table next;
		next.salt = salt;
		next.keys.resize(capacity), next.values.resize(capacity), next.states.resize(capacity);
		next.shift = 64 - std::countr_zero(capacity);
		for (std::size_t i = 0; i < states.size(); ++i) if (states[i] == 1) {
			std::size_t p = next.locate(keys[i]);
			next.keys[p] = keys[i], next.values[p] = values[i], next.states[p] = 1;
		}
		next.count = count;
		swap(next);
	}
	void prepare() {
		if (states.empty()) rehash(16);
		else if (count + deleted == states.size() / 2) {
			if (deleted > count / 2) rehash(states.size());
			else {
				if (states.size() > states.max_size() / 2 || states.size() > keys.max_size() / 2 || states.size() > values.max_size() / 2)
					throw std::length_error("FlatHashMap capacity");
				rehash(states.size() * 2);
			}
		}
	}
	std::size_t insert(K key) {
		prepare();
		std::size_t p = index(key), first = states.size(), mask = states.size() - 1;
		while (states[p]) {
			if (states[p] == 1 && keys[p] == key) return p;
			if (states[p] == 2 && first == states.size()) first = p;
			p = (p + 1) & mask;
		}
		if (first != states.size()) p = first, --deleted;
		keys[p] = key, values[p] = V{}, states[p] = 1;
		++count;
		return p;
	}
public:
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) noexcept { swap(other); }
	Table &operator=(Table other) noexcept { swap(other); return *this; }
	void swap(Table &other) noexcept {
		keys.swap(other.keys), values.swap(other.values), states.swap(other.states);
		std::swap(count, other.count), std::swap(deleted, other.deleted), std::swap(shift, other.shift), std::swap(salt, other.salt);
	}
	std::size_t size() const { return count; }
	bool empty() const { return !count; }
	V *find(K key) { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] ? &values[p] : nullptr; }
	const V *find(K key) const { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] ? &values[p] : nullptr; }
	V &operator[](K key) { return values[insert(key)]; }
	void set(K key, const V &value) { V copy = value; values[insert(key)] = copy; }
	bool erase(K key) {
		if (states.empty()) return false;
		std::size_t p = locate(key);
		if (!states[p]) return false;
		states[p] = 2, --count, ++deleted;
		return true;
	}
	void reserve(std::size_t n) {
		if (n <= states.size() / 2) return;
		std::size_t limit = std::min({states.max_size(), keys.max_size(), values.max_size()});
		if (n > limit / 2) throw std::length_error("FlatHashMap capacity");
		std::size_t capacity = std::bit_ceil(std::max(std::size_t(16), 2 * n));
		if (capacity > limit) throw std::length_error("FlatHashMap capacity");
		rehash(capacity);
	}
	void clear() { std::fill(states.begin(), states.end(), 0); count = deleted = 0; }
};
}

template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class FlatHashMap {
	flat_hash_map_detail::Table<Key, T, Hash, KeyEqual> table;
public:
	using key_type = Key;
	using mapped_type = T;
	using size_type = std::size_t;
	FlatHashMap() = default;
	explicit FlatHashMap(size_type n) { reserve(n); }
	bool empty() const { return table.empty(); }
	size_type size() const { return table.size(); }
	T *find(const Key &key) { return table.find(key); }
	const T *find(const Key &key) const { return table.find(key); }
	bool contains(const Key &key) const { return find(key) != nullptr; }
	size_type count(const Key &key) const { return contains(key); }
	std::optional<T> get(const Key &key) const { const T *p = find(key); return p ? std::optional<T>(*p) : std::nullopt; }
	T &at(const Key &key) { T *p = find(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	const T &at(const Key &key) const { const T *p = find(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	T &operator[](const Key &key) { return table[key]; }
	void set(const Key &key, const T &value) { table.set(key, value); }
	bool erase(const Key &key) { return table.erase(key); }
	void reserve(size_type n) { table.reserve(n); }
	void clear() { table.clear(); }
	void swap(FlatHashMap &other) noexcept(noexcept(table.swap(other.table))) { table.swap(other.table); }
	friend void swap(FlatHashMap &a, FlatHashMap &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 7 "components/fast_io.hpp"
#include <cassert>
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 15 "components/fast_io.hpp"
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


#line 5 "test/flat_hash_map.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	FlatHashMap<std::uint64_t, std::uint64_t> values;
	values.reserve(q);
	while (q--) {
		int type = 0;
		std::uint64_t key = 0;
		input.read(type), input.read(key);
		if (type == 0) {
			std::uint64_t value = 0;
			input.read(value);
			values.set(key, value);
		} else {
			const auto *value = values.find(key);
			output.println(value ? *value : 0);
		}
	}
}
