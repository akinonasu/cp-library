#line 1 "test/mod_pow_log.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_fixed_mod"
#line 1 "math/number_theory/mod_pow.hpp"



#line 1 "math/number_theory/mod_inv.hpp"



#line 1 "components/mod_fast_core.hpp"



#include <algorithm>
#include <cassert>
#include <bit>
#include <cstdint>
#include <utility>
#include <vector>

namespace mod_fast_detail {
struct Reducer {
	std::uint32_t mod;
	std::uint64_t reciprocal;
	explicit Reducer(std::uint32_t m) : mod(m), reciprocal(m > 1 ? std::uint64_t((__uint128_t(1) << 64) / m) : 0) {
		assert(1 <= m && m < (std::uint32_t(1) << 31));
	}
	std::uint32_t reduce(std::uint64_t x) const {
		if (mod == 1) return 0;
		std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64;
		std::uint32_t r = x - q * mod;
		return r < mod ? r : r - mod;
	}
	std::uint32_t mul(std::uint32_t a, std::uint32_t b) const { return reduce(std::uint64_t(a) * b); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t e) const {
		std::uint32_t r = mod == 1 ? 0 : 1;
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};

class RationalTable {
	struct Fraction { std::uint16_t a, b; };
	std::vector<Fraction> fractions_;
	std::uint32_t p_, shift_ = 0, limit_;

  public:
	explicit RationalTable(std::uint32_t p) : p_(p), limit_(p - 1) {
		assert(2 <= p && p < (std::uint32_t(1) << 31));
		if (p <= 4096) return;
		std::uint32_t d = 1;
		while (std::uint64_t(d) * d * d < p) d *= 2;
		std::uint32_t width = std::bit_ceil((p + d * d - 1) / (d * d));
		shift_ = std::countr_zero(width);
		limit_ = (p + d - 1) / d + d * width / 2;
		assert(limit_ < p);
		fractions_.resize((p + width - 1) / width);
		std::uint32_t a = 0, b = 1, c = 1, e = d;
		for (std::uint32_t i = 0; i < fractions_.size(); ++i) {
			std::uint64_t x = std::min((2ULL * i + 1) * width, 2ULL * p - 1);
			while (2ULL * p * c < x * e) {
				std::uint32_t k = (d + b) / e, next_a = k * c - a, next_b = k * e - b;
				a = c, b = e, c = next_a, e = next_b;
			}
			if (x * b - 2ULL * p * a <= 2ULL * p * c - x * e)
				fractions_[i] = {std::uint16_t(a), std::uint16_t(b)};
			else fractions_[i] = {std::uint16_t(c), std::uint16_t(e)};
		}
	}
	bool direct() const { return fractions_.empty(); }
	std::uint32_t limit() const { return limit_; }
	std::pair<int, std::uint32_t> fraction(std::uint32_t x) const {
		assert(0 < x && x < p_);
		if (direct()) return {int(x), 1};
		auto [a, b] = fractions_[x >> shift_];
		std::int64_t t = std::int64_t(x) * b - std::int64_t(a) * p_;
		assert(t != 0 && -std::int64_t(limit_) <= t && t <= limit_);
		return {int(t), b};
	}
};
}


#line 1 "math/number_theory/is_prime.hpp"



#line 1 "components/montgomery.hpp"



#line 6 "components/montgomery.hpp"

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


#line 6 "math/number_theory/is_prime.hpp"
#include <array>
#line 9 "math/number_theory/is_prime.hpp"
#include <initializer_list>

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

template <class Montgomery, std::size_t Count, class Multiply>
inline bool witnesses(const Montgomery &m, typename Montgomery::U d, int s,
	const std::array<typename Montgomery::U, Count> &bases, Multiply multiply) {
	using V = typename Montgomery::U;
	std::array<V, Count> a, x;
	for (std::size_t i = 0; i < Count; ++i) a[i] = x[i] = m.in(bases[i] % m.mod);
	for (V e = d >> 1; e; e >>= 1) {
		for (std::size_t i = 0; i < Count; ++i) a[i] = multiply(a[i], a[i]);
		if (e & 1) for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], a[i]);
	}
	for (std::size_t i = 0; i < Count; ++i) {
		V y = x[i] >= m.mod ? x[i] - m.mod : x[i];
		if (!y && bases[i] % m.mod == 0) continue;
		if (y == m.one || y == m.mod - m.one) continue;
		bool passed = false;
		for (int j = 1; j < s; ++j) {
			y = multiply(y, y);
			if (y >= m.mod) y -= m.mod;
			if (y == m.mod - m.one) { passed = true; break; }
		}
		if (!passed) return false;
	}
	return true;
}

