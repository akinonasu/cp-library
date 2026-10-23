#line 1 "test/berlekamp_massey.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/find_linear_recurrence"
#line 1 "fps/berlekamp_massey.hpp"



#line 1 "components/field_arithmetic.hpp"



#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <utility>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace field_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<T>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 31))
		&& std::is_convertible_v<decltype(std::declval<T>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

template <class T>
inline constexpr bool small_modint = SmallModint<T>::value;

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(FIELD_DISABLE_AVX2) && !defined(MATRIX_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

template <class T>
struct Field {
	using V = T;
	static T add(T a, T b) { return a + b; }
	static T mul(T a, T b) { return a * b; }
	static T neg(T a) { return -a; }
	static T inv(T a) { return T(1) / a; }
	static bool zero(T a, long double eps) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) <= eps;
		else { (void)eps; return a == T(0); }
	}
	static bool better(T a, T b) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) > std::abs(b);
		else { (void)a; (void)b; return false; }
	}
	static T dot(const T *a, const T *b, int n) {
		T result = 0;
		for (int i = 0; i < n; ++i) result += a[i] * b[i];
		return result;
	}
	static void scale(T *a, int n, T x) { for (int i = 0; i < n; ++i) a[i] *= x; }
	static void axpy(T *a, const T *b, int n, T x) { for (int i = 0; i < n; ++i) a[i] += x * b[i]; }
};

template <class T>
struct ModularField {
	using V = std::uint32_t;
	static constexpr V mod = T::mod();
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V mul(V a, V b) { return std::uint64_t(a) * b % mod; }
	static V neg(V a) { return a ? mod - a : 0; }
	static V dot(const V *a, const V *b, int n) {
		constexpr int step = mod <= (V(1) << 30) ? 16 : 4;
		std::uint64_t sum = 0;
		for (int i = 0; i < n; i += step) {
			for (int j = i; j < std::min(n, i + step); ++j) sum += std::uint64_t(a[j]) * b[j];
			sum %= mod;
		}
		return sum;
	}
	static V inv(V a) { return (T(1) / T::raw(a)).val(); }
	static bool zero(V a, long double) { return !a; }
	static bool better(V, V) { return false; }
	static V product(V a, V x, V q) {
		V r = a * x - V((std::uint64_t(a) * q) >> 32) * mod;
		return r < mod ? r : r - mod;
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx2"))) static __m256i product8(__m256i a, __m256i x, __m256i q) {
		__m256i p = _mm256_set1_epi32(mod);
		__m256i even = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		__m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		__m256i high = _mm256_or_si256(even, _mm256_slli_epi64(_mm256_srli_epi64(odd, 32), 32));
		__m256i r = _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(high, p));
		return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
	}
	template <bool add_to>
	__attribute__((target("avx2"))) static void apply8(V *a, const V *b, int n, V x, V q) {
		__m256i vx = _mm256_set1_epi32(x), vq = _mm256_set1_epi32(q), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			__m256i r = product8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), vx, vq);
			if constexpr (add_to) {
				r = _mm256_add_epi32(r, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)));
				r = _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	static void apply(V *a, const V *b, int n, V x) {
		V q = (std::uint64_t(x) << 32) / mod;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 16 && avx2()) { apply8<add_to>(a, b, n, x, q); return; }
#endif
		for (int i = 0; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	static void scale(V *a, int n, V x) { apply<false>(a, a, n, x); }
	static void axpy(V *a, const V *b, int n, V x) { apply<true>(a, b, n, x); }
};

}


#line 5 "fps/berlekamp_massey.hpp"
#include <cassert>
#include <limits>
#line 8 "fps/berlekamp_massey.hpp"
#include <vector>

namespace berlekamp_massey_detail {
template <class Ops>
std::vector<typename Ops::V> restore(const std::vector<typename Ops::V> &sequence) {
	using V = typename Ops::V;
	const int n = int(sequence.size());
	std::vector<V> reversed(sequence.rbegin(), sequence.rend()), current{V(1)}, previous{V(1)}, saved;
	current.reserve(sequence.size() + 1);
	previous.reserve(sequence.size() + 1);
	saved.reserve(sequence.size() + 1);
	int order = 0, shift = 1;
	V inverse = 1;
	for (int i = 0; i < n; ++i) {
		V discrepancy = Ops::dot(current.data(), reversed.data() + n - 1 - i, int(current.size()));
		if (Ops::zero(discrepancy, 0)) {
			++shift;
			continue;
		}
		bool grow = order <= i / 2;
		if (grow) saved.assign(current.begin(), current.end());
		V factor = Ops::neg(Ops::mul(discrepancy, inverse));
		if (current.size() < previous.size() + shift) current.resize(previous.size() + shift);
		Ops::axpy(current.data() + shift, previous.data(), int(previous.size()), factor);
		if (grow) {
			order = i + 1 - order;
			previous.swap(saved);
			inverse = Ops::inv(discrepancy);
			shift = 1;
		} else ++shift;
	}
	current.resize(std::size_t(order) + 1);
	current.erase(current.begin());
	for (auto &x : current) x = Ops::neg(x);
	return current;
}
}

template <class T>
std::vector<T> berlekamp_massey(const std::vector<T> &sequence) {
	static_assert(!std::is_integral_v<T> && !std::is_same_v<T, __int128_t> && !std::is_same_v<T, __uint128_t>);
	assert(sequence.size() < std::size_t(std::numeric_limits<int>::max()));
	if constexpr (field_detail::small_modint<T>) {
		std::vector<std::uint32_t> values(sequence.size());
		for (std::size_t i = 0; i < sequence.size(); ++i) values[i] = sequence[i].val();
		auto coefficients = berlekamp_massey_detail::restore<field_detail::ModularField<T>>(values);
		std::vector<T> result;
		result.reserve(coefficients.size());
		for (auto x : coefficients) result.push_back(T::raw(x));
		return result;
	} else return berlekamp_massey_detail::restore<field_detail::Field<T>>(sequence);
}


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	Mint operator+() const { return *this; }
	Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	Mint operator+(Mint b) const { return Mint(*this) += b; }
	Mint operator-(Mint b) const { return Mint(*this) -= b; }
	Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
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


#line 5 "test/berlekamp_massey.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<mint> a(n);
	for (auto &x : a) {
		unsigned value = 0;
		in.read(value);
		x = mint::raw(value);
	}
	auto c = berlekamp_massey(a);
	out.println(c.size());
	for (std::size_t i = 0; i < c.size(); ++i) {
		if (i) out.write(' ');
		out.write(c[i].val());
	}
	out.write('\n');
}
