#line 1 "test/enumerate_primes.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"
#line 1 "math/number_theory/prime_sieve.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	result.push_back(2);
	if (n <= 65537) {
		const std::size_t size = n / 2;
		std::vector<std::uint8_t> prime(size, 1);
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < size; j += p) prime[j] = 0;
		}
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= size; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (i + (std::countr_zero(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) result.push_back(2 * i + 1);
		return result;
	}
	result.push_back(3), result.push_back(5);
	constexpr std::array<unsigned, 8> residues{1, 7, 11, 13, 17, 19, 23, 29};
	constexpr auto index = [&] {
		std::array<unsigned, 30> index{};
		for (unsigned i = 0; i < 8; ++i) index[residues[i]] = i;
		return index;
	}();
	constexpr std::size_t period = 7 * 11 * 13, block_size = period * 32;
	static const auto pattern = [&] {
		std::array<std::uint8_t, period> pattern{};
		for (std::size_t i = 0; i < period; ++i)
			for (unsigned d = 0; d < 8; ++d) {
				const auto x = 30 * i + residues[d];
				pattern[i] |= std::uint8_t(bool(x % 7 && x % 11 && x % 13) << d);
			}
		return pattern;
	}();
	std::size_t root = std::sqrt(static_cast<long double>(n - 1));
	while (root > (n - 1) / root) --root;
	while (root + 1 <= (n - 1) / (root + 1)) ++root;
	const auto primes = prime_sieve(root + 1);
	struct State {
		std::size_t p;
		std::array<std::size_t, 8> next;
		std::array<std::uint8_t, 8> mask;
	};
	std::vector<State> state;
	state.reserve(primes.size());
	for (const auto p : primes) {
		if (p < 17) continue;
		State current;
		current.p = p;
		for (unsigned d = 0; d < 8; ++d) {
			const std::size_t k = p + (residues[d] + 30 - p % 30) % 30;
			const __uint128_t x = __uint128_t(p) * k;
			current.next[d] = x / 30;
			current.mask[d] = std::uint8_t(~(1u << index[x % 30]));
		}
		state.push_back(current);
	}
	const std::size_t size = (n - 1) / 30 + 1;
	std::vector<std::uint8_t> prime(block_size);
	constexpr auto offsets = [&] {
		std::array<unsigned, 64> offsets{};
		for (unsigned i = 0; i < 64; ++i) offsets[i] = 30 * (i / 8) + residues[i % 8];
		return offsets;
	}();
	for (std::size_t base = 0; base < size;) {
		const std::size_t length = std::min(block_size, size - base);
		for (std::size_t i = 0; i < length; i += period)
			std::copy_n(pattern.data(), std::min(period, length - i), prime.data() + i);
		if (base == 0) {
			prime[0] &= ~1u;
			prime[0] |= (1u << 1) | (1u << 2) | (1u << 3);
		}
		for (auto &current : state) {
			const auto p = current.p;
			for (unsigned d = 0; d < 8; ++d) {
				std::size_t j = current.next[d];
				const auto mask = current.mask[d];
				for (; j + 3 * p < length; j += 4 * p) {
					prime[j] &= mask;
					prime[j + p] &= mask;
					prime[j + 2 * p] &= mask;
					prime[j + 3 * p] &= mask;
				}
				for (; j < length; j += p) prime[j] &= mask;
				current.next[d] = j - length;
			}
		}
		if (base + length == size) {
			const auto remaining = n - 30 * (size - 1);
			for (unsigned d = 0; d < 8; ++d) if (residues[d] >= remaining) prime[length - 1] &= ~(1u << d);
		}
		std::fill(prime.begin() + length, prime.begin() + ((length + 7) / 8 * 8), 0);
		for (std::size_t i = 0; i < length; i += 8) {
			std::uint64_t word = 0;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			if constexpr (std::endian::native == std::endian::big) word = std::byteswap(word);
			else if constexpr (std::endian::native != std::endian::little) {
				word = 0;
				for (unsigned d = 0; d < 8; ++d) word |= std::uint64_t(prime[i + d]) << (8 * d);
			}
			while (word) {
				const auto x = 30 * (base + i) + offsets[std::countr_zero(word)];
				result.push_back(x);
				word &= word - 1;
			}
		}
		base += length;
	}
	return result;
}


#line 1 "components/fast_io.hpp"



#line 7 "components/fast_io.hpp"
#include <cassert>
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
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


#line 4 "test/enumerate_primes.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::size_t n = 0, a = 0, b = 0;
	input.read(n), input.read(a), input.read(b);
	const auto primes = prime_sieve(n + 1);
	const auto count = primes.size() > b ? (primes.size() - b - 1) / a + 1 : 0;
	output.write(primes.size()), output.write(' '), output.println(count);
	for (std::size_t i = b; i < primes.size(); i += a) {
		if (i != b) output.write(' ');
		output.write(primes[i]);
	}
	output.write('\n');
}
