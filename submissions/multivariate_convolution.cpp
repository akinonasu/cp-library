#line 1 "test/multivariate_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/multivariate_convolution"
#line 1 "convolution/multivariate_convolution.hpp"



#include <cassert>
#include <cstddef>
#include <vector>
#line 1 "components/multivariate_convolution_core.hpp"



#include <algorithm>
#include <bit>
#line 1 "components/mint_arithmetic.hpp"



#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#include <type_traits>

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


#line 9 "components/multivariate_convolution_core.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_convolution_detail {
struct Core : mint_detail::Ops {
	static int modulus(int k) {
		for (int m : {1, 2, 4, 7, 8, 14, 16, 17, 28, 32})
			if (m >= k) return m;
		return int(std::bit_ceil(unsigned(k)));
	}
	static bool available() {
#if (defined(__x86_64__) || defined(__i386__)) && !defined(MULTIVARIATE_CONVOLUTION_DISABLE_AVX2)
		return vectorized();
#else
		return false;
#endif
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void row8(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int row, const std::vector<int> &parts) {
		const auto p = _mm256_set1_epi32(mod), r = _mm256_set1_epi32(998244351), two = _mm256_set1_epi32(twice);
		for (std::size_t i = 0; i < out.size(); i += 8) {
			auto sum = _mm256_setzero_si256();
			for (std::size_t j = 0; j < parts.size(); j += 8) {
				auto even = _mm256_setzero_si256(), odd = even;
				for (std::size_t k = j; k < std::min(j + 8, parts.size()); ++k) {
					auto x = load8(reinterpret_cast<const U *>(a[parts[k]].data() + i));
					auto y = load8(reinterpret_cast<const U *>(b[row - parts[k]].data() + i));
					x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
					y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
					even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
					odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
				}
				even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, r), p));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, r), p));
				auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
				value = _mm256_min_epu32(value, _mm256_sub_epi32(value, two));
				sum = add8(sum, value);
			}
			store8(reinterpret_cast<U *>(out.data() + i), sum);
		}
	}
#endif
	static void row(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int index, const std::vector<int> &parts) {
#if defined(__x86_64__) || defined(__i386__)
		if (out.size() >= 8 && available()) return row8(out, a, b, index, parts);
#endif
		std::fill(out.begin(), out.end(), mint(0));
		for (int p : parts)
			for (std::size_t i = 0; i < out.size(); ++i) out[i] += a[p][i] * b[index - p][i];
	}
	template <class Transform, class Product>
	static std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b, std::vector<int> dims, Transform transform, Product product) {
		int n = a.size();
		std::vector<int> order(dims.size());
		for (int j = 0; j < int(order.size()); ++j) order[j] = j;
		std::stable_sort(order.begin(), order.end(), [&](int x, int y) { return dims[x] > dims[y]; });
		std::vector<int> sorted;
		for (int j : order) sorted.push_back(dims[j]);
		int split = sorted.size(), inner = n, outer = 1;
		double best = 1e300, pairs = 1;
		for (int k = int(sorted.size()), q = 1, size = n; k; size /= sorted[--k], q *= sorted[k]) {
			int length = std::bit_ceil(unsigned(2 * size - 1));
			double cost = modulus(k) * length * (3.0 * q * std::bit_width(unsigned(length)) + pairs * 5.0);
			if (cost < best) best = cost, split = k, inner = size, outer = q;
			pairs *= sorted[k - 1] * (sorted[k - 1] + 1.0) / 2;
			if (q > 128 / sorted[k - 1]) break;
		}
		std::vector<int> target(dims.size());
		for (int j = 0, s = 1; j < int(dims.size()); s *= sorted[j++]) target[order[j]] = s;
		std::vector<int> permutation(n), digit(dims.size());
		for (int i = 0, p = 0; i < n; ++i) {
			permutation[i] = p;
			for (int j = 0; j < int(dims.size()); ++j) {
				p += target[j];
				if (++digit[j] < dims[j]) break;
				digit[j] = 0, p -= target[j] * dims[j];
			}
		}
		std::vector<mint> f(n), g(n), result(n), output(n);
		for (int i = 0; i < n; ++i) f[permutation[i]] = a[i], g[permutation[i]] = b[i];
		int m = modulus(split), length = std::bit_ceil(unsigned(2 * inner - 1));
		std::vector<unsigned char> ranks(inner);
		for (int base = sorted[0], j = 1; j < split; base *= sorted[j++])
			for (int i = 0; i < inner; ++i) ranks[i] = (ranks[i] + i / base) % m;
		std::vector<std::vector<int>> parts(outer);
		for (int e = 0; e < outer; ++e)
			for (int x = 0; x <= e; ++x) {
				bool valid = true;
				for (int j = split, u = x, v = e; j < int(sorted.size()); u /= sorted[j], v /= sorted[j++])
					if (u % sorted[j] > v % sorted[j]) { valid = false; break; }
				if (valid) parts[e].push_back(x);
			}
		std::vector<std::vector<mint>> x(outer, std::vector<mint>(outer == 1 ? inner : length)), y = x;
		std::vector<mint> z(length), weights(m), inverse(m);
		mint root = mint(3).pow((mod - 1) / m), scale = mint(m).inv();
		for (int s = 0; s < m; ++s) {
			mint step = root.pow(s), back = step.inv();
			weights[0] = 1, inverse[0] = scale;
			for (int j = 1; j < m; ++j) weights[j] = weights[j - 1] * step, inverse[j] = inverse[j - 1] * back;
			for (int row = 0; row < outer; ++row) {
				std::fill(x[row].begin() + inner, x[row].end(), mint(0));
				std::fill(y[row].begin() + inner, y[row].end(), mint(0));
				for (int i = 0; i < inner; ++i) x[row][i] = f[row * inner + i] * weights[ranks[i]], y[row][i] = g[row * inner + i] * weights[ranks[i]];
				if (outer != 1) transform(x[row], false), transform(y[row], false);
			}
			for (int row = outer; row-- > 0;) {
				if (outer == 1) z = product(x[0], y[0]);
				else Core::row(z, x, y, row, parts[row]), transform(z, true);
				for (int i = 0; i < inner; ++i) result[row * inner + i] += z[i] * inverse[ranks[i]];
			}
		}
		for (int i = 0; i < n; ++i) output[i] = result[permutation[i]];
		return output;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/multivariate_ranked.hpp"



#line 5 "components/multivariate_ranked.hpp"
#include <array>
#line 7 "components/multivariate_ranked.hpp"
#include <cstdint>
#include <cstring>
#include <iterator>
#include <ranges>
#line 13 "components/multivariate_ranked.hpp"

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_ranked_detail {
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using Vec = __m256i;
constexpr u32 kP = 998244353;
constexpr u32 kPInverse = [] {
	u32 x = kP;
	for (int i = 0; i < 5; ++i) x *= 2 - kP * x;
	return x;
}();
constexpr u32 k2To32 = u32((u64(1) << 32) % kP);
constexpr u32 multiply_mod(u32 a, u32 b) { return u32(u64(a) * b % kP); }
constexpr u32 power_mod(u32 a, u64 e) {
	u32 r = 1;
	for (; e; e >>= 1, a = multiply_mod(a, a))
		if (e & 1) r = multiply_mod(r, a);
	return r;
}
constexpr u32 inverse_mod(u32 a) { return power_mod(a, kP - 2); }
constexpr u32 montgomery(u32 a) { return multiply_mod(a, k2To32); }
__attribute__((target("avx2"))) inline Vec all(u32 x) { return _mm256_set1_epi32(int(x)); }
__attribute__((target("avx2"))) inline Vec load(const void* p) { return _mm256_loadu_si256(static_cast<const Vec*>(p)); }
__attribute__((target("avx2"))) inline void store(void* p, Vec x) { _mm256_storeu_si256(static_cast<Vec*>(p), x); }
__attribute__((target("avx2"))) inline Vec add_mod(Vec x, Vec y) {
	const Vec s = _mm256_add_epi32(x, y);
	return _mm256_min_epu32(s, _mm256_sub_epi32(s, all(kP)));
}
__attribute__((target("avx2"))) inline Vec sub_mod(Vec x, Vec y) {
	const Vec d = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(d, _mm256_add_epi32(d, all(kP)));
}
template <bool Wide>
__attribute__((target("avx2"))) inline Vec reduce(Vec even, Vec odd) {
	Vec high = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
	if constexpr (Wide) {
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(2 * kP)));
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(2 * kP)));
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(kP)));
	}
	const Vec qe = _mm256_mul_epu32(even, all(kPInverse)), qo = _mm256_mul_epu32(odd, all(kPInverse));
	const Vec me = _mm256_mul_epu32(qe, all(kP)), mo = _mm256_mul_epu32(qo, all(kP));
	const Vec r = _mm256_sub_epi32(high, _mm256_blend_epi32(_mm256_srli_epi64(me, 32), mo, 0xAA));
	return _mm256_min_epu32(r, _mm256_add_epi32(r, all(kP)));
}
__attribute__((target("avx2"))) inline Vec fold(Vec s) {
	return _mm256_add_epi64(_mm256_and_si256(s, _mm256_set1_epi64x(0xFFFFFFFF)),
		_mm256_mul_epu32(_mm256_srli_epi64(s, 32), _mm256_set1_epi64x(k2To32)));
}
__attribute__((target("avx2"))) inline Vec montgomery_multiply(Vec x, Vec y) {
	const Vec even = _mm256_mul_epu32(x, y);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
	return reduce<false>(even, odd);
}
struct Buffer {
	Vec* data;
	explicit Buffer(std::size_t n) : data(new Vec[n]()) {}
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	~Buffer() { delete[] data; }
};
constexpr int kMaxRank = 24;
__attribute__((target("avx2"))) inline Vec by_popcount(const Vec* x) {
	const Vec low = _mm256_blend_epi32(x[0], x[1], 0x16);
	return _mm256_blend_epi32(_mm256_blend_epi32(low, x[2], 0x68), x[3], 0x80);
}
template <bool Inverse>
__attribute__((target("avx2"))) inline Vec lanes_transform(Vec x) {
	if constexpr (Inverse) {
		x = sub_mod(x, _mm256_slli_epi64(x, 32));
		x = sub_mod(x, _mm256_slli_si256(x, 8));
		return sub_mod(x, _mm256_permute2x128_si256(x, x, 0x08));
	} else {
		x = add_mod(x, _mm256_slli_epi64(x, 32));
		x = add_mod(x, _mm256_slli_si256(x, 8));
		return add_mod(x, _mm256_permute2x128_si256(x, x, 0x08));
	}
}
template <bool Inverse, int N>
__attribute__((target("avx2"))) inline void evaluate_line(Vec* v, int stride) {
	Vec &a0 = v[0], &a1 = v[stride];
	if constexpr (N == 2) {
		a1 = Inverse ? sub_mod(a1, a0) : add_mod(a1, a0);
	} else if constexpr (!Inverse) {
		Vec& a2 = v[2 * stride];
		const Vec s = add_mod(a0, a2);
		a2 = sub_mod(s, a1), a1 = add_mod(s, a1);
	} else {
		Vec& a2 = v[2 * stride];
		const Vec f0 = add_mod(a0, a0), f1 = a1, f2 = a2;
		a0 = f0, a1 = sub_mod(f1, f2), a2 = sub_mod(add_mod(f1, f2), f0);
	}
}
template <bool Inverse, int A, int B>
__attribute__((target("avx2"))) inline void variable_step(Vec* p, std::size_t sa, std::size_t sb, int first, int last) {
	for (int r = first; r <= last; ++r) {
		Vec v[A * B];
#pragma GCC unroll 9
		for (int j = 0; j < B; ++j)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) v[i + A * j] = p[r + i * sa + j * sb];
#pragma GCC unroll 3
		for (int j = 0; j < B; ++j) evaluate_line<Inverse, A>(v + A * j, 1);
		if constexpr (B > 1)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) evaluate_line<Inverse, B>(v + i, A);
