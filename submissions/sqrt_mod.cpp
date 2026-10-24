#line 1 "test/mod_sqrt.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_mod"
#line 1 "math/number_theory/mod_sqrt.hpp"



#line 1 "components/montgomery.hpp"



#include <cassert>
#include <cstdint>

struct Montgomery64 {
	using U = std::uint64_t;
	using W = __uint128_t;
	U mod, inverse = 1, r2, one;
	explicit Montgomery64(U n) : mod(n), r2(-W(n) % n) {
		for (int repeat_index = 0; repeat_index < 6; ++repeat_index) inverse *= 2 - n * inverse;
		assert(n > 1 && (n & 1));
		one = reduce(r2);
	}
	U reduce(W x) const {
		U a = x >> 64, b = (W(U(x) * inverse) * mod) >> 64;
		return a < b ? a - b + mod : a - b;
	}
	U in(U x) const { return reduce(W(x) * r2); }
	U mul(U a, U b) const { return reduce(W(a) * b); }
	U add(U a, U b) const {
		U c = a + b;
		return c < a || c >= mod ? c - mod : c;
	}
	U pow(U a, U e) const {
		U r = one;
		for (; e; e >>= 1, a = mul(a, a))
			if (e & 1) r = mul(r, a);
		return r;
	}
};


#line 1 "components/montgomery32.hpp"



#line 6 "components/montgomery32.hpp"

struct Montgomery32 {
	using U = std::uint32_t;
	using W = std::uint64_t;
	U mod, inverse = 1, r2, one;

	explicit Montgomery32(U n) : mod(n), r2(-W(n) % n) {
		for (int i = 0; i < 5; ++i) inverse *= 2 - n * inverse;
		assert(n > 1 && (n & 1));
		one = reduce(r2);
	}
	U reduce(W x) const {
		U a = x >> 32, b = (W(U(x) * inverse) * mod) >> 32;
		return a < b ? a - b + mod : a - b;
	}
	U in(U x) const { return reduce(W(x) * r2); }
	U mul(U a, U b) const { return reduce(W(a) * b); }
	U add(U a, U b) const {
		U c = a + b;
		return c < a || c >= mod ? c - mod : c;
	}
	U pow(U a, U e) const {
		U r = one;
		if (mod < (U(1) << 30)) {
			U inverse = -this->inverse;
			auto product = [&](U a, U b) {
				W x = W(a) * b;
				return U((x + W(U(x) * inverse) * mod) >> 32);
			};
			for (; e; e >>= 1, a = product(a, a)) if (e & 1) r = product(r, a);
			return r < mod ? r : r - mod;
		}
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};


#line 1 "components/splitmix64.hpp"



#line 5 "components/splitmix64.hpp"

inline std::uint64_t splitmix64(std::uint64_t x) {
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
	return x ^ (x >> 31);
}
class SplitMix64 {
	std::uint64_t state;

  public:
	explicit SplitMix64(std::uint64_t seed = 0) : state(seed) {}
	std::uint64_t operator()() { return splitmix64(state += 0x9e3779b97f4a7c15ULL); }
};


#line 7 "math/number_theory/mod_sqrt.hpp"
#include <algorithm>
#include <array>
#include <bit>
#line 12 "math/number_theory/mod_sqrt.hpp"
#include <limits>
#include <optional>

namespace mod_sqrt_detail {
template <class M>
std::optional<std::uint64_t> sqrt(std::uint64_t a, const M &m) {
	using U = typename M::U;
	U p = m.mod, value = m.in(U(a)), root;
	if ((p & 3) == 3) {
		root = m.pow(value, (p >> 2) + 1);
		if (m.mul(root, root) != value) return std::nullopt;
	} else {
		int s = std::countr_zero(U(p - 1));
		U q = (p - 1) >> s;
		U t = m.pow(value, q >> 1);
		root = m.mul(value, t);
		U b = m.mul(root, t), v = b;
		int k = 0;
		while (v != m.one && k < s) v = m.mul(v, v), ++k;
		if (k == s) return std::nullopt;
		if (k) {
			static thread_local std::array<U, std::numeric_limits<U>::digits> powers;
			static thread_local U cached_modulus = 0;
			if (cached_modulus != p) {
				SplitMix64 random(a ^ p ^ 0x9e3779b97f4a7c15ULL);
				U nonresidue = 0;
				if ((p & 7) == 5) nonresidue = 2;
				else if (p % 3 == 2) nonresidue = 3;
				else if (p % 5 == 2 || p % 5 == 3) nonresidue = 5;
				else if (p % 7 == 3 || p % 7 == 5 || p % 7 == 6) nonresidue = 7;
				for (;;) {
					U z = nonresidue ? nonresidue : U(random() % (p - 2) + 2);
					powers[0] = m.pow(m.in(z), q);
					for (int i = 1; i < s; ++i) powers[i] = m.mul(powers[i - 1], powers[i - 1]);
					if (powers[s - 1] == p - m.one) break;
				}
				cached_modulus = p;
			}
			if (k >= 24) {
				std::array<U, std::numeric_limits<U>::digits> inverse;
				inverse[s - 1] = powers[s - 1];
				for (int i = s - 1; i--;) inverse[i] = m.mul(powers[i], inverse[i + 1]);
				auto logarithm = [&](auto &&self, U value, int offset, int bits) -> std::uint64_t {
					if (value == m.one) return 0;
					if (bits <= 4) {
						std::uint64_t result = 0;
						for (int i = 0; i < bits; ++i) {
							U high = value;
							for (int j = i + 1; j < bits; ++j) high = m.mul(high, high);
							if (high != m.one) result |= std::uint64_t(1) << i, value = m.mul(value, inverse[offset + i]);
						}
						return result;
					}
					int half = bits / 2;
					U high = value;
					for (int i = half; i < bits; ++i) high = m.mul(high, high);
					std::uint64_t low = self(self, high, offset + bits - half, half);
					for (auto mask = low; mask; mask &= mask - 1) value = m.mul(value, inverse[offset + std::countr_zero(mask)]);
					return low | (self(self, value, offset + half, bits - half) << half);
				};
				auto exponent = logarithm(logarithm, b, s - k, k);
				for (; exponent; exponent &= exponent - 1) root = m.mul(root, inverse[s - k - 1 + std::countr_zero(exponent)]);
			} else {
				while (k) {
					root = m.mul(root, powers[s - k - 1]);
					b = m.mul(b, powers[s - k]);
					int limit = k;
					v = b; k = 0;
					while (v != m.one && k < limit) v = m.mul(v, v), ++k;
					assert(k < limit);
				}
			}
		}
	}
	std::uint64_t result = m.reduce(root);
	return std::min(result, m.mod - result);
}
}

inline std::optional<std::uint64_t> mod_sqrt(std::uint64_t a, std::uint64_t prime) {
	assert(prime >= 2 && (prime == 2 || (prime & 1)));
	if (prime == 2) return a & 1;
	a %= prime;
	if (a <= 1) return a;
	if (prime <= std::numeric_limits<std::uint32_t>::max())
		return mod_sqrt_detail::sqrt(a, Montgomery32(std::uint32_t(prime)));
	return mod_sqrt_detail::sqrt(a, Montgomery64(prime));
}


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#include <cstddef>
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


#line 4 "test/mod_sqrt.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t a = 0, p = 0;
		input.read(a), input.read(p);
		auto root = mod_sqrt(a, p);
		if (root) output.println(*root);
		else output.println(-1);
	}
}
