#line 1 "test/interval_map.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#line 1 "data_structure/interval_map.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class V = std::int64_t, class Size = std::conditional_t<
	std::is_integral_v<T> && sizeof(T) <= 8, std::uint64_t,
	std::conditional_t<std::is_same_v<T, __int128_t>, __uint128_t, T>>, class Equal = std::equal_to<V>>
class IntervalMap {
	static_assert(std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>);
	static_assert(std::numeric_limits<Size>::is_integer && !std::is_same_v<Size, bool>);

public:
	using coordinate_type = T;
	using mapped_type = V;
	using size_type = Size;
	using value_equal = Equal;
	struct interval_type { T left, right; V value; };
	struct segment_type { T right; V value; };
	using const_iterator = typename std::map<T, segment_type>::const_iterator;
	using const_reverse_iterator = typename std::map<T, segment_type>::const_reverse_iterator;

private:
	std::map<T, segment_type> ranges;
	Size total = 0;
	[[no_unique_address]] Equal equal;
	static Size length(const T &l, const T &r) { return static_cast<Size>(r) - static_cast<Size>(l); }
	auto split(const T &x) {
		auto it = ranges.lower_bound(x);
		if (it != ranges.end() && it->first == x) return it;
		if (it == ranges.begin()) return it;
		auto previous = std::prev(it);
		if (previous->second.right <= x) return it;
		auto next = ranges.emplace_hint(it, x, segment_type{previous->second.right, previous->second.value});
		previous->second.right = x;
		return next;
	}

public:
	explicit IntervalMap(Equal compare = Equal{}) : equal(std::move(compare)) {}
	IntervalMap(T l, T r, const V &value, Equal compare = Equal{}) : IntervalMap(std::move(compare)) { assign(l, r, value); }
	explicit IntervalMap(std::vector<interval_type> intervals, Equal compare = Equal{}) : IntervalMap(std::move(compare)) { assign(std::move(intervals)); }
	IntervalMap(std::initializer_list<interval_type> intervals, Equal compare = Equal{}) : IntervalMap(std::vector<interval_type>(intervals), std::move(compare)) {}
	IntervalMap(const IntervalMap &) = default;
	IntervalMap(IntervalMap &&other) noexcept(std::is_nothrow_copy_constructible_v<Equal> && std::is_nothrow_constructible_v<Size, int> && std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>)
		: equal(other.equal) { swap(other); }
	IntervalMap &operator=(IntervalMap other) noexcept(std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>) { swap(other); return *this; }

	void assign(T l, T r, const V &value) {
		assert(l <= r);
		if (l == r) return;
		auto existing = find(l);
		if (existing != end() && r <= existing->second.right && equal(existing->second.value, value)) return;
		V stored(value);
		auto last = split(r), first = split(l);
		for (auto it = first; it != last; ++it) total -= length(it->first, it->second.right);
		ranges.erase(first, last);
		auto it = ranges.emplace_hint(last, l, segment_type{r, std::move(stored)});
		total += length(l, r);
		if (it != ranges.begin()) {
			auto previous = std::prev(it);
			if (previous->second.right == l && equal(previous->second.value, it->second.value)) {
				previous->second.right = r;
				ranges.erase(it);
				it = previous;
			}
		}
		auto next = std::next(it);
		if (next != ranges.end() && it->second.right == next->first && equal(it->second.value, next->second.value)) {
			it->second.right = next->second.right;
			ranges.erase(next);
		}
	}
	void assign(std::vector<interval_type> intervals) {
		IntervalMap next(equal);
		for (const auto &[l, r, value] : intervals) next.assign(l, r, value);
		swap(next);
	}
	void set(T x, const V &value) {
		if constexpr (std::numeric_limits<T>::is_bounded) assert(x < (std::numeric_limits<T>::max)());
		assign(x, static_cast<T>(x + 1), value);
	}
	size_type erase(T l, T r) {
		assert(l <= r);
		if (l == r) return 0;
		auto last = split(r), first = split(l);
		size_type removed = 0;
		for (auto it = first; it != last; ++it) removed += length(it->first, it->second.right);
		ranges.erase(first, last);
		total -= removed;
		return removed;
	}
	size_type erase(T x) {
		if constexpr (std::numeric_limits<T>::is_bounded) if (x == (std::numeric_limits<T>::max)()) return 0;
		return erase(x, static_cast<T>(x + 1));
	}
	const_iterator find(T x) const {
		auto it = ranges.upper_bound(x);
		if (it == ranges.begin()) return ranges.end();
		--it;
		return x < it->second.right ? it : ranges.end();
	}
	const_iterator lower_bound(T x) const {
		auto it = ranges.upper_bound(x);
		if (it != ranges.begin() && x < std::prev(it)->second.right) --it;
		return it;
	}
	std::optional<V> get(T x) const {
		auto it = find(x);
		return it == end() ? std::nullopt : std::optional<V>(it->second.value);
	}
	const V &at(T x) const { auto it = find(x); assert(it != end()); return it->second.value; }
	bool contains(T x) const { return find(x) != end(); }
	bool contains(T l, T r) const {
		assert(l <= r);
		if (l == r) return true;
		auto it = find(l);
		if (it == end()) return false;
		T right = it->second.right;
		while (right < r) {
			++it;
			if (it == end() || right != it->first) return false;
			right = it->second.right;
		}
		return true;
	}
	bool contains(T l, T r, const V &value) const {
		assert(l <= r);
		if (l == r) return true;
		auto it = find(l);
		return it != end() && r <= it->second.right && equal(it->second.value, value);
	}
	bool intersects(T l, T r) const {
		assert(l <= r);
		if (l == r) return false;
		auto it = lower_bound(l);
		return it != end() && it->first < r;
	}
	template<class F> void for_each(T l, T r, F &&visit) const {
		assert(l <= r);
		if (l == r) return;
		for (auto it = lower_bound(l); it != end() && it->first < r; ++it)
			std::invoke(visit, (std::max)(l, it->first), (std::min)(r, it->second.right), it->second.value);
	}
	size_type count(T l, T r) const {
		size_type result = 0;
		for_each(l, r, [&](const T &a, const T &b, const V &) { result += length(a, b); });
		return result;
	}
	size_type size() const { return total; }
	std::size_t interval_count() const { return ranges.size(); }
	bool empty() const { return ranges.empty(); }
	const_iterator begin() const { return ranges.begin(); }
	const_iterator end() const { return ranges.end(); }
	const_reverse_iterator rbegin() const { return ranges.rbegin(); }
	const_reverse_iterator rend() const { return ranges.rend(); }
	void clear() { ranges.clear(); total = 0; }
	void swap(IntervalMap &other) noexcept(std::is_nothrow_swappable_v<Size> && std::is_nothrow_swappable_v<Equal>) {
		ranges.swap(other.ranges);
		using std::swap;
		swap(total, other.total); swap(equal, other.equal);
	}
	friend void swap(IntervalMap &a, IntervalMap &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
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


#line 4 "test/interval_map.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	IntervalMap<std::uint64_t, std::uint64_t> values;
	while (q--) {
		int type = 0;
		std::uint64_t key = 0;
		input.read(type), input.read(key);
		if (type == 0) {
			std::uint64_t value = 0;
			input.read(value);
			values.set(key, value);
		} else output.println(values.get(key).value_or(0));
	}
}
