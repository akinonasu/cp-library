#line 1 "test/subset_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/subset_convolution"
#line 1 "convolution/subset_convolution.hpp"



#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>
#line 1 "components/subset_convolution_modular.hpp"



#line 6 "components/subset_convolution_modular.hpp"
#include <cstdint>
#line 8 "components/subset_convolution_modular.hpp"
#include <type_traits>
#include <utility>
#line 11 "components/subset_convolution_modular.hpp"
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace subset_convolution_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<const T &>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 30) && (T::mod() & 1))
		&& std::is_convertible_v<decltype(std::declval<const T &>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(SUBSET_CONVOLUTION_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template <std::uint32_t mod>
struct Modular {
	using V = std::uint32_t;
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V sub(V a, V b) { return a >= b ? a - b : a + mod - b; }
	static constexpr int width = 24;
	__attribute__((target("avx2"))) static void transform(V *a, int n) {
		const __m256i p = _mm256_set1_epi32(mod);
		if (n >= 128) {
			int q = n / 4;
			for (int i = 0; i < q; ++i)
				for (int rank = 0; rank < width; rank += 8) {
					auto *p0 = reinterpret_cast<__m256i *>(a + std::size_t(i) * width + rank);
					auto *p1 = reinterpret_cast<__m256i *>(a + std::size_t(i + q) * width + rank);
					auto *p2 = reinterpret_cast<__m256i *>(a + std::size_t(i + q * 2) * width + rank);
					auto *p3 = reinterpret_cast<__m256i *>(a + std::size_t(i + q * 3) * width + rank);
					__m256i x0 = _mm256_loadu_si256(p0), x1 = _mm256_loadu_si256(p1), x2 = _mm256_loadu_si256(p2), x3 = _mm256_loadu_si256(p3);
					__m256i u = _mm256_add_epi32(x0, x2), v = _mm256_add_epi32(_mm256_sub_epi32(x0, x2), p);
					x0 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x2 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x1, x3);
					v = _mm256_add_epi32(_mm256_sub_epi32(x1, x3), p);
					x1 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x3 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x0, x1);
					v = _mm256_add_epi32(_mm256_sub_epi32(x0, x1), p);
					x0 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x1 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					u = _mm256_add_epi32(x2, x3);
					v = _mm256_add_epi32(_mm256_sub_epi32(x2, x3), p);
					x2 = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
					x3 = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
					_mm256_storeu_si256(p0, x0);
					_mm256_storeu_si256(p1, x1);
					_mm256_storeu_si256(p2, x2);
					_mm256_storeu_si256(p3, x3);
				}
			for (int i = 0; i < 4; ++i) transform(a + std::size_t(i) * q * width, q);
			return;
		}
		for (int half = 1; half < n; half *= 2)
			for (int start = 0; start < n; start += 2 * half)
				for (int j = 0; j < half; ++j)
					for (int rank = 0; rank < width; rank += 8) {
						auto *left = reinterpret_cast<__m256i *>(a + std::size_t(start + j) * width + rank);
						auto *right = reinterpret_cast<__m256i *>(a + std::size_t(start + half + j) * width + rank);
						__m256i x = _mm256_loadu_si256(left), y = _mm256_loadu_si256(right);
						__m256i u = _mm256_add_epi32(x, y), v = _mm256_add_epi32(_mm256_sub_epi32(x, y), p);
						_mm256_storeu_si256(left, _mm256_min_epu32(u, _mm256_sub_epi32(u, p)));
						_mm256_storeu_si256(right, _mm256_min_epu32(v, _mm256_sub_epi32(v, p)));
					}
	}
	static constexpr V inverse = [] {
		V x = 1;
		for (int i = 0; i < 5; ++i) x *= 2 - mod * x;
		return V(0) - x;
	}();
	__attribute__((target("avx2"))) static __m256i reduce(__m256i x) {
		const __m256i p = _mm256_set1_epi64x(mod), inv = _mm256_set1_epi64x(inverse);
		__m256i q = _mm256_mul_epu32(x, inv);
		x = _mm256_srli_epi64(_mm256_add_epi64(x, _mm256_mul_epu32(q, p)), 32);
		x = _mm256_min_epu32(x, _mm256_sub_epi64(x, p));
		return _mm256_min_epu32(x, _mm256_sub_epi64(x, p));
	}
	__attribute__((target("avx2"))) static void product(V *a, const V *b, int n) {
		constexpr V r2 = V((~std::uint64_t(0) % mod + 1) % mod);
		const __m256i p = _mm256_set1_epi64x(mod), scale = _mm256_set1_epi64x(r2);
		for (int pos = 0; pos < n; pos += 4) {
			__m256i x[20], y[20], out[20];
			for (int rank = 0; rank < 20; rank += 4) {
				for (int side = 0; side < 2; ++side) {
					const V *v = (side ? b : a) + std::size_t(pos) * width + rank;
					__m128i a0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v));
					__m128i a1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width));
					__m128i a2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width * 2));
					__m128i a3 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(v + width * 3));
					__m128i u = _mm_unpacklo_epi32(a0, a1), v0 = _mm_unpackhi_epi32(a0, a1), w = _mm_unpacklo_epi32(a2, a3), z = _mm_unpackhi_epi32(a2, a3);
					auto *dest = side ? y : x;
					dest[rank] = _mm256_cvtepu32_epi64(_mm_unpacklo_epi64(u, w));
					dest[rank + 1] = _mm256_cvtepu32_epi64(_mm_unpackhi_epi64(u, w));
					dest[rank + 2] = _mm256_cvtepu32_epi64(_mm_unpacklo_epi64(v0, z));
					dest[rank + 3] = _mm256_cvtepu32_epi64(_mm_unpackhi_epi64(v0, z));
				}
			}
			out[0] = _mm256_setzero_si256();