#pragma GCC unroll 9
		for (int j = 0; j < B; ++j)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) p[r + i * sa + j * sb] = v[i + A * j];
	}
}
__attribute__((target("avx2"))) inline void add_row(Vec* dst, const Vec* src, std::size_t count, bool negative, bool first) {
	const Vec zero = _mm256_setzero_si256();
	if (first && negative)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, sub_mod(zero, load(src + i)));
	else if (first)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, load(src + i));
	else if (negative)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, sub_mod(load(dst + i), load(src + i)));
	else
		for (std::size_t i = 0; i < count; ++i) store(dst + i, add_mod(load(dst + i), load(src + i)));
}
struct Term {
	u32 row;
	bool negative;
	std::uint8_t rank;
	bool first = false;
};
class Ranked {
public:
	Ranked(const std::vector<u32>& n, std::size_t size) : size_(size) {
		const std::size_t k = n.size();
		std::vector<u32> order;
		for (std::size_t l = 0; l < k && order.size() < 3; ++l)
			if (n[l] == 2) order.push_back(u32(l));
		for (std::size_t l = 0; l < k; ++l)
			if (std::ranges::find(order, u32(l)) == order.end()) order.push_back(u32(l));
		for (u32 l : order) size_of_.push_back(n[l]);
		for (u32 x : n) rank_ += int(x) - 1, threes_ += x == 3;
		bottom_ = 3, bottom_size_ = 8;
		while (bottom_ < k && 8 * std::size_t(rank_ + 1) * bottom_size_ * size_of_[bottom_] <= (640 << 10))
			bottom_size_ *= size_of_[bottom_++];
		top_size_ = size_ / bottom_size_;
		int top_rank = 0;
		for (std::size_t l = bottom_; l < k; ++l) top_rank += int(size_of_[l]) - 1;
		planes_ = top_rank + 1;
		if (!std::ranges::is_sorted(order)) {
			std::vector<u32> stride(k + 1);
			for (std::size_t l = 0, s = 1; l <= k; s *= l < k ? n[l] : 1, ++l) stride[l] = u32(s);
			for (int w = 0; w < 8; ++w)
				lane_offset_[w] = (w & 1 ? stride[order[0]] : 0) + (w & 2 ? stride[order[1]] : 0) + (w & 4 ? stride[order[2]] : 0);
			std::vector<u32> base{0};
			for (u32 l = 0; l < order[2]; ++l) {
				if (l == order[0] || l == order[1]) continue;
				const std::size_t count = base.size();
				for (u32 d = 1; d < n[l]; ++d)
					for (std::size_t i = 0; i < count; ++i) base.push_back(base[i] + d * stride[l]);
			}
			block_base_ = std::move(base);
		}
		describe_vectors();
		describe_ranges();
		describe_top();
	}
	__attribute__((target("avx2"))) void multiply(u32* f, u32* g, u32* c) {
		std::vector<u32> scratch_storage(reordered() ? 8 * block_base_.size() : 0);
		u32* scratch = scratch_storage.data();
		to_our_order(f, scratch);
		to_our_order(g, scratch);
		const u32 *fo = f, *go = g;
		u32* co = c;
		const std::size_t vectors = bottom_size_ / 8, plane = std::size_t(planes_) * vectors;
		const std::size_t rows = vectors / group_;
		Buffer top_f_storage(plane), top_g_storage(plane), a_storage(std::size_t(rank_ + 1) * vectors), b_storage(std::size_t(rank_ + 1) * vectors);
		Vec *top_f = top_f_storage.data, *top_g = top_g_storage.data, *a = a_storage.data, *b = b_storage.data;
		for (std::size_t t = 0; t < top_size_; ++t) {
			evaluate_top(fo, t, top_f);
			evaluate_top(go, t, top_g);
			if (const int z = top_zeros3_[t]) {
				const Vec factor = all(montgomery(power_mod(2, z)));
				for (std::size_t i = 0; i < plane; ++i) top_f[i] = montgomery_multiply(top_f[i], factor);
			}
			const int forward_extra = top_cap_[t] + 3, inverse_extra = planes_ + 2;
			for (std::size_t s = 0; s < vectors; s += group_) {
				spread(top_f, a, s);
				steps<false>(a, forward_, 0, forward_.split, s, 1, group_, forward_extra);
				spread(top_g, b, s);
				steps<false>(b, forward_, 0, forward_.split, s, 1, group_, forward_extra);
			}
			for (std::size_t s = 0; s < group_; ++s) {
				steps<false>(a, forward_, forward_.split, forward_.steps.size(), s, group_, rows, forward_extra);
				steps<false>(b, forward_, forward_.split, forward_.steps.size(), s, group_, rows, forward_extra);
				pointwise(a, b, s, group_, rows, top_cap_[t], top_nz_[t]);
				steps<true>(a, inverse_, 0, inverse_.split, s, group_, rows, inverse_extra);
			}
			for (std::size_t s = 0; s < vectors; s += group_) {
				steps<true>(a, inverse_, inverse_.split, inverse_.steps.size(), s, 1, group_, inverse_extra);
				gather(a, top_f, s);
			}
			interpolate_top(top_f, t, co);
		}
		const Vec scale = all(montgomery(multiply_mod(k2To32, power_mod(inverse_mod(2), threes_))));
		if (!reordered()) {
			for (std::size_t i = 0; i < size_; i += 8) store(c + i, montgomery_multiply(load(c + i), scale));
			return;
		}
		const std::size_t block = 8 * block_base_.size();
		for (std::size_t s = 0; s < size_; s += block) {
			std::memcpy(scratch, c + s, block * sizeof(u32));
			for (std::size_t u = 0; u < block_base_.size(); ++u) {
				alignas(32) u32 x[8];
				store(x, montgomery_multiply(load(scratch + 8 * u), scale));
				for (int w = 0; w < 8; ++w) c[s + block_base_[u] + lane_offset_[w]] = x[w];
			}
		}
	}
private:
	struct Range {
		std::uint8_t first, span;
	};
	struct Step {
		u32 first, width;
		std::size_t stride;
	};
	struct Pass {
		std::vector<Step> steps;
		std::size_t split = 0;
		std::vector<Range> ranges;
	};
	bool reordered() const { return !block_base_.empty(); }
	__attribute__((target("avx2"))) void to_our_order(u32* x, u32* scratch) const {
		if (!reordered()) return;
		const std::size_t block = 8 * block_base_.size();
		const Vec offset = load(lane_offset_.data());
		for (std::size_t s = 0; s < size_; s += block) {
			std::memcpy(scratch, x + s, block * sizeof(u32));
			for (std::size_t u = 0; u < block_base_.size(); ++u)
				store(x + s + 8 * u,
					  _mm256_i32gather_epi32(reinterpret_cast<const int*>(scratch + block_base_[u]), offset, 4));
		}
	}
	void describe_vectors() {
		const std::size_t count = bottom_size_ / 8;
		vector_sum_.resize(count), vector_cap_.resize(count), vector_nz_.resize(count);
		std::vector<u32> digit(bottom_, 0);
		int sum = 0, cap = 0, nz = 0;
		for (std::size_t u = 0; u < count; ++u) {
			vector_sum_[u] = std::uint8_t(sum), vector_cap_[u] = std::uint8_t(cap), vector_nz_[u] = std::uint8_t(nz);
			for (std::size_t l = 3; l < bottom_; ++l) {
				const int n = int(size_of_[l]);
				if (++digit[l] < u32(n)) {
					++sum;
					if (digit[l] == 1) cap += n - 1, ++nz;
					break;
				}
				digit[l] = 0, sum -= n - 1, cap -= n - 1, --nz;
			}
		}
	}
	void describe_ranges() {
		const std::size_t count = bottom_size_ / 8;
		std::size_t boundary = 3;
		for (group_ = 1; group_ * group_ < count; group_ *= size_of_[boundary++]) {
		}
		const auto pairs = [this](std::size_t from, std::size_t to, bool inverse) {
			std::vector<Step> steps;
			for (std::size_t l = from; l < to;) {
				const bool pair = l + 1 < to && (!inverse || (size_of_[l] == 2 && size_of_[l + 1] == 2));
				steps.push_back({u32(l), pair ? 2u : 1u, 0});
				l += pair ? 2 : 1;
			}
			if (inverse) std::ranges::reverse(steps);
			return steps;
		};
		forward_.steps = pairs(3, boundary, false);
		forward_.split = forward_.steps.size();
		std::ranges::copy(pairs(boundary, bottom_, false), std::back_inserter(forward_.steps));
		inverse_.steps = pairs(boundary, bottom_, true);
		inverse_.split = inverse_.steps.size();
		std::ranges::copy(pairs(3, boundary, true), std::back_inserter(inverse_.steps));
		for (const bool inverse : {false, true}) {
			Pass& pass = inverse ? inverse_ : forward_;
			for (Step& step : pass.steps) {
				step.stride = 1;
				for (std::size_t m = 3; m < step.first; ++m) step.stride *= size_of_[m];
			}
			pass.ranges.resize(pass.steps.size() * count);
			std::vector<u32> digit(bottom_, 0);
			for (std::size_t u = 0; u < count; ++u) {
				for (std::size_t k = 0; k < pass.steps.size(); ++k) {
					const std::size_t l = pass.steps[k].first, e = l + pass.steps[k].width - 1;
					int above = 0, cap = 0, own = 0, up_to = 0;
					for (std::size_t m = 3; m < bottom_; ++m) {
						const int d = int(digit[m]), n = int(size_of_[m]);
						if (m > e) above += d;
						if (m < l) cap += d ? n - 1 : 0;
						if (m >= l && m <= e) own += n - 1;
						if (m <= e) up_to += n - 1;
					}
					pass.ranges[k * count + u] = inverse ? Range{std::uint8_t(above), std::uint8_t(up_to)}
														 : Range{std::uint8_t(above), std::uint8_t(own + cap)};
				}
				for (std::size_t l = 3; l < bottom_ && ++digit[l] == size_of_[l]; ++l) digit[l] = 0;
			}
		}
	}
	void describe_top() {
		struct Entry {
			u32 digit;
			bool negative;
		};
		static constexpr Entry kEvaluate[2][3][3] = {
			{{{0, false}}, {{0, false}, {1, false}}, {}},
			{{{0, false}}, {{0, false}, {1, false}, {2, false}}, {{0, false}, {1, true}, {2, false}}}};
		static constexpr Entry kInterpolate[2][3][3] = {
			{{{0, false}, {1, true}}, {{1, false}}, {}},
			{{{0, false}, {2, true}}, {{1, false}, {2, false}}, {{1, true}, {2, false}}}};
		static constexpr int kEvaluateCount[2][3] = {{1, 2, 0}, {1, 3, 3}};
		static constexpr int kInterpolateCount[2][3] = {{2, 1, 0}, {2, 2, 2}};
		const std::size_t k = size_of_.size();
		evaluate_start_.push_back(0), interpolate_start_.push_back(0);
		std::vector<u32> point(k, 0);
		for (std::size_t t = 0; t < top_size_; ++t) {
			std::vector<Term> evaluate{{0, false, 0}}, interpolate{{0, false, 0}};
			int cap = 0, nz = 0, zeros3 = 0;
			u32 stride = 1;
			for (std::size_t l = bottom_; l < k; stride *= size_of_[l++]) {
				const u32 p = point[l];
				const bool three = size_of_[l] == 3;
				if (p) cap += int(size_of_[l]) - 1, ++nz;
				zeros3 += three && p == 0;
				const auto extend = [&](std::vector<Term>& terms, const Entry* table, int count) {
					std::vector<Term> next;
					for (const Term& x : terms)
						for (int e = 0; e < count; ++e) {
							const u32 d = table[e].digit;
							next.push_back({x.row + d * stride, x.negative != table[e].negative, std::uint8_t(x.rank + d)});
						}
					terms = std::move(next);
				};
				extend(evaluate, kEvaluate[three][p], kEvaluateCount[three][p]);
				extend(interpolate, kInterpolate[three][p], kInterpolateCount[three][p]);
			}
			std::vector<bool> seen(planes_, false);
			for (Term& term : evaluate) term.first = !seen[term.rank], seen[term.rank] = true;
			terms_.insert(terms_.end(), evaluate.begin(), evaluate.end());
			evaluate_start_.push_back(u32(terms_.size()));
			inverse_terms_.insert(inverse_terms_.end(), interpolate.begin(), interpolate.end());
			interpolate_start_.push_back(u32(inverse_terms_.size()));
			top_cap_.push_back(std::uint8_t(cap)), top_nz_.push_back(std::uint8_t(nz));
			top_zeros3_.push_back(std::uint8_t(zeros3));
			for (std::size_t l = bottom_; l < k && ++point[l] == size_of_[l]; ++l) point[l] = 0;
		}
	}
	__attribute__((target("avx2"))) void evaluate_top(const u32* x, std::size_t t, Vec* planes) const {
		const std::size_t vectors = bottom_size_ / 8, filled = top_cap_[t] + std::size_t(1);
		std::memset(static_cast<void*>(planes + filled * vectors), 0, (planes_ - filled) * vectors * sizeof(Vec));
		for (u32 i = evaluate_start_[t]; i < evaluate_start_[t + 1]; ++i) {
			const Term& term = terms_[i];
			add_row(planes + term.rank * vectors, reinterpret_cast<const Vec*>(x + term.row * bottom_size_), vectors,
					term.negative, term.first);
		}
	}
	__attribute__((target("avx2"))) void interpolate_top(const Vec* planes, std::size_t t, u32* c) const {
		const std::size_t vectors = bottom_size_ / 8;
		for (u32 i = interpolate_start_[t]; i < interpolate_start_[t + 1]; ++i) {
			const Term& term = inverse_terms_[i];
			add_row(reinterpret_cast<Vec*>(c + term.row * bottom_size_), planes + term.rank * vectors, vectors,
				term.negative, false);
		}
	}
	template <bool Inverse>
	__attribute__((target("avx2"))) void steps(Vec* x, const Pass& pass, std::size_t begin, std::size_t end, std::size_t o, std::size_t sigma,
			   std::size_t count, int extra) const {
		for (std::size_t k = begin; k < end; ++k) {
			const Step& s = pass.steps[k];
			const Range* range = pass.ranges.data() + k * (bottom_size_ / 8);
			const u32 a = size_of_[s.first], b = s.width == 2 ? size_of_[s.first + 1] : 1;
			switch (a * 4 + b) {
			case 9: block_step<Inverse, 2, 1>(x, s.stride, range, extra, o, sigma, count); break;
			case 13: block_step<Inverse, 3, 1>(x, s.stride, range, extra, o, sigma, count); break;
			case 10: block_step<Inverse, 2, 2>(x, s.stride, range, extra, o, sigma, count); break;
			case 11: block_step<Inverse, 2, 3>(x, s.stride, range, extra, o, sigma, count); break;
			case 14: block_step<Inverse, 3, 2>(x, s.stride, range, extra, o, sigma, count); break;
			default: block_step<Inverse, 3, 3>(x, s.stride, range, extra, o, sigma, count); break;
			}
		}
	}
	template <bool Inverse, int A, int B>
	__attribute__((target("avx2"))) void block_step(Vec* x, std::size_t stride, const Range* range, int extra, std::size_t o, std::size_t sigma,
					std::size_t count) const {
		const std::size_t ranks = std::size_t(rank_ + 1), inner = stride / sigma;
		for (std::size_t base = 0; base < count; base += inner * A * B)
			for (std::size_t q = base; q < base + inner; ++q) {
				const std::size_t u = o + sigma * q;
				variable_step<Inverse, A, B>(x + u * ranks, stride * ranks, stride * A * ranks, range[u].first,
											 std::min(rank_, range[u].first + range[u].span + extra));
			}
	}
	__attribute__((target("avx2"))) void spread(const Vec* planes, Vec* out, std::size_t s) const {
		const std::size_t vectors = bottom_size_ / 8;
		const Vec zero = _mm256_setzero_si256();
		Vec p[kMaxRank + 8];
		for (int i = 0; i < 3; ++i) p[i] = p[3 + planes_ + i] = zero;
		for (std::size_t u = s; u < s + group_; ++u) {
			for (int rho = 0; rho < planes_; ++rho) p[3 + rho] = planes[rho * vectors + u];
			const int d = vector_sum_[u], last = std::min(rank_, d + planes_ + 2);
			Vec* ranks = out + u * (rank_ + 1);
			for (int r = 0; r < d; ++r) ranks[r] = zero;
			for (int r = d; r <= last; ++r) {
				const Vec* q = p + 3 + r - d;
				const Vec x[4] = {q[0], q[-1], q[-2], q[-3]};
				ranks[r] = lanes_transform<false>(by_popcount(x));
			}
			for (int r = last + 1; r <= rank_; ++r) ranks[r] = zero;
		}
	}
	__attribute__((target("avx2"))) void gather(const Vec* in, Vec* planes, std::size_t s) const {
		const std::size_t vectors = bottom_size_ / 8;
		Vec m[kMaxRank + 4];
		for (std::size_t u = s; u < s + group_; ++u) {
			const int d = vector_sum_[u], last = std::min(rank_, d + planes_ + 2);
			for (int r = d; r <= last; ++r) m[r - d] = lanes_transform<true>(in[u * (rank_ + 1) + r]);
			for (int k = last - d + 1; k < planes_ + 3; ++k) m[k] = _mm256_setzero_si256();
			for (int rho = 0; rho < planes_; ++rho) planes[rho * vectors + u] = by_popcount(m + rho);
		}
	}
	__attribute__((target("avx2"))) void pointwise(Vec* a, const Vec* b, std::size_t o, std::size_t sigma, std::size_t count, int top_cap,
				   int top_nz) const {
		const Vec zero = _mm256_setzero_si256();
		Vec ae[kMaxRank], ao[kMaxRank], be[kMaxRank + 2 * kPad], bo[kMaxRank + 2 * kPad];
		for (int i = 0; i < kPad; ++i) be[i] = bo[i] = zero;
		for (std::size_t u = o; u < o + sigma * count; u += sigma) {
			const int cap = std::min(rank_, top_cap + vector_cap_[u] + 3);
			const int first = top_nz + vector_nz_[u], last = std::min(rank_, 2 * cap);
			Vec* x = a + u * (rank_ + 1);
			const Vec* y = b + u * (rank_ + 1);
			for (int i = 0; i <= cap; ++i) {
				ae[i] = x[i], ao[i] = _mm256_srli_epi64(ae[i], 32);
				be[kPad + i] = y[i], bo[kPad + i] = _mm256_srli_epi64(y[i], 32);
			}
			for (int i = cap + 1; i <= cap + kPad; ++i) be[kPad + i] = bo[kPad + i] = zero;
			for (int r = first; r <= last; r += kBlock) {
				const int lo = std::max(0, r - cap), count = std::min(r + kBlock - 1, cap) - lo + 1;
				Vec even[kBlock], odd[kBlock];
				block_sums(ae + lo, be + kPad + r - lo, count, even);
				block_sums(ao + lo, bo + kPad + r - lo, count, odd);
				for (int k = 0; k < kBlock && r + k <= last; ++k) x[r + k] = reduce<true>(even[k], odd[k]);
			}
		}
	}
	static constexpr int kBlock = 4, kPad = kBlock;
	__attribute__((target("avx2"))) static void block_sums(const Vec* x, const Vec* y, int count, Vec* sum) {
		Vec s0 = _mm256_setzero_si256(), s1 = s0, s2 = s0, s3 = s0;
		Vec w0 = y[0], w1 = y[1], w2 = y[2], w3 = y[3];
		const auto term = [&](Vec a, Vec y0, Vec y1, Vec y2, Vec y3) __attribute__((always_inline, target("avx2"))) {
			s0 = _mm256_add_epi64(s0, _mm256_mul_epu32(a, y0));
			s1 = _mm256_add_epi64(s1, _mm256_mul_epu32(a, y1));
			s2 = _mm256_add_epi64(s2, _mm256_mul_epu32(a, y2));
			s3 = _mm256_add_epi64(s3, _mm256_mul_epu32(a, y3));
			asm("" : "+x"(s0), "+x"(s1), "+x"(s2), "+x"(s3));
		};
		const auto run = [&](int n) __attribute__((always_inline, target("avx2"))) {
			for (; n >= 4; n -= 4, x += 4, y -= 4) {
				term(x[0], w0, w1, w2, w3), w3 = y[-1];
				term(x[1], w3, w0, w1, w2), w2 = y[-2];
				term(x[2], w2, w3, w0, w1), w1 = y[-3];
				term(x[3], w1, w2, w3, w0), w0 = y[-4];
			}
			if (n >= 1) term(x[0], w0, w1, w2, w3);
			if (n >= 2) term(x[1], y[-1], w0, w1, w2);
			if (n >= 3) term(x[2], y[-2], y[-1], w0, w1);
		};
		if (count <= 18) {
			run(count);
		} else {
			run(16);

			s0 = fold(s0), s1 = fold(s1), s2 = fold(s2), s3 = fold(s3);
			run(count - 16);
		}
		sum[0] = s0, sum[1] = s1, sum[2] = s2, sum[3] = s3;
	}
	std::size_t size_, bottom_size_ = 0, top_size_ = 0, bottom_ = 0;
	int rank_ = 0, planes_ = 0, threes_ = 0;
	std::vector<u32> size_of_;
	alignas(32) std::array<u32, 8> lane_offset_{};
	std::vector<u32> block_base_;
	std::vector<std::uint8_t> vector_sum_, vector_cap_, vector_nz_;
	std::size_t group_ = 1;
	Pass forward_, inverse_;
	std::vector<std::uint8_t> top_cap_, top_nz_, top_zeros3_;
	std::vector<Term> terms_, inverse_terms_;
	std::vector<u32> evaluate_start_, interpolate_start_;
};

