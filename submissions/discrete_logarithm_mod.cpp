#line 1 "test/discrete_log.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_mod"
#line 1 "math/discrete_log.hpp"



#line 1 "components/barrett32.hpp"



#include <cassert>
#include <cstdint>

struct Barrett32 {
	std::uint32_t mod;
	std::uint64_t reciprocal;
	explicit Barrett32(std::uint32_t m) : mod(m), reciprocal(m > 1 ? std::uint64_t((__uint128_t(1) << 64) / m) : 0) {
		assert(m > 0);
	}
	std::uint32_t reduce(std::uint64_t x) const {
		if (mod == 1) return 0;
		std::uint64_t q = (__uint128_t(x) * reciprocal) >> 64, r = x - q * mod;
		return r < mod ? r : r - mod;
	}
	std::uint32_t mul(std::uint32_t a, std::uint32_t b) const { return reduce(std::uint64_t(a) * b); }
	std::uint32_t pow(std::uint32_t a, std::uint64_t e) const {
		std::uint32_t r = mod == 1 ? 0 : 1;
		for (; e; e >>= 1, a = mul(a, a)) if (e & 1) r = mul(r, a);
		return r;
	}
};


#line 1 "math/factorize.hpp"



#line 1 "math/is_prime.hpp"



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


#line 6 "math/is_prime.hpp"
#include <algorithm>
#include <array>
#include <bit>
#line 10 "math/is_prime.hpp"

namespace is_prime_detail {
using U = std::uint64_t;
inline constexpr std::uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

template <class Montgomery, class Multiply>
inline bool witness(const Montgomery &m, typename Montgomery::U x, int s, Multiply multiply) {
	if (x >= m.mod) x -= m.mod;
	if (x == m.one || x == m.mod - m.one) return true;
	for (int j = 1; j < s; ++j) {
		x = multiply(x, x);
		if (x >= m.mod) x -= m.mod;
		if (x == m.mod - m.one) return true;
	}
	return false;
}

template <class Multiply>
inline bool witness2(const Montgomery64 &m, U d, int s, Multiply multiply) {
	int shift = std::max(0, int(std::bit_width(d)) - 6);
	U x = m.in(U(1) << (d >> shift));
	const bool lazy = m.mod < (U(1) << 62);
	const U twice = m.mod + m.mod;
	for (int bit = shift - 1; bit >= 0; --bit) {
		x = multiply(x, x);
		if ((d >> bit) & 1) {
			if (lazy) {
				x += x;
				if (x >= twice) x -= twice;
			} else x = m.add(x, x);
		}
	}
	return witness(m, x, s, multiply);
}

template <class Montgomery, std::size_t Count, class Multiply>
inline bool witnesses(const Montgomery &m, typename Montgomery::U d, int s,
	const std::array<typename Montgomery::U, Count> &bases, Multiply multiply) {
	using V = typename Montgomery::U;
	std::array<V, Count> x;
	if (Count <= 2 || d < 4096) {
		std::array<V, Count> a;
#pragma GCC unroll 8
		for (std::size_t i = 0; i < Count; ++i) a[i] = x[i] = m.in(bases[i] % m.mod);
		for (V e = d >> 1; e; e >>= 1) {
#pragma GCC unroll 8
			for (std::size_t i = 0; i < Count; ++i) a[i] = multiply(a[i], a[i]);
			if (e & 1) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], a[i]);
			}
		}
	} else {
		constexpr int window = 3;
		std::array<std::array<V, Count>, 1 << window> powers;
		for (std::size_t i = 0; i < Count; ++i) powers[1][i] = m.in(bases[i] % m.mod);
		for (int k = 2; k < (1 << window); ++k)
			for (std::size_t i = 0; i < Count; ++i) powers[k][i] = multiply(powers[k - 1][i], powers[1][i]);
		int shift = ((std::bit_width(d) - 1) / window) * window;
		x = powers[d >> shift];
		for (shift -= window; shift >= 0; shift -= window) {
			for (int k = 0; k < window; ++k) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], x[i]);
			}
			int digit = (d >> shift) & ((1 << window) - 1);
			if (digit) {
#pragma GCC unroll 8
				for (std::size_t i = 0; i < Count; ++i) x[i] = multiply(x[i], powers[digit][i]);
			}
		}
	}