#pragma GCC unroll 20
			for (int rank = 1; rank < 20; ++rank) {
				__m256i value = _mm256_setzero_si256();
				for (int first = 0; first < rank; first += 8) {
					__m256i sum = _mm256_setzero_si256();
					for (int i = first; i < std::min(rank, first + 8); ++i)
						sum = _mm256_add_epi64(sum, _mm256_mul_epu32(x[i], y[rank - i - 1]));
					value = _mm256_add_epi64(value, reduce(sum));
					value = _mm256_min_epu32(value, _mm256_sub_epi64(value, p));
				}
				out[rank] = reduce(_mm256_mul_epu32(value, scale));
			}
			for (int rank = 0; rank < 20; rank += 4) {
				__m128i z[4];
				for (int j = 0; j < 4; ++j) z[j] = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(out[rank + j], _mm256_setr_epi32(0, 2, 4, 6, 0, 2, 4, 6)));
				__m128i u = _mm_unpacklo_epi32(z[0], z[1]), v = _mm_unpackhi_epi32(z[0], z[1]), w = _mm_unpacklo_epi32(z[2], z[3]), zz = _mm_unpackhi_epi32(z[2], z[3]);
				V *dest = a + std::size_t(pos) * width + rank;
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest), _mm_unpacklo_epi64(u, w));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width), _mm_unpackhi_epi64(u, w));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width * 2), _mm_unpacklo_epi64(v, zz));
				_mm_storeu_si128(reinterpret_cast<__m128i *>(dest + width * 3), _mm_unpackhi_epi64(v, zz));
			}
		}
	}
	template <class T>
	__attribute__((target("avx2"))) static std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b, int n) {
		int levels = std::bit_width(unsigned(n)) - 1;
		int bottom = n >> (std::min(levels - 3, 2) + 1);
		std::vector<V> x(std::size_t(bottom) * width), y(x.size()), counts(n), result(n);
		std::vector<V> aa(n), bb(n);
		for (int i = 0; i < int(a.size()); ++i) aa[i] = a[i].val();
		for (int i = 0; i < int(b.size()); ++i) bb[i] = b[i].val();
		for (int i = 1; i < n; ++i) counts[i] = std::popcount(unsigned(i)) - 1;
		for (int top = 0; top < n / 2; top += bottom) {
			std::fill(x.begin(), x.end(), 0);
			std::fill(y.begin(), y.end(), 0);
			for (int start = 0; start < n; start += 2 * bottom) {
				bool negative = std::popcount(unsigned((start >> 1) & top)) & 1;
				for (int mask = start == 0; mask < 2 * bottom; ++mask) {
					std::size_t index = std::size_t(mask >> 1) * width + counts[start + mask];
					if (negative) x[index] = sub(x[index], aa[start + mask]), y[index] = sub(y[index], bb[start + mask]);
					else x[index] = add(x[index], aa[start + mask]), y[index] = add(y[index], bb[start + mask]);
				}
			}
			transform(x.data(), bottom);
			transform(y.data(), bottom);
			product(x.data(), y.data(), bottom);
			transform(x.data(), bottom);
			for (int start = 0; start < n; start += 2 * bottom) {
				bool negative = std::popcount(unsigned((start >> 1) & top)) & 1;
				for (int mask = start == 0; mask < 2 * bottom; ++mask) {
					V value = x[std::size_t(mask >> 1) * width + counts[start + mask]];
					result[start + mask] = negative ? sub(result[start + mask], value) : add(result[start + mask], value);
				}
			}
		}
		V scale = 1;
		for (int i = 1; i < levels; ++i) scale = std::uint64_t(scale) * ((mod + 1) / 2) % mod;
		std::vector<T> out(n, T(0));
		out[0] = T::raw(std::uint64_t(aa[0]) * bb[0] % mod);
		for (int i = 1; i < n; ++i) out[i] = T::raw((std::uint64_t(result[i]) * scale + std::uint64_t(aa[i]) * bb[0] + std::uint64_t(aa[0]) * bb[i]) % mod);
		return out;
	}
};
#endif
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 9 "convolution/subset_convolution.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class T>
std::vector<T> subset_convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::size_t n = std::bit_ceil(std::max(a.size(), b.size()));
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if constexpr (subset_convolution_detail::SmallModint<T>::value)
		if (n >= 256 && n <= (std::size_t(1) << 20) && subset_convolution_detail::avx2())
			return subset_convolution_detail::Modular<T::mod()>::convolution(a, b, int(n));
