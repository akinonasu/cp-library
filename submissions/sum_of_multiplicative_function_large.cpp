#line 1 "test/multiplicative_prefix_sum_large.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_multiplicative_function_large"
#define TIME_LIMIT 30
#line 1 "math/number_theory/multiplicative_prefix_sum.hpp"



#line 1 "components/floor_quotients.hpp"



#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

class FloorQuotients {
	std::uint64_t n_, root_ = 0;
	std::size_t large_ = 0;
	std::vector<std::uint64_t> values_;

  public:
	explicit FloorQuotients(std::uint64_t n = 0) : n_(n) {
		if (!n) return;
		root_ = std::sqrt(static_cast<long double>(n));
		while (__uint128_t(root_) * root_ > n) --root_;
		while (__uint128_t(root_ + 1) * (root_ + 1) <= n) ++root_;
		large_ = n / (root_ + 1);
		values_.reserve(large_ + root_);
		for (std::size_t i = 1; i <= large_; ++i) values_.push_back(n / i);
		for (auto x = root_; x; --x) values_.push_back(x);
	}
	FloorQuotients(const FloorQuotients&) = default;
	FloorQuotients& operator=(const FloorQuotients&) = default;
	FloorQuotients(FloorQuotients&& other) noexcept : FloorQuotients() { swap(other); }
	FloorQuotients& operator=(FloorQuotients&& other) noexcept {
		if (this != &other) { FloorQuotients empty; swap(empty); swap(other); }
		return *this;
	}
	void swap(FloorQuotients& other) noexcept {
		std::swap(n_, other.n_), std::swap(root_, other.root_), std::swap(large_, other.large_);
		values_.swap(other.values_);
	}
	std::uint64_t max_n() const { return n_; }
	std::uint64_t root() const { return root_; }
	std::size_t size() const { return values_.size(); }
	std::size_t large_size() const { return large_; }
	const std::vector<std::uint64_t>& values() const { return values_; }
	bool contains(std::uint64_t x) const { return x && x <= n_ && (x <= root_ || n_ / (n_ / x) == x); }
	std::size_t index(std::uint64_t x) const {
		assert(x && x <= n_);
		return x <= root_ ? size() - x : n_ / x - 1;
	}
	std::size_t divided_index(std::size_t i, std::uint64_t divisor) const {
		assert(i < size() && divisor && divisor <= values_[i]);
		if (i >= large_) return size() - values_[i] / divisor;
		const auto d = (i + 1) * divisor;
		return d <= large_ ? d - 1 : size() - n_ / d;
	}
};


#line 1 "math/number_theory/prime_sieve.hpp"



#include <algorithm>
#include <array>
#line 9 "math/number_theory/prime_sieve.hpp"
#include <cstring>
#line 11 "math/number_theory/prime_sieve.hpp"

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	result.push_back(2);
	const std::size_t count = n / 2;
	std::size_t block_size = 32768;
	std::vector<std::uint8_t> prime(std::min(count, block_size), 1);
	auto append = [&](std::size_t base, std::size_t length) {
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= length; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			while (word) {
				result.push_back(2 * (base + i + (__builtin_ctzll(word) >> 3)) + 1);
				word &= word - 1;
			}
		}
#endif
		for (; i < length; ++i) if (prime[i]) result.push_back(2 * (base + i) + 1);
	};
	if (count <= block_size) {
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < count; j += p) prime[j] = 0;
		}
		append(0, count);
		return result;
	}
	std::size_t limit = std::size_t(std::sqrt(static_cast<long double>(n - 1)));
	while (limit > (n - 1) / limit) --limit;
	while (limit + 1 <= (n - 1) / (limit + 1)) ++limit;
	constexpr std::size_t period = 1155;
	block_size = (std::max(block_size, limit) + period - 1) / period * period;
	prime.resize(block_size);
	constexpr auto pattern = [] {
		std::array<std::uint8_t, period> value{};
		for (std::size_t i = 0; i < period; ++i)
			value[i] = (2 * i + 1) % 3 && (2 * i + 1) % 5 && (2 * i + 1) % 7 && (2 * i + 1) % 11;
		return value;
	}();
	auto primes = prime_sieve(limit + 1);
	std::vector<std::size_t> next(primes.size());
	for (std::size_t i = 1; i < primes.size(); ++i) next[i] = primes[i] * primes[i] / 2;
	for (std::size_t base = 0; base < count;) {
		std::size_t length = std::min(block_size, count - base);
		for (std::size_t offset = 0; offset < length; offset += period)
			std::copy_n(pattern.begin(), std::min(period, length - offset), prime.begin() + offset);
		if (!base) {
			prime[0] = 0;
			prime[1] = prime[2] = prime[3] = prime[5] = 1;
		}
		for (std::size_t i = 5; i < primes.size(); ++i) {
			std::size_t j = next[i];
			for (; j < length; j += primes[i]) prime[j] = 0;
			next[i] = j - length;
		}
		append(base, length);
		base += length;
	}
	return result;
}