__attribute__((target("avx2"))) inline std::vector<mint> multiply(const std::vector<mint>& a, const std::vector<mint>& b, const std::vector<int>& dimensions) {
	std::vector<u32> dims(dimensions.begin(), dimensions.end()), f(a.size()), g(b.size()), result(a.size());
	for (std::size_t i = 0; i < a.size(); ++i) f[i] = a[i].val(), g[i] = b[i].val();
	Ranked(dims, a.size()).multiply(f.data(), g.data(), result.data());
	std::vector<mint> out;
	out.reserve(result.size());
	for (u32 x : result) out.push_back(mint::raw(x));
	return out;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#endif


#line 9 "convolution/multivariate_convolution.hpp"

#line 1 "components/convolution_dispatch.hpp"



#include <complex>
#include <limits>
#line 8 "components/convolution_dispatch.hpp"

#line 1 "components/mint_convolution.hpp"



#line 1 "components/ntt.hpp"



#line 7 "components/ntt.hpp"
#include <initializer_list>
#include <memory>
#include <new>
#line 11 "components/ntt.hpp"
#include <utility>
#line 13 "components/ntt.hpp"

#line 15 "components/ntt.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct NTT : mint_detail::Ops {
	using V = std::vector<unsigned int>;
	V roots, inverse_roots;
	V root_quotients, inverse_root_quotients;
	mutable V cube_roots, inverse_cube_roots, cube_quotients, inverse_cube_quotients;
	void prepare_cubes(int n) const {
		int begin = cube_roots.size();
		if (begin >= n) return;
		cube_roots.resize(n);
		inverse_cube_roots.resize(n);
		cube_quotients.resize(n);
		inverse_cube_quotients.resize(n);
		for (int k = begin; k < n; ++k) {
			U x = (unsigned long long)roots[k] * roots[2 * k] % mod;
			U y = (unsigned long long)inverse_roots[k] * inverse_roots[2 * k] % mod;
			cube_roots[k] = x;
			inverse_cube_roots[k] = y;
			cube_quotients[k] = quotient(x);
			inverse_cube_quotients[k] = quotient(y);
		}
	}
	NTT(int size, [[maybe_unused]] bool composition = true)
		: roots(size, one),
		  inverse_roots(size, one),
		  root_quotients(size),
		  inverse_root_quotients(size) {
		for (int h = 1; h < size; h <<= 1) {
			U r = in(T(3).pow((mod - 1) / (4 * h)).val());
			U ir = in(T(3).pow(mod - 1 - (mod - 1) / (4 * h)).val());
			for (int i = 0; i < h; ++i) {
				roots[h + i] = mul(roots[i], r);
				inverse_roots[h + i] = mul(inverse_roots[i], ir);
			}
		}
		for (int i = 0; i < size; ++i) {
			root_quotients[i] = (roots[i] < mod ? roots[i] : roots[i] - mod) * 998244351u;
			inverse_root_quotients[i] = (inverse_roots[i] < mod ? inverse_roots[i] : inverse_roots[i] - mod) * 998244351u;
			roots[i] = out(roots[i]);
			inverse_roots[i] = out(inverse_roots[i]);
		}
	}

#if defined(__x86_64__) || defined(__i386__)

	template <int stride>
	__attribute__((target("avx2"))) static I8 rates8(const V &values, int k) {
		if constexpr (stride == 1)
			return _mm256_cvtepu32_epi64(
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(values.data() + 4 * k)));
		if constexpr (stride == 2)
			return _mm256_permute4x64_epi64(
				_mm256_cvtepu32_epi64(
					_mm_loadl_epi64(reinterpret_cast<const __m128i *>(values.data() + 2 * k))),
				0x50);
		if constexpr (stride == 4) return _mm256_set1_epi32(values[k]);
	}
	template <int stride, bool inverse>
	__attribute__((target("avx2"))) I8 small8(I8 z, int k) const {
		if constexpr (stride == 4) {
			if (k == 0) {
				I8 x = _mm256_permute2x128_si256(z, z, 0x00),
				   y = _mm256_permute2x128_si256(z, z, 0x11);
				return _mm256_blend_epi32(add8(x, y), sub8(x, y), 0xf0);
			}
		}
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		I8 r = rates8<stride>(w, k), q = rates8<stride>(quot, k);
		if constexpr (inverse) {
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
			I8 y = _mm256_srli_epi64(z, 32), x = add8(z, y);
			I8 d = fixed_even8(sub8(z, y), r, q);
			z = _mm256_blend_epi32(x, _mm256_slli_epi64(d, 32), 0xaa);
		} else {
			if constexpr (stride == 1) z = _mm256_shuffle_epi32(z, 0xb1);
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0x72);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(4, 0, 5, 1, 6, 2, 7, 3));
			I8 y = fixed_even8(z, r, q);
			I8 x = _mm256_blend_epi32(_mm256_srli_epi64(z, 32), z, 0xaa);
			y = _mm256_blend_epi32(
				y, _mm256_sub_epi32(_mm256_set1_epi32(twice), _mm256_slli_epi64(y, 32)), 0xaa);
			z = add8(x, y);
		}
		if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
		if constexpr (stride == 4)
			z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
		return z;
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void fft8(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
		if constexpr (!inverse) {
			int h = limit / 2;
			for (; h >= 16; h >>= 2) radix4_8<false>(a, h / 2, length);
			if (h == 8) {
				if (limit >= 32)
					for (int i = 0; i < length; i += 8) {
						I8 x = load8(a + i), p = _mm256_set1_epi32(twice);
						store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
					}
				stage8<false>(a, h, length);
			}
		}
		for (int j = 0; j < length; j += 8) {
			I8 z = load8(a + j);
			if constexpr (!inverse)
				if ((limit >= 32) && (__builtin_ctz((unsigned int)limit) & 1)) {
					I8 p = _mm256_set1_epi32(twice);
					z = _mm256_min_epu32(z, _mm256_sub_epi32(z, p));
				}
			if constexpr (inverse)
				z = small8<4, true>(small8<2, true>(small8<1, true>(z, j / 8), j / 8), j / 8);
			else
				z = small8<1, false>(small8<2, false>(small8<4, false>(z, j / 8), j / 8),
									 j / 8);
			store8(a + j, z);
		}
		if constexpr (inverse) {
			int h = 8;
			for (; 4 * h <= limit; h <<= 2) radix4_8<true>(a, h, length);
			if (h < limit) stage8<true>(a, h, length);
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void radix4_8(U *a, int stride, int length) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		prepare_cubes(length / (4 * stride));
		const V &w3 = inverse ? inverse_cube_roots : cube_roots;
		const V &q3s = inverse ? inverse_cube_quotients : cube_quotients;
		I8 p = _mm256_set1_epi32(twice);
		I8 im = _mm256_set1_epi32(w[1]), iq = _mm256_set1_epi32(q[1]);
		for (int j = 0, k = 0; j < length; j += 4 * stride, ++k) {
			I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			I8 r3 = _mm256_set1_epi32(w3[k]), q3 = _mm256_set1_epi32(q3s[k]);
			for (int i = 0; i < stride; i += 8) {
				I8 x0 = load8(a + j + i), x1 = load8(a + j + stride + i);
				I8 x2 = load8(a + j + 2 * stride + i), x3 = load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					I8 u = add8(x0, x1), v = sub8(x0, x1);
					I8 s = add8(x2, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x2, p), x3), im, iq);
					x0 = add8(u, s);
					x1 = _mm256_add_epi32(v, t);
					x2 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					} else {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
				} else {
					x0 = _mm256_min_epu32(x0, _mm256_sub_epi32(x0, p));
					if (!k) {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					}
					I8 u = add8(x0, x2), v = sub8(x0, x2), s = add8(x1, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x1, p), x3), im, iq);
					x0 = _mm256_add_epi32(u, s);
					x1 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x2 = _mm256_add_epi32(v, t);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
				}
				store8(a + j + i, x0);
				store8(a + j + stride + i, x1);
				store8(a + j + 2 * stride + i, x2);
				store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void stage8(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		if (stride < 8) {
			for (int j = 0; j < length; j += 8) {
				I8 z = load8(a + j);
				if (stride == 1)
					z = small8<1, inverse>(z, j / 8);
				else if (stride == 2)
					z = small8<2, inverse>(z, j / 8);
				else
					z = small8<4, inverse>(z, j / 8);
				store8(a + j, z);
			}
		} else {
			for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
				I8 r = _mm256_set1_epi32(w[k]), q = _mm256_set1_epi32(quot[k]);
				for (int i = 0; i < stride; i += 8) {
					I8 x = load8(a + j + i), y = load8(a + j + stride + i);
					if constexpr (!inverse)
						if (k) y = fixed_mul8(y, r, q);
					I8 z = sub8(x, y);
					x = add8(x, y);
					if constexpr (inverse)
						if (k) z = fixed_mul8(z, r, q);
					store8(a + j + i, x);
					store8(a + j + stride + i, z);
				}
			}
		}
	}

	__attribute__((target("avx2"))) static void load_pairs(const U *a, I8 &x, I8 &y) {
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		I8 u = _mm256_permutevar8x32_epi32(load8(a), index),
		   v = _mm256_permutevar8x32_epi32(load8(a + 8), index);
		x = _mm256_permute2x128_si256(u, v, 0x20);
		y = _mm256_permute2x128_si256(u, v, 0x31);
	}
	__attribute__((target("avx2"))) static void scale_copy8(U *dst, const U *src, int length, U r) {
		U quotient_r = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient_r);
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(dst + i, fixed_mul8(load8(src + i), v, q));
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, quotient_r);
	}
	__attribute__((target("avx2"))) static void scale8(U *a, int length, U r) {
		scale_copy8(a, a, length, r);
	}
#endif

	template <bool inverse>
	void stage(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			stage8<inverse>(a, stride, length, first);
			return;
		}
#endif
		for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
			U r = w[k], q = quot[k];
			int i = 0;
			for (; i < stride; ++i) {
				U x = a[j + i], y = a[j + stride + i];
				if constexpr (!inverse)
					if (k) y = fixed_mul(y, r, q);
				a[j + i] = add(x, y);
				a[j + stride + i] = inverse && k ? fixed_mul(sub(x, y), r, q) : sub(x, y);
			}
		}
	}

	template <bool inverse>
	void fft(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
#if defined(__x86_64__) || defined(__i386__)
		if (limit >= 8 && vectorized()) {
			fft8<inverse>(a, length, limit);
			return;
		}
#endif
		if constexpr (inverse) {
			for (int h = 1; h < limit; h <<= 1) stage<true>(a, h, length);
		} else {
			for (int h = limit / 2; h; h >>= 1) stage<false>(a, h, length);
		}
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void product8(U *dst, const U *a, const U *b, int n) {
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = mul8(load8(a + i), load8(b + i));
			store8(dst + i, x);
		}
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
	__attribute__((target("avx2"))) static void weighted8(U *dst, const U *src, int n, int first) {
		alignas(32) U factors[8];
		for (int j = 0; j < 8; ++j) factors[j] = in(first + j);
		I8 r = load8(factors), step = _mm256_set1_epi32(in(8));
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(dst + i, mul8(load8(src + i), r));
			r = add8(r, step);
		}
		for (; i < n; ++i) dst[i] = mul(src[i], in(first + i));
	}
	__attribute__((target("avx2"), always_inline)) static I8 inverse8(I8 a) {
		I8 a2 = mul8(a, a), a3 = mul8(a2, a);
		I8 a7 = mul8(mul8(a3, a3), a), a31 = a7;
		for (int repeat_index = 0; repeat_index < 2; ++repeat_index) a31 = mul8(a31, a31);
		a31 = mul8(a31, a3);
		I8 r = a7;
		for (int repeat_index = 0; repeat_index < 3; ++repeat_index) r = mul8(r, r);
		r = mul8(r, a3);
		r = mul8(r, r);
		for (int j = 0; j < 4; ++j) {
			for (int repeat_index = 0; repeat_index < 5; ++repeat_index) r = mul8(r, r);
			r = mul8(r, a31);
		}
		for (int repeat_index = 0; repeat_index < 3; ++repeat_index) r = mul8(r, r);
		return mul8(r, a7);
	}
	__attribute__((target("avx2"))) static void batch_inverse8(U *dst, const U *src, int n) {
		if (n < 16) {
			for (int i = 0; i < n; ++i) dst[i] = T::word(src[i]).inv().v;
			return;
		}
		int count = n / 16 * 16;
		I8 p = load8(src), q = load8(src + 8);
		store8(dst, p);
		store8(dst + 8, q);
		for (int i = 16; i < count; i += 16) {
			p = mul8(p, load8(src + i));
			q = mul8(q, load8(src + i + 8));
			store8(dst + i, p);
			store8(dst + i + 8, q);
		}
		alignas(32) U lanes[16];
		store8(lanes, p);
		store8(lanes + 8, q);
		for ([[maybe_unused]] auto x : lanes) assert(out(x) != 0);
		I8 inverse = inverse8(p), other = inverse8(q);
		for (int i = count - 16; i >= 16; i -= 16) {
			store8(dst + i, mul8(inverse, load8(dst + i - 16)));
			store8(dst + i + 8, mul8(other, load8(dst + i - 8)));
			inverse = mul8(inverse, load8(src + i));
			other = mul8(other, load8(src + i + 8));
		}
		store8(dst, inverse);
		store8(dst + 8, other);
		for (int i = count; i < n; ++i) dst[i] = T::word(src[i]).inv().v;
	}
	__attribute__((target("avx2"))) static void inverse_sequence8(U *dst, int first, int n) {

		alignas(32) U initial[8];
		for (int j = 0; j < 8; ++j) initial[j] = in(first + j);
		I8 factor = load8(initial), step = _mm256_set1_epi32(in(8));
		I8 p = factor;
		store8(dst, p);
		for (int i = 8; i < n; i += 8) {
			factor = add8(factor, step);
			p = mul8(p, factor);
			store8(dst + i, p);
		}
		I8 inverse = inverse8(p);
		for (int i = n - 8; i >= 8; i -= 8) {
			store8(dst + i, mul8(inverse, load8(dst + i - 8)));
			inverse = mul8(inverse, factor);
			factor = sub8(factor, step);
		}
		store8(dst, inverse);
	}
	__attribute__((target("avx2"))) static void normalize8(U *a, int n) {
		I8 p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = load8(a + i);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
#endif
	static void weighted(U *dst, const U *src, int n, int first) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			weighted8(dst, src, n, first);
			return;
		}
