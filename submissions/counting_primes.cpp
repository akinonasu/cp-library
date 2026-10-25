#line 1 "test/prime_count.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/counting_primes"
#line 1 "math/prime_count.hpp"



#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace prime_count_detail {
template<bool FastDivide> std::uint64_t compute(std::uint64_t n) {
	if (n < 2) return 0;
	std::uint64_t root = std::sqrt(static_cast<long double>(n));
	while (__uint128_t(root) * root > n) --root;
	while (__uint128_t(root + 1) * (root + 1) <= n) ++root;
	std::size_t size = (root + 1) / 2;
	std::vector<std::uint32_t> small(size), rough(size);
	std::vector<std::uint64_t> large(size), quotient(size);
	std::vector<std::uint8_t> removed(root + 1);
	for (std::size_t i = 0; i < size; ++i) {
		small[i] = i;
		rough[i] = 2 * i + 1;
		if constexpr (FastDivide) quotient[i] = double(n) / rough[i];
		else quotient[i] = n / rough[i];
		large[i] = (quotient[i] - 1) / 2;
	}
	std::uint32_t count = 0;
	for (std::uint64_t p = 3; p * p <= root; p += 2) {
		if (removed[p]) continue;
		removed[p] = 1;
		for (std::uint64_t i = p * p; i <= root; i += 2 * p) removed[i] = 1;
		const std::uint64_t reciprocal = UINT64_MAX / p + FastDivide;
		auto divide = [=](std::uint64_t x) {
			std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
			if constexpr (FastDivide) return q;
			else return q + (x - q * p >= p);
		};
		std::size_t next = 0, k = 0;
		for (; k < size && std::uint64_t(rough[k]) * p <= root; ++k) {
			const std::uint32_t r = rough[k];
			if (removed[r]) continue;
			large[next] = large[k] - large[small[r * p / 2] - count] + count;
			quotient[next] = quotient[k];
			rough[next++] = r;
		}
		for (; k < size; ++k) {
			const std::uint32_t r = rough[k];
			if (removed[r]) continue;
			large[next] = large[k] - small[(divide(quotient[k]) - 1) / 2] + count;
			quotient[next] = quotient[k];
			rough[next++] = r;
		}
		size = next;
		std::size_t i = (root - 1) / 2;
		for (std::uint64_t j = (root / p - 1) | 1; j >= p; j -= 2) {
			const std::uint32_t delta = small[j / 2] - count;
			const std::size_t end = j * p / 2;
			for (; i >= end; --i) small[i] -= delta;
		}
		++count;
	}
	__int128_t result = 1 + __int128_t(large[0]) + __int128_t(size) * (size - 1) / 2;
	result += (__int128_t(count) - 1) * (size - 1);
	for (std::size_t k = 1; k < size; ++k) result -= large[k];
	for (std::size_t k = 1; k < size; ++k) {
		const std::uint64_t p = rough[k], reciprocal = UINT64_MAX / p + FastDivide;
		auto divide = [=](std::uint64_t x) {
			std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
			if constexpr (FastDivide) return q;
			else return q + (x - q * p >= p);
		};
		const std::size_t end = small[(divide(quotient[k]) - 1) / 2] - count;
		if (end <= k) break;
		for (std::size_t j = k + 1; j <= end; ++j)
			result += small[(divide(quotient[j]) - 1) / 2];
		result -= __int128_t(end - k) * (count + k - 1);
	}
	return static_cast<std::uint64_t>(result);
}
}

inline std::uint64_t prime_count(std::uint64_t n) {
	if constexpr (std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53)
		if (n <= (std::uint64_t(1) << 42)) return prime_count_detail::compute<true>(n);
	return prime_count_detail::compute<false>(n);
}


#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
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


#line 4 "test/prime_count.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint64_t n = 0;
	input.read(n);
	output.println(prime_count(n));
}
