#line 1 "test/bitwise_xor_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/bitwise_xor_convolution"
#line 1 "convolution/bitwise_convolution.hpp"



#line 1 "components/bitwise_zeta.hpp"



#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace bitwise_zeta_detail {
template <bool inverse, bool superset, class T>
void stages(std::vector<T> &a, std::size_t start, std::size_t n, std::size_t half) {
	auto step = [&](std::size_t low, std::size_t high) {
		T &to = a[superset ? low : high];
		const T &from = a[superset ? high : low];
		if constexpr (inverse) to -= from;
		else to += from;
	};
	for (; half < n / 2; half *= 4)
		for (std::size_t block = start; block < start + n; block += 4 * half)
			for (std::size_t j = block; j < block + half; ++j) {
				step(j, j + half);
				step(j + 2 * half, j + 3 * half);
				step(j, j + 2 * half);
				step(j + half, j + 3 * half);
			}
	if (half < n)
		for (std::size_t j = start; j < start + half; ++j) step(j, j + half);
}
}

template <bool inverse = false, bool superset = false, class T>
void bitwise_zeta(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	constexpr std::size_t block = 1024;
	if (a.size() <= block) bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), 1);
	else {
		for (std::size_t start = 0; start < a.size(); start += block)
			bitwise_zeta_detail::stages<inverse, superset>(a, start, block, 1);
		bitwise_zeta_detail::stages<inverse, superset>(a, 0, a.size(), block);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/walsh_hadamard.hpp"



#line 8 "components/walsh_hadamard.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace walsh_hadamard_detail {
template <class T>
void stages(std::vector<T> &a, std::size_t start, std::size_t n, std::size_t half) {
	auto step = [&](std::size_t low, std::size_t high) {
		T x = a[low], y = a[high];
		a[low] = x + y;
		a[high] = x - y;
	};
	for (; half < n / 2; half *= 4)
		for (std::size_t block = start; block < start + n; block += 4 * half)
			for (std::size_t j = block; j < block + half; ++j) {
				step(j, j + half);
				step(j + 2 * half, j + 3 * half);
				step(j, j + 2 * half);
				step(j + half, j + 3 * half);
			}
	if (half < n)
		for (std::size_t j = start; j < start + half; ++j) step(j, j + half);
}
}

