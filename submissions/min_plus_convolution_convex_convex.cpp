#line 1 "test/min_plus_convex_convex.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex"
#line 1 "convolution/min_plus_convolution.hpp"



#line 1 "components/smawk.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <vector>

template <class Better>
std::vector<int> smawk(int length, const std::vector<int> &columns, Better better) {
	assert(length >= 0 && (!length || !columns.empty()));
	if (!length) return {};
	std::vector<int> best(length);
	auto solve = [&](auto &&self, std::size_t first, std::size_t stride, const std::vector<int> &cols) -> void {
		int rows = (length - 1 - first) / stride + 1;
		std::vector<int> reduced;
		reduced.reserve(std::min<std::size_t>(rows, cols.size()));
		for (int col : cols) {
			while (!reduced.empty() && better(first + (int(reduced.size()) - 1) * stride, reduced.back(), col)) reduced.pop_back();
			if (int(reduced.size()) < rows) reduced.push_back(col);
		}
		if (rows > 1) self(self, first + stride, stride * 2, reduced);
		std::size_t left = 0;
		for (std::size_t row = first; row < std::size_t(length); row += 2 * stride) {
			std::size_t right = left;
			if (row + stride < std::size_t(length))
				while (reduced[right] != best[row + stride]) ++right;
			else right = reduced.size() - 1;
			best[row] = reduced[left];
			for (std::size_t i = left + 1; i <= right; ++i)
				if (better(row, best[row], reduced[i])) best[row] = reduced[i];
			left = right;
		}
	};
	solve(solve, 0, 1, columns);
	return best;
}

template <class Better>
std::vector<int> smawk(int rows, int columns, Better better) {
	assert(columns >= 0);
	std::vector<int> cols(columns);
	std::iota(cols.begin(), cols.end(), 0);
	return smawk(rows, cols, better);
}


#line 8 "convolution/min_plus_convolution.hpp"
#include <cstdint>
#include <functional>
#include <limits>
#line 13 "convolution/min_plus_convolution.hpp"

namespace min_plus_convolution_detail {

template <class T, class Compare>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = a.size(), m = b.size(), length = n + m - 1;
	std::vector<int> columns(m);
	std::iota(columns.begin(), columns.end(), 0);
	auto better = [&](int row, int x, int y) {
		if (row < y) return false;
		if (row - x >= n) return true;
		return compare(a[row - y] + b[y], a[row - x] + b[x]);
	};
	auto best = smawk(length, columns, better);
	std::vector<T> result;
	result.reserve(length);
	for (int i = 0; i < length; ++i) result.push_back(a[i - best[i]] + b[best[i]]);
	return result;
}
template <class T, class Compare>
std::vector<T> opposite(const std::vector<T> &a, const std::vector<T> &b, Compare compare) {
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() <= std::size_t(std::numeric_limits<int>::max()));
	int n = a.size(), m = b.size(), length = n + m - 1;
	std::vector<T> result(length, a[0] + b[0]);
	std::vector<std::uint8_t> done(length);
	auto solve = [&](auto &&self, int l, int r, int parent_l, int parent_r) -> void {
		int low = std::max(0, r - n), high = std::min(m, l + 1);
		std::vector<int> columns;
		for (int i = high - 1; i >= low;) {
			if (parent_l <= i && i < parent_r) i = parent_l - 1;
			else columns.push_back(i--);
		}
		if (!columns.empty()) {
			auto best = smawk(r - l, columns, [&](int row, int x, int y) {
				return compare(a[l + row - y] + b[y], a[l + row - x] + b[x]);
			});
			for (int i = l; i < r; ++i) {
				T value = a[i - best[i - l]] + b[best[i - l]];
				if (!done[i] || compare(value, result[i])) result[i] = value;
				done[i] = 1;
			}
		}
		if (r - l > 1) {
			int mid = l + (r - l) / 2;
			self(self, l, mid, low, high);
			self(self, mid, r, low, high);
		}
	};
	solve(solve, 0, length, 0, 0);
	return result;
}

}

template <class T>
std::vector<T> min_plus_convolution_convex(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::multiply(a, b, std::less<T>{});
}
template <class T>
std::vector<T> max_plus_convolution_concave(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::multiply(a, b, std::greater<T>{});
}

template <class T>
std::vector<T> min_plus_convolution_concave(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::opposite(a, b, std::less<T>{});
}
template <class T>
std::vector<T> max_plus_convolution_convex(const std::vector<T> &a, const std::vector<T> &b) {
	return min_plus_convolution_detail::opposite(a, b, std::greater<T>{});
}


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


#line 4 "test/min_plus_convex_convex.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	std::vector<std::int64_t> a(n), b(m);
	for (auto &x : a) input.read(x);
	for (auto &x : b) input.read(x);
	const auto result = min_plus_convolution_convex(a, b);
	for (std::size_t i = 0; i < result.size(); ++i) {
		if (i) output.write(' ');
		output.write(result[i]);
	}
	output.write('\n');
}