inline bool prime(const Montgomery32 &m) {
	using V = Montgomery32::U;
	V d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto multiply = [&](V a, V b) { return m.mul(a, b); };
	return witnesses(m, d, s, std::array<V, 1>{2}, multiply)
		&& witnesses(m, d, s, std::array<V, 2>{7, 61}, multiply);
}

inline bool prime(const Montgomery64 &m) {
	if (m.mod < (U(1) << 32)) return prime(Montgomery32(m.mod));
	U d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto check = [&](auto multiply) {
		return witnesses(m, d, s, std::array<U, 1>{2}, multiply)
			&& witnesses(m, d, s, std::array<U, 6>{325, 9375, 28178, 450775, 9780504, 1795265022}, multiply);
	};
	if (m.mod < (U(1) << 62)) {
		U inverse = -m.inverse;
		return check([&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U((x + __uint128_t(U(x) * inverse) * m.mod) >> 64);
		});
	}
	return check([&](U a, U b) { return m.mul(a, b); });
}
}

inline bool is_prime(std::uint64_t n) {
	if (n < 2) return false;
	if (n % 2 == 0) return n == 2;
	if (n % 3 == 0) return n == 3;
	if (n % 5 == 0) return n == 5;
	if (n % 7 == 0) return n == 7;
	if (n % 11 == 0) return n == 11;
	if (n % 13 == 0) return n == 13;
	if (n % 17 == 0) return n == 17;
	if (n % 19 == 0) return n == 19;
	if (n % 23 == 0) return n == 23;
	if (n % 29 == 0) return n == 29;
	if (n % 31 == 0) return n == 31;
	if (n % 37 == 0) return n == 37;
	if (n < 1681) return true;
	if (n < (std::uint64_t(1) << 32)) return is_prime_detail::prime(Montgomery32(n));
	return is_prime_detail::prime(Montgomery64(n));
}


#line 6 "math/number_theory/mod_inv.hpp"

class ModInv {
  protected:
	mod_fast_detail::Reducer modulus_;
	mod_fast_detail::RationalTable fractions_;
	std::vector<std::uint32_t> inverse_;
	int center_;