#line 12 "math/number_theory/multiplicative_prefix_sum.hpp"

template<class T> class MultiplicativePrefixSum {
	FloorQuotients quotients_;
	std::vector<std::size_t> primes_;

  public:
	explicit MultiplicativePrefixSum(std::uint64_t n = 0) : quotients_(n), primes_(prime_sieve(quotients_.root() + 1)) {}
	MultiplicativePrefixSum(const MultiplicativePrefixSum&) = default;
	MultiplicativePrefixSum& operator=(const MultiplicativePrefixSum&) = default;
	MultiplicativePrefixSum(MultiplicativePrefixSum&& other) noexcept : MultiplicativePrefixSum() { swap(other); }
	MultiplicativePrefixSum& operator=(MultiplicativePrefixSum&& other) noexcept {
		if (this != &other) { MultiplicativePrefixSum empty; swap(empty); swap(other); }
		return *this;
	}
	void swap(MultiplicativePrefixSum& other) noexcept { quotients_.swap(other.quotients_); primes_.swap(other.primes_); }
	std::uint64_t max_n() const { return quotients_.max_n(); }
	std::size_t size() const { return quotients_.size(); }
	const std::vector<std::uint64_t>& quotients() const { return quotients_.values(); }
	std::size_t index(std::uint64_t x) const { return quotients_.index(x); }
	T get(const std::vector<T>& table, std::uint64_t x) const {
		assert(table.size() == size());
		return x ? table[index(x)] : T(0);
	}
	template<class Prefix> std::vector<T> prime_sum(Prefix integer_prefix) const {
		std::vector<T> result;
		result.reserve(size());
		for (auto x : quotients()) result.push_back(integer_prefix(x) - T(1));
		const std::uint64_t n = max_n(), root = quotients_.root();
		const std::size_t large = quotients_.large_size(), length = size();
		for (std::uint64_t p : primes_) {
			const T weight = integer_prefix(p) - integer_prefix(p - 1);
			const T before = result[length - p + 1];
			const auto end = std::min<std::uint64_t>(large, n / (p * p));
			const auto split = std::min<std::uint64_t>(end, large / p);
			std::uint64_t i = 1;
			for (; i <= split; ++i) result[i - 1] -= weight * (result[i * p - 1] - before);
			for (; i <= end; ++i) result[i - 1] -= weight * (result[length - n / (i * p)] - before);
			std::uint64_t x = root;
			for (std::uint64_t q = root / p; q >= p; --q) {
				const T delta = weight * (result[length - q] - before);
				for (const auto stop = q * p; x >= stop; --x) result[length - x] -= delta;
			}
		}
		return result;
	}
	std::vector<T> prime_sum_linear(T constant, T linear) const {
		auto identity_prefix = [](std::uint64_t x) -> T {
			return x & 1 ? T(x) * (T(x / 2) + T(1)) : T(x / 2) * (T(x) + T(1));
		};
		if constexpr (requires { linear == T(0); }) {
			if (linear == T(0)) {
				if (constant == T(0)) return std::vector<T>(size(), T(0));
				auto result = prime_sum([](std::uint64_t x) { return T(x); });
				for (auto& value : result) value *= constant;
				return result;
			}
			if (constant == T(0)) {
				auto result = prime_sum(identity_prefix);
				for (auto& value : result) value *= linear;
				return result;
			}
		}
		std::vector<T> result, weighted;
		result.reserve(size()), weighted.reserve(size());
		for (auto x : quotients()) {
			result.push_back(T(x) - T(1));
			weighted.push_back(identity_prefix(x) - T(1));
		}
		const std::uint64_t n = max_n(), root = quotients_.root();
		const std::size_t large = quotients_.large_size(), length = size();
		for (std::uint64_t p : primes_) {
			const T weight = T(p), before = result[length - p + 1], weighted_before = weighted[length - p + 1];
			const auto end = std::min<std::uint64_t>(large, n / (p * p));
			const auto split = std::min<std::uint64_t>(end, large / p);
			std::uint64_t i = 1;
			for (; i <= split; ++i) {
				result[i - 1] -= result[i * p - 1] - before;
				weighted[i - 1] -= weight * (weighted[i * p - 1] - weighted_before);
			}
			for (; i <= end; ++i) {
				const auto target = length - n / (i * p);
				result[i - 1] -= result[target] - before;
				weighted[i - 1] -= weight * (weighted[target] - weighted_before);
			}
			std::uint64_t x = root;
			for (std::uint64_t q = root / p; q >= p; --q) {
				const T delta = result[length - q] - before;
				const T weighted_delta = weight * (weighted[length - q] - weighted_before);
				for (const auto stop = q * p; x >= stop; --x) {
					result[length - x] -= delta;
					weighted[length - x] -= weighted_delta;
				}
			}
		}
		for (std::size_t i = 0; i < size(); ++i) result[i] = result[i] * constant + weighted[i] * linear;
		return result;
	}
	template<class PrimePower> std::vector<T> prefix_sum(const std::vector<T>& prime_prefix, PrimePower prime_power) const {
		assert(prime_prefix.size() == size());
		std::vector<T> result = prime_prefix, weights;
		std::vector<std::uint64_t> powers;
		weights.reserve(64), powers.reserve(64);
		for (std::size_t j = primes_.size(); j--;) {
			const std::uint64_t p = primes_[j];
			const T before = prime_prefix[index(p)];
			weights.clear(), powers.clear();
			for (std::uint64_t power = p;; power *= p) {
				powers.push_back(power);
				weights.push_back(prime_power(p, static_cast<unsigned>(powers.size()), power));
				if (power > max_n() / p) break;
			}
			for (std::size_t i = 0; i < size() && quotients()[i] >= p * p; ++i) {
				T delta = T(0);
				for (std::size_t e = 0; e + 1 < powers.size() && powers[e + 1] <= quotients()[i]; ++e)
					delta += weights[e] * (result[quotients_.divided_index(i, powers[e])] - before) + weights[e + 1];
				result[i] += delta;
			}
		}
		for (auto& value : result) value += T(1);
		return result;
	}
	template<class PrimePower> T sum(const std::vector<T>& prime_prefix, PrimePower prime_power) const {
		assert(prime_prefix.size() == size());
		if (!max_n()) return T(0);
		std::vector<T> first, second;
		first.reserve(primes_.size()), second.reserve(primes_.size());
		for (std::uint64_t p : primes_) {
			first.push_back(prime_power(p, 1, p));
			second.push_back(prime_power(p, 2, p * p));
		}
		T result = prime_prefix[0] + T(1);
		const auto length = size(), large = quotients_.large_size();
		auto dfs = [&](auto&& self, std::size_t i, unsigned e, std::uint64_t product, std::uint64_t power, T coefficient) -> void {
			const std::uint64_t p = primes_[i], limit = max_n() / product;
			result += coefficient * (e == 1 ? second[i] : prime_power(p, e + 1, power * p));
			if (p * p <= limit) self(self, i, e + 1, product * p, power * p, coefficient);
			coefficient *= e == 1 ? first[i] : e == 2 ? second[i] : prime_power(p, e, power);
			if constexpr (requires { coefficient == T(0); }) if (coefficient == T(0)) return;
			result += coefficient * (prime_prefix[product <= large ? product - 1 : length - limit] - prime_prefix[length - p]);
			std::size_t j = i + 1;
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (__uint128_t(q) * q * q > limit) break;
				self(self, j, 1, product * q, q, coefficient);
			}
			for (; j < primes_.size(); ++j) {
				const std::uint64_t q = primes_[j];
				if (q * q > limit) break;
				result += coefficient * (second[j] + first[j] * (prime_prefix[quotients_.divided_index(0, product * q)] - prime_prefix[length - q]));
			}
		};
		for (std::size_t i = 0; i < primes_.size(); ++i) dfs(dfs, i, 1, primes_[i], primes_[i], T(1));
		return result;
	}
};