#endif
		for (int i = 0; i < n; ++i) dst[i] = mul(src[i], in(first + i));
	}
	static void normalize(U *a, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			normalize8(a, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
	static void product(U *dst, const U *a, const U *b, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			product8(dst, a, b, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"), always_inline)) void partial_stage8(U *a, int stride, int k) const {
		const I8 p = _mm256_set1_epi32(twice);
		const I8 r = _mm256_set1_epi32(roots[k]), q = _mm256_set1_epi32(root_quotients[k]);
		for (int i = 0; i < stride; i += 8) {
			I8 x = load8(a + i), y = load8(a + stride + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = k ? fixed_mul8(y, r, q) : _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			store8(a + i, _mm256_add_epi32(x, y));
			store8(a + stride + i, _mm256_sub_epi32(_mm256_add_epi32(x, p), y));
		}
	}
	template <int count, bool second = false>
	__attribute__((target("avx2"), always_inline)) void inverse_leaf_products8(U *a, U *b, const U *ws, const U *qs) const {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		alignas(32) U window[count][16], coefficients[count][8];
		I8 e[count], o[count];
		for (int k = 0; k < count; ++k) {
			I8 x = load8(a + 8 * k), y = load8(b + 8 * k);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			I8 low = fixed_mul8(x, _mm256_set1_epi32(ws[k]), _mm256_set1_epi32(qs[k]));
			store8(window[k], _mm256_min_epu32(low, _mm256_sub_epi32(low, p)));
			store8(window[k] + 8, x);
			store8(coefficients[k], y);
			if constexpr (!second) store8(b + 8 * k, y);
			e[k] = o[k] = _mm256_setzero_si256();
		}
		for (int i = 0; i < 8; ++i)
			for (int k = 0; k < count; ++k) {
				I8 z = load8(window[k] + 8 - i), r = _mm256_set1_epi32(coefficients[k][i]);
				e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(z, r));
				o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
			}
		for (int k = 0; k < count; ++k) {
			I8 x = reduce8(e[k], o[k]);
			store8(a + 8 * k, _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
	}
	__attribute__((target("avx2"))) static U reverse_dot8(const U *a, const U *b, int n) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 reversed = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0), total = _mm256_setzero_si256();
		int i = 0;
		for (; i + 8 <= n;) {
			I8 even = _mm256_setzero_si256(), odd = even;
			int end = std::min(n / 8 * 8, i + 64);
			for (; i < end; i += 8) {
				I8 x = load8(a + i), y = _mm256_permutevar8x32_epi32(load8(b - i - 7), reversed);
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			I8 value = reduce8(even, odd);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			total = add8(total, value);
		}
		total = add8(total, _mm256_permute2x128_si256(total, total, 1));
		total = add8(total, _mm256_shuffle_epi32(total, 0x4e));
		total = add8(total, _mm256_shuffle_epi32(total, 0xb1));
		U result = _mm_cvtsi128_si32(_mm256_castsi256_si128(total));
		for (; i < n; ++i) {
			U x = a[i], y = b[-i];
			if (x >= twice) x -= twice;
			if (y >= twice) y -= twice;
			if (x >= mod) x -= mod;
			if (y >= mod) y -= mod;
			result = add(result, mul(x, y));
		}
		return result;
	}

	template <bool inverse>
	__attribute__((target("avx2"))) void inverse_partial8(U *a, int n, int limit = 0) const {
		if (!limit) limit = n;
		if constexpr (!inverse) {
			int stride = limit / 2;
			for (; stride >= 16; stride >>= 2) radix4_8<false>(a, stride / 2, n);
			if (stride == 8)
				for (int i = 0; i < n; i += 16) partial_stage8(a + i, 8, i / 16);
		} else {
			int stride = 8;
			for (; 4 * stride <= n; stride <<= 2) radix4_8<true>(a, stride, n);
			if (stride < n) stage8<true>(a, stride, n);
		}
	}
	__attribute__((target("avx2"))) void inverse_newton8(U *a, U *b, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(roots.size()) >= n / 16);
		std::copy_n(b, n / 2, b + n / 2);
		inverse_partial8<false>(b, n, n / 2);
		inverse_partial8<false>(a, n);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = (k & 1) ? mod - roots[k / 2] : roots[k / 2];
				qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
			}
			inverse_leaf_products8<8>(a + i, b + i, ws, qs);
		}
		inverse_partial8<true>(a, n);
		std::fill_n(a, n / 2, U(0));
		inverse_partial8<false>(a, n);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = (k & 1) ? mod - roots[k / 2] : roots[k / 2];
				qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
			}
			inverse_leaf_products8<8, true>(a + i, b + i, ws, qs);
		}
		inverse_partial8<true>(a, n);
		U norm = mod - (mod - 1) / (n / 8);
		scale8(a + n / 2, n / 2, mod - (unsigned long long)norm * norm % mod);
	}

#endif
#if defined(__x86_64__) || defined(__i386__)
	template <bool odd>
	__attribute__((target("avx2"))) static void reduce_pairs8(U *a, U *b, const U *twist, int n) {
		for (int i = 0; i < n; i += 8) {
			I8 x, y, u, v;
			load_pairs(a + 2 * i, x, y);
			load_pairs(b + 2 * i, u, v);
			x = mul8(x, v);
			y = mul8(y, u);
			if constexpr (odd) x = mul8(sub8(x, y), load8(twist + i));
			else x = add8(x, y);
			store8(a + i, x);
			store8(b + i, mul8(u, v));
		}
	}
	__attribute__((target("avx2"))) static void combine8(U *a, const U *b, const U *x, const U *y, int n) {
		for (int i = 0; i < n; i += 8) store8(a + i, add8(mul8(load8(a + i), load8(y + i)), mul8(load8(b + i), load8(x + i))));
	}
#endif

	template <bool odd>
	static void reduce_pairs(U *a, U *b, const U *twist, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			reduce_pairs8<odd>(a, b, twist, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) {
			U u = mul(a[2 * i], b[2 * i + 1]), v = mul(a[2 * i + 1], b[2 * i]);
			a[i] = odd ? mul(sub(u, v), twist[i]) : add(u, v);
			b[i] = mul(b[2 * i], b[2 * i + 1]);
		}
	}
	static void combine(U *a, const U *b, const U *x, const U *y, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			combine8(a, b, x, y, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) a[i] = add(mul(a[i], y[i]), mul(b[i], x[i]));
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void geometric8(U *a, int n, U first, U step) {
		alignas(32) U factors[8];
		U power = one;
		for (int i = 0; i < 8; ++i) {
			factors[i] = mul(first, power);
			power = mul(power, step);
		}
		I8 r = load8(factors), ratio = _mm256_set1_epi32(power);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(a + i, mul8(load8(a + i), r));
			r = mul8(r, ratio);
		}
		store8(factors, r);
		for (int j = 0; i < n; ++i, ++j) a[i] = mul(a[i], factors[j]);
	}
#endif
	static void geometric(U *a, int n, U first, U step) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			geometric8(a, n, first, step);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) {
			a[i] = mul(a[i], first);
			first = mul(first, step);
		}
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void add_scaled8(U *a, const U *b, int n, U r) {
		U q = quotient(r);
		I8 v = _mm256_set1_epi32(r), vq = _mm256_set1_epi32(q);
		int i = 0;
		for (; i + 8 <= n; i += 8) store8(a + i, add8(load8(a + i), fixed_mul8(load8(b + i), v, vq)));
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
#endif
	static void add_scaled(U *a, const U *b, int n, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			add_scaled8(a, b, n, r);
			return;
		}
#endif
		int i = 0;
		U q = quotient(r);
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
	static void scale_copy(U *dst, const U *src, int length, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			scale_copy8(dst, src, length, r);
			return;
		}
#endif
		U q = quotient(r);
		int i = 0;
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, q);
	}
	void scale(U *a, int length, U r) const {
		scale_copy(a, a, length, r);
	}
	struct CompositionDelete {
		void operator()(unsigned int *p) const { ::operator delete[](p, std::align_val_t(64)); }
	};
	using CompositionWords = std::unique_ptr<unsigned int[], CompositionDelete>;

};
inline NTT &ntt_plan(int size) {
	static NTT t(1, false);
	if (static_cast<int>(t.roots.size()) < size) t = NTT(size, false);
	return t;
}

}

inline void ntt(std::vector<mint> &a, bool inverse = false) {
	assert(!a.empty() && !(a.size() & (a.size() - 1)) && a.size() <= (1U << 23));
	int n = static_cast<int>(a.size());
	auto &t = fps_detail::ntt_plan(std::max(1, n / 2));
	auto reorder = [&] {
		for (int i = 1, j = 0; i < n; ++i) {
			int bit = n >> 1;
			for (; j & bit; bit >>= 1) j ^= bit;
			j ^= bit;
			if (i < j) std::swap(a[i], a[j]);
		}
	};
	static_assert(sizeof(mint) == sizeof(fps_detail::NTT::U) && std::is_trivially_copyable_v<mint>);
	auto *words = reinterpret_cast<fps_detail::NTT::U *>(a.data());
	if (inverse) {
		reorder();
		t.fft<true>(words, n);
		t.scale(words, n, mint(n).inv().val());
	} else {
		t.fft<false>(words, n);
		reorder();
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fps_convolution.hpp"


#line 1 "components/fps_blocked.hpp"


#line 1 "components/fps_workspace.hpp"



#line 9 "components/fps_workspace.hpp"
#if defined(__linux__)
#include <sys/mman.h>
#endif

namespace fps_detail {

struct FpsWordsDelete {
	void *region = nullptr;
	std::size_t bytes = 0;
	void operator()(unsigned int *p) const {
#if defined(__linux__)
		if (region) {
			munmap(region, bytes);
			return;
		}
#endif
		::operator delete[](p, std::align_val_t(64));
	}
};

using FpsWords = std::unique_ptr<unsigned int[], FpsWordsDelete>;

inline FpsWords allocate_fps_words(std::size_t count) {
#if defined(__linux__) && defined(MADV_HUGEPAGE)
	constexpr std::size_t page = 1 << 21;
	if (count >= page / sizeof(unsigned int) &&
		count <= (std::numeric_limits<std::size_t>::max() - 2 * page) / sizeof(unsigned int)) {
		const std::size_t bytes = (count * sizeof(unsigned int) + page - 1) / page * page;
		void *region = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (region != MAP_FAILED) {
			const auto address = (reinterpret_cast<std::uintptr_t>(region) + page - 1) & ~(page - 1);
			auto *p = ::new (reinterpret_cast<void *>(address)) unsigned int[count];
			madvise(p, bytes, MADV_HUGEPAGE);
			return FpsWords(p, FpsWordsDelete{region, bytes + page});
		}
	}
#endif
	return FpsWords(new (std::align_val_t(64)) unsigned int[count]);
}

}


#line 1 "components/fps_output.hpp"



#line 8 "components/fps_output.hpp"
#include <stdexcept>
#line 11 "components/fps_output.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#include <sys/mman.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {

inline void prepare_fps_output(std::vector<mint> &out, std::size_t n) {
	if (n <= out.capacity()) return;
	out.reserve(n);
	if (n - out.size() < (1u << 17)) return;
	const auto first = reinterpret_cast<std::uintptr_t>(out.data() + out.size());
	const auto end = reinterpret_cast<std::uintptr_t>(out.data() + n);
	[[maybe_unused]] const auto advise = [first, end](std::uintptr_t alignment, int advice) {
		const auto lo = (first + alignment - 1) & ~(alignment - 1), hi = end & ~(alignment - 1);
		if (lo < hi) static_cast<void>(madvise(reinterpret_cast<void *>(lo), hi - lo, advice));
	};
#if defined(MADV_HUGEPAGE)
	advise(2u * 1024 * 1024, MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
	advise(4096, MADV_POPULATE_WRITE);
#endif
}

template <bool montgomery>
inline void append_fps_words(std::vector<mint> &out, const unsigned *in, std::size_t n) {
	static_assert(sizeof(mint) == sizeof(unsigned) && std::is_trivially_copyable_v<mint>);
	if (n == 0) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + n);
	std::array<mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if constexpr (montgomery) std::memcpy(static_cast<void *>(batch.data()), in + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (int j = 0; j < 64; j += 8) {
				const auto x = mint_detail::Ops::mul8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i + j)), _mm256_set1_epi32(932051910u));
				std::memcpy(static_cast<void *>(batch.data() + j), &x, sizeof(x));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	for (; i < n; ++i) {
		if constexpr (montgomery) out.emplace_back(mint::word(in[i]));
		else out.emplace_back(mint::raw(in[i]));
	}
}
}
#pragma GCC pop_options
#endif

