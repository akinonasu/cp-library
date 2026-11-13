#line 1 "test/enumerate_primes.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"
#line 1 "math/prime_sieve.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <utility>
#include <type_traits>
#if defined(__BMI2__) && defined(__x86_64__)
#include <immintrin.h>
#endif
#include <vector>

namespace prime_sieve_detail {
struct Sample {
	std::size_t skip, stride;
	std::size_t &count;
	std::vector<std::size_t> &values;
	void operator()(std::size_t p) {
		if (skip) --skip;
		else values.push_back(p), skip = stride - 1;
		++count;
	}
	template<class At> void word(std::uint64_t word, At at) {
		unsigned remaining = std::popcount(word);
		count += remaining;
		if (skip >= remaining) { skip -= remaining; return; }
		do {
			const unsigned rank = unsigned(skip);
			if (rank) {
#if defined(__BMI2__) && defined(__x86_64__)
				word &= -_pdep_u64(std::uint64_t(1) << rank, word);
#else
				for (unsigned i = 0; i < rank; ++i) word &= word - 1;
#endif
			}
			const unsigned bit = std::countr_zero(word);
			values.push_back(at(bit));
			word &= word - 1;
			remaining -= rank + 1;
			skip = stride - 1;
		} while (skip < remaining);
		skip -= remaining;
	}
};
template<class F> inline constexpr bool sampled = std::is_same_v<F, Sample>;
}

inline std::vector<std::size_t> prime_sieve(std::size_t n);

template<class F> void for_each_prime(std::size_t n, F emit) {
	if (n <= 2) return;
	emit(std::size_t(2));
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
			if constexpr (prime_sieve_detail::sampled<F>) emit.word(word, [&](unsigned bit) { return 2 * (i + bit / 8) + 1; });
			else {
				while (word) {
					emit(2 * (i + (std::countr_zero(word) >> 3)) + 1);
					word &= word - 1;
				}
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) emit(2 * i + 1);
		return;
	}
	emit(3), emit(5);
	constexpr std::array<unsigned, 8> residues{1, 7, 11, 13, 17, 19, 23, 29};
	constexpr auto index = [&] {
		std::array<unsigned, 30> index{};
		for (unsigned i = 0; i < 8; ++i) index[residues[i]] = i;
		return index;
	}();
	constexpr std::size_t period = 7 * 11 * 13;
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
	const std::size_t block_size = (std::max(period * 32, root) + period - 1) / period * period;
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
	std::vector<std::uint8_t> prime((block_size + 7) / 8 * 8);
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
			if constexpr (prime_sieve_detail::sampled<F>) emit.word(word, [&](unsigned bit) { return 30 * (base + i) + offsets[bit]; });
			else {
				while (word) {
					const auto x = 30 * (base + i) + offsets[std::countr_zero(word)];
					emit(x);
					word &= word - 1;
				}
			}
		}
		base += length;
	}
}

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	for_each_prime(n, [&](std::size_t p) { result.push_back(p); });
	return result;
}

inline std::pair<std::size_t, std::vector<std::size_t>> prime_sieve_sample(std::size_t n, std::size_t stride, std::size_t offset = 0) {
	assert(stride > 0);
	std::pair<std::size_t, std::vector<std::size_t>> result;
	if (stride <= 4) {
		std::size_t skip = offset;
		for_each_prime(n, [&](std::size_t p) {
			if (skip) --skip;
			else result.second.push_back(p), skip = stride - 1;
			++result.first;
		});
		return result;
	}
	for_each_prime(n, prime_sieve_detail::Sample{offset, stride, result.first, result.second});
	return result;
}


#line 1 "components/fast_io_output_values.hpp"



#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
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


#line 5 "components/fast_io_output.hpp"
#include <limits>
#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "components/fast_io_output_values.hpp"

namespace fast_io_detail {
struct UnsignedValues {
	static char *padded8(char *out, std::uint32_t value) {
		char digits[16];
		BulkOutput::number(digits, value + 100000000);
		std::memcpy(out, digits + 1, 8);
		return out + 8;
	}
	static char *number(char *out, std::uint64_t value) {
		constexpr std::uint64_t block = 100000000;
		if (value >= block * block) {
			out = BulkOutput::small(out, std::uint32_t(value / (block * block)));
			value %= block * block;
			out = padded8(out, std::uint32_t(value / block));
			return padded8(out, std::uint32_t(value % block));
		}
		if (value >= block) {
			out = BulkOutput::number(out, std::uint32_t(value / block));
			return padded8(out, std::uint32_t(value % block));
		}
		return BulkOutput::number(out, std::uint32_t(value));
	}
	template<class UInt, class Read> struct Block {
		std::size_t base, count;
		Read &read;
		char separator, ending;
		std::size_t decimal_capacity() const { return count * (sizeof(UInt) <= 4 ? 11 : 21); }
		char *to_chars(char *out) const {
			char *p = out;
			for (std::size_t i = 0; i < count; ++i) {
				if constexpr (sizeof(UInt) <= 4) p = BulkOutput::number(p, std::uint32_t(read(base + i)));
				else p = number(p, std::uint64_t(read(base + i)));
				*p++ = separator;
			}
			p[-1] = ending;
			return p;
		}
	};
};
}

template<class UInt = std::uint64_t, class Read>
void write_unsigned_values(FastOutput &output, std::size_t count, Read read, char separator = ' ', char ending = '\n') {
	static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && sizeof(UInt) <= 8 && !std::is_same_v<UInt, bool>);
	constexpr std::size_t block = (1 << 16) / (sizeof(UInt) <= 4 ? 11 : 21);
	for (std::size_t base = 0; base < count;) {
		const auto n = std::min(count - base, block);
		output.write(fast_io_detail::UnsignedValues::Block<UInt, Read>{base, n, read, separator, base + n == count ? ending : separator});
		base += n;
	}
}


#line 4 "test/enumerate_primes.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::size_t n = 0, a = 0, b = 0;
	input.read(n), input.read(a), input.read(b);
	auto [count, selected] = prime_sieve_sample(n + 1, a, b);
	output.write(count), output.write(' '), output.println(selected.size());
	if (selected.empty()) output.write('\n');
	else write_unsigned_values(output, selected.size(), [&](std::size_t i) { return selected[i]; });
}
