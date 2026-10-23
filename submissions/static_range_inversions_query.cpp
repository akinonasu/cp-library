#line 1 "test/mo_inversions.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/static_range_inversions_query"
#line 1 "data_structure/range_query/mo_order.hpp"



#line 1 "components/radix_sort.hpp"



#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

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


#line 6 "data_structure/range_query/mo_order.hpp"
#include <cassert>
#include <cmath>
#line 9 "data_structure/range_query/mo_order.hpp"
#include <limits>
#line 12 "data_structure/range_query/mo_order.hpp"

inline std::vector<int> mo_order(const std::vector<std::pair<int, int>>& ranges, int block_size = 0) {
	assert(ranges.size() <= std::size_t(std::numeric_limits<int>::max()) && block_size >= 0);
	if (ranges.empty()) return {};
	int min_l = ranges[0].first, max_l = min_l, min_r = ranges[0].second, max_r = min_r;
	for (auto [l, r] : ranges) {
		assert(0 <= l && l <= r);
		min_l = std::min(min_l, l), max_l = std::max(max_l, l);
		min_r = std::min(min_r, r), max_r = std::max(max_r, r);
	}
	if (!block_size) block_size = std::max(1, int(std::sqrt((double(max_l) - min_l + 1) * (double(max_r) - min_r + 1) / ranges.size())));
	std::vector<std::pair<std::uint64_t, int>> keys;
	keys.reserve(ranges.size());
	for (int i = 0; i < int(ranges.size()); ++i) {
		auto [l, r] = ranges[i];
		std::uint32_t block = (l - min_l) / block_size;
		std::uint32_t right = block & 1 ? ~std::uint32_t(r) : std::uint32_t(r);
		keys.emplace_back((std::uint64_t(block) << 32) | right, i);
	}
	if (keys.size() < 2048) std::sort(keys.begin(), keys.end());
	else stable_radix_sort(keys, [](const auto &x) { return x.first; });
	std::vector<int> order;
	order.reserve(keys.size());
	for (auto [key, i] : keys) { (void)key; order.push_back(i); }
	return order;
}


#line 1 "data_structure/fenwick_tree.hpp"



#line 9 "data_structure/fenwick_tree.hpp"

namespace fenwick_tree_detail {
template<class T, bool = std::is_integral_v<T>> struct Storage { using type = T; };
template<class T> struct Storage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct Storage<__int128_t, false> { using type = __uint128_t; };
#endif
}

template<class T, bool Padded = true> class FenwickTree {
	static_assert(!std::is_same_v<T, bool>);
	using U = typename fenwick_tree_detail::Storage<T>::type;
	std::vector<U> data_;
	int n_ = 0;
	static std::size_t index(unsigned i) {
		if constexpr (Padded) return std::size_t(i) + (i >> 10);
		else return i;
	}

  public:
	explicit FenwickTree(int n = 0) : n_(n) {
		assert(n >= 0);
		data_.resize(index(unsigned(n)) + 1);
	}
	explicit FenwickTree(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n_ = static_cast<int>(a.size());
		data_.resize(index(unsigned(n_)) + 1);
		for (unsigned i = 1; i <= unsigned(n_); ++i) {
			data_[index(i)] += U(a[i - 1]);
			unsigned p = i + (i & -i);
			if (p <= unsigned(n_)) data_[index(p)] += data_[index(i)];
		}
	}
	int size() const { return n_; }
	void add(int p, T x) {
		assert(0 <= p && p < n_);
		for (unsigned i = unsigned(p) + 1; i <= unsigned(n_); i += i & -i) data_[index(i)] += U(x);
	}
	T sum(int r) const {
		assert(0 <= r && r <= n_);
		U result{};
		for (unsigned i = unsigned(r); i; i &= i - 1) result += data_[index(i)];
		return T(result);
	}
	T sum(int l, int r) const {
		assert(0 <= l && l <= r && r <= n_);
		U result{};
		unsigned a = unsigned(l), b = unsigned(r);
		while (b > a) result += data_[index(b)], b &= b - 1;
		while (a > b) result -= data_[index(a)], a &= a - 1;
		return T(result);
	}
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


#line 6 "test/mo_inversions.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> a(n);
	for (auto &x : a) input.read(x);
	auto values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	for (auto &x : a) x = std::lower_bound(values.begin(), values.end(), x) - values.begin();
	std::vector<std::pair<int, int>> ranges(q);
	for (auto &[l, r] : ranges) input.read(l), input.read(r);
	std::vector<std::int64_t> answer(q);
	FenwickTree<int> counts(values.size());
	int l = 0, r = 0;
	std::int64_t inversions = 0;
	for (int i : mo_order(ranges)) {
		const auto [left, right] = ranges[i];
		while (l > left) { const int x = a[--l]; inversions += counts.sum(x); counts.add(x, 1); }
		while (r < right) { const int x = a[r]; inversions += r - l - counts.sum(x + 1); counts.add(x, 1); ++r; }
		while (l < left) { const int x = a[l++]; counts.add(x, -1); inversions -= counts.sum(x); }
		while (r > right) { const int x = a[--r]; counts.add(x, -1); inversions -= r - l - counts.sum(x + 1); }
		answer[i] = inversions;
	}
	for (auto x : answer) output.println(x);
}