#line 11 "components/fps_blocked.hpp"
#include <cstdlib>
#line 13 "components/fps_blocked.hpp"
#include <span>
#line 16 "components/fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
using Word = std::uint32_t;
using Words = std::span<Word>;
using ConstWords = std::span<const Word>;
using std::size_t;
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP, kNI = 998244351;

}
namespace blocked_ntt {

inline constexpr Word kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec, kernels::kP;

constexpr Word multiply_mod(Word x, Word y) {
	return Word(std::uint64_t(x) * y % kP);
}

constexpr Word power(Word x, Word e) {
	Word r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr Word quotient(Word w) { return Word((std::uint64_t(w) << 32) / kP); }

inline constexpr Word kR = Word((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<Word, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(Word x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, Word bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(Word value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(Word value, Word value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) { return mint_detail::Ops::fixed_mul8(x, f.w, f.q); }

inline size_t slot(size_t k) { return (k >> 3 << 4) | (k & 7); }

inline constexpr size_t table_words(int lg) { return (size_t(1) << lg) / 8 + 16; }

inline void build_table(Word* t, size_t count, const std::array<Word, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const size_t h = size_t(1) << level;
		if (h >= count) break;
		const Word root = roots[level];
		if (h < 8) {
			for (size_t j = 0; j < h; ++j) {
				const Word v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

}
}
namespace blocked_ntt::kernels {
using namespace detail;

template<bool Inverse, bool Identity>
inline void radix4(Vec* a, size_t h, const Word* x, const Word* y) {
	const Factor wx(Identity ? 1 : x[0], Identity ? 4 : x[8]);
	const Factor wy(Identity ? 1 : y[0], Identity ? 4 : y[8]), wz(y[1], y[9]);
#pragma GCC unroll 1
	for (size_t k = 0; k < h; ++k) {
		const Vec f0 = a[k], f1 = a[k + h], f2 = a[k + 2 * h], f3 = a[k + 3 * h];
		if constexpr (Inverse) {
			const Vec ab = reduce(add(f0, f1), 2 * kP), cd = reduce(add(f2, f3), 2 * kP);
			const Vec amb = Identity ? reduce(diff(f0, f1), 2 * kP) : multiply(diff(f0, f1), wy);
			const Vec cmd = multiply(diff(f2, f3), wz);
			a[k] = reduce(add(ab, cd), 2 * kP), a[k + h] = reduce(add(amb, cmd), 2 * kP);
			if constexpr (Identity) {
				a[k + 2 * h] = reduce(diff(ab, cd), 2 * kP);
				a[k + 3 * h] = reduce(diff(amb, cmd), 2 * kP);
			} else {
				a[k + 2 * h] = multiply(diff(ab, cd), wx);
				a[k + 3 * h] = multiply(diff(amb, cmd), wx);
			}
		} else {
			const Vec lo = reduce(f0, 2 * kP), hi = reduce(f1, 2 * kP);
			const Vec xc = Identity ? reduce(f2, 2 * kP) : multiply(f2, wx);
			const Vec xd = Identity ? reduce(f3, 2 * kP) : multiply(f3, wx);
			const Vec ac = reduce(add(lo, xc), 2 * kP), amc = reduce(diff(lo, xc), 2 * kP);
			const Vec bd = Identity ? reduce(add(hi, xd), 2 * kP) : multiply(add(hi, xd), wy);
			const Vec bmd = multiply(diff(hi, xd), wz);
			a[k] = add(ac, bd), a[k + h] = diff(ac, bd);
			a[k + 2 * h] = add(amc, bmd), a[k + 3 * h] = diff(amc, bmd);
		}
	}
}

inline void forward(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<false, false>(a, h, x, y);
}
inline void inverse(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<true, false>(a, h, x, y);
}
inline void forward_identity(Vec* a, size_t h, const Word* y) {
	radix4<false, true>(a, h, y, y);
}
inline void inverse_identity(Vec* a, size_t h, const Word* y) {
	radix4<true, true>(a, h, y, y);
}

}

namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::forward(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::inverse(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

}

namespace blocked_poly {

inline constexpr Word kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr size_t footprint(size_t n) { return (n + 7) / 8 * 8 + 16; }
	Words take(size_t n) {
		if (size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

	bool zero_initialized() const { return storage_.get_deleter().region != nullptr; }

private:
	FpsWords storage_;
	Word *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add, blocked_ntt::detail::broadcast, blocked_ntt::detail::diff, blocked_ntt::detail::Factor, blocked_ntt::detail::kP, blocked_ntt::detail::kR, blocked_ntt::detail::reduce, blocked_ntt::detail::slot, blocked_ntt::detail::Vec;

inline constexpr auto kInverseScales = [] {
	std::array<std::array<Word, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(Word(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Vec load(const Word* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(Word* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const Word* table, size_t k) {
	const Word* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const Word* roots, size_t p) {
	const Word* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const Word* table, size_t g)
		: x(entry(table, g)), y(entry(table, 2 * g)), z(entry(table, 2 * g + 1)) {}
};

[[gnu::always_inline]] inline void forward_h1(Vec (&f)[4], const Group& w) {
	const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], w.x), d = times(f[3], w.x);
	const Vec ac = low(add(a, c)), amc = low_difference(a, c);
	const Vec bd = times(add(b, d), w.y), bmd = times(diff(b, d), w.z);
	f[0] = add(ac, bd), f[1] = diff(ac, bd), f[2] = add(amc, bmd), f[3] = diff(amc, bmd);
}

[[gnu::always_inline]] inline void inverse_h1(Vec (&f)[4], const Group& w) {
	const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
	const Vec amb = times(diff(f[0], f[1]), w.y), cmd = times(diff(f[2], f[3]), w.z);
	f[0] = low(add(ab, cd)), f[1] = low(add(amb, cmd));
	f[2] = times(diff(ab, cd), w.x), f[3] = times(diff(amb, cmd), w.x);
}

struct alignas(64) Window {
	Word word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const Word* roots,
												size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const Word* b) {
	Vec even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) {
		const Vec bi = broadcast(b[i]);
		const Vec lo = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 8 - i));
		const Vec hi = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 9 - i));
		even = _mm256_add_epi64(even, _mm256_mul_epu32(lo, bi));
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(hi, bi));
	}
	return reduce(mint_detail::Ops::reduce8(even, odd), 2 * kP);
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Word* roots;

	void operator()(Word* a, size_t count, size_t first) const {
		for (size_t j = 0; j < count; ++j, a += 32) {
			const Group w(roots, first + j);
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, w);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
};

struct InverseBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word* inverse_roots;
	const Word* in;

	void operator()(Word* a, size_t count, size_t first) const {
		const Word* from = in + 32 * first;
		for (size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const Word* roots;
	const Word* inverse_roots;
	const Word* b;
	size_t origin = 0;

	[[gnu::always_inline]] void prepare(const Word* a, size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(Word* a, size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t - origin));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(Word* a, size_t count, size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const Word* a;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[4]) const {
		const Word* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(size_t g, Window (&window)[K][4]) const {
		for (size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t - terms[0].product.origin));
				for (size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t - terms[k].product.origin))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(Word* a, size_t count, size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const Word* b = product.b + 8 * (4 * (first + j) - product.origin);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const Word* roots, const Word* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(Word* a, size_t nv, size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr size_t kTile = 256;

	void tile(Word* a, size_t nv, size_t k) const {
		if constexpr (Bottom::kForward) {
			for (size_t h = nv / 4; h >= 16; h /= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (size_t h = 16; h < nv; h *= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(Word* a) { return reinterpret_cast<Vec*>(a); }

	void forward(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const Word *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const Word* in, size_t size, size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool within(size_t lo, size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const Word* in, size_t shift, size_t end, size_t v) {
		alignas(32) Word x[8] = {};
		for (size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const Word* in_;
	size_t shift_, end_;
	size_t first_, count_;
	size_t first_touched_, touched_;
};

template <class Input>
[[gnu::always_inline]] inline void forward_top4_body(Input in, Vec* out, size_t h, const Word* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

template <class Input>
[[gnu::always_inline]] inline void forward_top2_body(Input in, Vec* out, size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, size_t h, const Word* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, size_t h) {
	forward_top2_body(in, out, h);
}

template <class Bottom>
void radix2_halves(const Recursion<Bottom>& recursion, const Source& in, Word* a, size_t nv) {
	const size_t h = nv / 2;
	if constexpr (Bottom::kForward) forward_top2(in, reinterpret_cast<Vec*>(a), h);
	recursion.visit(a, h, 0);
	recursion.visit(a + 8 * h, h, 1);
}

inline void inverse_top4(Vec* f, size_t h, Half output, const Word* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output != Half::kUpper) f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		if (output != Half::kLower) f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

inline void inverse_top2(Vec* f, size_t h, Half output, Word scale) {
	const Factor s(scale);
	for (size_t j = 0; j < h; ++j) {
		const Vec lo = f[j], hi = f[j + h];
		if (output != Half::kUpper) f[j] = reduce(times(add(lo, hi), s), kP);
		if (output != Half::kLower) f[j + h] = reduce(times(diff(lo, hi), s), kP);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const size_t table = blocked_ntt::detail::table_words(lg_max), entries = (size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const Word* roots() const { return roots_; }
	const Word* inverse_roots() const { return inverse_roots_; }

	void forward(ConstWords in, size_t shift, Words out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(Words a) const { forward(a, 0, a); }

	void forward_upper(ConstWords in, size_t shift, Words out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(ConstWords in, Words out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), false);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(Words a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(ConstWords in, size_t shift, Words out,
						ConstWords b, Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(Words a, ConstWords b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(ConstWords a, ConstWords b, Words out,
						Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		ConstWords a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, Words out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, Words out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), true);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(ConstWords in, size_t shift, Words out,
						ConstWords b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(Words a, ConstWords b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(Words c, ConstWords a,
					ConstWords b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

protected:
	Word inverse_scale(size_t n, bool montgomery) const {
		check_length(n);
		return detail::kInverseScales[montgomery][std::countr_zero(n)];
	}

	void check_length(size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <size_t K>
	void inverse_products(std::span<const Pair> pairs, Words out, Half output) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), true);
		detail::InverseProductSumBottom<K> bottom{};
		for (size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	static detail::Source source(ConstWords in, size_t shift, size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(Words a, const detail::Source& in, const Bottom& bottom, Word scale,
			Half output) const {
		using namespace detail;
		check_length(a.size());
		const detail::Recursion recursion(roots_, inverse_roots_, bottom);
		const size_t nv = a.size() / 8;
		auto* v = reinterpret_cast<Vec*>(a.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const size_t h = nv / 4;
			if constexpr (Bottom::kForward) forward_top4(in, v, h, roots_);
			for (size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
			if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots_, Factor(scale));
		} else {
			const size_t h = nv / 2;
			radix2_halves(recursion, in, a.data(), nv);
			if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
		}
	}

	template <class Combine>
	void products(Word* out, const Word* a, const Word* b, size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	Word *roots_, *inverse_roots_;
};

namespace detail {

inline Vec load_unaligned(const Word* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(Word* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec montgomery(Vec a, Vec b) { return mint_detail::Ops::mul8(a, b); }

class Indices {
public:
	explicit Indices(size_t k) {
		alignas(32) Word x[8];
		for (Word i = 0; i < 8; ++i) x[i] = Word((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr Word kStep = Word((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline Word scalar_inverse(Word x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(size_t first, Words out, const A& a, Word factor = 1) {
	constexpr size_t kLanes = 32;
	const size_t n = out.size();
	if (n == 0) return;
	const size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(Word(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(Word(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(Word(n)), lanes);
	};
	const auto masked = [&](Vec value, size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	const Vec left = montgomery(prefix[0], prefix[1]), right = montgomery(prefix[2], prefix[3]);
	alignas(32) Word total[8], inverse[8];
	store(total, canonical(montgomery(left, right)));
	Word running = 1;
	for (size_t l = 0; l < 8; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (size_t l = 8; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	const Vec inv_left = montgomery(load(inverse), right), inv_right = montgomery(load(inverse), left);
	Vec inv[4] = {montgomery(inv_left, prefix[1]), montgomery(inv_left, prefix[0]),
				  montgomery(inv_right, prefix[3]), montgomery(inv_right, prefix[2])};
	const auto put = [&](size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * size_t(v) < n) put(8 * v, inv[v]);
}

}

inline Word mint_scalar(Word x) { return x; }
inline unsigned mint_scalar(mint x) { return std::min(x.v, x.v - mint::mod()); }
inline void copy_coefficients(const mint *in, Words out) {
	for (size_t i = 0; i < out.size(); ++i) out[i] = mint_scalar(in[i]);
}
template <class T> inline detail::Vec coefficients(std::span<const T> f, size_t at) {
	detail::Vec value;
	std::memcpy(&value, f.data() + at, sizeof(value));
	if constexpr (std::is_same_v<T, mint>) return detail::reduce(value, detail::kP);
	else return value;
}

template <class T> inline void derivative_impl(std::span<const T> f, Words d) {
	using namespace detail;
	const size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(coefficients(f, i + 1), index.value())));
	for (size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(Word(i + 1), mint_scalar(f[i + 1])) : 0;
}

inline void derivative(ConstWords f, Words d) { derivative_impl(f, d); }

inline void divide_by_index(ConstWords a, size_t first, Words q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	const size_t n = a.size();
	detail::divide_by_index(first, q, [a, n](size_t at) __attribute__((target("avx2"))) {
		if (at + 8 <= n) return detail::load_unaligned(a.data() + at);
		alignas(32) Word rest[8] = {};
		std::copy(a.begin() + at, a.end(), rest);
		return detail::load(rest);
	});
}

namespace detail {

inline constexpr size_t kInverseBase = 32;

inline Word direct_product(const Word* a, const Word* b, size_t n) {
	std::uint64_t sums[4] = {};
	size_t j = 0;
	for (; j + 4 <= n; j += 4)
		for (size_t k = 0; k < 4; ++k) sums[k] += std::uint64_t(a[j + k]) * b[n - 1 - j - k];
	std::uint64_t sum = 0;
	for (auto x : sums) sum += x % kP;
	for (; j < n; ++j) sum += std::uint64_t(a[j]) * b[n - 1 - j];
	return Word(sum % kP);
}

inline void inverse_direct(ConstWords f, Words g, Word inv0 = 0) {
	using blocked_ntt::detail::multiply_mod;
	if (!inv0) inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (size_t i = 1; i < g.size(); ++i) {
		const size_t count = std::min(i, f.size() - 1);
		const Word sum = direct_product(f.data() + 1, g.data() + i - count, count);
		g[i] = multiply_mod(sum ? kP - sum : 0, inv0);
	}
}

}

inline int inverse_log(size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<size_t>(n, 2) - 1)));
}

inline size_t inverse_scratch(size_t n) { return 2 * Arena::footprint(size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, ConstWords f, ConstWords g,
						Words to, Words gt, Words work, Word scale = detail::kP - 1) {
	const size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, scale);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, ConstWords f, Words g,
					Words scratch) {
	const size_t n = g.size();
	size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

namespace detail {

inline constexpr size_t kLogBase = 64;

inline constexpr size_t kLogBlocks = 4;

inline void log_direct(ConstWords f, Words g) {
	using blocked_ntt::detail::multiply_mod;
	Word a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / Word(i), inv[kP % i]);
	g[0] = 0;
	for (size_t i = 1; i < g.size(); ++i) {
		const Word sum = direct_product(kg + 1, a + 1, i - 1);
		kg[i] = (multiply_mod(Word(i), a[i]) + kP - sum) % kP;
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline size_t log_block(size_t n) {
	return std::max<size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline size_t log_blocks(size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

template <class T> inline void subtract_derivative(std::span<const T> f, size_t first, size_t count,
									Word* out, size_t size) {
	const size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(coefficients(f, first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (size_t c = full; c < count; ++c) {
		const Word d = c < inside ? blocked_ntt::detail::multiply_mod(Word(first + 1 + c), mint_scalar(f[first + 1 + c])) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}
inline void subtract_derivative_impl(ConstWords f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative(f, first, count, out, size);
}

inline void subtract_from_derivative(ConstWords f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative_impl(f, first, count, out, size);
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	Word* w3;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (size_t t = 0; t < 4; ++t) {
			const size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const Word* side;

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + (leaf - 8 * term.product.origin)), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline size_t log_buffers(size_t n) { return std::max<size_t>(log_blocks(n) + 2, 3); }

inline Vec mint_load(std::span<const mint> f, size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(size_t lo, size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const mint> f, size_t at) { return mint_load(f, at); }
	std::span<const mint> f_;
	size_t full_, touched_;
};
[[gnu::noinline]] inline void mint_forward_top4(MintSource in, Vec* out, size_t h, const Word* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void mint_forward_top2(MintSource in, Vec* out, size_t h) {
	forward_top2_body(in, out, h);
}

inline void mint_forward(const Transform& t, std::span<const mint> f, Words out) {
	const MintSource in(f);
	const detail::Recursion recursion(t.roots(), t.inverse_roots(), ForwardBottom{t.roots()});
	const size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		mint_forward_top4(in, v, h, t.roots());
		for (size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
	} else {
		const size_t h = nv / 2;
		mint_forward_top2(in, v, h);
		recursion.visit(out.data(), h, 0);
		recursion.visit(out.data() + 8 * h, h, 1);
	}
}
inline void mint_derivative(std::span<const mint> f, Words d) { derivative_impl(f, d); }
inline void mint_subtract_from_derivative(std::span<const mint> f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative(f, first, count, out, size);
}

struct NoLogTransform {
	void operator()(size_t, ConstWords) const {}
};

template <class Input, class Sink, class Transformed>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, Input f, size_t n,
												Words scratch, ConstWords prefix, const Sink& sink, const Transformed& transformed = {}) {
	const size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const Words ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, prefix, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (size_t s = 1; s < blocks; ++s) {
		const size_t from = std::min(f.size(), (s - 1) * k);
		if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (size_t j = 0; j < blocks; ++j) {
		const size_t first = j * k, count = std::min(k, n - 1 - first);
		const Words work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
			else derivative_impl(f.first(std::min(f.size(), count + 1)), work.first(count));
		} else {
			if (j == 1) {
				t.inverse_product(window(1), q0, work, Half::kUpper);
			} else if (blocks == 3) {
				const Transform::Pair pairs[2] = {{window(2), q0}, {window(1), work}};
				t.inverse_product_sum(pairs, work, Half::kUpper);
			} else if (j == 2) {
				t.inverse_with(ToeplitzBottom{t.roots(), t.inverse_roots(), window(1).data(), window(2).data(), q0.data(),
											work.data(), window(3).data()},
							work, Half::kUpper);
			} else {
				const InverseProductBottom product{{t.roots(), t.inverse_roots(), work.data()}, window(1).data()};
				t.inverse_with(SideProductBottom{product, window(3).data()}, work, Half::kUpper);
			}
			if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_subtract_from_derivative(f, first, count, work.data(), k);
			else subtract_derivative_impl(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, ConstWords(work.first(count)));
		if (j + 1 < blocks) {
			t.forward(work.first(k), 0, work);
			transformed(j, ConstWords(work));
		}
	}
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, ConstWords f, size_t n,
		Words scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative(t, f, n, scratch, f, sink, transformed);
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void mint_log_derivative(const Transform& t, std::span<const mint> f, size_t n,
		Words scratch, ConstWords prefix, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative(t, f, n, scratch, prefix, sink, transformed);
}

inline int log_derivative_log(size_t n) { return std::countr_zero(2 * log_block(n)); }

inline size_t log_derivative_scratch(size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}

inline int log_log(size_t n) {
	return n <= detail::kLogBase ? Transform::kMinLog : std::countr_zero(2 * detail::log_block(n));
}

inline size_t log_scratch(size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_buffers(n) * Arena::footprint(2 * detail::log_block(n));
}

inline void log(const Transform& t, ConstWords f, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

inline void native_log(const mint *in, size_t input, std::vector<mint> &out, size_t n, bool integrate) {
	prepare_fps_output(out, n);
	out.emplace_back(0);
	const size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	const bool direct = n >= 262144;
	const auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;
	Arena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(prefix_size), scratch = arena.take(log_scratch(n));
	copy_coefficients(in, f);
	const auto sink = [&out, integrate, n](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<Word *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
		}
		const auto *values = integrate ? q.data() + detail::log_block(n) : q.data();
		if (integrate) append_fps_words<true>(out, values, q.size());
		else append_fps_words<false>(out, values, q.size());
	};
	if (direct) detail::mint_log_derivative(t, {in, c}, n, scratch, f, sink);
	else detail::log_derivative(t, f, n, scratch, sink);
}
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_convolution_unbalanced.hpp"



#line 10 "components/fps_convolution_unbalanced.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::unbalanced {
using namespace blocked_poly;
inline bool fits(size_t n, size_t m) {
	const auto shorter = std::min(n, m), longer = std::max(n, m);
	return shorter > 32 && longer >= 32768 && shorter <= longer / 32;
}
inline std::vector<mint> multiply(std::span<const mint> a, std::span<const mint> b) {
	const size_t len = 8 * std::bit_ceil(std::min(a.size(), b.size()));
	const auto& x = a.size() >= b.size() ? a : b;
	const auto& y = a.size() >= b.size() ? b : a;
	const size_t n = x.size(), m = y.size(), stride = len - m + 1;
	const int lg = std::countr_zero(len);
	Arena arena(Transform::words(lg) + 3 * Arena::footprint(len));
	const Transform t(arena, lg);
	auto bt = arena.take(len), in = arena.take(len), out = arena.take(len);
	for (size_t i = 0; i < m; ++i) in[i] = y[i].val();
	t.forward(in.first(m), 0, bt);
	std::vector<mint> result;
	fps_detail::prepare_fps_output(result, n + m - 1);
	for (size_t p = 0; p < n; p += stride) {
		const size_t count = std::min(stride, n - p);
		for (size_t i = 0; i < count; ++i) {
			const unsigned v = x[p + i].v;
			in[i] = std::min(v, v - mint::mod());
		}
		t.cyclic_product(in.first(count), 0, out, bt);
		const size_t overlap = p == 0 ? 0 : m - 1;
		for (size_t i = 0; i < overlap; ++i) {
			const unsigned v = result[p + i].v + out[i];
			result[p + i] = mint::word(std::min(v, v - mint::mod()));
		}
		fps_detail::append_fps_words<true>(result, out.data() + overlap, count + m - 1 - overlap);
	}
	return result;
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_convolution_constant.hpp"



#line 11 "components/fps_convolution_constant.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::constant {
inline unsigned canonical(mint x) { return std::min(x.v, x.v - mint::mod()); }
inline bool same_eight(const mint* p, unsigned value) {
	__m256i v;
	std::memcpy(&v, p, sizeof(v));
	v = _mm256_min_epu32(v, _mm256_sub_epi32(v, _mm256_set1_epi32(mint::mod())));
	return _mm256_movemask_epi8(_mm256_cmpeq_epi32(v, _mm256_set1_epi32(value))) == -1;
}
inline bool remaining(std::span<const mint> a, unsigned value) {
	size_t i = 8;
	for (; i + 8 <= a.size(); i += 8) if (!same_eight(a.data() + i, value)) return false;
	for (; i < a.size(); ++i) if (canonical(a[i]) != value) return false;
	return true;
}
[[gnu::noinline]] inline bool fits(const std::vector<mint>& a, const std::vector<mint>& b) {
	if (std::min(a.size(), b.size()) < 4096) return false;
	const unsigned av = canonical(a[0]), bv = canonical(b[0]);
	if (!same_eight(a.data(), av) || !same_eight(b.data(), bv)) return false;
	return remaining(a, av) && (&a == &b || remaining(b, bv));
}
[[gnu::noinline]] inline std::vector<mint> multiply(std::span<const mint> a, std::span<const mint> b) {
	const size_t n = std::max(a.size(), b.size()), m = std::min(a.size(), b.size()), size = n + m - 1;
	const unsigned c = canonical(mint::word(canonical(a[0])) * mint::word(canonical(b[0])));
	std::vector<mint> result(size);
	if (c == 0) return result;
	alignas(32) unsigned first[8];
	for (unsigned i = 0; i < 8; ++i) first[i] = std::uint64_t(c) * (i + 1) % mint::mod();
	__m256i value = _mm256_load_si256(reinterpret_cast<const __m256i*>(first));
	const auto step = _mm256_set1_epi32(std::uint64_t(c) * 8 % mint::mod()), p = _mm256_set1_epi32(mint::mod());
	const auto reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	size_t i = 0;
	for (; i + 8 <= m; i += 8) {
		std::memcpy(static_cast<void*>(result.data() + i), &value, sizeof(value));
		const auto reflected = _mm256_permutevar8x32_epi32(value, reverse);
		std::memcpy(static_cast<void*>(result.data() + size - i - 8), &reflected, sizeof(reflected));
		value = _mm256_add_epi32(value, step);
		value = _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
	}
	unsigned current = _mm256_extract_epi32(value, 0);
	for (; i < m; ++i) {
		result[i] = result[size - i - 1] = mint::word(current);
		current += c;
		current = std::min(current, current - mint::mod());
	}
	if (n > m + 1) std::fill(result.begin() + m, result.begin() + n - 1, mint::word(std::uint64_t(c) * m % mint::mod()));
	return result;
}
}
#pragma GCC pop_options
#endif

#line 14 "components/fps_convolution.hpp"
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail {
namespace ntt {
namespace kernels = blocked_ntt::kernels;
namespace detail = blocked_ntt::detail;
class Convolution {
	size_t n_, m_, len_;
	blocked_poly::Arena arena_;
	blocked_poly::Transform transform_;
	Word *a_, *b_;
	static int log_length(size_t n, size_t m) {
		if (n == 0 || m == 0 || n > (1u << 23) || m > (1u << 23) || n + m - 1 > (1u << 23)) std::abort();
		return std::max(6, int(std::bit_width(n + m - 2)));
	}
public:
	Convolution(size_t n, size_t m) : Convolution(n, m, nullptr) {}
	Convolution(size_t n, size_t m, Word* borrowed)
		: n_(n), m_(m), len_(size_t(1) << log_length(n, m)),
		  arena_(blocked_poly::Transform::words(log_length(n, m)) + (borrowed ? 1 : 2) * blocked_poly::Arena::footprint(len_)),
		  transform_(arena_, log_length(n, m)), a_(borrowed ? borrowed : arena_.take(len_).data()), b_(arena_.take(len_).data()) {
		if (n == 0 || m == 0 || log_length(n, m) > 23) std::abort();
	}
	static bool fits(size_t n, size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= size_t(1) << lg;
	}
	Word* a() { return a_; }
	Word* b() { return b_; }
	const Word* multiply(bool square = false) {
		transform_.forward(ConstWords(b_, m_), 0, Words(b_, len_));
		if (square) transform_.inverse_product(ConstWords(b_, len_), ConstWords(b_, len_), Words(a_, len_),
			blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		else transform_.cyclic_product(ConstWords(a_, n_), 0,
			Words(a_, len_), ConstWords(b_, len_), blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		return a_;
	}
};
}
using Product = ntt::Convolution;
using ntt::detail::Vec, ntt::detail::Factor, ntt::detail::add, ntt::detail::diff, ntt::detail::kP, ntt::detail::reduce;
inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }
inline Vec diff_canonical(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y); }
class Subtrees {
	const Word *roots_, *inverse_;
public:
	Subtrees(const Word* r, const Word* ir) : roots_(r), inverse_(ir) {}
	void visit(Vec* a, Vec* b, size_t nv, size_t k) const {
		using namespace blocked_poly::detail;
		Recursion<ForwardBottom>(roots_, inverse_, ForwardBottom{roots_}).visit(reinterpret_cast<Word*>(b), nv, k);
		ProductBottom bottom{roots_, inverse_, reinterpret_cast<Word*>(b), k * nv};
		Recursion<ProductBottom>(roots_, inverse_, bottom).visit(reinterpret_cast<Word*>(a), nv, k);
	}
};
inline void forward_radix8(Vec* f, size_t q, const Word* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (size_t j = 0; j < q; ++j) {
		const Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];

		const Vec g0 = add(f0, f2), g1 = add(f1, f3);
		const Vec h0 = diff_canonical(f0, f2), ih1 = times(diff_canonical(f1, f3), i);
		f[j] = add(g0, g1), f[j + q] = diff(g0, g1);
		f[j + 2 * q] = add(h0, ih1), f[j + 3 * q] = diff(h0, ih1);

		const Vec if2 = times(f2, i), if3 = times(f3, i);
		const Vec u0 = reduce(add(f0, if2), 2 * kP), v0 = reduce(diff(f0, if2), 2 * kP);
		const Vec yu1 = times(add(f1, if3), y), zv1 = times(diff(f1, if3), z);
		f[j + 4 * q] = add(u0, yu1), f[j + 5 * q] = diff(u0, yu1);
		f[j + 6 * q] = add(v0, zv1), f[j + 7 * q] = diff(v0, zv1);
	}
}
inline void copy_input(const std::vector<mint> &in, Word *out, size_t len) {
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<mint>& in, mint* out) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = mint::word(std::min(in[i].v, in[i].v - mint::mod()));
}

template <class Plan>
inline std::vector<mint> run_borrowed_as(const std::vector<mint>& a, const std::vector<mint>& b) {
	const size_t len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr size_t alignment = 32;
	const size_t words = len + 16 + alignment / sizeof(mint) - 1, size = a.size() + b.size() - 1;
	std::vector<mint> result;
	prepare_fps_output(result, words);
	result.assign(words, mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(mint);
	mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<Word*>(to));
	if (&a != &b) copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply(&a == &b);
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(mint));
	result.resize(size);
	return result;
}

inline std::vector<mint> run_borrowed(const std::vector<mint>& a, const std::vector<mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<mint> run(const std::vector<mint> &a, const std::vector<mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<mint> result;
	const size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}


inline std::vector<mint> multiply(const std::vector<mint>& a, const std::vector<mint>& b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}
}
#pragma GCC pop_options
#endif

#line 10 "components/mint_convolution.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_convolution_detail {
inline std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b) {
	using NTT = fps_detail::NTT;
	using Word = NTT::U;
	if (a.empty() || b.empty()) return {};
	auto words = [](auto &v) { return reinterpret_cast<Word *>(v.data()); };
	auto input = [](const auto &v) { return reinterpret_cast<const Word *>(v.data()); };
	if (std::min(a.size(), b.size()) <= 32) {
		std::vector<mint> result(a.size() + b.size() - 1);
		const auto &small = a.size() < b.size() ? a : b;
		const auto &large = a.size() < b.size() ? b : a;
		for (int j = 0; j < static_cast<int>(small.size()); ++j)
			NTT::add_scaled(words(result) + j, input(large), large.size(), small[j].val());
		return result;
	}
	int n = 1, size = a.size() + b.size() - 1;
	while (n < size) n <<= 1;
	assert(n <= (1 << 23));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (NTT::vectorized()) return fps_detail::multiply_detail::multiply(a, b);
#endif
	std::vector<mint> x = a, y;
	x.resize(n);
	if (&a != &b) y = b, y.resize(n);
	auto &plan = fps_detail::ntt_plan(std::max(1, n / 2));
	plan.fft<false>(words(x), n);
	if (&a == &b) NTT::product(words(x), input(x), input(x), n);
	else {
		plan.fft<false>(words(y), n);
		NTT::product(words(x), input(x), input(y), n);
	}
	plan.fft<true>(words(x), n);
	plan.scale(words(x), n, mint(n).inv().val());
	x.resize(size);
	return x;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "convolution/convolution_fft.hpp"



#line 1 "components/fft.hpp"



#line 7 "components/fft.hpp"
#include <cmath>
#line 10 "components/fft.hpp"
#include <numbers>
#line 12 "components/fft.hpp"

template <class Real>
void fft(std::vector<std::complex<Real>> &a, bool inverse = false) {
	std::size_t n = a.size();
	assert(!n || std::has_single_bit(n));
	for (std::size_t i = 1, j = 0; i < n; ++i) {
		std::size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	for (std::size_t width = 2; width <= n; width *= 2) {
		Real angle = (inverse ? -2 : 2) * std::numbers::pi_v<Real> / Real(width);
		std::complex<Real> step(std::cos(angle), std::sin(angle));
		for (std::size_t start = 0; start < n; start += width) {
			std::complex<Real> w(1);
			for (std::size_t j = 0; j < width / 2; ++j, w *= step) {
				auto x = a[start + j], y = a[start + j + width / 2] * w;
				a[start + j] = x + y;
				a[start + j + width / 2] = x - y;
			}
		}
	}
	if (inverse)
		for (auto &x : a) x /= Real(n);
}


#line 11 "convolution/convolution_fft.hpp"

template <class Real>
std::vector<std::complex<Real>> convolution_fft(const std::vector<std::complex<Real>> &a, const std::vector<std::complex<Real>> &b) {
	if (a.empty() || b.empty()) return {};
	std::size_t length = a.size() + b.size() - 1, n = std::bit_ceil(length);
	std::vector<std::complex<Real>> x(a.begin(), a.end()), y(b.begin(), b.end());
	x.resize(n);
	y.resize(n);
	fft(x, false);
	if (&a == &b) y = x;
	else fft(y, false);
	for (std::size_t i = 0; i < n; ++i) x[i] *= y[i];
	fft(x, true);
	x.resize(length);
	return x;
}
template <class Real>
	requires std::is_floating_point_v<Real>
std::vector<Real> convolution_fft(const std::vector<Real> &a, const std::vector<Real> &b) {
	std::vector<std::complex<Real>> x(a.begin(), a.end()), y(b.begin(), b.end());
	auto values = &a == &b ? convolution_fft(x, x) : convolution_fft(x, y);
	std::vector<Real> result;
	result.reserve(values.size());
	for (const auto &x : values) result.push_back(x.real());
	return result;
}


#line 1 "convolution/convolution_mod.hpp"



#line 1 "components/ntt_mod.hpp"



#line 10 "components/ntt_mod.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace ntt_mod_detail {
template <std::uint32_t mod> struct Vector;

template <std::uint32_t mod>
std::uint32_t power(std::uint32_t a, std::uint32_t k) {
	std::uint32_t result = 1;
	for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
		if (k & 1) result = std::uint64_t(result) * a % mod;
	return result;
}

template <std::uint32_t mod>
void butterfly(std::uint32_t *a, std::size_t n, std::size_t half,
			   const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], v = a[start + j + half];
			std::uint32_t q = std::uint64_t(v) * quotients[j] >> 32;
			std::uint32_t y = v * roots[j] - q * mod;
			if (y >= mod) y -= mod;
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			a[start + j + half] = x >= y ? x - y : x + mod - y;
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void butterfly_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
					const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + half));
			auto r = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			auto q = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + j));
			y = V::multiply(y, r, q);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), V::add(x, y));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + half), V::subtract(x, y));
		}
}
#endif

template <std::uint32_t mod>
std::uint32_t shoup(std::uint32_t value, std::uint32_t factor, std::uint32_t quotient) {
	std::uint32_t q = std::uint64_t(value) * quotient >> 32;
	std::uint32_t result = value * factor - q * mod;
	return result < mod ? result : result - mod;
}


template <std::uint32_t mod>
void forward_stage(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], y = a[start + j + half];
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			std::uint32_t difference = x >= y ? x - y : x + mod - y;
			a[start + j + half] = shoup<mod>(difference, roots[j], quotients[j]);
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void forward_stage_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + half));
			auto r = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			auto q = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + j));
			auto difference = V::multiply(V::subtract(x, y), r, q);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), V::add(x, y));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + half), difference);
		}
}
#endif

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
struct Vector {
	static constexpr std::uint32_t range = mod < (std::uint32_t(1) << 30) ? 2 * mod : mod;
	__attribute__((target("avx2"), always_inline))
	static inline __m256i add(__m256i x, __m256i y) {
		auto sum = _mm256_add_epi32(x, y);
		return _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i subtract(__m256i x, __m256i y) {
		auto difference = _mm256_sub_epi32(x, y);
		return _mm256_min_epu32(difference, _mm256_add_epi32(difference, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i multiply(__m256i x, __m256i factor, __m256i quotient) {
		auto even = _mm256_mul_epu32(x, quotient);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(quotient, 32));
		auto q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
		auto result = _mm256_sub_epi32(_mm256_mullo_epi32(x, factor), _mm256_mullo_epi32(q, _mm256_set1_epi32(mod)));
		if constexpr (range == 2 * mod) return result;
		return _mm256_min_epu32(result, _mm256_sub_epi32(result, _mm256_set1_epi32(mod)));
	}
};

template <std::uint32_t mod, bool inverse>
__attribute__((target("avx2")))
void leaf_avx2(std::uint32_t *a, std::size_t n, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	auto r4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(roots + 4)));
	auto q4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(quotients + 4)));
	auto r2 = _mm256_setr_epi32(roots[2], roots[3], roots[2], roots[3], roots[2], roots[3], roots[2], roots[3]);
	auto q2 = _mm256_setr_epi32(quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3]);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		if constexpr (!inverse) {
			auto y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r4, q4), 0xf0);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
		} else {
			auto y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
			x = _mm256_blend_epi32(x, V::multiply(x, r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xcc);
			x = _mm256_blend_epi32(x, V::multiply(x, r4, q4), 0xf0);
			y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xf0);
		}
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
}

template <std::uint32_t mod, bool inverse>
__attribute__((target("avx2")))
void pair_avx2(std::uint32_t *a, std::size_t n, std::size_t h, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 4 * h)
		for (std::size_t j = 0; j < h; j += 8) {
			auto x0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto x1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + h));
			auto x2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + 2 * h));
			auto x3 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + 3 * h));
			auto r0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + 2 * h + j));
			auto q0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + 2 * h + j));
			auto r1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + 3 * h + j));
			auto q1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + 3 * h + j));
			auto r2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + h + j));
			auto q2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + h + j));
			if constexpr (!inverse) {
				auto y0 = V::add(x0, x2), y1 = V::add(x1, x3);
				auto y2 = V::multiply(V::subtract(x0, x2), r0, q0);
				auto y3 = V::multiply(V::subtract(x1, x3), r1, q1);
				x0 = V::add(y0, y1); x1 = V::multiply(V::subtract(y0, y1), r2, q2);
				x2 = V::add(y2, y3); x3 = V::multiply(V::subtract(y2, y3), r2, q2);
			} else {
				x1 = V::multiply(x1, r2, q2); x3 = V::multiply(x3, r2, q2);
				auto y0 = V::add(x0, x1), y1 = V::subtract(x0, x1);
				auto y2 = V::multiply(V::add(x2, x3), r0, q0);
				auto y3 = V::multiply(V::subtract(x2, x3), r1, q1);
				x0 = V::add(y0, y2); x2 = V::subtract(y0, y2);
				x1 = V::add(y1, y3); x3 = V::subtract(y1, y3);
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + h), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + 2 * h), x2);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + 3 * h), x3);
		}
}

#endif

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void build_roots_avx2(std::uint32_t *roots, std::uint32_t *quotients, std::size_t half, std::uint32_t step) {
	std::uint32_t initial[8], w = 1;
	for (int i = 0; i < 8; ++i) {
		initial[i] = w;
		w = std::uint64_t(w) * step % mod;
	}
	auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(initial));
	auto factor = _mm256_set1_epi32(w), quotient = _mm256_set1_epi32((std::uint64_t(w) << 32) / mod);
	for (std::size_t j = 0; j < half; j += 8) {
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(mod)));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(roots + j), x);
		for (int k = 0; k < 8; ++k) quotients[j + k] = (std::uint64_t(roots[j + k]) << 32) / mod;
		x = Vector<mod>::multiply(x, factor, quotient);
	}
}
#endif