#line 1 "components/fast_io.hpp"



#line 6 "components/fast_io.hpp"
#include <bit>
#line 8 "components/fast_io.hpp"
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


#line 1 "test/fixtures/multiplicative_prefix_sum_types.hpp"



#line 5 "test/fixtures/multiplicative_prefix_sum_types.hpp"

template<unsigned Mod> struct MultiplicativeSumModint {
	unsigned v = 0;
	MultiplicativeSumModint() = default;
	MultiplicativeSumModint(long long x) { auto r = x % Mod; v = r < 0 ? r + Mod : r; }
	unsigned val() const { return v; }
	MultiplicativeSumModint& operator+=(MultiplicativeSumModint b) { v += b.v; if (v >= Mod) v -= Mod; return *this; }
	MultiplicativeSumModint& operator-=(MultiplicativeSumModint b) { v = v >= b.v ? v - b.v : v + Mod - b.v; return *this; }
	MultiplicativeSumModint& operator*=(MultiplicativeSumModint b) { v = std::uint64_t(v) * b.v % Mod; return *this; }
	friend MultiplicativeSumModint operator+(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a += b; }
	friend MultiplicativeSumModint operator-(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a -= b; }
	friend MultiplicativeSumModint operator*(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a *= b; }
	friend bool operator==(MultiplicativeSumModint a, MultiplicativeSumModint b) { return a.v == b.v; }
};


#line 6 "test/multiplicative_prefix_sum_large.test.cpp"

int main() {
	using M = MultiplicativeSumModint<469762049>;
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t n = 0;
		unsigned a = 0, b = 0;
		input.read(n), input.read(a), input.read(b);
		MultiplicativePrefixSum<M> sieve(n);
		auto primes = sieve.prime_sum_linear(M(a), M(b));
		auto answer = sieve.sum(primes, [a, b](std::uint64_t p, unsigned e, std::uint64_t) { return M(a) * M(e) + M(b) * M(p); });
		output.println(answer.val());
	}
}