#pragma GCC unroll 8
	for (std::size_t i = 0; i < Count; ++i) {
		V y = x[i] >= m.mod ? x[i] - m.mod : x[i];
		if (!y && bases[i] % m.mod == 0) continue;
		if (!witness(m, y, s, multiply)) return false;
	}
	return true;
}

inline bool prime(const Montgomery32 &m) {
	using V = Montgomery32::U;
	V d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	auto check = [&](auto multiply) {
		if (!witnesses(m, d, s, std::array<V, 1>{2}, multiply)) return false;
		if (m.mod < 1373653) return witnesses(m, d, s, std::array<V, 1>{3}, multiply);
		return witnesses(m, d, s, std::array<V, 2>{7, 61}, multiply);
	};
	if (m.mod < (V(1) << 30)) return check([&](V a, V b) {
		std::uint64_t x = std::uint64_t(a) * b;
		return V(x >> 32) + m.mod - V((std::uint64_t(V(x) * m.inverse) * m.mod) >> 32);
	});
	return check([&](V a, V b) { return m.mul(a, b); });
}

inline bool prime(const Montgomery64 &m) {
	if (m.mod < (U(1) << 32)) return prime(Montgomery32(m.mod));
	U d = m.mod - 1;
	int s = std::countr_zero(d);
	d >>= s;
	const std::array<U, 6> bases{325, 9375, 28178, 450775, 9780504, 1795265022};
	if (m.mod < (U(1) << 62)) {
		if (!witness2(m, d, s, [&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U(x >> 64) + m.mod - U((__uint128_t(U(x) * m.inverse) * m.mod) >> 64);
		})) return false;
		U inverse = -m.inverse;
		return witnesses(m, d, s, bases, [&](U a, U b) {
			__uint128_t x = __uint128_t(a) * b;
			return U((x + __uint128_t(U(x) * inverse) * m.mod) >> 64);
		});
	}
	auto multiply = [&](U a, U b) { return m.mul(a, b); };
	return witnesses(m, d, s, std::array<U, 1>{2}, multiply) && witnesses(m, d, s, bases, multiply);
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


#line 8 "math/factorize.hpp"
#include <cmath>
#line 10 "math/factorize.hpp"
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

namespace factorize_detail {
using U = std::uint64_t;
using W = __uint128_t;

struct Factorizer {
	SplitMix64 rng;
	std::vector<U> result;
	explicit Factorizer(U n) : rng(n ^ 0x9e3779b97f4a7c15ULL) { result.reserve(64); }
	U random() { return rng(); }

	struct RhoArithmetic {
		U mod, inverse, one;
		explicit RhoArithmetic(const Montgomery64 &m) : mod(m.mod), inverse(m.inverse), one(m.one) {}
		U mul(U a, U b) const {
			W x = W(a) * b;
			return U(x >> 64) + mod - U((W(U(x) * inverse) * mod) >> 64);
		}
		U step(U x, U c) const {
			return mul(x, x) + c;
		}
	};

	U rho_parallel(const Montgomery64 &original) {
		RhoArithmetic m(original);
		const U n = m.mod;
		bool first = true;
		for (;;) {
			U y1 = first ? 1 : random() % n, y2 = first ? 2 : random() % n;
			const U c1 = first ? 1 : (random() & 31) + 1, c2 = first ? 2 : (random() & 31) + 1;
			first = false;
			bool restart = false;
			for (int length = 64; length <= (1 << 18) && !restart; length <<= 1) {
				const U x1 = y1 + 2 * n, x2 = y2 + 2 * n;
				for (int k = 0; k < length; k += 1024) {
					const U saved1 = y1, saved2 = y2;
					U q1 = m.one, q2 = m.one;
					const int count = std::min(1024, length - k);
					y1 = m.step(y1, c1), y2 = m.step(y2, c2);
					for (int i = 0; i < count; ++i) {
						const U d1 = x1 - y1, d2 = x2 - y2;
						y1 = m.step(y1, c1), y2 = m.step(y2, c2);
						q1 = m.mul(q1, d1), q2 = m.mul(q2, d2);
					}
					q1 = m.mul(q1, x1 - y1), q2 = m.mul(q2, x2 - y2);
					const U g = std::gcd(m.mul(q1, q2), n);
					if (g == 1) continue;
					if (g != n) return g;
					const U g1 = std::gcd(q1, n), g2 = std::gcd(q2, n);
					if (g1 > 1 && g1 < n) return g1;
					if (g2 > 1 && g2 < n) return g2;
					U y = g1 == n ? saved1 : saved2;
					const U x = g1 == n ? x1 : x2, c = g1 == n ? c1 : c2;
					for (int i = 0; i <= count; ++i) {
						y = m.step(y, c);
						const U d = std::gcd(x - y, n);
						if (d > 1 && d < n) return d;
						if (d == n) break;
					}
					restart = true;
					break;
				}
			}
		}
	}
	U rho(const Montgomery64 &m) {
		if ((m.mod >> 32) && m.mod < (U(1) << 61)) return rho_parallel(m);
		U n = m.mod;
		for (;;) {
			U y = m.in(random() % (n - 1) + 1), c = m.in(random() % (n - 1) + 1);
			U x = 0, saved = 0, g = 1;
			auto step = [&](U v) { return m.add(m.mul(v, v), c); };
			auto diff = [](U a, U b) { return a < b ? b - a : a - b; };
			for (int length = 1; g == 1 && length <= (1 << 18); length <<= 1) {
				x = y;
				for (int repeat_index = 0; repeat_index < length; ++repeat_index) y = step(y);
				for (int k = 0; k < length && g == 1; k += 128) {
					saved = y;
					U q = m.one;
					for (int repeat_index = 0; repeat_index < std::min(128, length - k); ++repeat_index) {
						y = step(y);
						q = m.mul(q, diff(x, y));
					}
					g = std::gcd(q, n);
				}
			}
			if (g == n)
				for (int repeat_index = 0; repeat_index < 128; ++repeat_index) {
					saved = step(saved);
					g = std::gcd(diff(x, saved), n);
					if (g != 1) break;
				}
			if (g > 1 && g < n) return g;
		}
	}
	void split(U n) {
		if (n < 1681) {
			result.push_back(n);
			return;
		}
		if ((n & 7) == 1) {
			U r = std::sqrt(static_cast<long double>(n));
			r -= W(r) * r > n;
			if (W(r) * r == n) {
				int begin = static_cast<int>(result.size());
				split(r);
				int end = static_cast<int>(result.size());
				for (int i = begin; i < end; ++i) result.push_back(result[i]);
				return;
			}
		}
		Montgomery64 m(n);
		if (is_prime_detail::prime(m)) result.push_back(n);
		else {
			U d = rho(m);
			split(d);
			split(n / d);
		}
	}
};
}

inline std::vector<std::uint64_t> factorize(std::uint64_t n) {
	assert(n > 0);
	if (n < 2) return {};
	factorize_detail::Factorizer f(n);
	for (auto p : is_prime_detail::small_primes)
		while (n % p == 0) f.result.push_back(p), n /= p;
	if (n > 1) f.split(n);
	std::sort(f.result.begin(), f.result.end());
	return std::move(f.result);
}

inline std::vector<std::pair<std::uint64_t, int>> factorize_count(std::uint64_t n) {
	std::vector<std::pair<std::uint64_t, int>> result;
	for (auto p : factorize(n)) {
		if (result.empty() || result.back().first != p) result.emplace_back(p, 1);
		else ++result.back().second;
	}
	return result;
}

inline std::vector<std::uint64_t> divisors(const std::vector<std::pair<std::uint64_t, int>> &factors, bool sorted = true) {
	std::size_t count = 1;
	for (auto [p, e] : factors) {
		assert(p >= 2 && e > 0);
		count *= std::size_t(e) + 1;
	}
	std::vector<std::uint64_t> result{1};
	result.reserve(count);
	for (auto [p, e] : factors) {
		std::size_t size = result.size();
		std::uint64_t power = 1;
		for (int i = 0; i < e; ++i) {
			assert(power <= std::numeric_limits<std::uint64_t>::max() / p);
			power *= p;
			for (std::size_t j = 0; j < size; ++j) {
				assert(result[j] <= std::numeric_limits<std::uint64_t>::max() / power);
				result.push_back(result[j] * power);
			}
		}
	}
	if (sorted) std::sort(result.begin(), result.end());
	return result;
}

inline std::vector<std::uint64_t> divisors(std::uint64_t n, bool sorted = true) {
	return divisors(factorize_count(n), sorted);
}


#line 9 "math/discrete_log.hpp"
#include <optional>
#line 11 "math/discrete_log.hpp"

namespace discrete_log_detail {
inline std::uint32_t inverse(std::uint32_t x, std::uint32_t mod) {
	std::int64_t u = 0, v = 1, r = mod, s = x;
	while (s) {
		std::int64_t q = r / s, t = r - q * s;
		r = s, s = t, t = u - q * v, u = v, v = t;
	}
	return std::uint32_t(u < 0 ? u + mod : u);
}

inline std::optional<std::uint32_t> search(std::uint32_t a, std::uint32_t b, const Barrett32 &m, std::uint32_t order) {
	std::uint32_t value = 1;
	for (std::uint32_t j = 0; j < 32; ++j, value = m.mul(value, a)) {
		if (value == b) return j;
		if (j && value == 1) return std::nullopt;
	}
	std::uint32_t step = std::sqrt(double(order));
	step += std::uint64_t(step) * step < order;
	std::uint32_t capacity = std::bit_ceil(2 * step), mask = capacity - 1;
	int shift = 64 - std::countr_zero(capacity);
	std::vector<std::uint64_t> table(capacity);
	auto hash = [&](std::uint32_t x) { return std::uint32_t((x * 11995408973635179863ULL) >> shift); };
	value = 1;
	for (std::uint32_t j = 0; j < step; ++j, value = m.mul(value, a)) {
		if (value == b) return j;
		if (j && value == 1) return std::nullopt;
		auto slot = hash(value);
		while (table[slot]) slot = (slot + 1) & mask;
		table[slot] = (std::uint64_t(value) << 32) | j;
	}
	std::uint32_t stride = inverse(value, m.mod), target = b;
	for (std::uint64_t i = step; i < order; i += step) {
		target = m.mul(target, stride);
		auto slot = hash(target);
		while (table[slot] && std::uint32_t(table[slot] >> 32) != target) slot = (slot + 1) & mask;
		if (table[slot]) {
			std::uint64_t answer = i + std::uint32_t(table[slot]);
			if (answer < order) return std::uint32_t(answer);
			break;
		}
	}
	return std::nullopt;
}
}

inline std::optional<std::uint32_t> discrete_log(std::uint64_t a, std::uint64_t b, std::uint32_t mod) {
	assert(mod > 0);
	a %= mod, b %= mod;
	std::uint32_t offset = 0, start = 1 % mod;
	for (;;) {
		if (b == start) return offset;
		std::uint32_t g = std::gcd(std::uint32_t(a), mod);
		if (g == 1) break;
		if (b % g) return std::nullopt;
		b /= g, mod /= g, ++offset;
		start = std::uint64_t(start) * (a / g) % mod;
	}
	if (std::gcd(std::uint32_t(b), mod) != 1) return std::nullopt;
	Barrett32 m(mod);
	a %= mod;
	b = m.mul(b, discrete_log_detail::inverse(start, mod));
	std::uint32_t value = 1;
	for (std::uint32_t j = 0; j < 32; ++j, value = m.mul(value, a)) {
		if (value == b) return offset + j;
		if (j && value == 1) return std::nullopt;
	}
	std::uint32_t order = mod;
	for (auto [p, e] : factorize_count(mod)) order -= order / p;
	auto factors = factorize_count(order);
	for (auto &[p, e] : factors) while (e && m.pow(a, order / p) == 1) order /= p, --e;
	if (m.pow(b, order) != 1) return std::nullopt;
	std::uint64_t answer = 0, period = 1;
	std::uint32_t inverse_a = m.pow(a, order - 1);
	for (auto [p, e] : factors) if (e) {
		std::uint32_t base = m.pow(a, order / p), target = b, residue = 0, power = 1, exponent = order;
		for (int i = 0; i < e; ++i) {
			exponent /= p;
			auto digit = discrete_log_detail::search(base, m.pow(target, exponent), m, p);
			if (!digit) return std::nullopt;
			std::uint32_t term = *digit * power;
			residue += term, power *= p;
			target = m.mul(target, m.pow(inverse_a, term));
		}
		std::uint32_t delta = (std::uint64_t(residue) + power - answer % power) % power;
		delta = std::uint64_t(delta) * discrete_log_detail::inverse(period % power, power) % power;
		answer += period * delta, period *= power;
	}
	if (m.pow(a, answer) != b) return std::nullopt;
	return std::uint32_t(answer + offset);
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


#line 4 "test/discrete_log.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint32_t a = 0, b = 0, mod = 0;
		input.read(a), input.read(b), input.read(mod);
		auto result = discrete_log(a, b, mod);
		if (result) output.println(*result);
		else output.println(-1);
	}
}