template <std::uint32_t mod, std::uint32_t root>
struct Plan {
	std::vector<std::uint32_t> roots, quotients;
	explicit Plan(std::size_t n, [[maybe_unused]] bool vectorized = false) : roots(n), quotients(n) {
		static_assert(mod > 2 && mod < (std::uint32_t(1) << 31));
		assert(std::has_single_bit(n) && (mod - 1) % n == 0);
		if (n == 1) return;
		std::size_t half = n / 2;
		std::uint32_t step = power<mod>(root, (mod - 1) / n), w = 1;
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized && half >= 8) build_roots_avx2<mod>(roots.data() + half, quotients.data() + half, half, step);
		else
#endif
		for (std::size_t j = 0; j < half; ++j) {
			roots[half + j] = w;
			quotients[half + j] = (std::uint64_t(w) << 32) / mod;
			w = std::uint64_t(w) * step % mod;
		}
		for (half /= 2; half; half /= 2)
			for (std::size_t j = 0; j < half; ++j) {
				roots[half + j] = roots[2 * half + 2 * j];
				quotients[half + j] = quotients[2 * half + 2 * j];
			}
	}
};

template <std::uint32_t mod, std::uint32_t root>
void forward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = a.size() / 2;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		for (; half >= 16; half /= 4) pair_avx2<mod, false>(a.data(), a.size(), half / 2, plan.roots.data(), plan.quotients.data());
		if (half == 8) forward_stage_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		leaf_avx2<mod, false>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		return;
	}