#endif
	int levels = std::bit_width(n) - 1, width = levels + 1;
	std::vector<T> x(n * width, T(0)), y(n * width, T(0)), result(n, T(0));
	for (std::size_t i = 0; i < a.size(); ++i) x[i * width + std::popcount(i)] = a[i];
	for (std::size_t i = 0; i < b.size(); ++i) y[i * width + std::popcount(i)] = b[i];
	auto zeta = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start);
				for (std::size_t j = start; j < start + half; ++j)
					for (int k = low, end = std::popcount(j); k <= end; ++k) {
						x[(j + half) * width + k] += x[j * width + k];
						y[(j + half) * width + k] += y[j * width + k];
					}
			}
	};
	std::size_t block = std::min<std::size_t>(n, 256);
	for (std::size_t start = 0; start < n; start += block) zeta(start, block, 1);
	zeta(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) {
		int count = std::popcount(mask);
		for (int k = std::min(levels, 2 * count); k >= count; --k) {
			T sum(0);
			for (int i = k - count; i <= count; ++i)
				sum += x[mask * width + i] * y[mask * width + k - i];
			x[mask * width + k] = sum;
		}
	}
	auto mobius = [&](std::size_t first, std::size_t length, std::size_t half) {
		for (; half < length; half *= 2)
			for (std::size_t start = first; start < first + length; start += 2 * half) {
				int low = std::popcount(start) + 1, high = levels - std::countr_zero(half);
				for (std::size_t j = start; j < start + half; ++j) {
					int count = std::popcount(j - start);
					for (int k = low + count; k <= high + count; ++k)
						x[(j + half) * width + k] -= x[j * width + k];
				}
			}
	};
	for (std::size_t start = 0; start < n; start += block) mobius(start, block, 1);
	mobius(0, n, block);
	for (std::size_t mask = 0; mask < n; ++mask) result[mask] = x[mask * width + std::popcount(mask)];
	return result;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "math/mint.hpp"


#include <cassert>
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


#line 6 "test/subset_convolution.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int exponent = 0;
	input.read(exponent);
	int n = 1 << exponent;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	for (auto &x : b) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	for (auto x : subset_convolution(a, b)) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}