  public:
	explicit ModInv(std::uint32_t prime = 998244353) : modulus_(prime), fractions_(prime), center_(fractions_.direct() ? 0 : fractions_.limit()) {
		assert(is_prime(prime));
		std::uint32_t limit = fractions_.limit();
		inverse_.resize(center_ + limit + 1);
		inverse_[center_ + 1] = 1;
		for (std::uint32_t i = 2; i <= limit; ++i)
			inverse_[center_ + i] = prime - modulus_.mul(prime / i, inverse_[center_ + prime % i]);
		if (center_) for (std::uint32_t i = 1; i <= limit; ++i) inverse_[center_ - i] = prime - inverse_[center_ + i];
	}
	std::uint32_t mod() const { return modulus_.mod; }
	std::uint32_t inv(std::uint32_t x) const {
		assert(0 < x && x < mod());
		if (x <= fractions_.limit()) return inverse_[center_ + x];
		auto [a, b] = fractions_.fraction(x);
		return modulus_.mul(inverse_[center_ + a], b);
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


#line 7 "math/number_theory/mod_pow.hpp"

class ModPow : public ModInv {
	mod_fast_detail::Reducer order_;
	std::uint32_t root_ = 1, shift_, mask_;
	std::vector<std::uint32_t> low_, high_, logarithm_;
	std::uint32_t power_(std::uint32_t e) const { return modulus_.mul(low_[e & mask_], high_[e >> shift_]); }
	void build_log_() {
		std::uint32_t p = mod(), limit = fractions_.limit(), order = p - 1;
		logarithm_.resize(inverse_.size());
		if (!center_) {
			std::uint32_t x = 1;
			for (std::uint32_t i = 0; i < order; ++i, x = modulus_.mul(x, root_)) logarithm_[x] = i;
			return;
		}
		std::vector<std::uint16_t> factor(limit + 1);
		for (std::uint32_t q = 2; std::uint64_t(q) * q <= limit; ++q) if (!factor[q]) {
			for (std::uint32_t i = q * q; i <= limit; i += q) if (!factor[i]) factor[i] = q;
		}
		std::uint32_t step = std::min(order, 4 * (mask_ + 1)), capacity = std::bit_ceil(2 * step);
		std::vector<std::uint64_t> baby(capacity);
		auto hash = [=](std::uint32_t x) {
			x ^= x >> 16, x *= 0x7feb352dU, x ^= x >> 15;
			return x & (capacity - 1);
		};
		std::uint32_t value = 1;
		for (std::uint32_t i = 0; i < step; ++i, value = modulus_.mul(value, root_)) {
			auto k = hash(value);
			while (baby[k]) k = (k + 1) & (capacity - 1);
			baby[k] = (std::uint64_t(value) << 32) | i;
		}
		std::uint32_t giant = power_(order - step);
		auto bsgs = [&](std::uint32_t x) {
			for (std::uint32_t i = 0; i < order; i += step, x = modulus_.mul(x, giant)) {
				auto k = hash(x);
				while (baby[k] && std::uint32_t(baby[k] >> 32) != x) k = (k + 1) & (capacity - 1);
				if (baby[k]) {
					std::uint32_t result = i + std::uint32_t(baby[k]);
					return result < order ? result : result - order;
				}
			}
			assert(false);
			return std::uint32_t(0);
		};
		SplitMix64 rng((std::uint64_t(p) << 32) | root_);
		for (std::uint32_t i = 2; i <= limit; ++i) {
			std::uint32_t result = 0;
			if (factor[i]) result = order_.reduce(std::uint64_t(logarithm_[center_ + factor[i]]) + logarithm_[center_ + i / factor[i]]);
			else if (std::uint64_t(i) * i > p) {
				result = order_.reduce(std::uint64_t(logarithm_[center_ + p % i]) + order / 2 + order - logarithm_[center_ + p / i]);
			} else {
				bool found = false;
				if (i >= 100) for (int attempt = 0; attempt < 64 && !found; ++attempt) {
					std::uint32_t k = order_.reduce(rng()), x = modulus_.mul(i, power_(k));
					auto [a, b] = fractions_.fraction(x);
					std::uint64_t sum = order - k + (a < 0 ? order / 2 : 0);
					auto accumulate = [&](std::uint32_t value, bool subtract) {
						while (value > 1) {
							std::uint32_t q = factor[value] ? factor[value] : value;
							if (q >= i) return false;
							sum += subtract ? order - logarithm_[center_ + q] : logarithm_[center_ + q];
							value /= q;
						}
						return true;
					};
					if (accumulate(a < 0 ? -a : a, false) && accumulate(b, true)) found = true, result = order_.reduce(sum);
				}
				if (!found) result = bsgs(i);
			}
			logarithm_[center_ + i] = result;
		}
		for (std::uint32_t i = 1; i <= limit; ++i)
			logarithm_[center_ - i] = order_.reduce(std::uint64_t(logarithm_[center_ + i]) + order / 2);
	}

  public:
	explicit ModPow(std::uint32_t prime = 998244353, std::uint32_t root = 0) : ModInv(prime), order_(prime - 1) {
		std::uint32_t remaining = prime - 1;
		std::vector<std::uint32_t> factors;
		for (std::uint32_t q = 2; std::uint64_t(q) * q <= remaining; ++q) if (remaining % q == 0) {
			factors.push_back(q);
			do { remaining /= q; } while (remaining % q == 0);
		}
		if (remaining > 1) factors.push_back(remaining);
		auto valid = [&](std::uint32_t g) {
			return 0 < g && g < prime && std::all_of(factors.begin(), factors.end(), [&](auto q) { return modulus_.pow(g, (prime - 1) / q) != 1; });
		};
		if (root) { assert(valid(root)); root_ = root; }
		else while (!valid(root_)) ++root_;
		shift_ = (std::bit_width(prime - 2) + 1) / 2;
		std::uint32_t width = std::uint32_t(1) << shift_;
		mask_ = width - 1;
		low_.resize(width), high_.resize((prime - 2) / width + 1);
		low_[0] = high_[0] = 1;
		for (std::uint32_t i = 1; i < width; ++i) low_[i] = modulus_.mul(low_[i - 1], root_);
		std::uint32_t stride = modulus_.mul(low_.back(), root_);
		for (std::uint32_t i = 1; i < high_.size(); ++i) high_[i] = modulus_.mul(high_[i - 1], stride);
		build_log_();
	}
	std::uint32_t primitive_root() const { return root_; }
	std::uint32_t log(std::uint32_t x) const {
		assert(0 < x && x < mod());
		if (x <= fractions_.limit()) return logarithm_[center_ + x];
		auto [a, b] = fractions_.fraction(x);
		std::uint32_t result = logarithm_[center_ + a] + (mod() - 1) - logarithm_[center_ + b];
		return result < mod() - 1 ? result : result - (mod() - 1);
	}
	std::uint32_t pow_root(std::uint64_t exponent) const { return power_(order_.reduce(exponent)); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t exponent) const {
		assert(a < mod());
		if (!a) return exponent == 0 ? 1 : 0;
		return power_(order_.mul(log(a), order_.reduce(exponent)));
	}
};


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


#line 4 "test/mod_pow_log.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::uint32_t p = 0, g = 0;
	int n = 0;
	input.read(p), input.read(g), input.read(n);
	ModPow power(p, g);
	while (n--) {
		std::uint32_t a = 0;
		input.read(a);
		output.println(power.log(a));
	}
}