#endif
	for (; half; half /= 2) forward_stage<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

template <std::uint32_t mod, std::uint32_t root>
void backward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = 1;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		leaf_avx2<mod, true>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		half = 8;

		for (; 2 * half < a.size(); half *= 4) pair_avx2<mod, true>(a.data(), a.size(), half, plan.roots.data(), plan.quotients.data());
		if (half < a.size()) butterfly_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		return;
	}
#endif
	for (; half < a.size(); half *= 2) butterfly<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void scale_avx2(std::uint32_t *a, std::size_t n, std::uint32_t factor, std::uint32_t quotient) {
	auto r = _mm256_set1_epi32(factor), q = _mm256_set1_epi32(quotient);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = Vector<mod>::multiply(x, r, q);
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), _mm256_min_epu32(y, _mm256_sub_epi32(y, _mm256_set1_epi32(mod))));
	}
}
#endif

template <std::uint32_t mod>
void scale(std::vector<std::uint32_t> &a, std::uint32_t factor, [[maybe_unused]] bool vectorized) {
	std::uint32_t quotient = (std::uint64_t(factor) << 32) / mod;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		scale_avx2<mod>(a.data(), a.size(), factor, quotient);
		return;
	}
#endif
	for (auto &x : a) x = shoup<mod>(x, factor, quotient);
}

inline void bit_reverse(std::vector<std::uint32_t> &a) {
	for (std::size_t i = 1, j = 0; i < a.size(); ++i) {
		std::size_t bit = a.size() >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
}

template <std::uint32_t mod, std::uint32_t root>
void transform(std::vector<std::uint32_t> &a, bool inverse, bool vectorized) {
	Plan<mod, root> plan(a.size(), vectorized);
	if (inverse) {
		bit_reverse(a);
		backward(a, plan, vectorized);
		std::reverse(a.begin() + 1, a.end());
		scale<mod>(a, mod - (mod - 1) / a.size(), vectorized);
	} else {
		forward(a, plan, vectorized);
		if constexpr (mod < (std::uint32_t(1) << 30)) for (auto &x : a) if (x >= mod) x -= mod;
		bit_reverse(a);
	}
}

template <std::uint32_t mod>
constexpr std::uint32_t negative_inverse = [] {
	std::uint32_t value = mod;
	for (int i = 0; i < 5; ++i) value *= 2 - mod * value;
	return 0 - value;
}();

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void pointwise_avx2(std::uint32_t *a, const std::uint32_t *b, std::size_t n) {
	auto p = _mm256_set1_epi32(mod), inverse = _mm256_set1_epi32(negative_inverse<mod>);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto even = _mm256_mul_epu32(x, y);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
		auto qe = _mm256_mul_epu32(even, inverse), qo = _mm256_mul_epu32(odd, inverse);
		even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(qe, p)), 32);
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(qo, p));
		x = _mm256_blend_epi32(even, odd, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
}
#endif

template <std::uint32_t mod>
void pointwise(std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, [[maybe_unused]] bool vectorized) {
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		pointwise_avx2<mod>(a.data(), b.data(), a.size());
		return;
	}
#endif
	for (std::size_t i = 0; i < a.size(); ++i) {
		std::uint64_t product = std::uint64_t(a[i]) * b[i];
		std::uint32_t q = std::uint32_t(product) * negative_inverse<mod>;
		std::uint32_t value = (product + std::uint64_t(q) * mod) >> 32;
		a[i] = value < mod ? value : value - mod;
	}
}

}

template <std::uint32_t mod, std::uint32_t root>
void ntt_mod(std::vector<std::uint32_t> &a, bool inverse = false) {
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::transform<mod, root>(a, inverse, vectorized);
}


#line 12 "convolution/convolution_mod.hpp"