template <class T>
void walsh_hadamard(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	constexpr std::size_t block = 1024;
	if (a.size() <= block) walsh_hadamard_detail::stages(a, 0, a.size(), 1);
	else {
		for (std::size_t start = 0; start < a.size(); start += block)
			walsh_hadamard_detail::stages(a, start, block, 1);
		walsh_hadamard_detail::stages(a, 0, a.size(), block);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/bitwise_convolution_modular.hpp"



#include <algorithm>
#line 7 "components/bitwise_convolution_modular.hpp"
#include <type_traits>
#line 1 "components/mint_arithmetic.hpp"



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
	[[gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[gnu::always_inline]] unsigned val() const {
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
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
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
	[[gnu::always_inline]] Mint operator+() const { return *this; }
	[[gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
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


#line 5 "components/mint_arithmetic.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
struct Ops {
	using T = mint;
	typedef unsigned int U __attribute__((may_alias));
	static constexpr U mod = 998244353, twice = 2 * mod, one = 301989884;
	static U reduce(unsigned long long x) { return T::reduce(x); }
	static U mul(U a, U b) { return reduce((unsigned long long)a * b); }
	static U add(U a, U b) {
		U c = a + b;
		return c < twice ? c : c - twice;
	}
	static U sub(U a, U b) {
		U c = a - b;
		return c < twice ? c : c + twice;
	}
	static U in(U a) { return mul(a, 932051910); }
	static U out(U a) {
		a = reduce(a);
		return a < mod ? a : a - mod;
	}
	static U quotient(U r) { return ((unsigned long long)r << 32) / mod; }
	static U fixed_mul(U a, U r, U q) {
		return a * r - U(((unsigned long long)a * q) >> 32) * mod;
	}

#if defined(__x86_64__) || defined(__i386__)
	static bool vectorized() {
#if defined(__AVX2__)
		return true;
#else
		static const bool available = __builtin_cpu_supports("avx2");
		return available;
#endif
	}
	using I8 = __m256i;
	__attribute__((target("avx2"))) static I8 load8(const U *a) {
		return _mm256_loadu_si256(reinterpret_cast<const I8 *>(a));
	}
	__attribute__((target("avx2"))) static void store8(U *a, I8 x) {
		_mm256_storeu_si256(reinterpret_cast<I8 *>(a), x);
	}
	__attribute__((target("avx2"), always_inline)) static inline I8 reduce8(I8 x, I8 y) {
		I8 r = _mm256_set1_epi32(998244351), p = _mm256_set1_epi32(mod);
		x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, r), p));
		y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, r), p));
		return _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
	}
	__attribute__((target("avx2"))) static I8 mul8(I8 a, I8 b) {
		return reduce8(_mm256_mul_epu32(a, b), _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
	}
	__attribute__((target("avx2"))) static I8 fixed_mul8(I8 a, I8 r, I8 q) {
		I8 x = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		I8 y = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		I8 t = _mm256_blend_epi32(x, y, 0xaa);
		return _mm256_sub_epi32(_mm256_mullo_epi32(a, r),
								_mm256_mullo_epi32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 fixed_even8(I8 a, I8 r, I8 q) {
		I8 t = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		return _mm256_sub_epi64(_mm256_mul_epu32(a, r),
								_mm256_mul_epu32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 add8(I8 a, I8 b) {
		I8 x = _mm256_add_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(twice)));
	}
	__attribute__((target("avx2"))) static I8 sub8(I8 a, I8 b) {
		I8 x = _mm256_sub_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_add_epi32(x, _mm256_set1_epi32(twice)));
	}

#endif
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 9 "components/bitwise_convolution_modular.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace bitwise_convolution_detail {
struct Modular : mint_detail::Ops {
	static_assert(sizeof(mint) == sizeof(U) && std::is_trivially_copyable_v<mint>);
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static I8 load8(const mint *a) {
		return mint_detail::Ops::load8(reinterpret_cast<const U *>(a));
	}
	__attribute__((target("avx2"))) static void store8(mint *a, I8 value) {
		mint_detail::Ops::store8(reinterpret_cast<U *>(a), value);
	}
	template <int kind, bool inverse>
	__attribute__((target("avx2"))) static void pair(I8 &x, I8 &y) {
		if constexpr (kind == 2) {
			I8 old = x;
			x = add8(x, y), y = sub8(old, y);
		} else if constexpr (kind == 0) x = inverse ? sub8(x, y) : add8(x, y);
		else y = inverse ? sub8(y, x) : add8(y, x);
	}
	template <int kind, bool inverse, int mask>
	__attribute__((target("avx2"))) static I8 lanes(I8 x, I8 y) {
		if constexpr (kind == 2) return _mm256_blend_epi32(add8(x, y), sub8(y, x), mask);
		else return _mm256_blend_epi32(x, inverse ? sub8(x, y) : add8(x, y), mask);
	}
	template <int kind, bool inverse>
	__attribute__((target("avx2"))) static void layers(mint *a, std::size_t start, std::size_t n, std::size_t half) {
		for (; half < n / 4; half *= 8)
			for (std::size_t block = start; block < start + n; block += 8 * half)
				for (std::size_t i = block; i < block + half; i += 8) {
					I8 x[8];
#pragma GCC unroll 8
					for (int j = 0; j < 8; ++j) x[j] = load8(a + i + j * half);
#pragma GCC unroll 3
					for (int h = 1; h <= 4; h *= 2)
#pragma GCC unroll 8
						for (int j = 0; j < 8; ++j) if (!(j & h)) pair<kind, inverse>(x[j], x[j + h]);
#pragma GCC unroll 8
					for (int j = 0; j < 8; ++j) store8(a + i + j * half, x[j]);
				}
		for (; half < n / 2; half *= 4)
			for (std::size_t block = start; block < start + n; block += 4 * half)
				for (std::size_t i = block; i < block + half; i += 8) {
					I8 x = load8(a + i), y = load8(a + i + half), z = load8(a + i + 2 * half), w = load8(a + i + 3 * half);
					pair<kind, inverse>(x, y), pair<kind, inverse>(z, w);
					pair<kind, inverse>(x, z), pair<kind, inverse>(y, w);
					store8(a + i, x), store8(a + i + half, y), store8(a + i + 2 * half, z), store8(a + i + 3 * half, w);
				}
		if (half < n)
			for (std::size_t i = start; i < start + half; i += 8) {
				I8 x = load8(a + i), y = load8(a + i + half);
				pair<kind, inverse>(x, y);
				store8(a + i, x), store8(a + i + half, y);
			}
	}
	template <int kind, bool inverse>
	__attribute__((target("avx2"))) static void transform(mint *a, std::size_t n) {
		constexpr std::size_t block = 1024;
		for (std::size_t start = 0; start < n; start += block) {
			std::size_t size = std::min(block, n - start);
			for (std::size_t i = start; i < start + size; i += 8) {
				I8 x = load8(a + i);
				x = lanes<kind, inverse, kind == 0 ? 0x55 : 0xaa>(x, _mm256_shuffle_epi32(x, 0xb1));
				x = lanes<kind, inverse, kind == 0 ? 0x33 : 0xcc>(x, _mm256_shuffle_epi32(x, 0x4e));
				x = lanes<kind, inverse, kind == 0 ? 0x0f : 0xf0>(x, _mm256_permute2x128_si256(x, x, 1));
				store8(a + i, x);
			}
			layers<kind, inverse>(a, start, size, 8);
		}
		if (n > block) layers<kind, inverse>(a, 0, n, block);
	}
	template <int kind>
	__attribute__((target("avx2"))) static void multiply(std::vector<mint> &a, std::vector<mint> &b) {
		std::size_t n = a.size();
		auto *x = a.data(), *y = b.data();
		transform<kind, false>(x, n), transform<kind, false>(y, n);
		I8 factor{};
		if constexpr (kind == 2) factor = _mm256_set1_epi32(mint(n).inv().v);
		for (std::size_t i = 0; i < n; i += 8) {
			I8 value = mul8(load8(x + i), load8(y + i));
			if constexpr (kind == 2) value = mul8(value, factor);
			store8(x + i, value);
		}
		transform<kind, true>(x, n);
	}
#endif
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 12 "convolution/bitwise_convolution.hpp"
#include <cstdint>
#include <limits>
#include <utility>
#line 16 "convolution/bitwise_convolution.hpp"

namespace bitwise_convolution_detail {
template <int kind, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
	a.resize(n, T(0));
	b.resize(n, T(0));
#if (defined(__x86_64__) || defined(__i386__)) && !defined(BITWISE_CONVOLUTION_DISABLE_AVX2)
	if constexpr (std::is_same_v<T, mint>) {
		if (n >= 8 && Modular::vectorized()) {
			Modular::multiply<kind>(a, b);
			return a;
		}
	}
#endif
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		walsh_hadamard(b);
	} else {
		bitwise_zeta<false, kind == 0>(a);
		bitwise_zeta<false, kind == 0>(b);
	}
	for (std::size_t i = 0; i < n; ++i) a[i] *= b[i];
	if constexpr (kind == 2) {
		walsh_hadamard(a);
		if constexpr (std::numeric_limits<T>::is_integer)
			for (T &x : a) x /= std::int64_t(n);
		else {
			T inverse = T(1) / T(n);
			for (T &x : a) x *= inverse;
		}
	} else bitwise_zeta<true, kind == 0>(a);
	return a;
}
}

template <class T>
std::vector<T> and_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<0>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> or_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<1>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> xor_convolution(std::vector<T> a, std::vector<T> b) {
	return bitwise_convolution_detail::multiply<2>(std::move(a), std::move(b));
}


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
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


#line 6 "components/fast_io_bulk.hpp"
#include <span>
#line 8 "components/fast_io_bulk.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		if (a < 1 || a > 10 || b <= a + 1 || b - a - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		const auto window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	static std::uint32_t one(const char *&p, bool &bad) {
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		bad |= value > 0xffffffffULL;
		p += n + 1;
		return std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				const auto g0 = two(p[0], invalid), g1 = two(p[1], invalid), g2 = two(p[2], invalid), g3 = two(p[3], invalid);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, _mm256_set1_epi32(42)),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, _mm256_set1_epi32(42)), _mm256_cmpgt_epi32(low, _mm256_set1_epi32(94967295))));
				invalid = _mm256_or_si256(invalid, overflow);
				const auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif

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

namespace fast_io_detail {
template <class Emit, std::size_t... I>
void emit_record(Emit &emit, const std::uint32_t *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 10 "test/bitwise_xor_convolution.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int exponent = 0;
	input.read(exponent);
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto *values : {&a, &b}) {
		std::size_t offset = 0;
		read_unsigned_blocks(input, n, [&](const std::uint32_t *block, std::size_t count) {
			for (std::size_t i = 0; i < count; ++i) (*values)[offset + i] = mint::raw(block[i]);
			offset += count;
		});
	}
	auto result = xor_convolution(std::move(a), std::move(b));
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array<std::uint32_t, 1>{result[i].val()};
	});
}