namespace convolution_mod_detail {
using Word = std::uint32_t;
template <Word mod, Word root, class T>
std::vector<Word> multiply(const std::vector<T> &a, const std::vector<T> &b, Word factor = 1) {
	const auto &small = a.size() <= b.size() ? a : b;
	const auto &large = a.size() <= b.size() ? b : a;
	std::size_t length = a.size() + b.size() - 1;
	bool blocked = small.size() > 32 && large.size() / small.size() >= 8, square = &a == &b;
	std::size_t n = std::bit_ceil(blocked ? 2 * small.size() - 1 : length), block = n - small.size() + 1;
	std::vector<Word> x(n), y(square ? 0 : n);
	auto reduce = [](T value) {
		__int128_t r = __int128_t(value) % mod;
		return Word(r < 0 ? r + mod : r);
	};
	for (std::size_t i = 0; i < small.size(); ++i) x[i] = reduce(small[i]);
	if (small.size() <= 32) {
		if (!square) for (std::size_t i = 0; i < large.size(); ++i) y[i] = reduce(large[i]);
		const auto &other = square ? x : y;
		std::vector<Word> result(length);
		for (std::size_t i = 0; i < small.size(); ++i)
			for (std::size_t j = 0; j < large.size(); ++j) result[i + j] = (result[i + j] + std::uint64_t(x[i]) * other[j]) % mod;
		if (factor != 1) for (auto &v : result) v = std::uint64_t(v) * factor % mod;
		return result;
	}
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::Plan<mod, root> plan(n, vectorized);
	ntt_mod_detail::forward(x, plan, vectorized);
	Word scale = (std::uint64_t(1) << 32) % mod * (mod - (mod - 1) / n) % mod * factor % mod;
	std::vector<Word> result(blocked ? length : 0);
	auto &work = square ? x : y;
	for (std::size_t start = 0; start < large.size(); start += block) {
		std::size_t size = std::min(block, large.size() - start);
		if (!square) {
			for (std::size_t i = 0; i < size; ++i) y[i] = reduce(large[start + i]);
			std::fill(y.begin() + size, y.end(), 0);
			ntt_mod_detail::forward(y, plan, vectorized);
		}
		ntt_mod_detail::pointwise<mod>(work, x, vectorized);
		ntt_mod_detail::backward(work, plan, vectorized);
		std::reverse(work.begin() + 1, work.end());
		ntt_mod_detail::scale<mod>(work, scale, vectorized);
		if (!blocked) {
			work.resize(length);
			return std::move(work);
		}
		for (std::size_t i = 0; i < size + small.size() - 1; ++i) {
			Word sum = result[start + i] + work[i];
			result[start + i] = sum < mod ? sum : sum - mod;
		}
	}
	return result;
}
inline constexpr Word p = 754974721, q = 1224736769, r = 2013265921;
inline constexpr __uint128_t modulus = __uint128_t(p) * q * r;
struct Reconstruction { std::uint64_t low; Word high; };
template <Word P = p, Word Q = q, Word R = r>
inline Reconstruction reconstruct(Word a, Word b, Word c) {
	static const Word inverse_p = ntt_mod_detail::power<Q>(P, Q - 2);
	static const Word inverse_pq = ntt_mod_detail::power<R>(std::uint64_t(P) * Q % R, R - 2);
	std::uint64_t x = std::uint64_t(b + Q - a) * inverse_p % Q;
	std::uint64_t low = a + std::uint64_t(P) * x;
	std::uint64_t y = std::uint64_t(c + R - low % R) * inverse_pq % R;
	return {low, Word(y)};
}
inline __uint128_t restore(Word a, Word b, Word c) {
	auto [low, high] = reconstruct(a, b, c);
	return low + __uint128_t(p) * q * high;
}
#if defined(__x86_64__) || defined(__i386__)
template <Word P, Word Q, Word R>
__attribute__((target("avx2")))
void combine(Word *a, const Word *b, const Word *c, std::size_t n) {
	static_assert(P > 256 && Q > 256 && R > 256 && P + std::uint64_t(Q) + R < (std::uint64_t(1) << 32));
	constexpr Word mod = 1000000007;
	constexpr auto form = [](std::uint64_t x) { return Word(x % mod * ((std::uint64_t(1) << 32) % mod) % mod); };
	constexpr Word place[3] = {form(std::uint64_t(Q) * R), form(std::uint64_t(P) * R), form(std::uint64_t(P) * Q)};
	constexpr Word wrap = form(mod - std::uint64_t(P) * Q % mod * R % mod);
	const auto p = _mm256_set1_epi32(mod), inverse = _mm256_set1_epi32(ntt_mod_detail::negative_inverse<mod>);
	std::size_t i = 0;
	for (; n - i >= 8; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto z = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(c + i));
		auto quotient = [&](__m256i a, __m256i b, __m256i c) __attribute__((target("avx2"), always_inline)) {
			auto s = _mm256_mul_epu32(a, _mm256_set1_epi32((std::uint64_t(1) << 40) / P + 1));
			s = _mm256_add_epi64(s, _mm256_mul_epu32(b, _mm256_set1_epi32((std::uint64_t(1) << 40) / Q + 1)));
			s = _mm256_add_epi64(s, _mm256_mul_epu32(c, _mm256_set1_epi32((std::uint64_t(1) << 40) / R + 1)));
			return _mm256_srli_epi64(_mm256_add_epi64(s, _mm256_set1_epi64x(std::uint64_t(1) << 39)), 40);
		};
		auto reduce = [&](__m256i a, __m256i b, __m256i c, __m256i t) __attribute__((target("avx2"), always_inline)) {
			auto s = _mm256_add_epi64(_mm256_mul_epu32(a, _mm256_set1_epi32(place[0])), _mm256_mul_epu32(b, _mm256_set1_epi32(place[1])));
			s = _mm256_add_epi64(s, _mm256_mul_epu32(c, _mm256_set1_epi32(place[2])));
			s = _mm256_add_epi64(s, _mm256_mul_epu32(t, _mm256_set1_epi32(wrap)));
			return _mm256_add_epi64(s, _mm256_mul_epu32(_mm256_mul_epu32(s, inverse), p));
		};
		auto xo = _mm256_srli_epi64(x, 32), yo = _mm256_srli_epi64(y, 32), zo = _mm256_srli_epi64(z, 32);
		auto even = reduce(x, y, z, quotient(x, y, z)), odd = reduce(xo, yo, zo, quotient(xo, yo, zo));
		x = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
	for (; i < n; ++i) {
		std::uint64_t t = (std::uint64_t(a[i]) * ((std::uint64_t(1) << 40) / P + 1) + std::uint64_t(b[i]) * ((std::uint64_t(1) << 40) / Q + 1) + std::uint64_t(c[i]) * ((std::uint64_t(1) << 40) / R + 1) + (std::uint64_t(1) << 39)) >> 40;
		std::uint64_t sum = std::uint64_t(a[i]) * place[0] + std::uint64_t(b[i]) * place[1] + std::uint64_t(c[i]) * place[2] + std::uint64_t(t) * wrap;
		auto q = Word(sum) * ntt_mod_detail::negative_inverse<mod>;
		Word value = (sum + std::uint64_t(q) * mod) >> 32;
		a[i] = value < mod ? value : value - mod;
	}
}
#endif
template <Word P, Word PR, Word Q, Word QR, Word R, Word RR>
std::vector<Word> modular(const std::vector<Word> &a, const std::vector<Word> &b, Word mod) {
#if defined(__x86_64__) || defined(__i386__)
	if constexpr (P + std::uint64_t(Q) + R < (std::uint64_t(1) << 32)) {
		if (mod == 1000000007 && __uint128_t(std::min(a.size(), b.size())) * (mod - 1) * (mod - 1) * 4 < __uint128_t(P) * Q * R && __builtin_cpu_supports("avx2")) {
			auto x = multiply<P, PR>(a, b, ntt_mod_detail::power<P>(std::uint64_t(Q) * R % P, P - 2));
			auto y = multiply<Q, QR>(a, b, ntt_mod_detail::power<Q>(std::uint64_t(P) * R % Q, Q - 2));
			auto z = multiply<R, RR>(a, b, ntt_mod_detail::power<R>(std::uint64_t(P) * Q % R, R - 2));
			combine<P, Q, R>(x.data(), y.data(), z.data(), x.size());
			return x;
		}
	}
#endif
	auto x = multiply<P, PR>(a, b);
	auto y = multiply<Q, QR>(a, b);
	auto z = multiply<R, RR>(a, b);
	std::uint64_t pq_mod = std::uint64_t(P) * Q % mod;
	for (std::size_t i = 0; i < x.size(); ++i) {
		auto [low, high] = reconstruct<P, Q, R>(x[i], y[i], z[i]);
		x[i] = (low % mod + pq_mod * high) % mod;
	}
	return x;
}

}

template <class T>
std::vector<T> convolution_integer(const std::vector<T> &a, const std::vector<T> &b) {
	static_assert(std::numeric_limits<T>::is_integer && sizeof(T) <= 8);
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() - 1 <= std::size_t(1) << 24);
	auto x = convolution_mod_detail::multiply<convolution_mod_detail::p, 11>(a, b);
	auto y = convolution_mod_detail::multiply<convolution_mod_detail::q, 3>(a, b);
	auto z = convolution_mod_detail::multiply<convolution_mod_detail::r, 31>(a, b);
	std::vector<T> result(x.size());
	for (std::size_t i = 0; i < x.size(); ++i) {
		auto value = convolution_mod_detail::restore(x[i], y[i], z[i]);
		if constexpr (std::numeric_limits<T>::is_signed) {
			__int128_t v = value > convolution_mod_detail::modulus / 2 ? __int128_t(value) - __int128_t(convolution_mod_detail::modulus) : __int128_t(value);
			assert(std::numeric_limits<T>::lowest() <= v && v <= std::numeric_limits<T>::max());
			result[i] = T(v);
		} else {
			assert(value <= std::numeric_limits<T>::max());
			result[i] = T(value);
		}
	}
	return result;
}

template <class T>
std::vector<std::uint32_t> convolution_mod(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t mod) {
	static_assert(std::numeric_limits<T>::is_integer && sizeof(T) <= 8);
	assert(mod > 0);
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() - 1 <= std::size_t(1) << 24);
	if (mod == 1) return std::vector<std::uint32_t>(a.size() + b.size() - 1);
	std::vector<std::uint32_t> x(a.size()), y(&a == &b ? 0 : b.size());
	auto reduce = [=](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	std::transform(a.begin(), a.end(), x.begin(), reduce);
	if (&a != &b) std::transform(b.begin(), b.end(), y.begin(), reduce);
	const auto &other = &a == &b ? x : y;

	if (std::min(x.size(), other.size()) <= 32) {
		std::vector<std::uint32_t> result(x.size() + other.size() - 1);
		for (std::size_t i = 0; i < x.size(); ++i)
			for (std::size_t j = 0; j < other.size(); ++j) result[i + j] = (result[i + j] + std::uint64_t(x[i]) * other[j]) % mod;
		return result;
	}
	if (mod == 998244353 && x.size() + other.size() - 1 <= std::size_t(1) << 23) return convolution_mod_detail::multiply<998244353, 3>(x, other);
	constexpr std::uint32_t P = 167772161, Q = 469762049, R = 754974721;
	if (__uint128_t(std::min(x.size(), other.size())) * (mod - 1) * (mod - 1) < __uint128_t(P) * Q * R)
		return convolution_mod_detail::modular<P, 3, Q, 3, R, 11>(x, other, mod);
	return convolution_mod_detail::modular<convolution_mod_detail::p, 11, convolution_mod_detail::q, 3, convolution_mod_detail::r, 31>(x, other, mod);
}


#line 1 "convolution/convolution_ring.hpp"



#line 8 "convolution/convolution_ring.hpp"

namespace convolution_ring_detail {
template <class T>
std::vector<T> multiply(std::span<const T> a, std::span<const T> b) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> result(a.size() + b.size() - 1, T(0));
	if (std::min(a.size(), b.size()) <= 32) {
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) result[i + j] += a[i] * b[j];
		return result;
	}
	std::size_t half = std::max(a.size(), b.size()) / 2;
	auto al = a.first(std::min(a.size(), half)), ar = a.subspan(al.size());
	auto bl = b.first(std::min(b.size(), half)), br = b.subspan(bl.size());
	auto low = multiply(al, bl), high = multiply(ar, br);
	std::vector<T> x(std::max(al.size(), ar.size()), T(0)), y(std::max(bl.size(), br.size()), T(0));
	for (std::size_t i = 0; i < al.size(); ++i) x[i] += al[i];
	for (std::size_t i = 0; i < ar.size(); ++i) x[i] += ar[i];
	for (std::size_t i = 0; i < bl.size(); ++i) y[i] += bl[i];
	for (std::size_t i = 0; i < br.size(); ++i) y[i] += br[i];
	auto middle = multiply<T>(x, y);
	for (std::size_t i = 0; i < low.size(); ++i) result[i] += low[i], middle[i] -= low[i];
	for (std::size_t i = 0; i < high.size(); ++i) result[i + 2 * half] += high[i], middle[i] -= high[i];
	for (std::size_t i = 0; i < middle.size() && i + half < result.size(); ++i) result[i + half] += middle[i];
	return result;
}
}

template <class T>
std::vector<T> convolution_ring(const std::vector<T> &a, const std::vector<T> &b) {
	return convolution_ring_detail::multiply<T>(a, b);
}


#line 13 "components/convolution_dispatch.hpp"

namespace convolution_detail {
template <class T>
struct IsComplex : std::false_type {};
template <class T>
struct IsComplex<std::complex<T>> : std::true_type {};
}

template <class T>
std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			if constexpr (std::is_same_v<T, mint>) return mint_convolution_detail::multiply(a, b);
			else {
				std::vector<mint> x(a.size()), y(&a == &b ? 0 : b.size());
				for (std::size_t i = 0; i < a.size(); ++i) x[i] = mint::raw(a[i].val());
				if (&a != &b) for (std::size_t i = 0; i < b.size(); ++i) y[i] = mint::raw(b[i].val());
				auto values = &a == &b ? mint_convolution_detail::multiply(x, x) : mint_convolution_detail::multiply(x, y);
				std::vector<T> result;
				result.reserve(values.size());
				for (auto x : values) result.push_back(T::raw(x.val()));
				return result;
			}
		} else {
			std::vector<std::uint32_t> x(a.size()), y(b.size());
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
			for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
			auto values = convolution_mod(x, y, T::mod());
			std::vector<T> result;
			result.reserve(values.size());
			for (auto x : values) result.push_back(T::raw(x));
			return result;
		}
	} else if constexpr (std::numeric_limits<T>::is_integer && sizeof(T) <= 8) return convolution_integer(a, b);
	else if constexpr (std::is_floating_point_v<T> || convolution_detail::IsComplex<T>::value) return convolution_fft(a, b);
	else return convolution_ring(a, b);
}


#line 12 "convolution/multivariate_convolution.hpp"

template <class T>
std::vector<T> multivariate_convolution(const std::vector<T> &a, const std::vector<T> &b, const std::vector<int> &dimensions) {
	std::size_t size = 1;
	std::vector<std::size_t> bases;
	for (int d : dimensions) {
		assert(d > 0 && size <= std::size_t(-1) / d);
		if (d == 1) continue;
		size *= d;
		bases.push_back(size);
	}
	assert(a.size() == size && b.size() == size);
	if (bases.empty()) return {a[0] * b[0]};
	if (bases.size() == 1) {
		auto result = convolution(a, b);
		result.resize(size);
		return result;
	}
	int colors = bases.size();

	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			assert(size <= std::size_t(1) << 22);
			std::vector<mint> x(size), y(size);
			std::vector<int> dims;
			for (int d : dimensions) if (d > 1) dims.push_back(d);
			for (std::size_t i = 0; i < size; ++i) x[i] = mint::raw(a[i].val()), y[i] = mint::raw(b[i].val());
			std::unique_ptr<fps_detail::NTT> ntt;
			auto transform = [&](std::vector<mint> &v, bool inverse) {
				using NTT = fps_detail::NTT;
				if (!ntt) ntt = std::make_unique<NTT>(v.size() / 2, false);
				auto *p = reinterpret_cast<NTT::U *>(v.data());
				if (inverse) ntt->fft<true>(p, v.size()), ntt->scale(p, v.size(), 998244353 - 998244352 / v.size());
				else ntt->fft<false>(p, v.size());
			};
			auto product = [&] {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
				int degree = 0;
				bool small = true;
				for (int d : dims) degree += d - 1, small &= d <= 3;
				if (small && degree < 24 && std::count(dims.begin(), dims.end(), 2) >= 3 && multivariate_convolution_detail::Core::available())
					return multivariate_ranked_detail::multiply(x, y, dims);
#endif
				return multivariate_convolution_detail::Core::multiply(x, y, dims, transform, convolution<mint>);
			}();
			std::vector<T> result;
			result.reserve(size);
			for (auto v : product) result.push_back(T::raw(v.val()));
			return result;
		}
	}
	std::vector<int> rank(size);
	for (std::size_t i = 0; i < size; ++i) {
		std::size_t value = 0;
		for (std::size_t base : bases) value += i / base;
		rank[i] = value % colors;
	}
	std::size_t stride = 2 * colors - 1;
	assert(size <= std::size_t(-1) / stride / 2);
	std::vector<T> x(size * stride, T(0)), y(size * stride, T(0));
	for (std::size_t i = 0; i < size; ++i) x[i * stride + rank[i]] = a[i], y[i * stride + rank[i]] = b[i];
	auto product = convolution(x, y);
	std::vector<T> result(size, T(0));
	for (std::size_t i = 0; i < size; ++i) {
		result[i] = product[i * stride + rank[i]];
		if (rank[i] + colors < int(stride)) result[i] += product[i * stride + rank[i] + colors];
	}
	return result;
}


#line 3 "test/multivariate_convolution.test.cpp"
#include <cstdio>
#include <charconv>
#line 8 "test/multivariate_convolution.test.cpp"
[[gnu::always_inline]] inline int read() {
	static char buffer[1 << 16];
	static std::size_t pos = 0, size = 0;
	if (std::endian::native == std::endian::little && size - pos >= 16) {
		std::uint64_t digits;
		std::memcpy(&digits, buffer + pos, 8);
		digits ^= 0x3030303030303030ULL;
		if (!(digits & 0xf0f0f0f0f0f0f0f0ULL)) {
			digits = (digits * 2561 >> 8) & 0x00ff00ff00ff00ffULL;
			digits = (digits * 6553601 >> 16) & 0x0000ffff0000ffffULL;
			int x = digits * 42949672960001ULL >> 32;
			for (pos += 8; buffer[pos] >= '0' && buffer[pos] <= '9'; ++pos) x = x * 10 + buffer[pos] - '0';
			++pos;
			return x;
		}
	}
	auto next = [&]() { if (pos == size) size = fread(buffer, 1, sizeof(buffer), stdin), pos = 0; return size ? buffer[pos++] : char(0); };
	char c;
	do c = next(); while (c && c <= ' ');
	int x = 0;
	while (c >= '0' && c <= '9') x = x * 10 + c - '0', c = next();
	return x;
}
int main() {
	int k = read(), n = 1;
	std::vector<int> dimensions(k);
	for (int &d : dimensions) d = read(), n *= d;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) x = mint::raw(read());
	for (auto &x : b) x = mint::raw(read());
	auto result = multivariate_convolution(a, b, dimensions);
	std::vector<char> output(std::size_t(n) * 11);
	char *end = output.data();
	for (auto x : result) {
		end = std::to_chars(end, end + 10, x.val()).ptr;
		*end++ = ' ';
	}
	fwrite(output.data(), 1, end - output.data(), stdout);
}
