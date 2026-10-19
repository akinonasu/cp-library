#line 1 "test/fps_linear_recurrence.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#line 1 "fps/linear_recurrence.hpp"



#line 1 "fps/bostan_mori.hpp"



#line 1 "fps/fps.hpp"



#line 1 "components/fps_operations.hpp"



#line 1 "components/ntt.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
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
} // namespace fps_detail

using fps_mint = fps_detail::Mint;
using mint = fps_mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 22 "components/ntt.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct NTT {
	using T = Mint;
	typedef unsigned int U __attribute__((may_alias));
	using V = std::vector<unsigned int>;
	static constexpr U mod = 998244353, twice = 2 * mod, one = 301989884;
	V roots, inverse_roots, twists, inverse_twists;
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
	static U reduce(unsigned long long x) {
		return (x + (unsigned long long)(U(x) * 998244351u) * mod) >> 32;
	}
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
	NTT(int size, bool composition = true)
		: roots(size, one),
		  inverse_roots(size, one),
		  twists(composition ? size : 0, one),
		  inverse_twists(composition ? size : 0, one),
		  root_quotients(size),
		  inverse_root_quotients(size) {
		for (int h = 1; h < size; h <<= 1) {
			U r = in(T(3).pow((mod - 1) / (4 * h)).val());
			U ir = in(T(3).pow(mod - 1 - (mod - 1) / (4 * h)).val());
			U t = in(T(3).pow((mod - 1) / (2 * h)).val());
			U it = in(T(3).pow(mod - 1 - (mod - 1) / (2 * h)).val());
			U a = in(mod - (mod - 1) / h), b = a;
			for (int i = 0; i < h; ++i) {
				roots[h + i] = mul(roots[i], r);
				inverse_roots[h + i] = mul(inverse_roots[i], ir);
				if (composition) twists[h + i] = a, inverse_twists[h + i] = b;
				a = mul(a, t);
				b = mul(b, it);
			}
		}
		for (int i = 0; i < size; ++i) {
			root_quotients[i] = (roots[i] < mod ? roots[i] : roots[i] - mod) * 998244351u;
			inverse_root_quotients[i] = (inverse_roots[i] < mod ? inverse_roots[i] : inverse_roots[i] - mod) * 998244351u;
			roots[i] = out(roots[i]);
			inverse_roots[i] = out(inverse_roots[i]);
			if (composition) twists[i] = out(twists[i]), inverse_twists[i] = out(inverse_twists[i]);
		}
	}

#if defined(__ARM_NEON)
	using I = uint32x4_t;
	static I mul4(I a, I b) {
		uint64x2_t x = vmull_u32(vget_low_u32(a), vget_low_u32(b));
		uint64x2_t y = vmull_u32(vget_high_u32(a), vget_high_u32(b));
		auto reduce2 = [](uint64x2_t z) {
			uint32x2_t q = vmul_u32(vmovn_u64(z), vdup_n_u32(998244351));
			return vshrn_n_u64(vaddq_u64(z, vmull_u32(q, vdup_n_u32(mod))), 32);
		};
		return vcombine_u32(reduce2(x), reduce2(y));
	}
	static I add4(I a, I b) {
		I c = vaddq_u32(a, b);
		return vminq_u32(c, vsubq_u32(c, vdupq_n_u32(twice)));
	}
	static I sub4(I a, I b) {
		I c = vsubq_u32(a, b);
		return vminq_u32(c, vaddq_u32(c, vdupq_n_u32(twice)));
	}
	static I fixed_mul4(I a, I r, I q) {
		uint64x2_t x = vmull_u32(vget_low_u32(a), vget_low_u32(q));
		uint64x2_t y = vmull_u32(vget_high_u32(a), vget_high_u32(q));
		I t = vcombine_u32(vshrn_n_u64(x, 32), vshrn_n_u64(y, 32));
		return vmlsq_u32(vmulq_u32(a, r), t, vdupq_n_u32(mod));
	}
	template <bool inverse>
	static void fixed_butterfly4(I &x, I &y, I r, I q, bool unit = false) {
		if constexpr (!inverse)
			if (!unit) y = fixed_mul4(y, r, q);
		I z = sub4(x, y);
		x = add4(x, y);
		y = inverse && !unit ? fixed_mul4(z, r, q) : z;
	}
#endif
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
	__attribute__((target("avx2"))) static I8 mul8(I8 a, I8 b) {
		I8 x = _mm256_mul_epu32(a, b);
		I8 y = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
		I8 r = _mm256_set1_epi32(998244351), p = _mm256_set1_epi32(mod);
		x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, r), p));
		y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, r), p));
		return _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
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

	template <bool inverse, bool lazy = false>
	__attribute__((target("avx2"), always_inline)) static void fixed_butterfly8(I8 &x, I8 &y, I8 r, I8 q, bool unit = false) {
		if constexpr (lazy && !inverse) {
			I8 p = _mm256_set1_epi32(twice);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = unit ? _mm256_min_epu32(y, _mm256_sub_epi32(y, p)) : fixed_mul8(y, r, q);
			I8 z = _mm256_sub_epi32(_mm256_add_epi32(x, p), y);
			x = _mm256_add_epi32(x, y);
			y = z;
		} else {
			if constexpr (!inverse)
				if (!unit) y = fixed_mul8(y, r, q);
			I8 z = sub8(x, y);
			x = add8(x, y);
			y = inverse && !unit ? fixed_mul8(z, r, q) : z;
		}
	}
	template <bool inverse, bool lazy = false>
	__attribute__((target("avx2"))) void stage_pair8(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		for (int j = 0, k = first; j < length; j += 4 * stride, ++k) {
			I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			I8 r1 = _mm256_set1_epi32(w[2 * k + 1]), q1 = _mm256_set1_epi32(q[2 * k + 1]);
			for (int i = 0; i < stride; i += 8) {
				I8 x0 = load8(a + j + i), x1 = load8(a + j + stride + i);
				I8 x2 = load8(a + j + 2 * stride + i), x3 = load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					fixed_butterfly8<inverse, lazy>(x0, x1, r0, q0, k == 0);
					fixed_butterfly8<inverse, lazy>(x2, x3, r1, q1, false);
					fixed_butterfly8<inverse, lazy>(x0, x2, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x1, x3, r, rq, k == 0);
				} else {
					fixed_butterfly8<inverse, lazy>(x0, x2, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x1, x3, r, rq, k == 0);
					fixed_butterfly8<inverse, lazy>(x0, x1, r0, q0, k == 0);
					fixed_butterfly8<inverse, lazy>(x2, x3, r1, q1, false);
				}
				store8(a + j + i, x0);
				store8(a + j + stride + i, x1);
				store8(a + j + 2 * stride + i, x2);
				store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	__attribute__((target("avx2"))) static void fold8(U *a, int columns) {
		for (int i = 0; i < columns; i += 8) {
			I8 x = add8(load8(a + i), load8(a + columns + i));
			store8(a + i, x);
			store8(a + columns + i, x);
		}
	}
	__attribute__((target("avx2"))) void project8_8(U *a, int rows) const {
		for (int j = 0; j < rows; ++j) {
			I8 x0 = load8(a + 16 * j + 0);
			x0 = small8<4, true>(small8<2, true>(small8<1, true>(x0, 0), 0), 0);
			I8 x1 = load8(a + 16 * j + 8);
			x1 = small8<4, true>(small8<2, true>(small8<1, true>(x1, 1), 1), 1);
			x0 = add8(x0, x1);
			x1 = x0;
			x0 = small8<1, false>(small8<2, false>(small8<4, false>(x0, 0), 0), 0);
			store8(a + 16 * j + 0, x0);
			x1 = small8<1, false>(small8<2, false>(small8<4, false>(x1, 1), 1), 1);
			store8(a + 16 * j + 8, x1);
		}
	}
	__attribute__((target("avx2"))) void project16_8(U *a, int rows) const {
		for (int j = 0; j < rows; ++j) {
			I8 x0 = load8(a + 32 * j + 0);
			x0 = small8<4, true>(small8<2, true>(small8<1, true>(x0, 0), 0), 0);
			I8 x1 = load8(a + 32 * j + 8);
			x1 = small8<4, true>(small8<2, true>(small8<1, true>(x1, 1), 1), 1);
			I8 x2 = load8(a + 32 * j + 16);
			x2 = small8<4, true>(small8<2, true>(small8<1, true>(x2, 2), 2), 2);
			I8 x3 = load8(a + 32 * j + 24);
			x3 = small8<4, true>(small8<2, true>(small8<1, true>(x3, 3), 3), 3);
			fixed_butterfly8<true>(x0, x1, _mm256_set1_epi32(inverse_roots[0]), _mm256_set1_epi32(inverse_root_quotients[0]), true);
			fixed_butterfly8<true>(x2, x3, _mm256_set1_epi32(inverse_roots[1]), _mm256_set1_epi32(inverse_root_quotients[1]), false);
			x0 = add8(x0, x2);
			x2 = x0;
			x1 = add8(x1, x3);
			x3 = x1;
			fixed_butterfly8<false>(x0, x1, _mm256_set1_epi32(roots[0]), _mm256_set1_epi32(root_quotients[0]), true);
			fixed_butterfly8<false>(x2, x3, _mm256_set1_epi32(roots[1]), _mm256_set1_epi32(root_quotients[1]), false);
			x0 = small8<1, false>(small8<2, false>(small8<4, false>(x0, 0), 0), 0);
			store8(a + 32 * j + 0, x0);
			x1 = small8<1, false>(small8<2, false>(small8<4, false>(x1, 1), 1), 1);
			store8(a + 32 * j + 8, x1);
			x2 = small8<1, false>(small8<2, false>(small8<4, false>(x2, 2), 2), 2);
			store8(a + 32 * j + 16, x2);
			x3 = small8<1, false>(small8<2, false>(small8<4, false>(x3, 3), 3), 3);
			store8(a + 32 * j + 24, x3);
		}
	}
	__attribute__((target("avx2"))) static void scale_rows8(U *a, int rows, int columns, const U *factors) {
		if (columns == 2) {
			int j = 0;
			for (; j + 1 < rows; j += 2) {
				U w0 = factors[j], w1 = factors[j + 1], q0 = quotient(w0), q1 = quotient(w1);
				I8 w = _mm256_setr_epi32(w0, w0, w0, w0, w1, w1, w1, w1);
				I8 q = _mm256_setr_epi32(q0, q0, q0, q0, q1, q1, q1, q1);
				store8(a + 4 * j, fixed_mul8(load8(a + 4 * j), w, q));
			}
			if (j < rows)
				for (int i = 0; i < 4; ++i) a[4 * j + i] = fixed_mul(a[4 * j + i], factors[j], quotient(factors[j]));
		} else {
			for (int j = 0; j < rows; ++j) {
				U w = factors[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				for (int i = 0; i < 2 * columns; i += 8) store8(a + 2 * columns * j + i, fixed_mul8(load8(a + 2 * columns * j + i), v, q));
			}
		}
	}
	__attribute__((target("avx2"))) void project4_8(U *a, int rows) const {
		for (int j = 0; j < rows; ++j) {
			I8 x = small8<2, true>(small8<1, true>(load8(a + 8 * j), 0), 0);
			x = add8(x, _mm256_permute2x128_si256(x, x, 1));
			x = small8<1, false>(small8<2, false>(x, 0), 0);
			store8(a + 8 * j, x);
		}
	}
	__attribute__((target("avx2"))) static void load_pairs(const U *a, I8 &x, I8 &y) {
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		I8 u = _mm256_permutevar8x32_epi32(load8(a), index),
		   v = _mm256_permutevar8x32_epi32(load8(a + 8), index);
		x = _mm256_permute2x128_si256(u, v, 0x20);
		y = _mm256_permute2x128_si256(u, v, 0x31);
	}
	__attribute__((target("avx2"))) static void store_pairs(U *a, I8 x, I8 y) {
		I8 index = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
		store8(a, _mm256_permutevar8x32_epi32(_mm256_permute2x128_si256(x, y, 0x20), index));
		store8(a + 8,
			   _mm256_permutevar8x32_epi32(_mm256_permute2x128_si256(x, y, 0x31), index));
	}
	__attribute__((target("avx2"))) static void scale_copy8(U *dst, const U *src, int length, U r) {
		U quotient_r = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient_r);
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(dst + i, fixed_mul8(load8(src + i), v, q));
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, quotient_r);
	}
	__attribute__((target("avx2"))) static void scale_normalize8(U *a, int n, U r) {
		U qr = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(qr), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = fixed_mul8(load8(a + i), v, q);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (; i < n; ++i) {
			U x = fixed_mul(a[i], r, qr);
			a[i] = x < mod ? x : x - mod;
		}
	}
	__attribute__((target("avx2"))) static void square_fold8(U *a, const U *q, int n, bool negative) {
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(a + i), y = load8(a + n + i);
			I8 u = mul8(load8(q + i), load8(q + i));
			I8 v = mul8(load8(q + n + i), load8(q + n + i));
			x = add8(x, x);
			store8(a + i, negative ? sub8(x, u) : add8(x, u));
			store8(a + n + i, add8(add8(y, y), v));
		}
	}
	__attribute__((target("avx2"))) static void scale8(U *a, int length, U r) {
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient(r));
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(a + i, fixed_mul8(load8(a + i), v, q));
		for (; i < length; ++i) a[i] = fixed_mul(a[i], r, quotient(r));
	}
	__attribute__((target("avx2"))) static void pair_product8(U *a, const U *b, int length) {
		I8 ni = _mm256_set1_epi32(998244351), modulus = _mm256_set1_epi32(mod), p = _mm256_set1_epi32(twice);
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		for (int i = 0; i < length; i += 8) {
			I8 x = load8(b + 2 * i), y = load8(b + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			x = _mm256_mul_epu32(x, _mm256_srli_epi64(x, 32));
			y = _mm256_mul_epu32(y, _mm256_srli_epi64(y, 32));
			x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, ni), modulus));
			y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, ni), modulus));
			x = _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
			store8(a + i, _mm256_permutevar8x32_epi32(x, index));
		}
	}
	__attribute__((target("avx2"))) static void pair_multiply8(U *a, const U *b, int length) {
		I8 index0 = _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3), index1 = _mm256_setr_epi32(4, 4, 5, 5, 6, 6, 7, 7);
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < length; i += 8) {
			I8 r = load8(b + i), x = load8(a + 2 * i), y = load8(a + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			x = _mm256_shuffle_epi32(x, 0xb1);
			y = _mm256_shuffle_epi32(y, 0xb1);
			store8(a + 2 * i, mul8(x, _mm256_permutevar8x32_epi32(r, index0)));
			store8(a + 2 * i + 8, mul8(y, _mm256_permutevar8x32_epi32(r, index1)));
		}
	}
	__attribute__((target("avx2"))) static void difference8(U *a, const U *b, const U *c,
															int length) {
		for (int i = 0; i < length; i += 8) store8(a + i, sub8(load8(b + i), load8(c + i)));
	}
	__attribute__((target("avx2"))) static void project1_8(U *a, int rows) {
		for (int j = 0; j < rows; j += 8) {
			I8 x, y;
			load_pairs(a + 2 * j, x, y);
			x = add8(x, y);
			store_pairs(a + 2 * j, x, x);
		}
	}
	__attribute__((target("avx2"))) void project2_8(U *a, int rows) const {
		const I8 index = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
		for (int j = 0; j < rows; j += 8) {
			U *p = a + 4 * j;
			I8 u = load8(p), v = load8(p + 8), w = load8(p + 16), z = load8(p + 24);
			I8 uv0 = _mm256_unpacklo_epi32(u, v), uv1 = _mm256_unpackhi_epi32(u, v);
			I8 wz0 = _mm256_unpacklo_epi32(w, z), wz1 = _mm256_unpackhi_epi32(w, z);
			I8 a0 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(uv0, wz0), index);
			I8 a1 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(uv0, wz0), index);
			I8 a2 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(uv1, wz1), index);
			I8 a3 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(uv1, wz1), index);
			I8 x = add8(add8(a0, a1), add8(a2, a3));
			I8 y =
				add8(sub8(a0, a1), fixed_mul8(sub8(a2, a3), _mm256_set1_epi32(inverse_roots[1]),
											  _mm256_set1_epi32(inverse_root_quotients[1])));
			a0 = add8(x, y);
			a1 = sub8(x, y);
			y = fixed_mul8(y, _mm256_set1_epi32(roots[1]),
						   _mm256_set1_epi32(root_quotients[1]));
			a2 = add8(x, y);
			a3 = sub8(x, y);
			uv0 = _mm256_unpacklo_epi32(a0, a1);
			uv1 = _mm256_unpackhi_epi32(a0, a1);
			wz0 = _mm256_unpacklo_epi32(a2, a3);
			wz1 = _mm256_unpackhi_epi32(a2, a3);
			u = _mm256_unpacklo_epi64(uv0, wz0);
			v = _mm256_unpackhi_epi64(uv0, wz0);
			w = _mm256_unpacklo_epi64(uv1, wz1);
			z = _mm256_unpackhi_epi64(uv1, wz1);
			store8(p, _mm256_permute2x128_si256(u, v, 0x20));
			store8(p + 8, _mm256_permute2x128_si256(w, z, 0x20));
			store8(p + 16, _mm256_permute2x128_si256(u, v, 0x31));
			store8(p + 24, _mm256_permute2x128_si256(w, z, 0x31));
		}
	}
#endif

#if defined(__ARM_NEON)
	template <int stride, bool inverse>
	void small4(I &a, I &b, int k) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		if constexpr (stride == 4) {
			if (k == 0) {
				I x = sub4(a, b);
				a = add4(a, b);
				b = x;
			} else
				fixed_butterfly4<inverse>(a, b, vdupq_n_u32(w[k]), vdupq_n_u32(quot[k]));
		}
		if constexpr (stride == 2) {
			I x = vcombine_u32(vget_low_u32(a), vget_low_u32(b));
			I y = vcombine_u32(vget_high_u32(a), vget_high_u32(b));
			fixed_butterfly4<inverse>(
				x, y, vcombine_u32(vdup_n_u32(w[2 * k]), vdup_n_u32(w[2 * k + 1])),
				vcombine_u32(vdup_n_u32(quot[2 * k]), vdup_n_u32(quot[2 * k + 1])));
			a = vcombine_u32(vget_low_u32(x), vget_low_u32(y));
			b = vcombine_u32(vget_high_u32(x), vget_high_u32(y));
		}
		if constexpr (stride == 1) {
			uint32x4x2_t z = vuzpq_u32(a, b);
			fixed_butterfly4<inverse>(z.val[0], z.val[1], vld1q_u32(w.data() + 4 * k),
									  vld1q_u32(quot.data() + 4 * k));
			z = vzipq_u32(z.val[0], z.val[1]);
			a = z.val[0];
			b = z.val[1];
		}
	}
	template <bool inverse>
	void fft4(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
		if constexpr (!inverse) {
			int h = limit / 2;
			for (; h >= 16; h >>= 2) stage_pair<false>(a, h / 2, length);
			if (h == 8) stage<false>(a, h, length);
		}
		for (int j = 0; j < length; j += 8) {
			I x = vld1q_u32(a + j), y = vld1q_u32(a + j + 4);
			if constexpr (inverse) {
				small4<1, true>(x, y, j / 8);
				small4<2, true>(x, y, j / 8);
				small4<4, true>(x, y, j / 8);
			} else {
				small4<4, false>(x, y, j / 8);
				small4<2, false>(x, y, j / 8);
				small4<1, false>(x, y, j / 8);
			}
			vst1q_u32(a + j, x);
			vst1q_u32(a + j + 4, y);
		}
		if constexpr (inverse) {
			int h = 8;
			for (; 4 * h <= limit; h <<= 2) stage_pair<true>(a, h, length);
			if (h < limit) stage<true>(a, h, length);
		}
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
#if defined(__ARM_NEON)
			for (; i + 4 <= stride; i += 4) {
				I x = vld1q_u32(a + j + i), y = vld1q_u32(a + j + stride + i);
				if constexpr (!inverse)
					if (k) y = fixed_mul4(y, vdupq_n_u32(r), vdupq_n_u32(q));
				I z = sub4(x, y);
				x = add4(x, y);
				if constexpr (inverse)
					if (k) z = fixed_mul4(z, vdupq_n_u32(r), vdupq_n_u32(q));
				y = z;
				vst1q_u32(a + j + i, x);
				vst1q_u32(a + j + stride + i, y);
			}
#endif
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
	void stage_pair(U *a, int stride, int length, int first = 0) const {
#if defined(__x86_64__) || defined(__i386__)
		if (stride >= 8 && vectorized()) {
			stage_pair8<inverse>(a, stride, length, first);
			return;
		}
#endif
#if defined(__ARM_NEON)
		if (stride >= 4) {
			const V &w = inverse ? inverse_roots : roots;
			const V &q = inverse ? inverse_root_quotients : root_quotients;
			for (int j = 0, k = first; j < length; j += 4 * stride, ++k) {
				I r = vdupq_n_u32(w[k]), rq = vdupq_n_u32(q[k]);
				I r0 = vdupq_n_u32(w[2 * k]), q0 = vdupq_n_u32(q[2 * k]);
				I r1 = vdupq_n_u32(w[2 * k + 1]), q1 = vdupq_n_u32(q[2 * k + 1]);
				for (int i = 0; i < stride; i += 4) {
					I x0 = vld1q_u32(a + j + i), x1 = vld1q_u32(a + j + stride + i);
					I x2 = vld1q_u32(a + j + 2 * stride + i),
					  x3 = vld1q_u32(a + j + 3 * stride + i);
					if constexpr (inverse) {
						fixed_butterfly4<true>(x0, x1, r0, q0, k == 0);
						fixed_butterfly4<true>(x2, x3, r1, q1);
						fixed_butterfly4<true>(x0, x2, r, rq, k == 0);
						fixed_butterfly4<true>(x1, x3, r, rq, k == 0);
					} else {
						fixed_butterfly4<false>(x0, x2, r, rq, k == 0);
						fixed_butterfly4<false>(x1, x3, r, rq, k == 0);
						fixed_butterfly4<false>(x0, x1, r0, q0, k == 0);
						fixed_butterfly4<false>(x2, x3, r1, q1);
					}
					vst1q_u32(a + j + i, x0);
					vst1q_u32(a + j + stride + i, x1);
					vst1q_u32(a + j + 2 * stride + i, x2);
					vst1q_u32(a + j + 3 * stride + i, x3);
				}
			}
			return;
		}
#endif
		if constexpr (inverse)
			stage<true>(a, stride, length, 2 * first), stage<true>(a, 2 * stride, length, first);
		else
			stage<false>(a, 2 * stride, length, first), stage<false>(a, stride, length, 2 * first);
	}
	template <bool inverse>
	void fft(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
#if defined(__ARM_NEON)
		if (limit >= 8) {
			fft4<inverse>(a, length, limit);
			return;
		}
#elif defined(__x86_64__) || defined(__i386__)
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
	__attribute__((target("avx2"))) static void evaluate_leaf8(U *dst, const U *cur, const U *tree, int degree, const U *points, int count) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 x = load8(cur);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		alignas(32) U window[16] = {}, coefficients[8] = {}, polynomial[8], tail[8] = {};
		store8(window + 8, x);
		std::copy_n(tree, std::min(degree, 8), coefficients);
		I8 y = load8(coefficients);
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
		store8(coefficients, _mm256_min_epu32(y, _mm256_sub_epi32(y, p)));
		I8 e = _mm256_setzero_si256(), o = e;
		for (int i = 0; i < 8; ++i) {
			I8 z = load8(window + 8 - i), r = _mm256_set1_epi32(coefficients[i]);
			e = _mm256_add_epi64(e, _mm256_mul_epu32(z, r));
			o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
		}
		I8 ni = _mm256_set1_epi32(998244351);
		e = _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, ni), p));
		o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, ni), p));
		x = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		store8(polynomial, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		if (count == 8) x = load8(points);
		else {
			std::copy_n(points, count, tail);
			x = load8(tail);
		}
		I8 value = _mm256_setzero_si256();
		for (int i = 0; i < 8; ++i) value = add8(mul8(value, x), _mm256_set1_epi32(polynomial[i]));
		if (count == 8) store8(dst, value);
		else {
			store8(tail, value);
			std::copy_n(tail, count, dst);
		}
	}
	__attribute__((target("avx2"), always_inline)) static U sum_product8(I8 a, I8 b) {
		I8 p = _mm256_set1_epi32(mod);
		a = _mm256_min_epu32(a, _mm256_sub_epi32(a, p));
		b = _mm256_min_epu32(b, _mm256_sub_epi32(b, p));
		I8 sum = _mm256_add_epi64(_mm256_mul_epu32(a, b), _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
		sum = _mm256_add_epi64(sum, _mm256_permute2x128_si256(sum, sum, 1));
		sum = _mm256_add_epi64(sum, _mm256_shuffle_epi32(sum, 0x4e));
		U result = reduce((unsigned long long)_mm_cvtsi128_si64(_mm256_castsi256_si128(sum)));
		return result < twice ? result : result - twice;
	}
	__attribute__((target("avx2"))) static void interpolate_leaf8(U *dst, const U *q, const U *points, const U *weights) {
		I8 x = load8(points), w = load8(weights), c = _mm256_set1_epi32(one);
		I8 sum = add8(w, _mm256_permute2x128_si256(w, w, 1));
		sum = add8(sum, _mm256_shuffle_epi32(sum, 0x4e));
		sum = add8(sum, _mm256_shuffle_epi32(sum, 0xb1));
		dst[7] = _mm_cvtsi128_si32(_mm256_castsi256_si128(sum));
		for (int k = 7 - 1; k >= 0; --k) {
			c = add8(mul8(c, x), _mm256_set1_epi32(q[k + 1]));
			dst[k] = sum_product8(c, w);
		}
	}
	__attribute__((target("avx2"))) static void evaluate_middle8(U *left, U *right, const U *cur, const U *ql, int dl, const U *qr, int dr) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		alignas(32) U window[16], coefficients[2][9] = {};
		for (int i = 0; i < 16; i += 8) {
			I8 x = load8(cur + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			store8(window + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (int i = 0; i < std::min(dl, 9); ++i) {
			U x = ql[i];
			if (x >= twice) x -= twice;
			if (x >= mod) x -= mod;
			coefficients[0][i] = x;
		}
		for (int i = 0; i < std::min(dr, 9); ++i) {
			U x = qr[i];
			if (x >= twice) x -= twice;
			if (x >= mod) x -= mod;
			coefficients[1][i] = x;
		}
		I8 e[2] = {}, o[2] = {};
		for (int i = 0; i < 9; ++i) {
			I8 z = load8(window + 8 - i), high = _mm256_srli_epi64(z, 32);
			for (int k = 0; k < 2; ++k) {
				I8 r = _mm256_set1_epi32(coefficients[k][i]);
				e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(z, r));
				o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(high, r));
			}
		}
		U *dst[2] = {left, right};
		for (int k = 0; k < 2; ++k) {
			e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(_mm256_mul_epu32(e[k], ni), p));
			o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(_mm256_mul_epu32(o[k], ni), p));
			I8 x = _mm256_blend_epi32(_mm256_srli_epi64(e[k], 32), o[k], 0xaa);
			store8(dst[k], _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
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
		I8 inverse = _mm256_set1_epi32(one);
		for (U e = mod - 2; e; e >>= 1, p = mul8(p, p))
			if (e & 1) inverse = mul8(inverse, p);
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
	static void scale_normalize(U *a, int n, U r) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			scale_normalize8(a, n, r);
			return;
		}
#endif
		U qr = quotient(r);
		int i = 0;
#if defined(__ARM_NEON)
		I v = vdupq_n_u32(r), q = vdupq_n_u32(qr), p = vdupq_n_u32(mod);
		for (; i + 4 <= n; i += 4) {
			I x = fixed_mul4(vld1q_u32(a + i), v, q);
			vst1q_u32(a + i, vminq_u32(x, vsubq_u32(x, p)));
		}
#endif
		for (; i < n; ++i) {
			U x = fixed_mul(a[i], r, qr);
			a[i] = x < mod ? x : x - mod;
		}
	}
	static void square_fold(U *a, const U *q, int n, bool negative) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			square_fold8(a, q, n, negative);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(a + i), y = vld1q_u32(a + n + i);
			I u = mul4(vld1q_u32(q + i), vld1q_u32(q + i));
			I v = mul4(vld1q_u32(q + n + i), vld1q_u32(q + n + i));
			x = add4(x, x);
			vst1q_u32(a + i, negative ? sub4(x, u) : add4(x, u));
			vst1q_u32(a + n + i, add4(add4(y, y), v));
		}
#endif
		for (; i < n; ++i) {
			U u = mul(q[i], q[i]), v = mul(q[n + i], q[n + i]);
			U x = add(a[i], a[i]);
			a[i] = negative ? sub(x, u) : add(x, u);
			a[n + i] = add(add(a[n + i], a[n + i]), v);
		}
	}
	static void normalize(U *a, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			normalize8(a, n);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(a + i);
			vst1q_u32(a + i, vminq_u32(x, vsubq_u32(x, vdupq_n_u32(mod))));
		}
#endif
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
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = mul4(vld1q_u32(a + i), vld1q_u32(b + i));
			vst1q_u32(dst + i, x);
		}
#endif
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
#if defined(__x86_64__) || defined(__i386__)
	template <bool inverse>
	__attribute__((target("avx2"), always_inline)) void convolution_stage8(U *a, int stride, int k) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
		for (int i = 0; i < stride; i += 8) {
			I8 x = load8(a + i), y = load8(a + stride + i);
			fixed_butterfly8<inverse, true>(x, y, r, rq, k == 0);
			store8(a + i, x);
			store8(a + stride + i, y);
		}
	}
	__attribute__((target("avx2"), always_inline)) void leaf8(U *a, const U *b, int k) const {
		U w = roots[k / 2], q = root_quotients[k / 2];
		if (k & 1) w = mod - w, q = ~q;
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 x = load8(a), y = load8(b);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
		alignas(32) U window[16], coefficients[8];
		I8 low = fixed_mul8(x, _mm256_set1_epi32(w), _mm256_set1_epi32(q));
		store8(window, _mm256_min_epu32(low, _mm256_sub_epi32(low, p)));
		store8(window + 8, x);
		store8(coefficients, y);
		I8 e = _mm256_setzero_si256(), o = e;
		for (int i = 0; i < 8; ++i) {
			I8 z = load8(window + 8 - i), r = _mm256_set1_epi32(coefficients[i]);
			e = _mm256_add_epi64(e, _mm256_mul_epu32(z, r));
			o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
		}
		I8 ni = _mm256_set1_epi32(998244351);
		e = _mm256_add_epi64(e, _mm256_mul_epu32(_mm256_mul_epu32(e, ni), p));
		o = _mm256_add_epi64(o, _mm256_mul_epu32(_mm256_mul_epu32(o, ni), p));
		x = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
		store8(a, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
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
		I8 ni = _mm256_set1_epi32(998244351);
		for (int k = 0; k < count; ++k) {
			I8 x = e[k], y = o[k];
			x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, ni), p));
			y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, ni), p));
			x = _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
			store8(a + 8 * k, _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
	}
	__attribute__((target("avx2"))) static U reverse_dot8(const U *a, const U *b, int n) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
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
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
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

	__attribute__((target("avx2"))) static void inverse_fold8(U *a, const U *f, int n) {
		int i = 0;
		for (; i + 8 <= n; i += 8) store8(a + i, add8(load8(a + i), load8(f + i)));
		for (; i < n; ++i) {
			a[i] += f[i];
			if (a[i] >= twice) a[i] -= twice;
		}
	}
	__attribute__((target("avx2"))) static void inverse_recover8(U *g, const U *a, int m) {
		int n = 2 * m;
		U ratio = T(3).pow((mod - 1) / (2 * (n / 8))).inv().val();
		U first = mod - (mod - 1) / (2 * (n / 8));
		U phase = T(3).pow((mod - 1) / 4).inv().val();
		U powers[8];
		powers[0] = first;
		for (int j = 1; j < 8; ++j) powers[j] = (unsigned long long)powers[j - 1] * ratio % mod;
		U step = T(ratio).pow(8).val();
		I8 factors = load8(powers), s = _mm256_set1_epi32(step), sq = _mm256_set1_epi32(quotient(step));
		I8 vphase = _mm256_set1_epi32(phase), qphase = _mm256_set1_epi32(quotient(phase)), p = _mm256_set1_epi32(mod), z = _mm256_setzero_si256();
		for (int i = 0; i < m; i += 64) {
			store8(powers, factors);
			for (int j = 0; j < 8; ++j) {
				int at = i + 8 * j;
				U w = powers[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				I8 r = _mm256_add_epi32(load8(a + at), fixed_mul8(load8(a + m + at), vphase, qphase));
				r = fixed_mul8(r, v, q);
				store8(g + m + at, sub8(z, add8(r, load8(g + m + at))));
			}
			factors = fixed_mul8(factors, s, sq);
			factors = _mm256_min_epu32(factors, _mm256_sub_epi32(factors, p));
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void inverse_partial8(U *a, int n, int limit = 0) const {
		if (!limit) limit = n;
		if constexpr (!inverse) {
			int stride = limit / 2;
			for (; stride >= 16; stride >>= 2) radix4_8<false>(a, stride / 2, n);
			if (stride == 8)
				for (int i = 0; i < n; i += 16) convolution_stage8<false>(a + i, 8, i / 16);
		} else {
			int stride = 8;
			for (; 4 * stride <= n; stride <<= 2) radix4_8<true>(a, stride, n);
			if (stride < n) stage8<true>(a, stride, n);
		}
	}
	__attribute__((target("avx2"))) static void inverse_twist8(U *a, U *b, int n, U ratio, U first = one) {
		U powers[8];
		powers[0] = out(first);
		U r = out(ratio);
		for (int j = 1; j < 8; ++j) powers[j] = (unsigned long long)powers[j - 1] * r % mod;
		U step = T::word(ratio).pow(8).val();
		I8 factors = load8(powers), s = _mm256_set1_epi32(step), sq = _mm256_set1_epi32(quotient(step)), p = _mm256_set1_epi32(mod);
		for (int i = 0; i < n; i += 64) {
			store8(powers, factors);
			for (int j = 0; j < 8 && i + 8 * j < n; ++j) {
				U w = powers[j];
				I8 v = _mm256_set1_epi32(w), q = _mm256_set1_epi32(quotient(w));
				store8(a + i + 8 * j, fixed_mul8(load8(a + i + 8 * j), v, q));
				if (b && i + 8 * j < n / 2) store8(b + i + 8 * j, fixed_mul8(load8(b + i + 8 * j), v, q));
			}
			factors = fixed_mul8(factors, s, sq);
			factors = _mm256_min_epu32(factors, _mm256_sub_epi32(factors, p));
		}
	}
	template <bool negative>
	__attribute__((target("avx2"))) void inverse_cubic8(U *a, U *b, int n) const {
		U shift = one;
		if constexpr (negative) {
			T psi = T(3).pow((mod - 1) / (2 * (n / 8)));
			shift = psi.v;
			if (shift >= mod) shift -= mod;
			inverse_twist8(a, b, n, shift);
		}
		inverse_partial8<false>(a, n);
		if constexpr (negative) {
			std::copy_n(b, n / 2, b + n / 2);
			inverse_partial8<false>(b, n, n / 2);
		} else inverse_partial8<false>(b, n);
		for (int i = 0; i < n; i += 8 * 8) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				U w = roots[k / 2];
				if (k & 1) w = mod - w;
				if constexpr (negative) {
					w = mul(w, shift);
					if (w >= mod) w -= mod;
					qs[j] = quotient(w);
				} else qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
				ws[j] = w;
			}
			inverse_leaf_products8<8>(a + i, b + i, ws, qs);
			inverse_leaf_products8<8, true>(a + i, b + i, ws, qs);
			for (int j = 0; j < 8; ++j) store8(a + i + 8 * j, sub8(load8(a + i + 8 * j), load8(b + i + 8 * j)));
		}
		inverse_partial8<true>(a, n);
		U normal = mod - (mod - 1) / (2 * (n / 8));
		if constexpr (!negative) scale8(a, n, normal);
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

	template <int n>
	__attribute__((target("avx2"), always_inline)) void convolution_tile8(U *a, U *b, int k) const {
		if constexpr (n == 8) leaf8(a, b, k);
		else if constexpr (n == 16) {
			convolution_stage8<false>(a, n / 2, k);
			convolution_stage8<false>(b, n / 2, k);
			convolution_tile8<n / 2>(a, b, 2 * k);
			convolution_tile8<n / 2>(a + n / 2, b + n / 2, 2 * k + 1);
			convolution_stage8<true>(a, n / 2, k);
		} else {
			stage_pair8<false, true>(a, n / 4, n, k);
			stage_pair8<false, true>(b, n / 4, n, k);
			for (int i = 0; i < 4; ++i) convolution_tile8<n / 4>(a + i * (n / 4), b + i * (n / 4), 4 * k + i);
			stage_pair8<true>(a, n / 4, n, k);
		}
	}
	__attribute__((target("avx2"))) void convolution8(U *a, U *b, int n, int k = 0) const {
		if (n == 256) {
			convolution_tile8<256>(a, b, k);
			return;
		}
		if (n == 128) {
			convolution_tile8<128>(a, b, k);
			return;
		}
		if (n == 64) {
			convolution_tile8<64>(a, b, k);
			return;
		}
		stage_pair8<false, true>(a, n / 4, n, k);
		stage_pair8<false, true>(b, n / 4, n, k);
		for (int i = 0; i < 4; ++i) convolution8(a + i * (n / 4), b + i * (n / 4), n / 4, 4 * k + i);
		stage_pair8<true>(a, n / 4, n, k);
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
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			auto p = vld2q_u32(a + 2 * i), q = vld2q_u32(b + 2 * i);
			I x = mul4(p.val[0], q.val[1]), y = mul4(p.val[1], q.val[0]);
			if constexpr (odd) x = mul4(sub4(x, y), vld1q_u32(twist + i));
			else x = add4(x, y);
			vst1q_u32(a + i, x);
			vst1q_u32(b + i, mul4(q.val[0], q.val[1]));
		}
#endif
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
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) vst1q_u32(a + i, add4(mul4(vld1q_u32(a + i), vld1q_u32(y + i)), mul4(vld1q_u32(b + i), vld1q_u32(x + i))));
#endif
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
#if defined(__ARM_NEON)
		U factors[4], power = one;
		for (int j = 0; j < 4; ++j) {
			factors[j] = mul(first, power);
			power = mul(power, step);
		}
		I r = vld1q_u32(factors), ratio = vdupq_n_u32(power);
		for (; i + 4 <= n; i += 4) {
			vst1q_u32(a + i, mul4(vld1q_u32(a + i), r));
			r = mul4(r, ratio);
		}
		vst1q_u32(factors, r);
		first = factors[0];
#endif
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
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) vst1q_u32(a + i, add4(vld1q_u32(a + i), fixed_mul4(vld1q_u32(b + i), vdupq_n_u32(r), vdupq_n_u32(q))));
#endif
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
	void leaf(U *a, const U *b, int k) const {
		U w = roots[k / 2], q = root_quotients[k / 2];
		if (k & 1) w = mod - w, q = ~q;
		U window[16], coefficients[8];
		for (int i = 0; i < 8; ++i) {
			U x = a[i] < mod ? a[i] : a[i] - mod;
			U z = fixed_mul(x, w, q);
			window[i] = z < mod ? z : z - mod;
			window[8 + i] = x;
			coefficients[i] = b[i] < mod ? b[i] : b[i] - mod;
		}
#if defined(__ARM_NEON)
		for (int j = 0; j < 8; j += 4) {
			uint64x2_t lo = vdupq_n_u64(0), hi = lo;
			for (int i = 0; i < 8; ++i) {
				I z = vld1q_u32(window + 8 - i + j);
				lo = vmlal_n_u32(lo, vget_low_u32(z), coefficients[i]);
				hi = vmlal_n_u32(hi, vget_high_u32(z), coefficients[i]);
			}
			uint32x2_t x = vmul_n_u32(vmovn_u64(lo), 998244351u), y = vmul_n_u32(vmovn_u64(hi), 998244351u);
			I z = vcombine_u32(vshrn_n_u64(vmlal_n_u32(lo, x, mod), 32), vshrn_n_u64(vmlal_n_u32(hi, y, mod), 32));
			z = vminq_u32(z, vsubq_u32(z, vdupq_n_u32(twice)));
			vst1q_u32(a + j, vminq_u32(z, vsubq_u32(z, vdupq_n_u32(mod))));
		}
#else
		for (int j = 0; j < 8; ++j) {
			unsigned long long sum = 0;
			for (int i = 0; i < 8; ++i) sum += (unsigned long long)window[8 - i + j] * coefficients[i];
			U x = reduce(sum);
			if (x >= twice) x -= twice;
			a[j] = x < mod ? x : x - mod;
		}
#endif
	}
	template <int n>
	void convolution_tile(U *a, U *b, int k) const {
		if constexpr (n == 8) leaf(a, b, k);
		else if constexpr (n == 16) {
			stage<false>(a, n / 2, n, k);
			stage<false>(b, n / 2, n, k);
			convolution_tile<n / 2>(a, b, 2 * k);
			convolution_tile<n / 2>(a + n / 2, b + n / 2, 2 * k + 1);
			stage<true>(a, n / 2, n, k);
		} else {
			stage_pair<false>(a, n / 4, n, k);
			stage_pair<false>(b, n / 4, n, k);
			for (int i = 0; i < 4; ++i) convolution_tile<n / 4>(a + i * (n / 4), b + i * (n / 4), 4 * k + i);
			stage_pair<true>(a, n / 4, n, k);
		}
	}
	void convolution(U *a, U *b, int n, int k = 0) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			convolution8(a, b, n, k);
			return;
		}
#endif
		if (n == 256) {
			convolution_tile<256>(a, b, k);
			return;
		}
		if (n == 128) {
			convolution_tile<128>(a, b, k);
			return;
		}
		if (n == 64) {
			convolution_tile<64>(a, b, k);
			return;
		}
		stage_pair<false>(a, n / 4, n, k);
		stage_pair<false>(b, n / 4, n, k);
		for (int i = 0; i < 4; ++i) convolution(a + i * (n / 4), b + i * (n / 4), n / 4, 4 * k + i);
		stage_pair<true>(a, n / 4, n, k);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void accumulate8(U *dst, const U *a, const U *b, const U *c, int n, bool minus) {
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(b + i), y = load8(c + i);
			x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, _mm256_set1_epi32(twice)), y)
					  : _mm256_add_epi32(x, y);
			store8(dst + i, add8(load8(dst + i), mul8(load8(a + i), x)));
		}
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	template <int count>
	__attribute__((target("avx2"))) static void accumulate_batch8(U *dst, const U *const *a, const U *const *b,
																  const U *const *c, int n, bool minus) {
		static_assert(count >= 1 && count <= 3);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		int offset = minus ? n : 0;
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 3
			for (int k = 0; k < count; ++k) {
				I8 x = load8(b[k] + offset + i), y = load8(c[k] + offset + i);
				x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, pp), y) : _mm256_add_epi32(x, y);
				I8 z = load8(a[k] + offset + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(z, x));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), _mm256_srli_epi64(x, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			store8(dst + i, add8(load8(dst + i), value));
		}
	}
#endif
#if defined(__x86_64__) || defined(__i386__)
	template <int count, bool first, bool canonical>
	__attribute__((target("avx2"))) static void accumulate_wide8(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
		static_assert(count >= 1 && count <= 7);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice), ni = _mm256_set1_epi32(998244351);
		int offset = minus ? n : 0;
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
			for (int k = 0; k < count; ++k) {
				I8 x = load8(b[k] + offset + i), y = load8(c[k] + offset + i);
				x = minus ? _mm256_sub_epi32(_mm256_add_epi32(x, canonical ? p : pp), y) : _mm256_add_epi32(x, y);
				if constexpr (!canonical) x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
				I8 z = load8(a[k] + offset + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(z, x));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(z, 32), _mm256_srli_epi64(x, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (count == 7) value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (!first) value = add8(load8(dst + i), value);
			store8(dst + i, value);
		}
	}
#endif
	template <int count, bool first, bool canonical>
	static void accumulate_wide(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate_wide8<count, first, canonical>(dst, a, b, c, n, minus);
			return;
		}
#endif
		if constexpr (first) std::fill_n(dst, n, 0u);
		int offset = minus ? n : 0;
		for (int k = 0; k < count; ++k) accumulate(dst, a[k] + offset, b[k] + offset, c[k] + offset, n, minus);
	}
	template <int count>
	static void accumulate_batch(U *dst, const U *const *a, const U *const *b, const U *const *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate_batch8<count>(dst, a, b, c, n, minus);
			return;
		}
#endif
		int offset = minus ? n : 0;
		for (int k = 0; k < count; ++k) accumulate(dst, a[k] + offset, b[k] + offset, c[k] + offset, n, minus);
	}
	static void accumulate(U *dst, const U *a, const U *b, const U *c, int n, bool minus) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			accumulate8(dst, a, b, c, n, minus);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= n; i += 4) {
			I x = vld1q_u32(b + i), y = vld1q_u32(c + i);
			x = minus ? sub4(x, y) : add4(x, y);
			vst1q_u32(dst + i, add4(vld1q_u32(dst + i), mul4(vld1q_u32(a + i), x)));
		}
#endif
		for (; i < n; ++i) dst[i] = add(dst[i], mul(a[i], minus ? sub(b[i], c[i]) : add(b[i], c[i])));
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
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) vst1q_u32(dst + i, fixed_mul4(vld1q_u32(src + i), vdupq_n_u32(r), vdupq_n_u32(q)));
#endif
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, q);
	}
	void scale(U *a, int length, U r) const {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			scale8(a, length, r);
			return;
		}
#endif
		int i = 0;
		U q = quotient(r);
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4)
			vst1q_u32(a + i, fixed_mul4(vld1q_u32(a + i), vdupq_n_u32(r), vdupq_n_u32(q)));
#endif
		for (; i < length; ++i) a[i] = fixed_mul(a[i], r, q);
	}

	static void pair_product(U *a, const U *b, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			pair_product8(a, b, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) {
			uint32x4x2_t v = vld2q_u32(b + 2 * i);
			vst1q_u32(a + i, mul4(v.val[0], v.val[1]));
		}
#endif
		for (; i < length; ++i) a[i] = mul(b[2 * i], b[2 * i + 1]);
	}
	static void pair_multiply(U *a, const U *b, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			pair_multiply8(a, b, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4) {
			uint32x4x2_t v = vld2q_u32(a + 2 * i);
			I x = mul4(v.val[1], vld1q_u32(b + i));
			v.val[1] = mul4(v.val[0], vld1q_u32(b + i));
			v.val[0] = x;
			vst2q_u32(a + 2 * i, v);
		}
#endif
		for (; i < length; ++i) {
			U x = a[2 * i];
			a[2 * i] = mul(b[i], a[2 * i + 1]);
			a[2 * i + 1] = mul(b[i], x);
		}
	}
	static void difference(U *a, const U *b, const U *c, int length) {
#if defined(__x86_64__) || defined(__i386__)
		if (length >= 8 && vectorized()) {
			difference8(a, b, c, length);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= length; i += 4)
			vst1q_u32(a + i, sub4(vld1q_u32(b + i), vld1q_u32(c + i)));
#endif
		for (; i < length; ++i) a[i] = sub(b[i], c[i]);
	}
	static void fold(U *a, int columns) {
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 8 && vectorized()) {
			fold8(a, columns);
			return;
		}
#endif
		int i = 0;
#if defined(__ARM_NEON)
		for (; i + 4 <= columns; i += 4) {
			I x = add4(vld1q_u32(a + i), vld1q_u32(a + columns + i));
			vst1q_u32(a + i, x);
			vst1q_u32(a + columns + i, x);
		}
#endif
		for (; i < columns; ++i) a[i] = a[columns + i] = add(a[i], a[columns + i]);
	}
	void project(U *a, int rows, int columns) const {
		if (columns == 1) {
#if defined(__x86_64__) || defined(__i386__)
			if (rows >= 8 && vectorized()) {
				project1_8(a, rows);
				return;
			}
#endif
			int j = 0;
#if defined(__ARM_NEON)
			for (; j + 4 <= rows; j += 4) {
				uint32x4x2_t v = vld2q_u32(a + 2 * j);
				v.val[0] = v.val[1] = add4(v.val[0], v.val[1]);
				vst2q_u32(a + 2 * j, v);
			}
#endif
			for (; j < rows; ++j) a[2 * j] = a[2 * j + 1] = add(a[2 * j], a[2 * j + 1]);
			return;
		}
		if (columns == 2) {
#if defined(__x86_64__) || defined(__i386__)
			if (rows >= 8 && vectorized()) {
				project2_8(a, rows);
				return;
			}
#endif
			int j = 0;
#if defined(__ARM_NEON)
			for (; j + 4 <= rows; j += 4) {
				uint32x4x4_t v = vld4q_u32(a + 4 * j);
				I x = add4(add4(v.val[0], v.val[1]), add4(v.val[2], v.val[3]));
				I y = add4(sub4(v.val[0], v.val[1]),
						   fixed_mul4(sub4(v.val[2], v.val[3]), vdupq_n_u32(inverse_roots[1]),
									  vdupq_n_u32(inverse_root_quotients[1])));
				v.val[0] = add4(x, y);
				v.val[1] = sub4(x, y);
				y = fixed_mul4(y, vdupq_n_u32(roots[1]), vdupq_n_u32(root_quotients[1]));
				v.val[2] = add4(x, y);
				v.val[3] = sub4(x, y);
				vst4q_u32(a + 4 * j, v);
			}
#endif
			for (; j < rows; ++j) {
				U *r = a + 4 * j;
				U x = add(add(r[0], r[1]), add(r[2], r[3]));
				U y = add(sub(r[0], r[1]), fixed_mul(sub(r[2], r[3]), inverse_roots[1],
													 inverse_root_quotients[1]));
				r[0] = add(x, y);
				r[1] = sub(x, y);
				y = fixed_mul(y, roots[1], root_quotients[1]);
				r[2] = add(x, y);
				r[3] = sub(x, y);
			}
			return;
		}
		if (columns == 4) {
#if defined(__x86_64__) || defined(__i386__)
			if (vectorized()) {
				project4_8(a, rows);
				return;
			}
#endif
#if defined(__ARM_NEON)
			for (int j = 0; j < rows; ++j) {
				I x = vld1q_u32(a + 8 * j), y = vld1q_u32(a + 8 * j + 4);
				small4<1, true>(x, y, 0);
				small4<2, true>(x, y, 0);
				x = y = add4(x, y);
				small4<2, false>(x, y, 0);
				small4<1, false>(x, y, 0);
				vst1q_u32(a + 8 * j, x);
				vst1q_u32(a + 8 * j + 4, y);
			}
			return;
#endif
		}
#if defined(__x86_64__) || defined(__i386__)
		if (columns == 8 && vectorized()) {
			project8_8(a, rows);
			return;
		}
#endif
#if defined(__x86_64__) || defined(__i386__)
		if (columns == 16 && vectorized()) {
			project16_8(a, rows);
			return;
		}
#endif
		for (int j = 0; j < rows; ++j) {
			U *row = a + 2 * columns * j;
			fft<true>(row, 2 * columns, columns);
			fold(row, columns);
			fft<false>(row, 2 * columns, columns);
		}
	}
	void composition_difference(U *a, const U *b, const U *c, int n) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			composition_difference8(a, b, c, n);
			return;
		}
#endif
		difference(a, b, c, n);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_difference8(U *a, const U *b, const U *c, int n) {
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(c + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			store8(a + i, sub8(load8(b + i), x));
		}
	}
#endif
	void composition_finish(U *p, const U *v, int columns, U factor) const {
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 8 && vectorized()) {
			composition_finish8(p, v, columns, out(factor));
			return;
		}
#endif
		for (int i = 0; i < columns; ++i) p[i] = mul(sub(add(v[i], v[columns + i]), add(v[2 * columns + i], v[3 * columns + i])), factor);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_finish8(U *dst, const U *v, int n, U factor) {
		I8 r = _mm256_set1_epi32(factor), q = _mm256_set1_epi32(quotient(factor));
		for (int i = 0; i < n; i += 8) {
			I8 x = add8(load8(v + i), load8(v + n + i)), y = add8(load8(v + 2 * n + i), load8(v + 3 * n + i));
			store8(dst + i, fixed_mul8(sub8(x, y), r, q));
		}
	}
#endif
	struct CompositionDelete {
		void operator()(unsigned int *p) const { ::operator delete[](p, std::align_val_t(64)); }
	};
	using CompositionWords = std::unique_ptr<unsigned int[], CompositionDelete>;
	void composition_forward_y(U *a, int size, int columns) const {
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized()) {
			int h = size;
			bool lazy = false;
			for (; h / 2 > columns && h / 2 >= 8; h >>= 2) {
				radix4_8<false>(a, h / 2, 2 * size);
				lazy = true;
			}
			if (lazy && h > columns) composition_shrink8(a, 2 * size);
			for (; h / 2 > columns; h >>= 2) stage_pair<false>(a, h / 2, 2 * size);
			if (h > columns) stage<false>(a, h, 2 * size);
			return;
		}
#endif
		int h = size;
		for (; h / 2 > columns; h >>= 2) stage_pair<false>(a, h / 2, 2 * size);
		if (h > columns) stage<false>(a, h, 2 * size);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void composition_shrink8(U *a, int n) {
		I8 p = _mm256_set1_epi32(twice);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(a + i);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
	}
#endif
	void double_y(U *a, int rows, int columns, bool inverse) const {
		const int size = rows * columns;
		int h = 2 * columns;
		for (; h <= size / 2; h <<= 2) {
#if defined(__x86_64__) || defined(__i386__)
			if (h >= 8 && vectorized()) radix4_8<true>(a, h, 2 * size);
			else
#endif
				stage_pair<true>(a, h, 2 * size);
		}
		if (h <= size) stage<true>(a, h, 2 * size);
#if defined(__x86_64__) || defined(__i386__)
		if (columns >= 2 && vectorized()) {
			scale_rows8(a, rows, columns, (inverse ? inverse_twists : twists).data() + rows);
			return;
		}
#endif
		for (int j = 0; j < rows; ++j)
			scale(a + 2 * columns * j, 2 * columns,
				  (inverse ? inverse_twists : twists)[rows + j]);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void projection_dot8(U *p, const U *q, int n) {
		I8 ni = _mm256_set1_epi32(998244351), modulus = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		for (int i = 0; i < n; i += 8) {
			I8 x = load8(p + 2 * i), y = load8(p + 2 * i + 8), a = load8(q + 2 * i), b = load8(q + 2 * i + 8);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
			a = _mm256_min_epu32(a, _mm256_sub_epi32(a, pp));
			b = _mm256_min_epu32(b, _mm256_sub_epi32(b, pp));
			I8 lo = _mm256_add_epi64(_mm256_mul_epu32(x, _mm256_srli_epi64(a, 32)), _mm256_mul_epu32(_mm256_srli_epi64(x, 32), a));
			I8 hi = _mm256_add_epi64(_mm256_mul_epu32(y, _mm256_srli_epi64(b, 32)), _mm256_mul_epu32(_mm256_srli_epi64(y, 32), b));
			lo = _mm256_add_epi64(lo, _mm256_mul_epu32(_mm256_mul_epu32(lo, ni), modulus));
			hi = _mm256_add_epi64(hi, _mm256_mul_epu32(_mm256_mul_epu32(hi, ni), modulus));
			I8 r = _mm256_permutevar8x32_epi32(_mm256_blend_epi32(_mm256_srli_epi64(lo, 32), hi, 0xaa), index);
			store8(p + i, _mm256_min_epu32(r, _mm256_sub_epi32(r, pp)));
		}
	}
#endif

	static void projection_dot(U *p, const U *q, int n) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && vectorized()) {
			projection_dot8(p, q, n);
			return;
		}
#endif
		for (int i = 0; i < n; ++i) {
			U x = p[2 * i], y = p[2 * i + 1], a = q[2 * i], b = q[2 * i + 1];
			if (x >= twice) x -= twice;
			if (y >= twice) y -= twice;
			if (a >= twice) a -= twice;
			if (b >= twice) b -= twice;
			U r = reduce((unsigned long long)x * b + (unsigned long long)y * a);
			p[i] = r < twice ? r : r - twice;
		}
	}
	void transpose_roots() {
		roots.swap(inverse_roots);
		root_quotients.swap(inverse_root_quotients);
		cube_roots.swap(inverse_cube_roots);
		cube_quotients.swap(inverse_cube_quotients);
	}
	U reversion_projection(U *p, U *q, int size, const U *weights = nullptr) {
		int levels = __builtin_ctz((unsigned int)size);
		V saved(levels, one);
		U q_one = one, p_one = one;
		for (int i = 1, columns = size / 2; columns > 1; ++i, columns >>= 1) {
			q_one = mul(mul(q_one, q_one), in(2 * columns));
			saved[i] = q_one;
		}
		for (int i = levels - 1, columns = 2; i >= 0; --i, columns <<= 1) {
			p_one = mul(p_one, saved[i]);
			if (i) p_one = mul(p_one, in(4 * columns));
		}
		U factor = T::word(mul(p_one, in(4 * size))).inv().v;
		fft<false>(q, 2 * size);
		for (int i = 0; i < 2 * size; ++i) {
			U x = q[i];
			q[i] = add(x, one);
			q[2 * size + i] = sub(x, one);
		}

		if (weights) {
			for (int i = 0; i < size; ++i) {
				p[i] = p[size + i] = weights[i];
			}
			transpose_roots();
			fft<false>(p, 2 * size, size);
			transpose_roots();
			for (int i = 0; i < 2 * size; ++i) p[2 * size + i] = sub(0, p[i]);
			projection_dot(p, q, 2 * size);
		} else {
			for (int i = 0; i < size; ++i) {
				U x = fixed_mul(sub(q[2 * i + 1], q[2 * i]), roots[i], root_quotients[i]);
				p[i] = i < size / 2 ? x : sub(0, x);
				p[size + i] = sub(0, p[i]);
			}
		}
		q_one = one;
		for (int columns = size / 2, rows = 2; columns > 1; columns >>= 1, rows <<= 1) {
			pair_product(q, q, 2 * size);
			q_one = mul(q_one, q_one);
			project(q, rows, columns);
			std::copy_n(q, 2 * size, q + 2 * size);
			double_y(q + 2 * size, rows, columns, false);
			q_one = mul(q_one, in(2 * columns));
			U correction = add(q_one, q_one);
			for (int i = 0; i < 2 * columns; ++i) q[2 * size + i] = sub(q[2 * size + i], correction);
			composition_forward_y(q + 2 * size, size, columns);
			transpose_roots();
			project(p, rows, columns);
			for (int i = 0; i < 2 * size; ++i) p[2 * size + i] = sub(0, p[i]);
			double_y(p + 2 * size, rows, columns, true);
			composition_forward_y(p + 2 * size, size, columns);
			transpose_roots();
			projection_dot(p, q, 2 * size);
		}
		for (int i = 0; i < size; ++i) p[i] = add(p[2 * i], p[2 * i + 1]);
		transpose_roots();
		fft<true>(p, size);
		transpose_roots();
		return factor;
	}

	void compose(U *p, const U *q, int rows, int columns, U &p_one, U &q_one, U *workspace = nullptr, int level = 0) const {
		const int size = rows * columns;
		if (columns == 1) {
			fft<false>(p, rows);
			for (int i = rows - 1; i >= 0; --i) p[2 * i] = p[2 * i + 1] = p[i];
			return;
		}
		CompositionWords storage;
		if (!workspace) {
			int levels = __builtin_ctz((unsigned int)columns);
			storage.reset(static_cast<unsigned int *>(::operator new[]((4 * size + 16) * levels * sizeof(unsigned int), std::align_val_t(64))));
			workspace = storage.get();
		}
		U *v = workspace + (4 * size + 16) * level;
		if (rows == 1) {
			std::fill_n(v + columns, columns, U(0));
			for (int i = 0; i < columns; ++i) v[i] = sub(0, q[i]);
			fft<false>(v, 2 * columns);
			for (int i = 0; i < 2 * columns; ++i) {
				U x = v[i];
				v[i] = add(x, one);
				v[i + 2 * columns] = sub(x, one);
			}
		} else {
			pair_product(v, q, 2 * size);
			q_one = mul(q_one, q_one);
			project(v, rows, columns);
			std::copy_n(v, 2 * size, v + 2 * size);
			U *upper = v + 2 * size;
			double_y(upper, rows, columns, false);
			q_one = mul(q_one, in(2 * columns));
			U correction = add(q_one, q_one);
			for (int i = 0; i < 2 * columns; ++i) upper[i] = sub(upper[i], correction);
			composition_forward_y(upper, size, columns);
		}
		U saved = q_one;
		compose(p, v, 2 * rows, columns / 2, p_one, q_one, workspace, level + 1);
		pair_multiply(v, p, 2 * size);
		p_one = mul(p_one, saved);
		if (rows == 1) {
			fft<true>(v, 2 * columns, columns);
			fft<true>(v + 2 * columns, 2 * columns, columns);
			U factor = in(T(out(mul(p_one, in(4 * size)))).inv().val());
			composition_finish(p, v, columns, factor);
		} else {
			U *upper = v + 2 * size;
			double_y(upper, rows, columns, true);
			composition_forward_y(upper, size, columns);
			p_one = add(p_one, p_one);
			composition_difference(p, v, upper, 2 * size);
			project(p, rows, columns);
			p_one = mul(p_one, in(2 * columns));
		}
	}
};
inline NTT &ntt_plan(int size) {
	static NTT t(1, false);
	if (static_cast<int>(t.roots.size()) < size) t = NTT(size, false);
	return t;
}

}

inline void ntt(std::vector<fps_mint> &a, bool inverse = false) {
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
	static_assert(sizeof(fps_mint) == sizeof(fps_detail::NTT::U) && std::is_trivially_copyable_v<fps_mint>);
	auto *words = reinterpret_cast<fps_detail::NTT::U *>(a.data());
	if (inverse) {
		reorder();
		t.fft<true>(words, n);
		t.scale(words, n, fps_mint(n).inv().val());
	} else {
		t.fft<false>(words, n);
		reorder();
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 7 "components/fps_operations.hpp"
#include <numeric>
#line 10 "components/fps_operations.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T> struct FormalPowerSeries;

namespace fps_detail {

template<class T, class Work = Mint>
struct Operations {
	using F = FormalPowerSeries<T>;
	using W = FormalPowerSeries<Work>;
	using base = std::vector<T>;
	using Word = NTT::U;

	static W import_work(const base &a) {
		W r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = fps_detail::Mint::raw(a[i].val());
		return r;
	}
	static F export_work(const W &a) {
		F r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = T::raw(a[i].val());
		return r;
	}
	static fps_detail::NTT &transform(int n) { return fps_detail::ntt_plan(n); }
	static Word *words(base &a) {
		static_assert(sizeof(T) == sizeof(Word) && std::is_trivially_copyable_v<T>);
		return reinterpret_cast<Word *>(a.data());
	}
	static const Word *words(const base &a) { return reinterpret_cast<const Word *>(a.data()); }
	template <bool inverse>
	static void transform_inplace(base &a) {
		int n = a.size();
		assert(n && !(n & (n - 1)) && n <= (1 << 23));
		auto &t = transform(std::max(1, n / 2));
		t.template fft<inverse>(words(a), n);
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) t.normalize(words(a), n);
	}
	static void ntt(base &a) { transform_inplace<false>(a); }
	static void ntt_half(Word *a, int n) {
		std::copy_n(a, n / 2, a + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(a, n, n / 2);
	}
	static void ntt_half(base &a) {
		int n = a.size();
		std::copy_n(a.begin(), n / 2, a.begin() + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(words(a), n, n / 2);
	}
	static void ntt_inverse(base &a) { transform_inplace<true>(a); }
	static void scale_words(base &a, T factor) {
		if (a.empty()) return;
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			auto &t = transform(1);
			t.scale(words(a), a.size(), factor.val());
		} else
			for (T &x : a) x *= factor;
	}
	static void geometric_scale(base &a, T first, T step) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::geometric(words(a), a.size(), first.v, step.v);
		else
			for (T &x : a) {
				x *= first;
				first *= step;
			}
	}
	static void batch_inverse(base &dst, const base &src) {
		int n = src.size();
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 128 && fps_detail::NTT::vectorized()) {
				fps_detail::NTT::batch_inverse8(words(dst), words(src), n);
				return;
			}
		}
#endif
		base prefix(n);
		T p = 1;
		for (int i = 0; i < n; ++i) prefix[i] = p, p *= src[i];
		assert(p != T(0));
		p = p.inv();
		for (int i = n - 1; i >= 0; --i) {
			dst[i] = prefix[i] * p;
			p *= src[i];
		}
	}
	static void pointwise(base &r, const base &a, const base &b, int n) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::product(words(r), words(a), words(b), n);
		else
			for (int i = 0; i < n; ++i) r[i] = a[i] * b[i];
	}
#if defined(__x86_64__) || defined(__i386__)
	template <int count, bool first>
	__attribute__((target("avx2"))) static void accumulate_products8(Word *dst, const Word *const *a, const Word *const *b, int n) {
		using I8 = fps_detail::NTT::I8;
		I8 p = _mm256_set1_epi32(T::mod()), pp = _mm256_set1_epi32(2 * T::mod()), ni = _mm256_set1_epi32(998244351);
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
			for (int k = 0; k < count; ++k) {
				I8 x = fps_detail::NTT::load8(a[k] + i), y = fps_detail::NTT::load8(b[k] + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (count == 7) value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (!first) value = fps_detail::NTT::add8(fps_detail::NTT::load8(dst + i), value);
			fps_detail::NTT::store8(dst + i, value);
		}
	}
#endif
	template <int count, bool first>
	static void accumulate_products(Word *dst, const Word *const *a, const Word *const *b, int n) {
		static_assert(count >= 1 && count <= 7);
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && fps_detail::NTT::vectorized()) {
			accumulate_products8<count, first>(dst, a, b, n);
			return;
		}
#endif
		for (int i = 0; i < n; ++i) {
			Word value = 0;
			for (int k = 0; k < count; ++k) value = fps_detail::NTT::add(value, fps_detail::NTT::mul(a[k][i], b[k][i]));
			if constexpr (!first) value = fps_detail::NTT::add(value, dst[i]);
			dst[i] = value;
		}
	}
	static void join_windows(Word *blocks, int length, int count) {
		join_windows_strided(blocks, length, count, length);
	}
	static void join_windows_strided(Word *blocks, int length, int count, int pitch) {
		for (int i = count - 1; i > 0; --i) {
			Word *a = blocks + i * pitch;
			const Word *b = a - pitch;
			for (int j = 0; j < length / 2; ++j) a[j] = fps_detail::NTT::add(a[j], b[j]);
			for (int j = length / 2; j < length; ++j) a[j] = fps_detail::NTT::sub(a[j], b[j]);
		}
	}
	static void inverse_extend(F &h, const base &a, const base &cache) {
		const int m = static_cast<int>(cache.size());
		const T im = T::raw(T::mod() - (T::mod() - 1) / m);
		base b(m);
		pointwise(b, a, cache, m);
		ntt_inverse(b);
		std::fill(b.begin(), b.begin() + m / 2, T(0));
		ntt(b);
		pointwise(b, b, cache, m);
		ntt_inverse(b);
		h.resize(m);
		for (int i = m / 2; i < m; ++i) h[i] = -b[i] * im * im;
	}
	static T dot(const T *a, const T *b, int n) {

#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 64 && fps_detail::NTT::vectorized())
				return T::word(fps_detail::NTT::reverse_dot8(reinterpret_cast<const Word *>(a), reinterpret_cast<const Word *>(b), n));
		}
#endif
		unsigned int result = 0;
		for (int i = 0; i < n; i += 16) {
			unsigned long long sum = 0;
			for (int j = i; j < std::min(n, i + 16); ++j)
				sum += (unsigned long long)a[j].val() * b[-j].val();
			result += sum % T::mod();
			if (result >= T::mod()) result -= T::mod();
		}
		return T::raw(result);
	}
	static int prepare_sparse(std::vector<std::pair<int, T>> &terms) {
		int stride = 0;
		for (auto &[j, x] : terms) {
			stride = std::gcd(stride, j);
			if constexpr (std::is_same_v<T, fps_detail::Mint>) if (x.v >= T::mod()) x.v -= T::mod();
		}
		return stride;
	}
	static T sparse_dot(const std::vector<std::pair<int, T>> &terms, const F &g, int i) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			static_assert(7ULL * (T::mod() - 1) * (2ULL * T::mod() - 1) + 0xffffffffULL * T::mod() < ~0ULL);
			unsigned long long sum = 0;
			Word result = 0;
			int count = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				sum += (unsigned long long)x.v * g[i - j].v;
				if (++count == 7) {
					result = fps_detail::NTT::add(result, fps_detail::NTT::reduce(sum) % T::mod());
					sum = 0;
					count = 0;
				}
			}
			return T::word(fps_detail::NTT::add(result, fps_detail::NTT::reduce(sum) % T::mod()));
		} else {
			T result = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				result += x * g[i - j];
			}
			return result;
		}
	}

	static void intt(base &a) {
		ntt_inverse(a);
		const T inv = T::raw(T::mod() - (T::mod() - 1) / static_cast<int>(a.size()));
		scale_words(a, inv);
	}
	static T mod_sqrt(T a) {
		if (a == T(0) || a.pow((T::mod() - 1) / 2) != T(1)) return 0;
		T r = a.pow(60), t = a.pow(119), c = T(3).pow(119);
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T u = t; u != T(1); u *= u) ++i;
			T b = c.pow(1 << (m - i - 1));
			r *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return r;
	}
	static const base &inverse_ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m)).inv();
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const base &ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m));
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const std::vector<T> &inverses(int n) {
		assert(unsigned(n) < unsigned(T::mod()));
		static std::vector<T> inv{0, 1};
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			int first = inv.size(), count = (n + 1 - first) / 8 * 8;
			if (count >= 256 && fps_detail::NTT::vectorized()) {
				inv.reserve(n + 1);
				inv.resize(first + count);
				fps_detail::NTT::inverse_sequence8(reinterpret_cast<Word *>(inv.data()) + first, first, count);
			}
		}
#endif
		while (static_cast<int>(inv.size()) <= n) {
			const int i = static_cast<int>(inv.size());
			inv.push_back(-inv[T::mod() % i] * (T::mod() / i));
		}
		return inv;
	}
	static void trim(F &a) {
		while (!a.empty() && a.back() == T(0)) a.pop_back();
	}
};

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fps_horner.hpp"



#line 5 "components/fps_horner.hpp"
#include <array>
#include <cstring>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
namespace horner_detail {
template<int K, class T>
inline T scalar(const T* a, std::size_t n, T x) {
	static_assert(K > 0);
	T step = 1;
	for (int i = 0; i < K; ++i) step *= x;
	std::array<T, K> sum{};
	std::size_t groups = n / K, tail = n % K;
	for (std::size_t i = 0; i < tail; ++i) sum[i] = a[groups * K + i];
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) sum[i] = sum[i] * step + a[groups * K + i];
	}
	T result = 0;
	for (int i = K; i-- > 0;) result = result * x + sum[i];
	return result;
}

#if defined(__x86_64__) || defined(__i386__)
template<int K>
__attribute__((target("avx2"))) inline Mint vector(const Mint* a, std::size_t n, Mint x) {
	static_assert(K == 1 || K == 2 || K == 4);
	using V = NTT::I8;
	static_assert(std::is_trivially_copyable_v<Mint> && sizeof(Mint) == sizeof(unsigned));
	Mint step = x;
	for (int i = 0; i < 3; ++i) step *= step;
	const unsigned x8 = step.val();
	for (int i = 1; i < K; i *= 2) step *= step;
	const unsigned value = step.val();
	const V factor = _mm256_set1_epi32(value), quotient = _mm256_set1_epi32(NTT::quotient(value));
	V sum[K];
	for (auto& v : sum) v = _mm256_setzero_si256();
	constexpr std::size_t width = 8 * K;
	std::size_t groups = n / width, tail = n % width;
	if (tail) {
		alignas(32) Mint block[width]{};
		std::memcpy(static_cast<void*>(block), a + groups * width, tail * sizeof(Mint));
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], block + 8 * i, sizeof(V));
	} else if (groups) {
		--groups;
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], a + groups * width + 8 * i, sizeof(V));
	}
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) {
			V coefficient;
			std::memcpy(&coefficient, a + groups * width + 8 * i, sizeof(V));
			sum[i] = NTT::add8(value == 1 ? sum[i] : NTT::fixed_mul8(sum[i], factor, quotient), coefficient);
		}
	}
	const V step8 = _mm256_set1_epi32(x8), quotient8 = _mm256_set1_epi32(NTT::quotient(x8));
	V joined = sum[K - 1];
	for (int i = K - 1; i-- > 0;) joined = NTT::add8(NTT::fixed_mul8(joined, step8, quotient8), sum[i]);
	alignas(32) Mint word[8];
	std::memcpy(static_cast<void*>(word), &joined, sizeof(joined));
	Mint result = 0;
	for (int i = 8; i-- > 0;) result = result * x + word[i];
	return result;
}
#endif
}

inline Mint horner(const Mint* a, std::size_t n, Mint x) {
	if (!n) return Mint(0);
	if (x == Mint(0)) return a[0];
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 32 && NTT::vectorized()) {
		if (n < 64) return horner_detail::vector<1>(a, n, x);
		return horner_detail::vector<4>(a, n, x);
	}
#endif
	if (n >= 32) return horner_detail::scalar<8>(a, n, x);
	Mint result = 0;
	while (n) result = result * x + a[--n];
	return result;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fps_factorials.hpp"



#line 5 "components/fps_factorials.hpp"

namespace fps_detail {

#if defined(__x86_64__) || defined(__i386__)

__attribute__((target("avx2"))) inline void factorial_transpose(NTT::I8 (&x)[8]) {
	NTT::I8 a[8], b[8];
	for (int i = 0; i < 8; i += 2) {
		a[i] = _mm256_unpacklo_epi32(x[i], x[i + 1]);
		a[i + 1] = _mm256_unpackhi_epi32(x[i], x[i + 1]);
	}
	for (int i = 0; i < 8; i += 4) {
		b[i] = _mm256_unpacklo_epi64(a[i], a[i + 2]);
		b[i + 1] = _mm256_unpackhi_epi64(a[i], a[i + 2]);
		b[i + 2] = _mm256_unpacklo_epi64(a[i + 1], a[i + 3]);
		b[i + 3] = _mm256_unpackhi_epi64(a[i + 1], a[i + 3]);
	}
	for (int i = 0; i < 4; ++i) {
		x[i] = _mm256_permute2x128_si256(b[i], b[i + 4], 0x20);
		x[i + 4] = _mm256_permute2x128_si256(b[i], b[i + 4], 0x31);
	}
}

template<bool reverse>
__attribute__((target("avx2"))) inline void factorial_chain(NTT::U* out, int chunk, NTT::I8 value, NTT::I8 factor) {
	using I = NTT::I8;
	I step = _mm256_set1_epi32(NTT::one);
	I order = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	for (int j = 0; j < chunk; j += 8) {
		I block[8];
		for (int k = 0; k < 8; ++k) {
			block[k] = value;
			value = NTT::mul8(value, factor);
			if constexpr (reverse) factor = NTT::sub8(factor, step);
			else factor = NTT::add8(factor, step);
		}
		factorial_transpose(block);
		for (int k = 0; k < 8; ++k) {
			if constexpr (reverse) NTT::store8(out + (k + 1) * chunk - j - 8, _mm256_permutevar8x32_epi32(block[k], order));
			else NTT::store8(out + k * chunk + j, block[k]);
		}
	}
}

__attribute__((target("avx2"))) inline void factorials8(NTT::U* fact, NTT::U* invfact, int n) {
	using I = NTT::I8;
	using U = NTT::U;
	const int chunk = n / 256 * 8, covered = chunk * 32;
	alignas(32) U factor[32], products[32], starts[33], inverse[33];
	I p[4], x[4], step = _mm256_set1_epi32(NTT::one);
	for (int i = 0; i < 32; ++i) factor[i] = NTT::in(i * chunk + 1);
	for (int k = 0; k < 4; ++k) p[k] = _mm256_set1_epi32(NTT::one), x[k] = NTT::load8(factor + 8 * k);
	for (int i = 0; i < chunk; ++i) {
		for (int k = 0; k < 4; ++k) p[k] = NTT::mul8(p[k], x[k]), x[k] = NTT::add8(x[k], step);
	}
	for (int k = 0; k < 4; ++k) NTT::store8(products + 8 * k, p[k]);
	starts[0] = NTT::one;
	for (int i = 0; i < 32; ++i) starts[i + 1] = NTT::mul(starts[i], products[i]);
	U last = starts[32];
	for (int i = covered; i < n; ++i) fact[i] = last, last = NTT::mul(last, NTT::in(i + 1));
	last = Mint::word(last).inv().v;
	for (int i = n - 1; i >= covered; --i) last = NTT::mul(last, NTT::in(i + 1)), invfact[i] = last;
	inverse[32] = last;
	for (int i = 31; i >= 0; --i) inverse[i] = NTT::mul(inverse[i + 1], products[i]);
	for (int k = 0; k < 4; ++k) {
		factorial_chain<false>(fact + 8 * k * chunk, chunk, NTT::load8(starts + 8 * k), NTT::load8(factor + 8 * k));
		alignas(32) U values[8], factors[8];
		for (int j = 0; j < 8; ++j) {
			const int end = (8 * k + j + 1) * chunk;
			values[j] = NTT::mul(inverse[8 * k + j + 1], NTT::in(end));
			factors[j] = NTT::in(end - 1);
		}
		factorial_chain<true>(invfact + 8 * k * chunk, chunk, NTT::load8(values), NTT::load8(factors));
	}
}
#endif

inline void factorials(NTT::U* fact, NTT::U* invfact, int n) {
	if (!n) return;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 256 && NTT::vectorized()) {
		factorials8(fact, invfact, n);
		return;
	}
#endif
	NTT::U value = NTT::one;
	for (int i = 0; i < n; ++i) fact[i] = value, value = NTT::mul(value, NTT::in(i + 1));
	value = Mint::word(fact[n - 1]).inv().v;
	for (int i = n - 1; i >= 0; --i) invfact[i] = value, value = NTT::mul(value, NTT::in(i));
}

template<class T> requires std::is_same_v<T, Mint>
inline void factorials(T* fact, T* invfact, int n) {
	assert(0 <= n && unsigned(n) <= Mint::mod());
	static_assert(sizeof(Mint) == sizeof(NTT::U) && std::is_trivially_copyable_v<Mint>);
	if (unsigned(n) == Mint::mod()) {
		factorials(fact, invfact, n - 1);
		fact[n - 1] = -fact[n - 2];
		invfact[n - 1] = -invfact[n - 2];
		return;
	}
	factorials(reinterpret_cast<NTT::U*>(fact), reinterpret_cast<NTT::U*>(invfact), n);
}

}


#line 1 "components/fps_workspace.hpp"



#line 5 "components/fps_workspace.hpp"
#include <cstdint>
#include <limits>
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


#line 1 "components/fps_log.hpp"



#line 5 "components/fps_log.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {

#if defined(__x86_64__) || defined(__i386__)
struct LogTransform {
	using U = NTT::U;
	using I8 = NTT::I8;
	static constexpr U mod = NTT::mod, twice = NTT::twice, one = NTT::one;
	const NTT &plan;

	__attribute__((target("avx2"))) void forward(U *a, int n, U norm = 1, bool half = false) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		if (half) std::copy_n(a, n / 2, a + n / 2);
		plan.template inverse_partial8<false>(a, n, half ? n / 2 : n);
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 r = _mm256_set1_epi32(norm), rq = _mm256_set1_epi32(NTT::quotient(norm));
		for (int i = 0; i < n; i += 8) {
			I8 x = NTT::load8(a + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			if (norm != 1) x = NTT::fixed_mul8(x, r, rq);
			NTT::store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
	}
	__attribute__((target("avx2"))) void inverse(U *a, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		plan.template inverse_partial8<true>(a, n);
	}
	template <bool upper> __attribute__((target("avx2"))) void inverse_half(U *a, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(plan.roots.size()) >= n / 16);
		int h = 8;
		for (; 4 * h < n; h <<= 2) plan.template radix4_8<true>(a, h, n);
		if (2 * h == n) {
			for (int i = 0; i < h; i += 8) {
				I8 x = upper ? NTT::sub8(NTT::load8(a + i), NTT::load8(a + h + i)) : NTT::add8(NTT::load8(a + i), NTT::load8(a + h + i));
				NTT::store8(a + (upper ? h : 0) + i, x);
			}
		} else {
			I8 im = _mm256_set1_epi32(plan.inverse_roots[1]), iq = _mm256_set1_epi32(plan.inverse_root_quotients[1]);
			for (int i = 0; i < h; i += 8) {
				I8 x0 = NTT::load8(a + i), x1 = NTT::load8(a + h + i), x2 = NTT::load8(a + 2 * h + i), x3 = NTT::load8(a + 3 * h + i);
				I8 a0 = NTT::add8(x0, x1), a1 = NTT::sub8(x0, x1), a2 = NTT::add8(x2, x3), a3 = NTT::fixed_mul8(NTT::sub8(x2, x3), im, iq);
				NTT::store8(a + (upper ? 2 * h : 0) + i, upper ? NTT::sub8(a0, a2) : NTT::add8(a0, a2));
				NTT::store8(a + (upper ? 3 * h : h) + i, upper ? NTT::sub8(a1, a3) : NTT::add8(a1, a3));
			}
		}
	}
	__attribute__((target("avx2"))) void products(U *dst, const U *const *a, const U *const *b, int terms, int n, const U *side = nullptr) const {
		assert(n >= 64 && n % 64 == 0 && terms >= 1 && terms <= 2 && static_cast<int>(plan.roots.size()) >= n / 16);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = k & 1 ? mod - plan.roots[k / 2] : plan.roots[k / 2];
				qs[j] = k & 1 ? ~plan.root_quotients[k / 2] : plan.root_quotients[k / 2];
			}
			alignas(32) U x[64], sum[64];
			for (int t = 0; t < terms; ++t) {
				std::copy_n(a[t] + i, 64, x);
				plan.template inverse_leaf_products8<8, true>(x, const_cast<U *>(b[t]) + i, ws, qs);
				for (int j = 0; j < 64; j += 8) NTT::store8(sum + j, t ? NTT::add8(NTT::load8(sum + j), NTT::load8(x + j)) : NTT::load8(x + j));
			}
			for (int j = 0; j < 64; j += 8)
				NTT::store8(dst + i + j, side ? NTT::add8(NTT::load8(sum + j), NTT::load8(side + i + j)) : NTT::load8(sum + j));
		}
	}

	__attribute__((target("avx2"))) static void integrate(U *dst, const U *src, U *prefix, int first, int n) {
		assert(n >= 0 && first >= 1 && static_cast<unsigned long long>(first) + n <= mod);
		if (!n) return;
		I8 x[4], prod[4], inv[4], step = _mm256_set1_epi32(NTT::in(32)), onev = _mm256_set1_epi32(one);
		alignas(32) U initial[32];
		for (int i = 0; i < 32; ++i) initial[i] = NTT::in(first + i);
		const I8 lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), limit = _mm256_set1_epi32(n);
		auto mask = [&](int at) __attribute__((target("avx2"))) { return _mm256_cmpgt_epi32(limit, _mm256_add_epi32(_mm256_set1_epi32(at), lanes)); };
		auto factor = [&](I8 a, int at) __attribute__((target("avx2"))) { return at + 8 <= n ? a : _mm256_blendv_epi8(onev, a, mask(at)); };
		for (int v = 0; v < 4; ++v) x[v] = NTT::load8(initial + 8 * v), prod[v] = factor(x[v], 8 * v);
		for (int i = 32; i < n; i += 32)
			for (int v = 0; v < 4; ++v) {
				NTT::store8(prefix + i - 32 + 8 * v, prod[v]);
				x[v] = NTT::add8(x[v], step);
				prod[v] = NTT::mul8(prod[v], factor(x[v], i + 8 * v));
			}
		for (int v = 0; v < 4; ++v) inv[v] = NTT::inverse8(prod[v]);
		auto put = [&](int at, I8 reciprocal) __attribute__((target("avx2"))) {
			if (at >= n) return;
			I8 a = at + 8 <= n ? NTT::load8(src + at) : _mm256_maskload_epi32(reinterpret_cast<const int *>(src + at), mask(at));
			I8 r = NTT::mul8(a, reciprocal);
			if (at + 8 <= n)
				NTT::store8(dst + at, r);
			else
				_mm256_maskstore_epi32(reinterpret_cast<int *>(dst + at), mask(at), r);
		};
		for (int i = (n - 1) / 32 * 32; i >= 32; i -= 32)
			for (int v = 0; v < 4; ++v) {
				int at = i + 8 * v;
				if (at < n) put(at, NTT::mul8(inv[v], NTT::load8(prefix + at - 32)));
				inv[v] = NTT::mul8(inv[v], factor(x[v], at));
				x[v] = NTT::sub8(x[v], step);
			}
		for (int v = 0; v < 4; ++v) put(8 * v, inv[v]);
	}
	__attribute__((target("avx2"))) void toeplitz(U *dst, U *side, const U *w1, const U *w2, const U *q0, const U *q1, int n) const {
		assert(n >= 64 && n % 64 == 0 && static_cast<int>(plan.roots.size()) >= n / 16);
		I8 p = _mm256_set1_epi32(mod);
		auto canonical = [&](I8 x) __attribute__((target("avx2"))) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, p)); };
		for (int i = 0; i < n; i += 64) {
			alignas(32) U x[64], y[64], z[64];
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = k & 1 ? mod - plan.roots[k / 2] : plan.roots[k / 2];
				qs[j] = k & 1 ? ~plan.root_quotients[k / 2] : plan.root_quotients[k / 2];
				NTT::store8(x + 8 * j, canonical(_mm256_add_epi32(NTT::load8(q0 + i + 8 * j), NTT::load8(q1 + i + 8 * j))));
				NTT::store8(y + 8 * j, canonical(NTT::sub8(NTT::load8(w1 + i + 8 * j), NTT::load8(w2 + i + 8 * j))));
				NTT::store8(z + 8 * j, canonical(NTT::sub8(NTT::load8(side + i + 8 * j), NTT::load8(w2 + i + 8 * j))));
			}
			plan.template inverse_leaf_products8<8, true>(x, const_cast<U *>(w2) + i, ws, qs);
			plan.template inverse_leaf_products8<8, true>(y, const_cast<U *>(q1) + i, ws, qs);
			plan.template inverse_leaf_products8<8, true>(z, const_cast<U *>(q0) + i, ws, qs);
			for (int j = 0; j < 64; j += 8) {
				NTT::store8(dst + i + j, NTT::add8(NTT::load8(x + j), NTT::load8(y + j)));
				NTT::store8(side + i + j, NTT::add8(NTT::load8(x + j), NTT::load8(z + j)));
			}
		}
	}
};
#endif

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fps_blocked.hpp"


#line 1 "components/fps_asm.hpp"



#define CP_FPS_ASM_ADD(a,b) "add " #a ", " #b "\n\t"
#define CP_FPS_ASM_AND(a,b) "and " #a ", " #b "\n\t"
#define CP_FPS_ASM_CMP(a,b) "cmp " #a ", " #b "\n\t"
#define CP_FPS_ASM_INC(a) "inc " #a "\n\t"
#define CP_FPS_ASM_JNE(a) "jne " #a "\n\t"
#define CP_FPS_ASM_LEA(a,b) "lea " #a ", " #b "\n\t"
#define CP_FPS_ASM_MOV(a,b) "mov " #a ", " #b "\n\t"
#define CP_FPS_ASM_VBROADCASTSS(a,b) "vbroadcastss " #a ", " #b "\n\t"
#define CP_FPS_ASM_VMOVDQA(a,b) "vmovdqa " #a ", " #b "\n\t"
#define CP_FPS_ASM_VMOVDQU(a,b) "vmovdqu " #a ", " #b "\n\t"
#define CP_FPS_ASM_VPADDD(a,b,c) "vpaddd " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPADDQ(a,b,c) "vpaddq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPBLENDD(a,b,c,d) "vpblendd " #a ", " #b ", " #c ", " #d "\n\t"
#define CP_FPS_ASM_VPBROADCASTD(a,b) "vpbroadcastd " #a ", " #b "\n\t"
#define CP_FPS_ASM_VPMINUD(a,b,c) "vpminud " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPMULLD(a,b,c) "vpmulld " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPMULUDQ(a,b,c) "vpmuludq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPSRLQ(a,b,c) "vpsrlq " #a ", " #b ", " #c "\n\t"
#define CP_FPS_ASM_VPSUBD(a,b,c) "vpsubd " #a ", " #b ", " #c "\n\t"


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
namespace fps_output_detail {
inline __m256i montgomery(__m256i a) {
	const auto b = _mm256_set1_epi32(932051910u), ni = _mm256_set1_epi32(998244351u), p = _mm256_set1_epi32(Mint::mod());
	auto even = _mm256_mul_epu32(a, b), odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), b);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}
}

inline void prepare_fps_output(std::vector<Mint> &out, std::size_t n) {
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
inline void append_fps_words(std::vector<Mint> &out, const unsigned *in, std::size_t n) {
	static_assert(sizeof(Mint) == sizeof(unsigned) && std::is_trivially_copyable_v<Mint>);
	if (n == 0) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + n);
	std::array<Mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if constexpr (montgomery) std::memcpy(static_cast<void *>(batch.data()), in + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (int j = 0; j < 64; j += 8) {
				const auto x = fps_output_detail::montgomery(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i + j)));
				std::memcpy(static_cast<void *>(batch.data() + j), &x, sizeof(x));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	for (; i < n; ++i) {
		if constexpr (montgomery) out.emplace_back(Mint::word(in[i]));
		else out.emplace_back(Mint::raw(in[i]));
	}
}
}
#pragma GCC pop_options
#endif

#line 9 "components/fps_blocked.hpp"
#include <bit>
#line 12 "components/fps_blocked.hpp"
#include <cstdlib>
#line 14 "components/fps_blocked.hpp"
#include <span>
#line 17 "components/fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP, kNI = 998244351;

inline void forward(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
R(%[P],z) R(%[P2],r) D((%[x]),d) m(d,G)\
D(32(%[x]),w)\
D((%[y]),d)\
m(d,L)\
D(32(%[y]),E)\
D(4(%[y]),d)\
m(d,M)\
D(36(%[y]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(H,d)\
m(J,c)\
m(N,k)\
m(Q,g)\
q(H,w,u)\
q(J,w,s)\
q(4(%[a],%[h3]),w,e)\
q(36(%[a],%[h3]),w,i)\
m(I,l)\
v(t,u,u)\
m(K,n)\
v(t,s,s)\
B(A,e,u,e)\
B(A,i,s,i)\
m((%[a]),s)\
p(G,d,d)\
p(z,e,e)\
m(O,u)\
p(G,c,c)\
p(z,i,i)\
j(r,k,f)\
C(f,k,f)\
q(I,w,k)\
j(e,d,e)\
j(r,g,d)\
C(d,g,d)\
j(i,c,i)\
q(K,w,c)\
q(4(%[a],%[h],2),w,g)\
v(t,k,k)\
p(G,l,l)\
p(G,n,n)\
v(t,c,c)\
B(A,g,k,g)\
q(36(%[a],%[h],2),w,k)\
p(z,g,g)\
B(A,k,c,k)\
o(r,f,c)\
o(e,f,f)\
j(e,c,e)\
p(z,k,k)\
j(g,l,g)\
o(r,d,l)\
o(i,d,d)\
j(i,l,i)\
v(t,f,l)\
q(E,f,c)\
q(E,l,l)\
p(L,f,f)\
j(k,n,k)\
v(t,e,n)\
q(F,n,n)\
v(t,c,c)\
B(A,l,c,l)\
q(F,e,c)\
p(M,e,e)\
p(z,l,l)\
v(t,c,c)\
B(A,n,c,n)\
j(l,f,l)\
v(t,d,f)\
q(E,d,c)\
q(E,f,f)\
p(z,n,n)\
p(L,d,d)\
v(t,c,c)\
B(A,f,c,f)\
j(n,e,n)\
v(t,i,e)\
q(F,i,c)\
q(F,e,e)\
p(z,f,f)\
p(M,i,i)\
v(t,c,c)\
B(A,e,c,e)\
j(f,d,f)\
j(r,s,d)\
j(r,u,c)\
C(d,s,d)\
C(c,u,c)\
p(z,e,e)\
o(g,d,u)\
j(g,d,g)\
o(k,c,d)\
j(k,c,k)\
j(r,u,c)\
o(r,g,s)\
C(c,u,c)\
C(s,g,s)\
j(e,i,e)\
j(r,d,i)\
o(r,k,g)\
C(i,d,i)\
o(r,c,d)\
o(r,s,u)\
C(g,k,g)\
o(l,c,c)\
j(l,d,l)\
o(n,s,s)\
o(r,i,d)\
o(r,g,k)\
j(n,u,n)\
o(f,i,i)\
m(c,(%[a]))\
j(f,d,f)\
o(e,g,g)\
j(e,k,e)\
m(l,N)\
m(s,I)\
m(n,H)\
m(i,O)\
m(f,Q)\
m(g,K)\
m(e,J)\
S($64,%[a])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm1,%%ymm0,%%ymm6,%%ymm10,%%ymm3,%%ymm7,CP_FPS_ASM_VPSUBD,%%ymm2,%%ymm8,CP_FPS_ASM_VMOVDQA,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,%%ymm5,$32,%%ymm4,CP_FPS_ASM_VPSRLQ,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h],2),32(%[a],%[h3]),32(%[a],%[h],2),32(%[w]),64(%[w]),(%[a],%[h]),32(%[a]),32(%[a],%[h]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) \
Q(%[P],w) Q(%[P2],k) C((%[x]),c) r(c,F)\
C(32(%[x]),v)\
C((%[y]),c)\
r(c,K)\
C(32(%[y]),D)\
C(4(%[y]),c)\
r(c,L)\
C(36(%[y]),E)\
".p2align 5\n\t" \
"1:\n\t" \
r((%[a]),c)\
r(N,f)\
r(M,s)\
r(O,t)\
n(k,c,i)\
n(H,c,c)\
n(k,f,j)\
h(H,i,i)\
n(J,f,f)\
h(k,c,u)\
n(k,s,e)\
h(J,j,j)\
q(l,i,g)\
p(D,i,d)\
n(k,t,m)\
h(G,e,e)\
p(D,g,g)\
o(K,i,i)\
h(I,m,m)\
n(G,s,s)\
B(u,c,u)\
q(l,j,c)\
p(D,c,c)\
q(l,d,d)\
n(I,t,t)\
A(z,g,d,g)\
p(D,j,d)\
o(K,j,j)\
o(w,g,g)\
q(l,d,d)\
A(z,c,d,c)\
h(g,i,g)\
q(l,e,i)\
p(E,e,d)\
p(E,i,i)\
o(w,c,c)\
o(L,e,e)\
q(l,d,d)\
A(z,i,d,i)\
h(c,j,c)\
q(l,m,j)\
p(E,m,d)\
p(E,j,j)\
o(w,i,i)\
o(L,m,m)\
q(l,d,d)\
A(z,j,d,j)\
h(i,e,i)\
h(k,f,e)\
h(k,s,d)\
o(w,j,j)\
B(e,f,e)\
B(d,s,d)\
h(k,t,s)\
n(k,u,f)\
B(s,t,s)\
n(k,e,t)\
n(d,u,u)\
h(d,f,d)\
h(s,t,t)\
n(s,e,s)\
n(k,g,e)\
h(j,m,j)\
n(k,c,m)\
h(i,e,e)\
q(l,d,f)\
h(j,m,m)\
p(v,f,f)\
n(i,g,i)\
n(j,c,j)\
p(v,d,c)\
q(l,e,g)\
o(F,d,d)\
p(v,g,g)\
q(l,c,c)\
A(z,f,c,f)\
p(v,e,c)\
o(F,e,e)\
o(w,f,f)\
q(l,c,c)\
A(z,g,c,g)\
h(f,d,f)\
q(l,t,d)\
p(v,t,c)\
p(v,d,d)\
o(w,g,g)\
r(f,M)\
q(l,m,f)\
p(v,f,f)\
o(F,t,t)\
q(l,c,c)\
A(z,d,c,d)\
h(g,e,g)\
p(v,m,e)\
o(F,m,m)\
o(w,d,d)\
h(k,u,c)\
r(g,G)\
h(k,i,g)\
B(c,u,c)\
q(l,e,e)\
h(k,s,u)\
B(g,i,g)\
r(c,(%[a]))\
A(z,f,e,f)\
h(k,j,e)\
B(u,s,u)\
h(d,t,d)\
r(g,H)\
o(w,f,f)\
B(e,j,e)\
r(u,N)\
r(e,J)\
r(d,O)\
h(f,m,f)\
r(f,I)\
R($64,%[a])\
S(%[end],%[a])\
T(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm9,%%ymm7,%%ymm1,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm4,%%ymm5,%%ymm14,$32,%%ymm10,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm2,%%ymm3,%%ymm6,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h]),32(%[a],%[h3]),32(%[a],%[h]),32(%[w]),64(%[w]),(%[a],%[h],2),32(%[a]),32(%[a],%[h],2),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void forward_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) \
K(%[P],A) K(%[P2],d) I(4(%[y]),z) I(36(%[y]),t)\
".p2align 5\n\t" \
"1:\n\t" \
j(C,o)\
j(G,g)\
j(B,p)\
j(F,l)\
j((%[a]),i)\
j(D,m)\
j(E,q)\
j(H,r)\
c(d,o,h)\
c(d,g,f)\
n(h,o,h)\
c(d,p,o)\
n(f,g,f)\
k(d,h,g)\
c(d,l,s)\
n(o,p,o)\
k(d,f,p)\
n(s,l,s)\
c(o,g,g)\
c(d,i,l)\
k(o,h,o)\
c(s,p,p)\
x(u,g,h)\
w(t,g,e)\
n(l,i,l)\
x(u,p,i)\
w(t,h,h)\
v(z,g,g)\
k(s,f,s)\
w(t,p,f)\
w(t,i,i)\
x(u,e,e)\
v(z,p,p)\
J($170,h,e,h)\
c(d,m,e)\
x(u,f,f)\
n(e,m,e)\
v(A,h,h)\
c(d,q,m)\
n(m,q,m)\
J($170,i,f,i)\
c(d,r,f)\
k(e,l,q)\
n(f,r,f)\
c(e,l,e)\
v(A,i,i)\
c(d,q,l)\
k(f,m,r)\
c(f,m,f)\
k(d,e,m)\
n(l,q,l)\
c(d,r,q)\
n(m,e,m)\
c(d,o,e)\
c(h,g,h)\
k(d,f,g)\
n(q,r,q)\
c(d,s,r)\
n(e,o,e)\
n(g,f,g)\
k(d,l,f)\
k(d,m,o)\
n(r,s,r)\
c(i,p,i)\
k(d,q,p)\
k(d,g,s)\
k(e,l,l)\
c(e,f,e)\
k(h,m,m)\
c(h,o,h)\
k(r,q,q)\
j(l,(%[a]))\
c(r,p,r)\
k(i,g,g)\
c(i,s,i)\
j(e,C)\
j(m,D)\
j(h,B)\
j(q,E)\
j(r,G)\
j(g,H)\
j(i,F)\
L($64,%[a])\
M(%[end],%[a])\
N(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm11,%%ymm9,%%ymm1,%%ymm8,%%ymm4,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,%%ymm3,%%ymm5,CP_FPS_ASM_VPMINUD,%%ymm0,%%ymm2,%%ymm6,%%ymm7,%%ymm10,%%ymm13,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm12,%%ymm15,(%[a],%[h3]),(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h3]),32(%[a],%[h]),32(%[a],%[h],2),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) \
K(%[P],A) K(%[P2],f) I(4(%[y]),z) I(36(%[y]),t)\
".p2align 5\n\t" \
"1:\n\t" \
p(F,c)\
p(H,r)\
p((%[a]),n)\
p(G,j)\
h(f,c,o)\
h(B,c,c)\
h(f,r,i)\
e(B,o,o)\
h(D,r,r)\
e(f,c,s)\
e(D,i,i)\
x(u,o,d)\
w(t,o,g)\
v(z,o,o)\
x(u,i,k)\
w(t,i,m)\
w(t,d,d)\
h(C,n,q)\
w(t,k,k)\
v(z,i,i)\
e(C,n,n)\
l(s,c,s)\
x(u,g,g)\
h(E,j,c)\
e(E,j,j)\
x(u,m,m)\
J($170,d,g,d)\
e(f,q,g)\
J($170,k,m,k)\
v(A,d,d)\
l(g,q,g)\
h(f,n,q)\
v(A,k,k)\
e(f,c,m)\
l(q,n,q)\
h(s,g,n)\
l(m,c,m)\
e(s,g,s)\
e(f,r,g)\
h(f,j,c)\
e(d,o,d)\
l(g,r,g)\
l(c,j,c)\
e(f,n,j)\
e(k,i,k)\
h(d,q,i)\
e(d,q,d)\
h(g,m,q)\
h(k,c,r)\
e(g,m,g)\
e(k,c,k)\
e(f,i,c)\
h(f,s,m)\
h(f,d,o)\
l(j,n,j)\
l(c,i,c)\
e(f,q,i)\
e(f,r,n)\
l(m,s,m)\
l(o,d,o)\
p(j,(%[a]))\
h(f,g,j)\
h(f,k,d)\
l(i,q,i)\
l(n,r,n)\
p(c,C)\
l(j,g,j)\
l(d,k,d)\
p(m,F)\
p(o,B)\
p(i,G)\
p(n,E)\
p(j,H)\
p(d,D)\
L($64,%[a])\
M(%[end],%[a])\
N(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm7,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm8,CP_FPS_ASM_VPADDD,%%ymm5,%%ymm3,%%ymm9,CP_FPS_ASM_VPMINUD,%%ymm10,%%ymm2,%%ymm4,CP_FPS_ASM_VMOVDQA,%%ymm11,%%ymm1,%%ymm6,%%ymm13,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm12,%%ymm15,(%[a],%[h3]),(%[a],%[h]),32(%[a],%[h3]),32(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h],2),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void scale_radix2(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K) \
H(%[P],m) H(%[P2],C) G(4(%[y]),s) G(36(%[y]),l)\
".p2align 5\n\t" \
"1:\n\t" \
r((%[a]),c)\
r(F,d)\
z(C,c,e)\
z(D,c,c)\
z(C,d,f)\
k(D,e,e)\
z(E,d,d)\
q(n,c,h)\
k(E,f,f)\
p(l,c,v)\
q(n,e,i)\
p(l,e,w)\
q(n,d,j)\
p(l,d,x)\
q(n,f,g)\
p(l,f,u)\
p(l,h,h)\
p(l,i,i)\
p(l,j,j)\
p(l,g,g)\
q(n,v,v)\
q(n,w,w)\
q(n,x,x)\
q(n,u,u)\
o(s,c,c)\
o(s,e,e)\
A(t,h,v,h)\
A(t,i,w,i)\
o(s,d,d)\
o(s,f,f)\
A(t,j,x,j)\
A(t,g,u,g)\
o(m,h,h)\
o(m,i,i)\
o(m,j,j)\
o(m,g,g)\
k(h,c,h)\
k(i,e,i)\
k(j,d,j)\
k(g,f,g)\
k(m,h,f)\
k(m,i,d)\
k(m,j,e)\
k(m,g,c)\
B(f,h,f)\
B(d,i,d)\
B(e,j,e)\
B(c,g,c)\
r(f,(%[a]))\
r(d,D)\
r(e,F)\
r(c,E)\
I($64,%[a])\
J(%[end],%[a])\
K(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm1,%%ymm2,%%ymm3,%%ymm10,%%ymm4,%%ymm6,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm13,%%ymm15,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm12,$170,%%ymm11,%%ymm5,%%ymm7,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm14,(%[a],%[h]),32(%[a],%[h]),32(%[a]),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}
namespace blocked_ntt {

inline constexpr std::uint32_t kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec;
using kernels::kP;

constexpr std::uint32_t multiply_mod(std::uint32_t x, std::uint32_t y) {
	return std::uint32_t(std::uint64_t(x) * y % kP);
}

constexpr std::uint32_t power(std::uint32_t x, std::uint32_t e) {
	std::uint32_t r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr std::uint32_t quotient(std::uint32_t w) { return std::uint32_t((std::uint64_t(w) << 32) / kP); }

inline constexpr std::uint32_t kR = std::uint32_t((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<std::uint32_t, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(std::uint32_t x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, std::uint32_t bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(std::uint32_t value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(std::uint32_t value, std::uint32_t value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, f.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline std::size_t slot(std::size_t k) { return (k >> 3 << 4) | (k & 7); }

inline std::size_t table_words(int lg) { return (std::size_t(1) << lg) / 8 + 16; }

inline void build_table(std::uint32_t* t, std::size_t count, const std::array<std::uint32_t, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const std::size_t h = std::size_t(1) << level;
		if (h >= count) break;
		const std::uint32_t root = roots[level];
		if (h < 8) {
			for (std::size_t j = 0; j < h; ++j) {
				const std::uint32_t v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (std::size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

inline void inverse_radix4(Vec* f, std::size_t h, const std::uint32_t* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(multiply(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = reduce(add(p0, p1), 2 * kP), cd = reduce(add(p2, p3), 2 * kP);
		const Vec amb = reduce(diff(p0, p1), 2 * kP), cmd = multiply(diff(p2, p3), z);
		f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

}
}
namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4) \
_0(%[P],w) _0(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
_4(H,E)\
Z($-8,E)\
Y(H,E)\
G((%[table],%[tx],4),E)\
G((%[k],%[k]),F)\
Z($-8,F)\
G((%[ty],%[k],2),F)\
G((%[table],%[ty],4),F)\
D(32(%[tx]),p)\
D((%[tx]),A)\
s(R,d)\
s((%[a]),u)\
o(M,p,e)\
o(388(%[a]),p,g)\
n(M,A,c)\
i(q,d,f)\
o(I,p,C)\
z(f,d,f)\
o(260(%[a]),p,d)\
n(I,A,v)\
i(q,u,B)\
t(r,e,e)\
z(B,u,B)\
y(x,g,e,g)\
m(q,f,e)\
t(r,C,C)\
D(32(%[ty]),u)\
n(w,g,g)\
y(x,d,C,d)\
n(w,d,d)\
D((%[ty]),C)\
i(g,c,g)\
m(g,f,f)\
i(g,e,g)\
i(d,v,d)\
D(36(%[ty]),v)\
t(r,f,e)\
t(r,g,c)\
m(d,B,j)\
i(d,B,d)\
D(4(%[ty]),B)\
i(q,j,h)\
m(q,d,l)\
z(h,j,h)\
z(l,d,l)\
o(u,f,d)\
o(u,e,e)\
m(q,h,j)\
n(C,f,f)\
t(r,d,d)\
y(x,e,d,e)\
m(q,l,d)\
o(v,c,c)\
n(w,e,e)\
i(e,f,e)\
s(V,f)\
m(e,h,h)\
i(e,j,e)\
o(v,g,j)\
n(B,g,g)\
s(h,(%[a]))\
s(S,h)\
s(e,R)\
o(J,p,e)\
t(r,j,j)\
y(x,c,j,c)\
o(292(%[a]),p,j)\
t(r,e,e)\
n(w,c,c)\
y(x,j,e,j)\
o(N,p,e)\
n(w,j,j)\
i(c,g,c)\
o(420(%[a]),p,g)\
m(c,l,l)\
i(c,d,c)\
n(N,A,d)\
t(r,e,e)\
s(l,I)\
i(q,h,l)\
y(x,g,e,g)\
z(l,h,l)\
s(c,M)\
n(J,A,c)\
i(q,f,h)\
n(w,g,g)\
m(q,l,e)\
z(h,f,h)\
i(j,c,j)\
i(g,d,g)\
m(j,h,d)\
i(j,h,j)\
m(g,l,l)\
i(g,e,g)\
i(q,d,e)\
m(q,j,h)\
t(r,l,c)\
o(u,l,f)\
n(C,l,l)\
z(e,d,e)\
t(r,g,d)\
o(u,c,c)\
z(h,j,h)\
m(q,e,j)\
o(v,d,d)\
t(r,f,f)\
y(x,c,f,c)\
o(v,g,f)\
n(B,g,g)\
n(w,c,c)\
t(r,f,f)\
y(x,d,f,d)\
i(c,l,c)\
s(W,l)\
m(q,h,f)\
n(w,d,d)\
m(c,e,e)\
i(c,j,c)\
s(T,j)\
s(e,V)\
o(K,p,e)\
s(c,S)\
o(324(%[a]),p,c)\
i(d,g,d)\
o(O,p,g)\
t(r,e,e)\
m(d,h,h)\
i(d,f,d)\
o(452(%[a]),p,f)\
y(x,c,e,c)\
s(h,J)\
n(O,A,h)\
n(K,A,e)\
t(r,g,g)\
n(w,c,c)\
s(d,N)\
y(x,f,g,f)\
i(q,l,g)\
n(w,f,f)\
i(q,j,d)\
z(g,l,g)\
z(d,j,d)\
i(c,e,c)\
m(q,d,e)\
m(c,g,j)\
i(c,g,c)\
i(f,h,f)\
i(q,j,h)\
m(q,c,g)\
m(f,d,d)\
i(f,e,f)\
z(h,j,h)\
z(g,c,g)\
t(r,d,c)\
o(u,d,j)\
t(r,f,e)\
o(v,f,l)\
o(u,c,c)\
o(v,e,e)\
n(C,d,d)\
n(B,f,f)\
t(r,j,j)\
t(r,l,l)\
y(x,c,j,c)\
y(x,e,l,e)\
m(q,h,l)\
m(q,g,j)\
n(w,c,c)\
n(w,e,e)\
i(c,d,c)\
i(e,f,e)\
s(U,f)\
s(X,d)\
m(c,h,h)\
i(c,l,c)\
m(e,g,g)\
i(e,j,e)\
o(Q,p,j)\
o(L,p,l)\
s(h,W)\
o(484(%[a]),p,h)\
o(356(%[a]),p,p)\
s(c,T)\
n(Q,A,c)\
n(L,A,A)\
s(g,K)\
t(r,j,j)\
t(r,l,l)\
s(e,O)\
y(x,h,j,h)\
y(x,p,l,p)\
n(w,h,h)\
n(w,p,p)\
i(q,f,l)\
z(l,f,l)\
i(q,d,f)\
i(h,c,h)\
m(q,l,c)\
z(f,d,f)\
i(p,A,p)\
m(h,l,l)\
i(h,c,h)\
m(p,f,c)\
i(p,f,p)\
t(r,l,f)\
o(u,l,A)\
t(r,h,d)\
o(v,h,j)\
o(u,f,u)\
o(v,d,v)\
i(q,c,d)\
m(q,p,f)\
n(C,l,C)\
n(B,h,B)\
z(d,c,d)\
z(f,p,f)\
t(r,A,A)\
t(r,j,j)\
m(q,d,p)\
m(q,f,c)\
y(x,u,A,u)\
y(x,v,j,v)\
n(w,u,u)\
n(w,v,v)\
i(u,C,u)\
i(v,B,v)\
m(u,d,d)\
i(u,p,u)\
m(v,f,f)\
i(v,c,v)\
s(d,X)\
s(u,U)\
s(f,L)\
s(v,Q)\
Y($512,%[a])\
_2(H)\
_1(%[end],%[a])\
_3(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm2,%%ymm4,%%ymm7,%%ymm5,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm11,%%ymm13,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm0,%%ymm14,$32,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPSRLQ,%%ymm3,%%ymm9,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm1,%%ymm10,%%ymm8,CP_FPS_ASM_VBROADCASTSS,%[tx],%[ty],CP_FPS_ASM_LEA,%[k],256(%[a]),288(%[a]),320(%[a]),352(%[a]),384(%[a]),416(%[a]),448(%[a]),480(%[a]),128(%[a]),160(%[a]),192(%[a]),224(%[a]),32(%[a]),64(%[a]),96(%[a]),CP_FPS_ASM_ADD,CP_FPS_ASM_AND,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_INC,CP_FPS_ASM_JNE,CP_FPS_ASM_MOV)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4) \
_0(%[P],x) _0(%[P2],n) ".p2align 5\n\t" "1:\n\t" \
_4(H,E)\
Z($-8,E)\
Y(H,E)\
G((%[table],%[tx],4),E)\
G((%[k],%[k]),F)\
Z($-8,F)\
G((%[ty],%[k],2),F)\
G((%[table],%[ty],4),F)\
t((%[a]),l)\
t(R,m)\
D(32(%[ty]),v)\
D((%[ty]),C)\
p(n,l,w)\
p(I,l,l)\
i(I,w,w)\
p(n,m,g)\
i(n,l,c)\
p(M,m,m)\
D(36(%[ty]),u)\
s(o,w,d)\
r(v,w,e)\
i(M,g,g)\
q(C,w,w)\
D(4(%[ty]),B)\
r(v,d,d)\
s(o,g,f)\
A(c,l,c)\
i(n,m,l)\
A(l,m,l)\
s(o,e,e)\
p(l,c,m)\
z(y,d,e,d)\
i(n,m,e)\
q(x,d,d)\
p(n,c,c)\
A(e,m,e)\
D(32(%[tx]),m)\
i(l,c,l)\
t(e,(%[a]))\
s(o,l,e)\
r(u,g,c)\
r(u,f,f)\
i(d,w,d)\
q(B,g,g)\
D((%[tx]),w)\
s(o,c,c)\
z(y,f,c,f)\
p(n,d,c)\
q(x,f,f)\
r(m,l,h)\
r(m,e,e)\
i(f,g,f)\
i(f,c,c)\
s(o,h,h)\
q(w,l,l)\
p(f,d,f)\
s(o,c,d)\
r(m,c,g)\
z(y,e,h,e)\
q(w,c,c)\
t(T,h)\
r(m,d,d)\
q(x,e,e)\
i(n,f,j)\
A(j,f,j)\
t(S,f)\
s(o,g,g)\
t(j,I)\
z(y,d,g,d)\
i(e,l,e)\
q(x,d,d)\
t(e,R)\
i(d,c,d)\
p(n,h,c)\
p(J,h,h)\
t(d,M)\
i(J,c,c)\
p(n,f,d)\
i(n,h,e)\
p(N,f,f)\
s(o,c,l)\
r(v,c,g)\
i(N,d,d)\
q(C,c,c)\
r(v,l,l)\
s(o,d,j)\
A(e,h,e)\
i(n,f,h)\
r(u,j,j)\
A(h,f,h)\
r(u,d,f)\
s(o,g,g)\
q(B,d,d)\
z(y,l,g,l)\
p(n,e,g)\
p(h,e,e)\
s(o,f,f)\
q(x,l,l)\
i(h,g,h)\
i(n,e,g)\
z(y,j,f,j)\
A(g,e,g)\
s(o,h,e)\
r(m,h,f)\
q(x,j,j)\
r(m,e,e)\
t(g,T)\
i(l,c,l)\
q(w,h,h)\
p(n,l,c)\
s(o,f,f)\
i(j,d,j)\
z(y,e,f,e)\
i(j,c,c)\
q(x,e,e)\
p(j,l,j)\
t(W,l)\
s(o,c,f)\
r(m,c,d)\
q(w,c,c)\
i(n,j,g)\
r(m,f,f)\
A(g,j,g)\
t(U,j)\
i(e,h,e)\
t(g,J)\
s(o,d,d)\
t(e,S)\
z(y,f,d,f)\
q(x,f,f)\
p(n,l,d)\
p(K,l,l)\
i(f,c,f)\
i(K,d,d)\
p(n,j,c)\
i(n,l,e)\
p(O,j,j)\
t(f,N)\
s(o,d,f)\
r(v,d,g)\
i(O,c,c)\
q(C,d,d)\
r(v,f,f)\
s(o,c,h)\
A(e,l,e)\
i(n,j,l)\
r(u,h,h)\
A(l,j,l)\
r(u,c,j)\
s(o,g,g)\
q(B,c,c)\
z(y,f,g,f)\
p(n,e,g)\
p(l,e,e)\
s(o,j,j)\
q(x,f,f)\
i(l,g,l)\
i(n,e,g)\
z(y,h,j,h)\
A(g,e,g)\
s(o,l,e)\
r(m,l,j)\
q(x,h,h)\
r(m,e,e)\
t(g,W)\
i(f,d,f)\
q(w,l,l)\
p(n,f,d)\
s(o,j,j)\
i(h,c,h)\
z(y,e,j,e)\
i(h,d,d)\
q(x,e,e)\
p(h,f,h)\
t(X,f)\
s(o,d,j)\
r(m,d,c)\
q(w,d,d)\
i(n,h,g)\
r(m,j,j)\
A(g,h,g)\
t(V,h)\
i(e,l,e)\
t(g,K)\
s(o,c,c)\
t(e,U)\
z(y,j,c,j)\
q(x,j,j)\
p(n,f,c)\
p(L,f,f)\
i(j,d,j)\
i(L,c,c)\
p(n,h,d)\
i(n,f,e)\
p(Q,h,h)\
t(j,O)\
s(o,c,j)\
r(v,c,g)\
i(Q,d,d)\
q(C,c,C)\
r(v,j,v)\
s(o,d,j)\
r(u,d,c)\
A(e,f,e)\
r(u,j,u)\
q(B,d,B)\
i(n,h,d)\
s(o,g,g)\
A(d,h,d)\
z(y,v,g,v)\
s(o,c,c)\
p(n,e,g)\
p(d,e,e)\
q(x,v,v)\
z(y,u,c,u)\
i(d,g,d)\
i(n,e,g)\
q(x,u,u)\
s(o,d,c)\
r(m,d,h)\
A(g,e,g)\
r(m,c,c)\
q(w,d,d)\
t(g,X)\
i(v,C,v)\
i(u,B,u)\
p(n,v,B)\
s(o,h,h)\
i(u,B,B)\
z(y,c,h,c)\
p(u,v,u)\
s(o,B,v)\
r(m,B,h)\
q(x,c,c)\
i(n,u,C)\
r(m,v,m)\
q(w,B,w)\
A(C,u,C)\
t(C,L)\
s(o,h,h)\
i(c,d,c)\
z(y,m,h,m)\
t(c,V)\
q(x,m,m)\
i(m,w,m)\
t(m,Q)\
Y($512,%[a])\
_2(H)\
_1(%[end],%[a])\
_3(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm8,%%ymm9,%%ymm11,%%ymm5,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm13,%%ymm0,%%ymm1,%%ymm14,$32,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm7,%%ymm2,%%ymm4,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm10,%%ymm3,CP_FPS_ASM_VBROADCASTSS,%[tx],%[ty],CP_FPS_ASM_LEA,%[k],128(%[a]),160(%[a]),192(%[a]),224(%[a]),384(%[a]),416(%[a]),448(%[a]),480(%[a]),256(%[a]),288(%[a]),32(%[a]),320(%[a]),352(%[a]),64(%[a]),96(%[a]),CP_FPS_ASM_ADD,CP_FPS_ASM_AND,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_INC,CP_FPS_ASM_JNE,CP_FPS_ASM_MOV)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y) {
	Vec* const end = a + 4 * count;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) \
_12(%[P],t) _12(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
A(32(%[x]),i)\
A((%[x]),d)\
B(_1,n)\
A(32(%[y]),j)\
A(36(%[y]),h)\
B((%[a]),o)\
A((%[y]),r)\
w(T,i,f)\
w(100(%[a]),i,e)\
v(T,d,s)\
c(q,n,g)\
w(N,i,k)\
l(g,n,g)\
w(68(%[a]),i,i)\
A(4(%[y]),n)\
C(z,f,f)\
u(q,g,m)\
v(N,d,d)\
c(q,o,p)\
E(D,e,f,e)\
C(z,k,k)\
l(p,o,p)\
v(t,e,e)\
E(D,i,k,i)\
v(t,i,i)\
c(e,s,e)\
u(e,g,g)\
c(e,m,e)\
c(i,d,i)\
C(z,g,d)\
w(j,g,m)\
C(z,e,s)\
w(h,e,k)\
w(j,d,j)\
w(h,s,h)\
u(i,p,s)\
c(i,p,i)\
v(r,g,r)\
v(n,e,n)\
c(q,s,e)\
u(q,i,g)\
B(Y,p)\
C(z,m,m)\
C(z,k,k)\
l(e,s,e)\
l(g,i,g)\
A(36(%[x]),i)\
B(X,s)\
E(D,j,m,j)\
E(D,h,k,h)\
u(q,e,k)\
u(q,g,m)\
A(4(%[x]),d)\
v(t,j,j)\
v(t,h,h)\
c(j,r,j)\
c(h,n,h)\
u(j,e,e)\
c(j,k,j)\
u(h,g,g)\
c(h,m,h)\
c(q,p,m)\
c(q,e,k)\
c(q,j,n)\
c(q,g,r)\
w(G,i,o)\
w(228(%[a]),i,f)\
l(m,p,m)\
c(q,s,p)\
l(p,s,p)\
l(k,e,k)\
l(n,j,n)\
l(r,g,r)\
A(40(%[y]),g)\
v(G,d,j)\
u(q,m,e)\
c(q,h,s)\
C(z,o,o)\
v(F,d,d)\
l(s,h,s)\
w(F,i,h)\
E(D,f,o,f)\
w(196(%[a]),i,i)\
A(44(%[y]),o)\
v(t,f,f)\
C(z,h,h)\
E(D,i,h,i)\
A(8(%[y]),h)\
c(f,j,f)\
v(t,i,i)\
A(12(%[y]),j)\
u(f,m,m)\
c(f,e,f)\
c(t,k,e)\
l(e,k,e)\
C(z,m,k)\
w(g,k,k)\
c(i,d,i)\
B(e,(%[a]))\
w(g,m,g)\
C(z,f,e)\
w(o,e,e)\
w(o,f,o)\
u(i,p,d)\
c(i,p,i)\
c(q,d,p)\
C(z,g,g)\
l(p,d,p)\
u(q,i,d)\
C(z,o,o)\
E(D,k,g,k)\
v(h,m,h)\
l(d,i,d)\
E(D,e,o,e)\
v(t,k,k)\
v(j,f,j)\
u(q,p,f)\
B(Z,o)\
v(t,e,e)\
u(q,d,i)\
c(t,n,m)\
c(t,r,g)\
l(m,n,m)\
l(g,r,g)\
A(40(%[x]),r)\
B(_0,n)\
c(k,h,k)\
B(m,_1)\
A(8(%[x]),m)\
c(t,s,h)\
c(e,j,e)\
u(k,p,p)\
c(k,f,k)\
l(h,s,h)\
B(g,N)\
u(e,d,d)\
c(e,i,e)\
c(q,p,i)\
c(q,k,g)\
B(h,T)\
c(q,d,h)\
c(q,e,s)\
l(i,p,i)\
l(g,k,g)\
l(h,d,h)\
l(s,e,s)\
c(t,i,e)\
c(t,g,d)\
A(48(%[y]),k)\
c(t,h,p)\
c(t,s,f)\
l(e,i,e)\
l(d,g,d)\
A(16(%[y]),g)\
c(q,o,i)\
l(p,h,p)\
l(f,s,f)\
B(e,X)\
w(I,r,e)\
w(356(%[a]),r,s)\
c(q,n,h)\
l(i,o,i)\
B(d,Y)\
A(52(%[y]),d)\
v(I,m,o)\
l(h,n,h)\
B(p,F)\
w(H,r,p)\
w(324(%[a]),r,r)\
u(q,h,n)\
B(f,G)\
C(z,e,e)\
A(20(%[y]),f)\
v(H,m,m)\
E(D,s,e,s)\
v(t,s,s)\
C(z,p,p)\
E(D,r,p,r)\
v(t,r,r)\
c(s,o,s)\
u(s,h,h)\
c(s,n,s)\
C(z,h,n)\
w(k,h,o)\
C(z,s,p)\
w(d,s,e)\
w(k,n,k)\
w(d,p,d)\
c(r,m,r)\
u(r,i,m)\
c(r,i,r)\
v(g,h,g)\
v(f,s,f)\
B(_2,s)\
C(z,o,o)\
C(z,e,e)\
c(q,m,h)\
u(q,r,i)\
A(44(%[x]),p)\
B(_3,n)\
E(D,k,o,k)\
E(D,d,e,d)\
l(h,m,h)\
l(i,r,i)\
A(12(%[x]),r)\
v(t,k,k)\
v(t,d,d)\
u(q,h,m)\
u(q,i,e)\
c(k,g,k)\
c(d,f,d)\
u(k,h,h)\
c(k,m,k)\
u(d,i,i)\
c(d,e,d)\
c(q,s,e)\
c(q,h,m)\
c(q,k,f)\
c(q,i,g)\
w(K,p,o)\
w(484(%[a]),p,j)\
l(e,s,e)\
c(q,d,s)\
l(m,h,m)\
l(f,k,f)\
l(g,i,g)\
l(s,d,s)\
A(56(%[y]),d)\
v(K,r,i)\
c(q,n,k)\
c(t,m,h)\
C(z,o,o)\
l(k,n,k)\
v(J,r,r)\
l(h,m,h)\
A(24(%[y]),m)\
E(D,j,o,j)\
B(h,Z)\
A(60(%[y]),h)\
w(J,p,o)\
u(q,k,n)\
v(t,j,j)\
w(452(%[a]),p,p)\
C(z,o,o)\
c(j,i,j)\
E(D,p,o,p)\
A(28(%[y]),o)\
c(t,f,i)\
u(j,k,k)\
c(j,n,j)\
v(t,p,p)\
l(i,f,i)\
C(z,k,f)\
w(d,k,n)\
B(i,_0)\
w(d,f,d)\
v(m,k,m)\
C(z,j,k)\
c(t,g,f)\
w(h,k,k)\
c(p,r,p)\
l(f,g,f)\
w(h,j,h)\
C(z,n,n)\
u(p,e,g)\
c(p,e,p)\
c(t,s,e)\
B(f,H)\
E(D,d,n,d)\
c(q,g,n)\
u(q,p,f)\
l(e,s,e)\
B(_5,s)\
C(z,h,h)\
v(t,d,d)\
l(n,g,n)\
l(f,p,f)\
B(e,I)\
A(48(%[x]),e)\
B(_4,p)\
A(16(%[x]),g)\
E(D,k,h,k)\
v(o,j,o)\
u(q,n,j)\
u(q,f,h)\
v(t,k,k)\
c(d,m,d)\
u(d,n,n)\
c(d,j,d)\
c(q,n,j)\
c(q,d,m)\
c(k,o,k)\
l(j,n,j)\
l(m,d,m)\
u(k,f,f)\
c(k,h,k)\
c(t,j,h)\
c(t,m,d)\
c(q,s,n)\
c(q,f,o)\
c(q,k,r)\
l(h,j,h)\
w(M,e,j)\
w(612(%[a]),e,i)\
l(n,s,n)\
c(q,p,s)\
B(h,_2)\
l(s,p,s)\
l(o,f,o)\
l(r,k,r)\
l(d,m,d)\
A(96(%[y]),m)\
A(64(%[y]),k)\
A(100(%[y]),f)\
v(M,g,p)\
u(q,n,h)\
B(d,_3)\
C(z,j,j)\
v(L,g,g)\
w(L,e,d)\
E(D,i,j,i)\
w(580(%[a]),e,e)\
A(68(%[y]),j)\
v(t,i,i)\
C(z,d,d)\
E(D,e,d,e)\
c(t,o,d)\
c(i,p,i)\
v(t,e,e)\
l(d,o,d)\
c(t,r,o)\
u(i,n,n)\
c(i,h,i)\
l(o,r,o)\
B(d,J)\
C(z,n,d)\
w(m,n,r)\
C(z,i,h)\
w(f,i,p)\
B(o,K)\
w(m,d,m)\
w(f,h,f)\
c(e,g,e)\
u(e,s,g)\
c(e,s,e)\
v(k,n,k)\
v(j,i,j)\
B(_6,i)\
C(z,r,r)\
C(z,p,p)\
c(q,g,n)\
u(q,e,s)\
A(52(%[x]),h)\
B(_7,d)\
E(D,m,r,m)\
E(D,f,p,f)\
l(n,g,n)\
l(s,e,s)\
A(20(%[x]),e)\
v(t,m,m)\
v(t,f,f)\
u(q,n,g)\
u(q,s,p)\
c(m,k,m)\
c(f,j,f)\
u(m,n,n)\
c(m,g,m)\
u(f,s,s)\
c(f,p,f)\
c(q,i,p)\
c(q,n,g)\
c(q,m,j)\
c(q,s,k)\
w(Q,h,r)\
w(740(%[a]),h,o)\
l(p,i,p)\
c(q,f,i)\
l(g,n,g)\
l(j,m,j)\
l(k,s,k)\
l(i,f,i)\
A(104(%[y]),f)\
v(Q,e,s)\
c(q,d,m)\
c(t,g,n)\
C(z,r,r)\
l(m,d,m)\
v(O,e,e)\
l(n,g,n)\
A(72(%[y]),g)\
E(D,o,r,o)\
B(n,_4)\
A(108(%[y]),n)\
w(O,h,r)\
u(q,m,d)\
v(t,o,o)\
w(708(%[a]),h,h)\
C(z,r,r)\
c(o,s,o)\
E(D,h,r,h)\
A(76(%[y]),r)\
c(t,j,s)\
u(o,m,m)\
c(o,d,o)\
v(t,h,h)\
l(s,j,s)\
C(z,m,j)\
w(f,m,d)\
B(s,_5)\
w(f,j,f)\
v(g,m,g)\
C(z,o,m)\
c(t,k,j)\
w(n,m,m)\
c(h,e,h)\
l(j,k,j)\
w(n,o,n)\
C(z,d,d)\
u(h,p,k)\
c(h,p,h)\
c(t,i,p)\
B(j,L)\
E(D,f,d,f)\
c(q,k,d)\
u(q,h,j)\
l(p,i,p)\
B(_9,i)\
C(z,n,n)\
v(t,f,f)\
l(d,k,d)\
l(j,h,j)\
B(p,M)\
A(56(%[x]),p)\
B(_8,h)\
A(24(%[x]),k)\
E(D,m,n,m)\
v(r,o,r)\
u(q,d,o)\
u(q,j,n)\
v(t,m,m)\
c(f,g,f)\
u(f,d,d)\
c(f,o,f)\
c(q,d,o)\
c(q,f,g)\
c(m,r,m)\
l(o,d,o)\
l(g,f,g)\
u(m,j,j)\
c(m,n,m)\
c(t,o,n)\
c(t,g,f)\
c(q,i,d)\
c(q,j,r)\
c(q,m,e)\
l(n,o,n)\
w(S,p,o)\
w(868(%[a]),p,s)\
l(d,i,d)\
c(q,h,i)\
B(n,_6)\
l(i,h,i)\
l(r,j,r)\
l(e,m,e)\
l(f,g,f)\
A(112(%[y]),g)\
A(80(%[y]),m)\
A(116(%[y]),j)\
v(S,k,h)\
u(q,d,n)\
B(f,_7)\
C(z,o,o)\
v(R,k,k)\
w(R,p,f)\
E(D,s,o,s)\
w(836(%[a]),p,p)\
A(84(%[y]),o)\
v(t,s,s)\
C(z,f,f)\
E(D,p,f,p)\
c(t,r,f)\
c(s,h,s)\
v(t,p,p)\
l(f,r,f)\
c(t,e,r)\
u(s,d,d)\
c(s,n,s)\
l(r,e,r)\
B(f,O)\
C(z,d,f)\
w(g,d,e)\
C(z,s,n)\
w(j,s,h)\
B(r,Q)\
w(g,f,g)\
w(j,n,j)\
c(p,k,p)\
u(p,i,k)\
c(p,i,p)\
v(m,d,m)\
v(o,s,o)\
B(_10,s)\
C(z,e,e)\
C(z,h,h)\
c(q,k,d)\
u(q,p,i)\
A(60(%[x]),n)\
B(_11,f)\
E(D,g,e,g)\
E(D,j,h,j)\
l(d,k,d)\
l(i,p,i)\
A(28(%[x]),p)\
v(t,g,g)\
v(t,j,j)\
u(q,d,k)\
u(q,i,h)\
c(g,m,g)\
c(j,o,j)\
u(g,d,d)\
c(g,k,g)\
u(j,i,i)\
c(j,h,j)\
c(q,s,h)\
c(q,d,k)\
c(q,g,o)\
c(q,i,m)\
w(V,n,e)\
w(996(%[a]),n,r)\
l(h,s,h)\
c(q,j,s)\
l(k,d,k)\
l(o,g,o)\
l(m,i,m)\
l(s,j,s)\
A(120(%[y]),j)\
v(V,p,i)\
c(q,f,g)\
c(t,k,d)\
C(z,e,e)\
l(g,f,g)\
v(U,p,p)\
l(d,k,d)\
A(88(%[y]),k)\
E(D,r,e,r)\
B(d,_8)\
A(124(%[y]),d)\
w(U,n,e)\
u(q,g,f)\
v(t,r,r)\
w(964(%[a]),n,n)\
C(z,e,e)\
c(r,i,r)\
E(D,n,e,n)\
A(92(%[y]),e)\
c(t,o,i)\
u(r,g,g)\
c(r,f,r)\
v(t,n,n)\
l(i,o,i)\
C(z,g,o)\
w(j,g,f)\
B(i,_9)\
w(j,o,j)\
v(k,g,k)\
C(z,r,g)\
c(t,m,o)\
w(d,g,g)\
c(n,p,n)\
l(o,m,o)\
w(d,r,d)\
C(z,f,f)\
u(n,h,m)\
c(n,h,n)\
c(t,s,h)\
B(o,R)\
E(D,j,f,j)\
c(q,m,f)\
u(q,n,o)\
l(h,s,h)\
C(z,d,d)\
v(t,j,j)\
l(f,m,f)\
l(o,n,o)\
B(h,S)\
E(D,g,d,g)\
v(e,r,e)\
u(q,f,r)\
u(q,o,d)\
v(t,g,g)\
c(j,k,j)\
u(j,f,f)\
c(j,r,j)\
c(g,e,g)\
c(q,f,e)\
c(q,j,r)\
u(g,o,o)\
c(g,d,g)\
l(e,f,e)\
l(r,j,r)\
c(q,o,j)\
c(q,g,f)\
c(t,e,d)\
c(t,r,k)\
l(j,o,j)\
l(f,g,f)\
l(d,e,d)\
l(k,r,k)\
c(t,j,r)\
c(t,f,e)\
B(d,_10)\
l(r,j,r)\
l(e,f,e)\
B(k,_11)\
B(r,U)\
B(e,V)\
W($1024,%[a])\
W($64,%[x])\
W($128,%[y])\
_13(%[end],%[a])\
_14(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm1,%%ymm8,%%ymm7,%%ymm10,%%ymm4,%%ymm0,%%ymm3,%%ymm11,CP_FPS_ASM_VPMINUD,%%ymm12,%%ymm2,%%ymm5,%%ymm13,%%ymm14,%%ymm6,%%ymm9,%%ymm15,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,$32,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPSRLQ,$170,CP_FPS_ASM_VPBLENDD,192(%[a]),224(%[a]),320(%[a]),352(%[a]),448(%[a]),480(%[a]),576(%[a]),608(%[a]),64(%[a]),704(%[a]),736(%[a]),832(%[a]),864(%[a]),96(%[a]),960(%[a]),992(%[a]),CP_FPS_ASM_ADD,128(%[a]),160(%[a]),256(%[a]),288(%[a]),32(%[a]),384(%[a]),416(%[a]),512(%[a]),544(%[a]),640(%[a]),672(%[a]),768(%[a]),800(%[a]),896(%[a]),928(%[a]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y, const Vec* from) {
	Vec* const end = a + 4 * count;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z) \
X(%[P],B) X(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
A((%[from]),p)\
A(64(%[from]),i)\
z(32(%[y]),c)\
z(36(%[y]),k)\
z((%[y]),w)\
z(4(%[y]),f)\
z(32(%[x]),n)\
s(q,p,g)\
s(J,p,p)\
e(J,g,g)\
s(q,i,j)\
e(q,p,o)\
s(V,i,i)\
v(r,g,m)\
u(c,g,h)\
e(V,j,j)\
E(o,p,o)\
z((%[x]),p)\
u(c,m,c)\
v(r,j,m)\
u(k,j,d)\
e(q,i,l)\
u(k,m,k)\
t(w,g,w)\
E(l,i,l)\
s(q,o,i)\
A(128(%[from]),g)\
v(r,h,h)\
t(f,j,f)\
e(l,i,i)\
s(l,o,l)\
A(192(%[from]),o)\
D(C,c,h,c)\
v(r,d,d)\
v(r,i,h)\
u(n,i,j)\
t(B,c,c)\
D(C,k,d,k)\
u(n,h,h)\
z(40(%[y]),d)\
e(q,l,m)\
t(B,k,k)\
E(m,l,m)\
z(8(%[y]),l)\
v(r,j,j)\
A(m,(%[a]))\
e(c,w,c)\
D(C,h,j,h)\
t(p,i,i)\
z(44(%[y]),j)\
e(k,f,k)\
s(q,c,f)\
t(B,h,h)\
z(12(%[y]),w)\
e(k,f,f)\
s(k,c,k)\
s(q,g,c)\
s(G,g,g)\
v(r,f,m)\
t(p,f,p)\
e(G,c,c)\
u(n,f,f)\
u(n,m,n)\
e(h,i,h)\
s(q,o,i)\
v(r,c,m)\
s(H,o,o)\
A(h,64(%[a]))\
e(H,i,i)\
u(d,m,m)\
t(l,c,l)\
z(36(%[x]),h)\
u(d,c,d)\
v(r,f,f)\
v(r,i,c)\
D(C,n,f,n)\
e(q,g,f)\
u(j,c,c)\
E(f,g,f)\
t(B,n,n)\
e(q,o,g)\
u(j,i,j)\
v(r,d,d)\
t(w,i,w)\
E(g,o,g)\
z(4(%[x]),o)\
D(C,m,d,m)\
s(q,f,d)\
s(g,f,f)\
e(q,k,i)\
t(B,m,m)\
e(g,d,g)\
E(i,k,i)\
e(n,p,n)\
A(256(%[from]),p)\
A(320(%[from]),k)\
v(r,j,j)\
v(r,g,d)\
A(i,32(%[a]))\
D(C,c,j,c)\
u(h,d,d)\
A(n,96(%[a]))\
u(h,g,n)\
e(q,f,j)\
t(B,c,c)\
e(m,l,m)\
E(j,f,j)\
z(48(%[y]),f)\
z(16(%[y]),l)\
s(q,m,i)\
A(j,128(%[a]))\
v(r,n,n)\
e(c,w,c)\
D(C,d,n,d)\
t(o,g,g)\
z(52(%[y]),n)\
e(c,i,i)\
t(B,d,d)\
s(c,m,c)\
s(q,p,m)\
z(20(%[y]),w)\
s(I,p,p)\
v(r,i,j)\
t(o,i,o)\
e(I,m,m)\
u(h,i,i)\
u(h,j,h)\
s(q,k,j)\
e(K,j,j)\
s(K,k,k)\
e(d,g,d)\
v(r,m,g)\
u(f,g,g)\
A(d,192(%[a]))\
u(f,m,f)\
v(r,j,d)\
t(l,m,l)\
v(r,i,i)\
z(40(%[x]),m)\
D(C,h,i,h)\
e(q,p,i)\
u(n,d,d)\
v(r,f,f)\
E(i,p,i)\
t(B,h,h)\
z(8(%[x]),p)\
u(n,j,n)\
D(C,g,f,g)\
e(q,k,f)\
t(B,g,g)\
t(w,j,w)\
E(f,k,f)\
s(q,i,k)\
e(f,k,k)\
s(f,i,f)\
e(h,o,h)\
A(384(%[from]),o)\
e(q,c,i)\
v(r,n,n)\
E(i,c,i)\
A(h,224(%[a]))\
A(448(%[from]),h)\
v(r,k,c)\
e(q,f,j)\
D(C,d,n,d)\
e(g,l,g)\
E(j,f,j)\
A(i,160(%[a]))\
z(56(%[y]),i)\
z(24(%[y]),f)\
t(B,d,d)\
s(q,g,l)\
u(m,k,n)\
A(j,256(%[a]))\
u(m,c,c)\
t(p,k,k)\
e(d,w,d)\
v(r,n,n)\
z(60(%[y]),w)\
e(d,l,l)\
D(C,c,n,c)\
s(d,g,d)\
s(q,o,g)\
z(28(%[y]),n)\
s(L,o,o)\
v(r,l,j)\
t(B,c,c)\
e(L,g,g)\
u(m,j,j)\
t(p,l,p)\
u(m,l,m)\
s(q,h,l)\
s(M,h,h)\
e(M,l,l)\
t(f,g,f)\
e(c,k,c)\
v(r,g,k)\
u(i,g,g)\
u(i,k,i)\
A(c,320(%[a]))\
v(r,l,c)\
z(44(%[x]),k)\
v(r,m,m)\
u(w,c,c)\
D(C,j,m,j)\
u(w,l,w)\
e(q,o,m)\
v(r,g,g)\
E(m,o,m)\
t(B,j,j)\
e(q,h,o)\
D(C,i,g,i)\
t(n,l,n)\
E(o,h,o)\
z(12(%[x]),h)\
s(q,m,l)\
v(r,w,w)\
t(B,i,i)\
e(o,l,l)\
s(o,m,o)\
A(512(%[from]),m)\
D(C,c,w,c)\
v(r,l,w)\
e(q,d,g)\
e(j,p,j)\
t(B,c,c)\
E(g,d,g)\
A(j,352(%[a]))\
A(576(%[from]),j)\
e(q,o,d)\
e(i,f,i)\
u(k,w,w)\
E(d,o,d)\
A(g,288(%[a]))\
z(96(%[y]),g)\
u(k,l,o)\
z(64(%[y]),f)\
s(q,i,p)\
A(d,384(%[a]))\
e(c,n,c)\
z(100(%[y]),n)\
e(c,p,p)\
v(r,o,o)\
s(c,i,c)\
v(r,p,i)\
u(k,p,d)\
D(C,w,o,w)\
t(h,l,l)\
u(k,i,k)\
t(B,w,w)\
z(68(%[y]),i)\
e(q,c,o)\
t(h,p,h)\
E(o,c,o)\
s(q,m,c)\
s(N,m,m)\
e(N,c,c)\
v(r,d,d)\
A(o,416(%[a]))\
s(q,j,o)\
e(q,m,p)\
e(O,o,o)\
s(O,j,j)\
E(p,m,p)\
D(C,k,d,k)\
v(r,c,d)\
u(g,c,m)\
t(f,c,f)\
e(q,j,c)\
u(g,d,g)\
E(c,j,c)\
t(B,k,k)\
e(w,l,w)\
z(48(%[x]),l)\
z(16(%[x]),j)\
A(w,448(%[a]))\
v(r,o,w)\
u(n,o,d)\
u(n,w,n)\
v(r,m,m)\
s(q,p,w)\
s(c,p,p)\
D(C,g,m,g)\
t(i,o,i)\
e(c,w,c)\
e(k,h,k)\
A(640(%[from]),h)\
A(704(%[from]),w)\
v(r,d,d)\
t(B,g,g)\
v(r,c,o)\
e(q,p,m)\
A(k,480(%[a]))\
D(C,n,d,n)\
E(m,p,m)\
z(104(%[y]),p)\
t(B,n,n)\
A(m,512(%[a]))\
z(72(%[y]),m)\
e(g,f,g)\
s(q,g,f)\
u(l,c,d)\
u(l,o,o)\
e(n,i,n)\
t(j,c,c)\
z(108(%[y]),i)\
e(n,f,f)\
s(n,g,n)\
s(q,h,g)\
s(Q,h,h)\
v(r,f,k)\
v(r,d,d)\
e(Q,g,g)\
u(l,k,k)\
D(C,o,d,o)\
t(j,f,j)\
z(76(%[y]),d)\
u(l,f,l)\
t(B,o,o)\
s(q,w,f)\
s(R,w,w)\
e(R,f,f)\
t(m,g,m)\
v(r,l,l)\
e(o,c,o)\
v(r,g,c)\
u(p,g,g)\
u(p,c,p)\
D(C,k,l,k)\
A(o,576(%[a]))\
v(r,f,o)\
z(52(%[x]),l)\
e(q,h,c)\
u(i,o,o)\
E(c,h,c)\
t(B,k,k)\
e(q,w,h)\
u(i,f,i)\
v(r,g,g)\
E(h,w,h)\
z(20(%[x]),w)\
D(C,p,g,p)\
t(d,f,d)\
s(q,c,f)\
s(h,c,c)\
t(B,p,p)\
e(h,f,h)\
e(k,j,k)\
A(768(%[from]),j)\
e(q,n,f)\
v(r,i,i)\
E(f,n,f)\
A(k,608(%[a]))\
A(832(%[from]),k)\
v(r,h,n)\
e(q,c,g)\
D(C,o,i,o)\
E(g,c,g)\
A(f,544(%[a]))\
z(112(%[y]),f)\
t(B,o,o)\
e(p,m,p)\
A(g,640(%[a]))\
z(80(%[y]),g)\
s(q,p,m)\
u(l,h,c)\
u(l,n,n)\
e(o,d,o)\
t(w,h,h)\
z(116(%[y]),d)\
e(o,m,m)\
v(r,c,c)\
s(o,p,o)\
s(q,j,p)\
z(84(%[y]),i)\
s(S,j,j)\
D(C,n,c,n)\
t(w,m,w)\
e(S,p,p)\
t(B,n,n)\
s(q,k,c)\
s(T,k,k)\
e(T,c,c)\
t(g,p,g)\
e(n,h,n)\
v(r,p,h)\
u(f,p,p)\
u(f,h,f)\
A(n,704(%[a]))\
v(r,c,n)\
e(q,j,h)\
E(h,j,h)\
z(56(%[x]),j)\
u(d,n,n)\
v(r,p,p)\
u(d,c,d)\
D(C,f,p,f)\
t(i,c,i)\
e(q,k,c)\
z(24(%[x]),p)\
t(B,f,f)\
E(c,k,c)\
s(q,h,k)\
v(r,d,d)\
e(c,k,k)\
s(c,h,c)\
v(r,m,h)\
D(C,n,d,n)\
u(l,m,m)\
u(l,h,l)\
v(r,k,h)\
t(B,n,n)\
e(f,g,f)\
A(896(%[from]),g)\
e(q,o,d)\
E(d,o,d)\
A(960(%[from]),o)\
u(j,h,h)\
v(r,m,m)\
A(d,672(%[a]))\
s(q,f,d)\
e(n,i,n)\
D(C,l,m,l)\
z(120(%[y]),m)\
u(j,k,i)\
e(n,d,d)\
t(B,l,l)\
t(p,k,k)\
s(n,f,n)\
z(88(%[y]),f)\
t(p,d,p)\
v(r,i,i)\
D(C,h,i,h)\
e(l,w,l)\
z(124(%[y]),w)\
v(r,d,i)\
u(j,d,d)\
u(j,i,j)\
t(B,h,h)\
A(l,736(%[a]))\
s(q,g,l)\
z(92(%[y]),i)\
s(U,g,g)\
e(U,l,l)\
v(r,d,d)\
D(C,j,d,j)\
e(h,k,h)\
s(q,o,k)\
v(r,l,d)\
e(W,k,k)\
u(m,d,d)\
t(f,l,f)\
s(W,o,o)\
A(h,832(%[a]))\
u(m,l,m)\
t(B,j,j)\
v(r,k,l)\
z(60(%[x]),h)\
u(w,l,l)\
u(w,k,w)\
v(r,m,m)\
t(i,k,i)\
e(j,p,j)\
e(q,g,p)\
z(28(%[x]),k)\
D(C,d,m,d)\
E(p,g,p)\
A(j,864(%[a]))\
e(q,o,j)\
e(q,c,g)\
v(r,w,w)\
t(B,d,d)\
E(j,o,j)\
E(g,c,g)\
D(C,l,w,l)\
s(q,p,w)\
s(j,p,p)\
e(q,n,c)\
A(g,768(%[a]))\
t(B,l,l)\
e(j,w,j)\
e(q,p,w)\
E(c,n,c)\
e(d,f,d)\
v(r,j,f)\
u(h,j,n)\
E(w,p,w)\
A(c,800(%[a]))\
s(q,d,c)\
u(h,f,f)\
A(w,896(%[a]))\
e(l,i,l)\
e(l,c,c)\
v(r,n,n)\
t(k,j,j)\
s(l,d,l)\
v(r,c,d)\
u(h,c,i)\
D(C,f,n,f)\
t(k,c,k)\
u(h,d,h)\
t(B,f,f)\
e(q,l,d)\
E(d,l,d)\
v(r,i,i)\
A(d,928(%[a]))\
D(C,h,i,h)\
e(f,j,f)\
t(B,h,h)\
A(f,960(%[a]))\
e(h,k,h)\
A(h,992(%[a]))\
F($1024,%[a])\
F($1024,%[from])\
F($64,%[x])\
F($128,%[y])\
Y(%[end],%[a])\
Z(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm2,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm5,%%ymm7,%%ymm11,%%ymm1,%%ymm8,%%ymm3,%%ymm13,%%ymm10,%%ymm6,%%ymm9,%%ymm0,%%ymm14,$32,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm4,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VMOVDQA,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_ADD,160(%[from]),224(%[from]),288(%[from]),32(%[from]),352(%[from]),416(%[from]),480(%[from]),544(%[from]),608(%[from]),672(%[from]),736(%[from]),800(%[from]),864(%[from]),928(%[from]),96(%[from]),992(%[from]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y), [from] "+r"(from)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_top8_lower(Vec* a, std::size_t q, const std::uint32_t* table) {
	alignas(32) Vec w[3];
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],y) S(%[P2],x) D(4(%[t]),d) m(d,0(%[w]))\
D(36(%[t]),z)\
D(8(%[t]),d)\
m(d,H)\
D(40(%[t]),E)\
D(12(%[t]),d)\
m(d,I)\
D(44(%[t]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(J,d)\
m(L,o)\
m(K,n)\
m((%[a]),p)\
v(q,o,f)\
u(z,o,i)\
u(z,f,f)\
k(y,n,c)\
s(C,o,e)\
k(d,n,r)\
v(q,d,j)\
u(z,d,g)\
l(d,c,c)\
u(z,j,j)\
v(q,i,i)\
s(C,d,d)\
B(A,f,i,f)\
v(q,c,i)\
v(q,g,g)\
u(z,i,i)\
s(y,f,f)\
B(A,j,g,j)\
u(z,c,g)\
s(C,c,c)\
s(y,j,j)\
l(f,e,f)\
k(x,p,e)\
v(q,g,g)\
l(f,e,e)\
k(f,p,f)\
l(j,d,j)\
B(A,i,g,i)\
k(y,p,g)\
l(x,f,d)\
s(y,i,i)\
k(o,p,p)\
l(o,g,o)\
G(d,f,d)\
k(x,n,f)\
k(j,n,n)\
l(x,e,g)\
l(j,f,j)\
G(g,e,g)\
v(q,n,e)\
u(E,n,f)\
u(E,e,e)\
s(H,n,n)\
l(i,c,i)\
v(q,j,c)\
u(F,c,c)\
v(q,f,f)\
B(A,e,f,e)\
u(F,j,f)\
s(I,j,j)\
s(y,e,e)\
v(q,f,f)\
B(A,c,f,c)\
l(e,n,e)\
m(N,n)\
k(x,p,f)\
k(r,p,p)\
s(y,c,c)\
l(r,f,r)\
m(p,(%[a]))\
m(Q,p)\
k(x,o,f)\
k(i,o,o)\
l(i,f,i)\
m(r,K)\
m(O,r)\
k(x,d,f)\
k(e,d,d)\
l(e,f,e)\
m(o,L)\
m(M,o)\
k(x,g,f)\
l(c,j,c)\
m(i,J)\
k(c,g,g)\
l(c,f,c)\
m(d,(%[b]))\
m(e,(%[b],%[h]))\
m(g,(%[b],%[h],2))\
m(c,(%[b],%[h3]))\
v(q,p,c)\
u(z,p,g)\
u(z,c,c)\
k(y,r,e)\
s(C,p,d)\
k(n,r,f)\
v(q,n,i)\
u(z,n,j)\
l(n,e,e)\
u(z,i,i)\
v(q,g,g)\
s(C,n,n)\
B(A,c,g,c)\
v(q,e,g)\
v(q,j,j)\
u(z,g,g)\
s(y,c,c)\
B(A,i,j,i)\
u(z,e,j)\
s(C,e,e)\
s(y,i,i)\
l(c,d,c)\
k(x,o,d)\
v(q,j,j)\
l(c,d,d)\
k(c,o,c)\
l(i,n,i)\
B(A,g,j,g)\
k(y,o,j)\
l(x,c,n)\
s(y,g,g)\
k(p,o,o)\
l(p,j,p)\
G(n,c,n)\
k(x,r,c)\
k(i,r,r)\
l(x,d,j)\
l(i,c,i)\
G(j,d,j)\
v(q,r,d)\
u(E,r,c)\
u(E,d,d)\
s(H,r,r)\
l(g,e,g)\
v(q,i,e)\
u(F,e,e)\
v(q,c,c)\
B(A,d,c,d)\
u(F,i,c)\
s(I,i,i)\
s(y,d,d)\
v(q,c,c)\
B(A,e,c,e)\
l(d,r,d)\
k(x,o,r)\
k(x,p,c)\
s(y,e,e)\
k(f,o,o)\
l(f,r,f)\
k(g,p,p)\
k(x,n,r)\
l(g,c,g)\
k(d,n,n)\
m(o,M)\
k(x,j,o)\
l(d,r,d)\
m(f,O)\
l(e,i,e)\
m(p,Q)\
k(e,j,j)\
l(e,o,e)\
m(g,N)\
m(n,32(%[b]))\
m(d,32(%[b],%[h]))\
m(j,32(%[b],%[h],2))\
m(e,32(%[b],%[h3]))\
R($64,%[a])\
R($64,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm0,%%ymm7,%%ymm4,%%ymm10,%%ymm5,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPSUBD,CP_FPS_ASM_VMOVDQA,%%ymm2,%%ymm1,%%ymm3,$32,%%ymm8,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm14,%%ymm15,%%ymm11,$170,CP_FPS_ASM_VPBLENDD,(%[w]),CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,CP_FPS_ASM_VPMINUD,32(%[w]),64(%[w]),(%[a],%[h3]),(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h3]),32(%[a],%[h]),32(%[a],%[h],2),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [t] "r"(table), [w] "r"(w), [P] "m"(kP),
		  [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_top8_upper(Vec* a, std::size_t q, const std::uint32_t* table) {
	alignas(32) Vec w[3];
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],o) S(%[P2],y) D(4(%[t]),n) m(n,0(%[w]))\
D(36(%[t]),z)\
D(8(%[t]),n)\
m(n,H)\
D(40(%[t]),E)\
D(12(%[t]),n)\
m(n,I)\
D(44(%[t]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(J,n)\
m(L,v)\
m(K,e)\
m((%[b]),k)\
j(n,o,c)\
j(v,o,f)\
u(q,f,p)\
s(z,f,d)\
j(e,o,g)\
l(o,e,i)\
s(z,p,p)\
j(n,i,i)\
r(C,f,f)\
j(k,o,x)\
l(n,e,n)\
u(q,c,e)\
s(z,e,e)\
u(q,d,d)\
B(A,p,d,p)\
s(z,c,d)\
r(C,c,c)\
r(o,p,p)\
u(q,d,d)\
B(A,e,d,e)\
j(p,f,p)\
u(q,i,f)\
s(z,i,d)\
r(o,e,e)\
s(z,f,f)\
r(C,i,i)\
u(q,d,d)\
j(e,c,e)\
B(A,f,d,f)\
l(y,x,d)\
l(p,x,x)\
l(e,g,c)\
r(o,f,f)\
j(p,d,p)\
l(o,k,d)\
l(y,g,g)\
l(v,k,k)\
j(v,d,v)\
u(q,c,d)\
j(e,g,e)\
s(E,d,d)\
s(E,c,g)\
r(H,c,c)\
j(f,i,f)\
u(q,e,i)\
s(F,i,i)\
u(q,g,g)\
B(A,d,g,d)\
s(F,e,g)\
r(I,e,e)\
r(o,d,d)\
u(q,g,g)\
B(A,i,g,i)\
j(d,c,d)\
j(y,x,c)\
j(y,p,g)\
r(o,i,i)\
G(c,x,c)\
G(g,p,g)\
m(N,p)\
l(y,k,x)\
l(n,k,k)\
j(n,x,n)\
m(Q,x)\
m(k,(%[a]))\
m(O,k)\
j(i,e,i)\
m(n,(%[a],%[h]))\
m(M,n)\
l(y,v,e)\
l(f,v,v)\
j(f,e,f)\
m(v,(%[a],%[h],2))\
l(y,c,v)\
l(y,g,e)\
l(d,c,c)\
j(d,v,d)\
l(i,g,g)\
j(i,e,i)\
m(f,(%[a],%[h3]))\
m(c,(%[b]))\
m(d,K)\
j(p,o,d)\
m(g,L)\
j(x,o,g)\
m(i,J)\
u(q,g,i)\
s(z,g,c)\
j(k,o,f)\
l(o,k,e)\
s(z,i,i)\
j(p,e,e)\
r(C,g,g)\
j(n,o,v)\
l(p,k,p)\
u(q,d,k)\
s(z,k,k)\
u(q,c,c)\
B(A,i,c,i)\
s(z,d,c)\
r(C,d,d)\
r(o,i,i)\
u(q,c,c)\
B(A,k,c,k)\
j(i,g,i)\
u(q,e,g)\
s(z,e,c)\
r(o,k,k)\
s(z,g,g)\
r(C,e,e)\
u(q,c,c)\
j(k,d,k)\
B(A,g,c,g)\
l(y,v,c)\
l(i,v,v)\
l(k,f,d)\
r(o,g,g)\
j(i,c,i)\
l(o,n,c)\
l(y,f,f)\
l(x,n,n)\
j(x,c,x)\
u(q,d,c)\
j(k,f,k)\
s(E,c,c)\
s(E,d,f)\
r(H,d,d)\
j(g,e,g)\
u(q,k,e)\
s(F,e,e)\
u(q,f,f)\
B(A,c,f,c)\
s(F,k,f)\
r(I,k,k)\
r(o,c,c)\
u(q,f,f)\
B(A,e,f,e)\
j(c,d,c)\
j(y,v,d)\
j(y,i,f)\
r(o,e,e)\
G(d,v,d)\
G(f,i,f)\
l(y,n,i)\
l(y,x,v)\
l(p,n,n)\
j(p,i,p)\
l(g,x,x)\
l(y,d,i)\
j(g,v,g)\
l(c,d,d)\
m(n,32(%[a]))\
j(e,k,e)\
l(y,f,k)\
j(c,i,c)\
m(p,32(%[a],%[h]))\
l(e,f,f)\
j(e,k,e)\
m(x,32(%[a],%[h],2))\
m(g,32(%[a],%[h3]))\
m(d,M)\
m(c,O)\
m(f,Q)\
m(e,N)\
R($64,%[a])\
R($64,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm4,%%ymm7,%%ymm2,%%ymm5,%%ymm8,%%ymm9,CP_FPS_ASM_VPSUBD,%%ymm3,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VMOVDQA,%%ymm0,%%ymm15,%%ymm6,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm1,%%ymm10,%%ymm14,%%ymm11,$170,CP_FPS_ASM_VPBLENDD,(%[w]),CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,CP_FPS_ASM_VPMINUD,32(%[w]),64(%[w]),(%[b],%[h3]),(%[b],%[h]),(%[b],%[h],2),32(%[b]),32(%[b],%[h3]),32(%[b],%[h]),32(%[b],%[h],2),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [t] "r"(table), [w] "r"(w), [P] "m"(kP),
		  [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}

namespace blocked_poly {

inline constexpr std::uint32_t kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(std::size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr std::size_t footprint(std::size_t n) { return (n + 7) / 8 * 8 + 16; }
	std::span<std::uint32_t> take(std::size_t n) {
		if (std::size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

	bool zero_initialized() const { return storage_.get_deleter().region != nullptr; }

private:
	FpsWords storage_;
	std::uint32_t *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add;
using blocked_ntt::detail::broadcast;
using blocked_ntt::detail::diff;
using blocked_ntt::detail::Factor;
using blocked_ntt::detail::kP;
using blocked_ntt::detail::kR;
using blocked_ntt::detail::reduce;
using blocked_ntt::detail::slot;
using blocked_ntt::detail::Vec;

inline constexpr auto kInverseScales = [] {
	std::array<std::array<std::uint32_t, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(std::uint32_t(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Vec load(const std::uint32_t* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(std::uint32_t* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const std::uint32_t* roots, std::size_t p) {
	const std::uint32_t* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const std::uint32_t* table, std::size_t g)
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
	std::uint32_t word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const std::uint32_t* roots,
												std::size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const std::uint32_t* b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2, t3;
	#define CP_FPS_ASM_BLOCK(a,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r,s,t,u,v) \
i((%[b]),a) d(36(%[w]),a,e) d(r,a,c) i(4(%[b]),a)\
d(r,a,g)\
d(q,a,h)\
i(8(%[b]),a)\
d(q,a,j)\
d(o,a,k)\
f(g,e,e)\
f(h,c,c)\
i(12(%[b]),a)\
d(o,a,g)\
d(n,a,h)\
f(j,e,e)\
f(k,c,c)\
i(16(%[b]),a)\
d(n,a,j)\
d(m,a,k)\
f(g,e,e)\
f(h,c,c)\
i(20(%[b]),a)\
d(m,a,g)\
d(l,a,h)\
f(j,e,e)\
f(k,c,c)\
i(24(%[b]),a)\
d(l,a,j)\
d(s,a,k)\
f(g,e,e)\
f(h,c,c)\
i(28(%[b]),a)\
d(s,a,g)\
d(4(%[w]),a,h)\
f(j,e,e)\
f(k,c,c)\
f(g,e,e)\
f(h,c,c)\
d(%[ni],c,g)\
d(%[ni],e,h)\
d(%[p],g,g)\
d(%[p],h,h)\
f(g,c,c)\
f(h,e,e)\
u($32,c,c)\
t($0xaa,e,c,c)\
v(%[p2],c,g)\
"vpminud %[t0], %[even], %[even]"
	asm(
		CP_FPS_ASM_BLOCK(%[bi],%[even],CP_FPS_ASM_VPMULUDQ,%[odd],CP_FPS_ASM_VPADDQ,%[t0],%[t1],CP_FPS_ASM_VPBROADCASTD,%[t2],%[t3],12(%[w]),16(%[w]),20(%[w]),24(%[w]),28(%[w]),32(%[w]),8(%[w]),CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VPSUBD)
#undef CP_FPS_ASM_BLOCK
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2),
		[t3] "=&x"(t3)
		: [w] "r"(window.word), [b] "r"(b), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)),
		"m"(*reinterpret_cast<const std::uint32_t(*)[8]>(b)));
	return even;
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const std::uint32_t* roots;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		if (count % 8 == 0)
			return kernels::forward_bottom(reinterpret_cast<Vec*>(a), count, roots + slot(first), roots + slot(2 * first));
		for (std::size_t j = 0; j < count; ++j, a += 32) {
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
	const std::uint32_t* inverse_roots;
	const std::uint32_t* in;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const std::uint32_t* from = in + 32 * first;
		if (count % 8 == 0)
			return kernels::inverse_bottom(reinterpret_cast<Vec*>(a), count, inverse_roots + slot(first),
										inverse_roots + slot(2 * first), reinterpret_cast<const Vec*>(from));
		for (std::size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const std::uint32_t* roots;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* b;

	[[gnu::always_inline]] void prepare(const std::uint32_t* a, std::size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(std::uint32_t* a, std::size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const std::uint32_t* a;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[4]) const {
		const std::uint32_t* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <std::size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(std::size_t g, Window (&window)[K][4]) const {
		for (std::size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t));
				for (std::size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const std::uint32_t* b = product.b + 32 * (first + j);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const std::uint32_t* roots, const std::uint32_t* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const std::size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

	static constexpr bool radix8(std::size_t nv) { return nv / 2 > kTile; }

	void below(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		const std::size_t h = nv / 4;
		for (std::size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr std::size_t kTile = 256;

	void tile(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if constexpr (Bottom::kForward) {
			for (std::size_t h = nv / 4; h >= 16; h /= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (std::size_t h = 16; h < nv; h *= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(std::uint32_t* a) { return reinterpret_cast<Vec*>(a); }

	void forward(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const std::uint32_t *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const std::uint32_t* in, std::size_t size, std::size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(std::size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool in_place(const std::uint32_t* out) const { return in_ == out + shift_; }

	void zero_outside(std::uint32_t* out, std::size_t lo, std::size_t hi) const {
		const std::size_t begin = std::clamp(shift_, lo, hi), end = std::clamp(end_, begin, hi);
		std::fill(out + lo, out + begin, 0);
		std::fill(out + end, out + hi, 0);
	}


	bool within(std::size_t lo, std::size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const std::uint32_t* in, std::size_t shift, std::size_t end, std::size_t v) {
		alignas(32) std::uint32_t x[8] = {};
		for (std::size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const std::uint32_t* in_;
	std::size_t shift_, end_;
	std::size_t first_, count_;
	std::size_t first_touched_, touched_;
};

template <class Input>
[[gnu::always_inline]] inline void forward_top4_body(Input in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

template <class Input>
[[gnu::always_inline]] inline void forward_top2_body(Input in, Vec* out, std::size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (std::size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}

inline bool forward_top8(const Source& in, std::uint32_t* a, std::size_t nv, const std::uint32_t* roots) {
	const bool lower = in.within(0, nv / 2);
	if (!in.in_place(a) || !(lower || in.within(nv / 2, nv))) return false;
	const std::size_t half = 4 * nv;
	in.zero_outside(a, lower ? 0 : half, lower ? half : 2 * half);
	auto* v = reinterpret_cast<Vec*>(a);
	if (lower) kernels::forward_top8_lower(v, nv / 8, roots);
	else kernels::forward_top8_upper(v, nv / 8, roots);
	return true;
}

template <class Bottom>
void radix2_halves(const Recursion<Bottom>& recursion, const Source& in, std::uint32_t* a, std::size_t nv,
				   const std::uint32_t* roots) {
	const std::size_t h = nv / 2;
	if constexpr (Bottom::kForward) {
		if (recursion.radix8(nv) && forward_top8(in, a, nv, roots)) {
			recursion.below(a, h, 0);
			recursion.below(a + 8 * h, h, 1);
			return;
		}
		forward_top2(in, reinterpret_cast<Vec*>(a), h);
	}
	recursion.visit(a, h, 0);
	recursion.visit(a + 8 * h, h, 1);
}

inline void inverse_top4(Vec* f, std::size_t h, Half output, const std::uint32_t* inverse_roots, const Factor& s) {
	if (output == Half::kBoth) return blocked_ntt::detail::inverse_radix4(f, h, inverse_roots, s);
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output == Half::kLower) {
			f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		} else {
			f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
		}
	}
}

inline void inverse_top2(Vec* f, std::size_t h, Half output, std::uint32_t scale) {
	if (output == Half::kBoth) {
		alignas(32) std::uint32_t s[16] = {};
		s[1] = scale;
		s[9] = blocked_ntt::detail::quotient(scale);
		return blocked_ntt::kernels::scale_radix2(f, h, s);
	}
	const Factor s(scale);
	for (std::size_t j = 0; j < h; ++j) {
		if (output == Half::kLower) f[j] = reduce(times(add(f[j], f[j + h]), s), kP);
		else f[j + h] = reduce(times(diff(f[j], f[j + h]), s), kP);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr std::size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const std::size_t table = blocked_ntt::detail::table_words(lg_max), entries = (std::size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const std::uint32_t* roots() const { return roots_; }
	const std::uint32_t* inverse_roots() const { return inverse_roots_; }

	void forward(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(std::span<std::uint32_t> a) const { forward(a, 0, a); }

	void forward_upper(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const std::size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (std::size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const std::size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(std::span<const std::uint32_t> in, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), false);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(std::span<std::uint32_t> a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b, Half output = Half::kBoth, std::uint32_t c = 1) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(inverse_scale(out.size(), true), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(std::span<const std::uint32_t> a, std::span<const std::uint32_t> b, std::span<std::uint32_t> out,
						Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		std::span<const std::uint32_t> a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(std::span<std::uint32_t> c, std::span<const std::uint32_t> a,
					std::span<const std::uint32_t> b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

private:
	std::uint32_t inverse_scale(std::size_t n, bool montgomery) const {
		check_length(n);
		return detail::kInverseScales[montgomery][std::countr_zero(n)];
	}

	void check_length(std::size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <std::size_t K>
	void inverse_products(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		detail::InverseProductSumBottom<K> bottom{};
		for (std::size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	static detail::Source source(std::span<const std::uint32_t> in, std::size_t shift, std::size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(std::span<std::uint32_t> a, const detail::Source& in, const Bottom& bottom, std::uint32_t scale,
			Half output) const {
		using namespace detail;
		check_length(a.size());
		const detail::Recursion recursion(roots_, inverse_roots_, bottom);
		const std::size_t nv = a.size() / 8;
		auto* v = reinterpret_cast<Vec*>(a.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			if constexpr (Bottom::kForward) forward_top4(in, v, h, roots_);
			for (std::size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
			if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots_, Factor(scale));
		} else {
			const std::size_t h = nv / 2;
			radix2_halves(recursion, in, a.data(), nv, roots_);
			if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
		}
	}

	template <class Combine>
	void products(std::uint32_t* out, const std::uint32_t* a, const std::uint32_t* b, std::size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (std::size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	std::uint32_t *roots_, *inverse_roots_;
};

}

namespace blocked_poly {

namespace detail {

inline Vec load_unaligned(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(std::uint32_t* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec montgomery(Vec a, Vec b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b);
	Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

class Indices {
public:
	explicit Indices(std::size_t k) {
		alignas(32) std::uint32_t x[8];
		for (std::uint32_t i = 0; i < 8; ++i) x[i] = std::uint32_t((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr std::uint32_t kStep = std::uint32_t((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline std::uint32_t scalar_inverse(std::uint32_t x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(std::size_t first, std::span<std::uint32_t> out, const A& a, std::uint32_t factor = 1) {
	constexpr std::size_t kLanes = 32;
	const std::size_t n = out.size();
	if (n == 0) return;
	const std::size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(std::uint32_t(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](std::size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(std::uint32_t(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(std::uint32_t(n)), lanes);
	};
	const auto masked = [&](Vec value, std::size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (std::size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	alignas(32) std::uint32_t total[kLanes], inverse[kLanes];
	for (int v = 0; v < 4; ++v) store(total + 8 * v, canonical(prefix[v]));
	std::uint32_t running = 1;
	for (std::size_t l = 0; l < kLanes; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (std::size_t l = kLanes; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	Vec inv[4];
	for (int v = 0; v < 4; ++v) inv[v] = load(inverse + 8 * v);
	const auto put = [&](std::size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (std::size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const std::size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * std::size_t(v) < n) put(8 * v, inv[v]);
}

}

inline void derivative(std::span<const std::uint32_t> f, std::span<std::uint32_t> d) {
	using namespace detail;
	const std::size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(load_unaligned(f.data() + i + 1), index.value())));
	for (std::size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(std::uint32_t(i + 1), f[i + 1]) : 0;
}

inline void divide_by_index(std::span<const std::uint32_t> a, std::size_t first, std::span<std::uint32_t> q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	const std::size_t n = a.size();
	detail::divide_by_index(first, q, [a, n](std::size_t at) __attribute__((target("avx2"))) {
		if (at + 8 <= n) return detail::load_unaligned(a.data() + at);
		alignas(32) std::uint32_t rest[8] = {};
		std::copy(a.begin() + at, a.end(), rest);
		return detail::load(rest);
	});
}

}

#line 3557 "components/fps_blocked.hpp"

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kInverseBase = 32;

inline void inverse_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	const std::uint32_t inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (std::size_t j = 1; j <= std::min(i, f.size() - 1); ++j) {
			sum += std::uint64_t(f[j]) * g[i - j] % kP;
		}
		g[i] = multiply_mod(std::uint32_t(sum % kP ? kP - sum % kP : 0), inv0);
	}
}

}

inline int inverse_log(std::size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)));
}

inline std::size_t inverse_scratch(std::size_t n) { return 2 * Arena::footprint(std::size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
						std::span<std::uint32_t> to, std::span<std::uint32_t> gt, std::span<std::uint32_t> work) {
	const std::size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, detail::kP - 1);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
					std::span<std::uint32_t> scratch) {
	const std::size_t n = g.size();
	std::size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const std::size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

}

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kLogBase = 64;

inline constexpr std::size_t kLogBlocks = 4;

inline void log_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	std::uint32_t a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (std::size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / std::uint32_t(i), inv[kP % i]);
	g[0] = 0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = multiply_mod(std::uint32_t(i), a[i]);
		for (std::size_t k = 1; k < i; ++k) sum += kP - std::uint64_t(kg[k]) * a[i - k] % kP;
		kg[i] = std::uint32_t(sum % kP);
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline std::size_t log_block(std::size_t n) {
	return std::max<std::size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline std::size_t log_blocks(std::size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

inline void subtract_from_derivative(std::span<const std::uint32_t> f, std::size_t first, std::size_t count,
									std::uint32_t* out, std::size_t size) {
	const std::size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (std::size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(load_unaligned(f.data() + first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (std::size_t c = full; c < count; ++c) {
		const std::uint32_t d = c < inside ? blocked_ntt::detail::multiply_mod(std::uint32_t(first + 1 + c), f[first + 1 + c]) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	std::uint32_t* w3;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (std::size_t t = 0; t < 4; ++t) {
			const std::size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const std::uint32_t* side;

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + leaf), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline std::size_t log_buffers(std::size_t n) { return std::max<std::size_t>(log_blocks(n) + 2, 3); }

inline unsigned mint_scalar(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec mint_load(std::span<const Mint> f, std::size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (std::size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const Mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(std::size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(std::size_t lo, std::size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const Mint> f, std::size_t at) { return mint_load(f, at); }
	std::span<const Mint> f_;
	std::size_t full_, touched_;
};
[[gnu::noinline]] inline void mint_forward_top4(MintSource in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void mint_forward_top2(MintSource in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}

inline void mint_forward(const Transform& t, std::span<const Mint> f, std::span<std::uint32_t> out) {
	const MintSource in(f);
	const detail::Recursion recursion(t.roots(), t.inverse_roots(), ForwardBottom{t.roots()});
	const std::size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		mint_forward_top4(in, v, h, t.roots());
		for (std::size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
	} else {
		const std::size_t h = nv / 2;
		mint_forward_top2(in, v, h);
		recursion.visit(out.data(), h, 0);
		recursion.visit(out.data() + 8 * h, h, 1);
	}
}
inline void mint_derivative(std::span<const Mint> f, std::span<std::uint32_t> d) {
	using namespace detail;
	const std::size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(mint_load(f, i + 1), index.value())));
	for (std::size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(std::uint32_t(i + 1), mint_scalar(f[i + 1])) : 0;
}

inline void mint_subtract_from_derivative(std::span<const Mint> f, std::size_t first, std::size_t count,
									std::uint32_t* out, std::size_t size) {
	const std::size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (std::size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(mint_load(f, first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (std::size_t c = full; c < count; ++c) {
		const std::uint32_t d = c < inside ? blocked_ntt::detail::multiply_mod(std::uint32_t(first + 1 + c), mint_scalar(f[first + 1 + c])) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct NoLogTransform {
	void operator()(std::size_t, std::span<const std::uint32_t>) const {}
};

template <class Input, class Sink, class Transformed>
[[gnu::always_inline]] inline void log_derivative_impl(const Transform& t, Input f, std::size_t n,
												std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink, const Transformed& transformed = {}) {
	const std::size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](std::size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const std::span<std::uint32_t> ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](std::size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, prefix, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (std::size_t s = 1; s < blocks; ++s) {
		const std::size_t from = std::min(f.size(), (s - 1) * k);
		if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (std::size_t j = 0; j < blocks; ++j) {
		const std::size_t first = j * k, count = std::min(k, n - 1 - first);
		const std::span<std::uint32_t> work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
			else derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
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
			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_subtract_from_derivative(f, first, count, work.data(), k);
			else subtract_from_derivative(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, std::span<const std::uint32_t>(work.first(count)));
		if (j + 1 < blocks) {
			t.forward(work.first(k), 0, work);
			transformed(j, std::span<const std::uint32_t>(work));
		}
	}
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, std::span<const std::uint32_t> f, std::size_t n,
		std::span<std::uint32_t> scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, f, sink, transformed);
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void mint_log_derivative(const Transform& t, std::span<const Mint> f, std::size_t n,
		std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, prefix, sink, transformed);
}

inline int log_derivative_log(std::size_t n) { return std::countr_zero(2 * log_block(n)); }

inline std::size_t log_derivative_scratch(std::size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}

inline int log_log(std::size_t n) {
	return n <= detail::kLogBase ? Transform::kMinLog : std::countr_zero(2 * detail::log_block(n));
}

inline std::size_t log_scratch(std::size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_buffers(n) * Arena::footprint(2 * detail::log_block(n));
}

inline void log(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](std::size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

}

namespace blocked_poly {
inline void native_log(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool integrate) {
	prepare_fps_output(out, n);
	out.emplace_back(0);
	const std::size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	const bool direct = n >= 262144;
	const auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;
	Arena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(prefix_size), scratch = arena.take(log_scratch(n));
	for (std::size_t i = 0; i < prefix_size; ++i) {
		const unsigned v = in[i].v;
		f[i] = std::min(v, v - Mint::mod());
	}
	const auto sink = [&out, integrate, n](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<std::uint32_t *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](std::size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
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

#line 1 "components/fps_exp_blocked.hpp"



#line 5 "components/fps_exp_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_detail {
namespace blocked_poly {
namespace detail {
struct Factors {
	Vec w, q;
};

inline Vec times(Vec x, const Factors& w) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, w.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(w.q, 32));
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline Factors entries(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return {load(e), load(e + 8)};
}


}
}
namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kExpBase = 64;

inline void exp_direct(std::span<const std::uint32_t> q, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	std::uint32_t inv[kExpBase] = {0, 1};
	for (std::size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / std::uint32_t(i), inv[kP % i]);
	g[0] = 1;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (std::size_t k = 1; k <= i; ++k) sum += std::uint64_t(q[k]) * g[i - k] % kP;
		g[i] = multiply_mod(std::uint32_t(sum % kP), inv[i]);
	}
}

inline void multiply_by_index(std::span<const std::uint32_t> a, std::size_t first, std::span<std::uint32_t> out) {
	const std::size_t n = out.size(), full = n / 8 * 8;
	Indices index(first);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(out.data() + i, canonical(montgomery(load_unaligned(a.data() + i), index.value())));
	for (std::size_t i = full; i < n; ++i) out[i] = blocked_ntt::detail::multiply_mod(std::uint32_t((first + i) % kP), a[i]);
}

inline std::size_t exp_length(std::size_t n) {
	return std::size_t(1) << std::max(Transform::kMinLog + 1, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)));
}

}

inline int exp_log(std::size_t n) { return std::countr_zero(detail::exp_length(n)) - 1; }

namespace detail {

struct QTransforms {
	std::span<const std::uint32_t> low, block2;
};
struct NoQTransforms {};

template <class Bottom>
void product_to(const Transform& t, std::span<std::uint32_t> out, const Source& in, const Bottom& bottom,
				std::uint32_t scale, Half output, std::span<std::uint32_t> to) {
	const std::size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	const Recursion recursion(t.roots(), t.inverse_roots(), bottom);
	const Factor s(scale);
	const auto put = [to, &s](std::size_t j, Vec x) __attribute__((target("avx2"))) {
		const Vec y = reduce(times(x, s), kP);
		if (8 * j + 8 <= to.size()) return store_unaligned(to.data() + 8 * j, y);
		if (8 * j >= to.size()) return;
		const Vec lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		const Vec mask = _mm256_cmpgt_epi32(broadcast(std::uint32_t(to.size() - 8 * j)), lanes);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(to.data() + 8 * j), mask, y);
	};
	const bool lower = output == Half::kLower;
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward_top4(in, v, h, t.roots());
		for (std::size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec ab = low(add(v[j], v[j + h])), cd = low(add(v[j + 2 * h], v[j + 3 * h]));
			const Vec amb = low(diff(v[j], v[j + h])), cmd = times(diff(v[j + 2 * h], v[j + 3 * h]), z);
			put(j, lower ? add(ab, cd) : diff(ab, cd));
			put(j + h, lower ? add(amb, cmd) : diff(amb, cmd));
		}
	} else {
		const std::size_t h = nv / 2;
		radix2_halves(recursion, in, out.data(), nv, t.roots());
		for (std::size_t j = 0; j < h; ++j) put(j, lower ? add(v[j], v[j + h]) : diff(v[j], v[j + h]));
	}
}

template <class Given>
[[gnu::always_inline]] inline void exp_last_step(const Transform& t, std::span<std::uint32_t> lo, std::span<std::uint32_t> g,
												 std::size_t m, bool first, std::span<std::uint32_t> gt,
												 std::span<const std::uint32_t> ht, std::span<std::uint32_t> w,
												 std::span<std::uint32_t> work, const Given& given) {
	const std::size_t n = g.size(), half = m / 2, rest = n - m, count = std::min(rest, half);
	const bool upper = rest > half;
	const std::span<std::uint32_t> g0t = gt.first(m), glt = gt.subspan(m, m), own = w.first(m), r = w.subspan(m, m);
	std::span<const std::uint32_t> qt = own, q2t = lo;
	bool given_low = false, given_block2 = false;
	if constexpr (std::is_same_v<Given, QTransforms>) {
		given_low = !given.low.empty(), given_block2 = !given.block2.empty();
		if (given_low) qt = given.low;
		if (given_block2) q2t = given.block2;
	}
	const auto q = [g, n](std::size_t i) __attribute__((target("avx2"))) {
		if (i + 8 <= n) return load_unaligned(g.data() + i);
		alignas(32) std::uint32_t x[8] = {};
		std::copy(g.begin() + std::ptrdiff_t(i), g.end(), x);
		return load(x);
	};
	if (first) t.forward(g.first(half), 0, g0t);
	t.forward(g.first(m), 0, glt);
	if (given_low) {
		t.inverse_product(qt, glt, r, upper ? Half::kBoth : Half::kLower);
	} else if (upper) {
		t.forward(lo, 0, own);
		t.inverse_product(own, glt, r);
	} else {
		t.cyclic_product(lo, 0, r, glt, Half::kLower);
	}

	t.forward(r.first(half), 0, work);
	t.inverse_product(ht, work, work, Half::kLower);
	detail::divide_by_index(m, r.first(count), [&](std::size_t i) __attribute__((target("avx2"))) {
		const Vec x = reduce(add(load(work.data() + i), q(m + i)), kP);
		return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
	});
	if (upper && !given_block2) t.forward(g.subspan(m, half), 0, lo);
	t.forward(r.first(count), 0, work);
	const std::uint32_t scale = kInverseScales[1][std::countr_zero(m)];
	const Source none(nullptr, 0, 0);
	product_to(t, work, none, InverseProductBottom{{t.roots(), t.inverse_roots(), work.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m, count));
	if (!upper) return;

	t.forward(g.subspan(m, half), 0, work);
	const Transform::Pair pairs[] = {{work, qt}, {q2t, glt}};
	t.inverse_product_sum(pairs, work, Half::kUpper);
	Indices index(half);
	for (std::size_t i = half; i < m; i += 8, index.next()) {
		const Vec xg = montgomery(load_unaligned(g.data() + i), index.value());
		const Vec sum = add(load(r.data() + i), load(work.data() + i));
		store(r.data() + i, canonical(_mm256_sub_epi32(add(sum, broadcast(2 * kP)), xg)));
	}
	t.forward(r.subspan(half, half), 0, own);
	t.inverse_product(ht, own, own, Half::kLower);
	detail::divide_by_index(m + half, r.first(rest - half),
							[&](std::size_t i) __attribute__((target("avx2"))) { return add(load(own.data() + i), q(m + half + i)); });
	t.forward(r.first(rest - half), 0, own);
	product_to(t, own, none, InverseProductBottom{{t.roots(), t.inverse_roots(), own.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m + half, rest - half));
}

template <class Given = NoQTransforms>
[[gnu::always_inline]] inline void exp_newton(const Transform& t, std::span<std::uint32_t> lo, std::span<std::uint32_t> g,
											  std::span<std::uint32_t> scratch, const Given& given = {}) {
	const std::size_t n = g.size(), len = 2 * lo.size();
	const auto take = [&scratch](std::size_t words) __attribute__((target("avx2"))) {
		const std::span<std::uint32_t> s = scratch.first(words);
		scratch = scratch.subspan(Arena::footprint(words));
		return s;
	};
	const std::span<std::uint32_t> h = take(len / 2), gt = take(len), ht = take(len / 2), w = take(len);
	std::size_t m = kExpBase;
	inverse_direct(g, h.first(m / 2));
	t.forward(h.first(m / 2), 0, ht.first(m));
	for (; 2 * m < n; m *= 2) {
		const std::size_t half = m / 2;
		const std::span<std::uint32_t> g_low = gt.first(m), h_low = ht.first(m), w_low = w.first(m);
		t.forward(g.first(m), 0, g_low);

		t.inverse_product(g_low, h_low, w_low, Half::kUpper);
		const std::uint32_t negated = blocked_ntt::detail::multiply_mod(kInverseScales[1][std::countr_zero(m)], kP - 1);
		product_to(t, w_low, Source(w_low.data() + half, half, half), ProductBottom{t.roots(), t.inverse_roots(), h_low.data()},
				   negated, Half::kUpper, h.subspan(half, half));

		t.forward_product(lo.first(m), 0, w_low, g_low);
		t.inverse(w_low, gt.subspan(m, m));

		t.forward(h.first(m), 0, ht.first(2 * m));
		t.forward_upper(gt.subspan(m, m), 0, w.subspan(m, m));
		t.inverse_product(w.first(2 * m), ht.first(2 * m), w.first(2 * m), Half::kLower);

		detail::divide_by_index(m, w.subspan(m, m), [&](std::size_t i) __attribute__((target("avx2"))) {
			const Vec x = reduce(add(load(w.data() + i), load(lo.data() + m + i)), kP);
			return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
		});
		t.forward_upper(g.first(m), 0, gt.subspan(m, m));
		product_to(t, w.first(2 * m), Source(w.data() + m, m, m), ProductBottom{t.roots(), t.inverse_roots(), gt.data()},
				   kInverseScales[1][std::countr_zero(2 * m)], Half::kUpper, g.subspan(m, m));
	}
	exp_last_step(t, lo, g, m, m == kExpBase, gt, ht, w, h, given);
}

inline std::size_t exp_newton_scratch(std::size_t n) {
	const std::size_t len = exp_length(n);
	return 2 * Arena::footprint(len) + 2 * Arena::footprint(len / 2);
}

}

inline std::size_t exp_scratch(std::size_t n) {
	return Arena::footprint(detail::exp_length(n) / 2) + detail::exp_newton_scratch(n);
}

inline void exp(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size(), m = exp_length(n) / 2, size = std::min(f.size(), n), below = std::min(size, m);
	const std::span<std::uint32_t> lo = scratch.first(m);
	multiply_by_index(f.first(below), 0, lo.first(below));
	std::fill(lo.begin() + std::ptrdiff_t(below), lo.end(), 0);
	if (size > m) multiply_by_index(f.subspan(m, size - m), m, g.subspan(m, size - m));
	if (n > std::max(size, m)) std::fill(g.begin() + std::ptrdiff_t(std::max(size, m)), g.end(), 0);
	exp_direct(lo, g.first(std::min(n, kExpBase)));
	if (n > kExpBase) exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
}

}

namespace blocked_poly {

namespace detail {

template <bool kSum>
void times_x(const std::uint32_t* roots, const std::uint32_t* a, const std::uint32_t* b, std::uint32_t e,
			 std::uint32_t alpha, std::uint32_t beta, std::span<std::uint32_t> out) {
	const std::size_t leaves = out.size() / 8;
	const Factor scale(e);
	const Vec rotate = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), odd = _mm256_setr_epi32(0, -1, 0, -1, 0, -1, 0, -1);
	const std::uint32_t constants[2] = {(alpha + beta) % kP, (alpha + kP - beta) % kP};
	for (std::size_t p = 0; p < leaves; p += 8) {
		const bool upper = p >= leaves / 2;

		const Factors r = entries(roots, p / 16 * 8);
		const Vec pick = p % 16 ? _mm256_setr_epi32(4, 4, 5, 5, 6, 6, 7, 7) : _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3);
		const Vec rw = _mm256_permutevar8x32_epi32(r.w, pick), rq = _mm256_permutevar8x32_epi32(r.q, pick);
		const Factors w{_mm256_blend_epi32(rw, _mm256_sub_epi32(broadcast(kP), rw), 0xAA), _mm256_xor_si256(rq, odd)};
		Vec y[8];
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i) {
			Vec c = load(a + 8 * (p + i));
			if constexpr (kSum) c = upper ? difference(c, load(b + 8 * (p + i))) : reduce(add(c, load(b + 8 * (p + i))), kP);
			y[i] = reduce(times(c, scale), kP);
		}

		const Vec t01 = _mm256_unpackhi_epi32(y[0], y[1]), t23 = _mm256_unpackhi_epi32(y[2], y[3]);
		const Vec t45 = _mm256_unpackhi_epi32(y[4], y[5]), t67 = _mm256_unpackhi_epi32(y[6], y[7]);
		const Vec u03 = _mm256_unpackhi_epi64(t01, t23), u47 = _mm256_unpackhi_epi64(t45, t67);
		const Vec z = canonical(add(times(_mm256_permute2x128_si256(u03, u47, 0x31), w), broadcast(constants[upper])));
		const Vec zh = _mm256_permute2x128_si256(z, z, 0x11);
		const Vec lane0[8] = {z,  _mm256_srli_si256(z, 4),  _mm256_srli_si256(z, 8),  _mm256_srli_si256(z, 12),
							  zh, _mm256_srli_si256(zh, 4), _mm256_srli_si256(zh, 8), _mm256_srli_si256(zh, 12)};
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i)
			store(out.data() + 8 * (p + i), _mm256_blend_epi32(_mm256_permutevar8x32_epi32(y[i], rotate), lane0[i], 1));
	}
}

}

inline int power_log(std::size_t n) { return std::max(detail::log_derivative_log(n), exp_log(n)); }

inline std::size_t power_scratch(std::size_t n) {
	return 3 * Arena::footprint(detail::exp_length(n) / 2) +
		   std::max(detail::log_derivative_scratch(n), detail::exp_newton_scratch(n));
}

inline void power(const Transform& t, std::span<const std::uint32_t> f, std::uint32_t e, std::uint32_t c,
				  std::span<std::uint32_t> g, std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size(), m = exp_length(n) / 2, k = m / 2, stride = Arena::footprint(m);
	if (f.empty() || f[0] == 0) std::abort();
	const std::span<std::uint32_t> lo = scratch.first(m), rest = scratch.subspan(3 * stride);
	const auto q = [&](std::size_t i) __attribute__((target("avx2"))) { return i < m ? lo.data() + i : g.data() + i; };
	const Factor factor(e);

	const bool reuse = m >= 128 && log_block(n) == k && log_blocks(n) >= 3;
	const bool block2 = reuse && log_blocks(n) == 4 && n - m > k;
	const std::span<std::uint32_t> qt = scratch.subspan(stride, reuse ? m : 0);
	const std::span<std::uint32_t> q2t = scratch.subspan(2 * stride, block2 ? m : 0);
	const std::uint32_t* d0t = nullptr;
	lo[0] = 0;
	if (n >= 2)
		log_derivative(
			t, f, n, rest,
			[&](std::size_t first, std::span<const std::uint32_t> d) __attribute__((target("avx2"))) {
				std::size_t i = 0;
				for (; i + 8 <= d.size(); i += 8) {
					const std::size_t at = first + 1 + i;
					const Vec x = reduce(times(load(d.data() + i), factor), kP);
					if (at + 8 <= m || at >= m) {
						store_unaligned(q(at), x);
					} else {
						alignas(32) std::uint32_t words[8];
						store(words, x);
						for (std::size_t j = 0; j < 8; ++j) *q(at + j) = words[j];
					}
				}
				for (; i < d.size(); ++i) *q(first + 1 + i) = blocked_ntt::detail::multiply_mod(d[i], e);
			},
			[&](std::size_t j, std::span<const std::uint32_t> dt) __attribute__((target("avx2"))) {
				if (j == 0) d0t = dt.data();
				if (j == 1 && reuse) times_x<true>(t.roots(), d0t, dt.data(), e, kP - *q(m), 0, qt);
				if (j == 2 && block2) times_x<false>(t.roots(), dt.data(), nullptr, e, *q(m), kP - *q(m + k), q2t);
			});
	const std::span<std::uint32_t> start = g.first(std::min(n, kExpBase));
	exp_direct(lo, start);
	for (std::uint32_t& x : start) x = blocked_ntt::detail::multiply_mod(x, c);
	if (n > kExpBase) exp_newton(t, lo, g, rest, QTransforms{qt, q2t});
}

}
namespace blocked_poly {
namespace detail {
template <bool differentiated>
[[gnu::noinline]] Mint exp_tail_coefficient(const Mint* in, std::size_t input, const Mint* g, std::size_t n) {
	const std::size_t count = std::min(n, input ? input - 1 : 0), full = count / 8 * 8;
	const Vec reversed = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0), ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec total = _mm256_setzero_si256();
	Indices index(1);
	std::size_t j = 0;
	for (; j < full;) {
		Vec even = _mm256_setzero_si256(), odd = even;
		const std::size_t end = std::min(full, j + 64);
		for (; j < end; j += 8, index.next()) {
			Vec a, b;
			std::memcpy(&a, in + j + 1, sizeof(a));
			std::memcpy(&b, g + n - j - 8, sizeof(b));
			a = reduce(a, kP);
			if constexpr (!differentiated) a = canonical(montgomery(a, index.value()));
			b = _mm256_permutevar8x32_epi32(reduce(b, kP), reversed);
			even = _mm256_add_epi64(even, _mm256_mul_epu32(a, b));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
		}
		even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
		const Vec value = canonical(_mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA));
		total = low(add(total, value));
	}
	total = low(add(total, _mm256_permute2x128_si256(total, total, 1)));
	total = low(add(total, _mm256_shuffle_epi32(total, 0x4E)));
	total = low(add(total, _mm256_shuffle_epi32(total, 0xB1)));
	Mint result = Mint::word(std::uint32_t(_mm_cvtsi128_si32(_mm256_castsi256_si128(total))));
	for (; j < count; ++j) {
		Mint value = in[j + 1] * g[n - j - 1];
		if constexpr (!differentiated) value *= Mint(j + 1);
		result += value;
	}
	return result / Mint(n);
}

template <bool differentiated>
[[gnu::noinline]] void import_exp_input(const Mint* in, std::size_t first, std::size_t count, std::uint32_t* to) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	std::size_t i = 0;
	const Factor undo(blocked_ntt::detail::power(kR, kP - 2));
	auto index = add(broadcast(std::uint32_t(first)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	for (; i + 8 <= count; i += 8, index = add(index, broadcast(8))) {
		Vec value;
		std::memcpy(&value, in + first + i, sizeof(value));
		if constexpr (differentiated) store_unaligned(to + i, reduce(times(value, undo), kP));
		else store_unaligned(to + i, reduce(montgomery(value, index), kP));
	}
	for (; i < count; ++i) {
		if constexpr (differentiated) to[i] = in[first + i].val();
		else to[i] = blocked_ntt::detail::multiply_mod(in[first + i].val(), first + i);
	}
}
}

inline void native_exp_regular(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool differentiated) {
	const std::size_t m = detail::exp_length(n) / 2, size = std::min(input, n), below = std::min(size, m);
	const int lg = exp_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(exp_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(exp_scratch(n));
	auto lo = scratch.first(m);
	const auto import = [&](std::size_t first, std::size_t count, std::uint32_t* to) {
		if (differentiated) detail::import_exp_input<true>(in, first, count, to);
		else detail::import_exp_input<false>(in, first, count, to);
	};
	import(0, below, lo.data());
	std::fill(lo.begin() + below, lo.end(), 0u);
	if (size > m) import(m, size - m, g.data() + m);
	if (n > std::max(size, m)) std::fill(g.begin() + std::max(size, m), g.end(), 0u);
	detail::exp_direct(lo, g.first(std::min(n, detail::kExpBase)));
	for (auto &x : g.first(std::min(n, detail::kExpBase))) x = blocked_ntt::detail::multiply_mod(x, detail::kR);
	if (n > detail::kExpBase) detail::exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
	append_fps_words<true>(out, g.data(), n);
}

inline void native_exp(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool differentiated = false) {
	const std::size_t prefix = std::bit_floor(n), tail = n - prefix;
	if (prefix >= 4096 && tail && tail <= 4) {
		const std::size_t offset = out.size();
		prepare_fps_output(out, offset + n);
		native_exp_regular(in, input, out, prefix, differentiated);
		for (std::size_t at = prefix; at < n; ++at) {
			const Mint value = differentiated ? detail::exp_tail_coefficient<true>(in, input, out.data() + offset, at)
											 : detail::exp_tail_coefficient<false>(in, input, out.data() + offset, at);
			out.push_back(value);
		}
		return;
	}
	native_exp_regular(in, input, out, n, differentiated);
}

inline void native_power(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, std::uint32_t e, Mint c) {
	const int lg = power_log(n);
	const std::size_t size = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(power_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(power_scratch(n));
	for (std::size_t i = 0; i < size; ++i) {
		const auto x = in[i].v;
		g[i] = std::min(x, x - Mint::mod());
	}
	power(t, g.first(size), e, std::min(c.v, c.v - Mint::mod()), g, scratch);
	append_fps_words<true>(out, g.data(), n);
}
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_inverse_blocked.hpp"



#line 5 "components/fps_inverse_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {
inline void append_scaled(std::span<std::uint32_t> a, std::vector<Mint> &out, std::uint32_t scale) {
	const detail::Factor factor(scale);
	if (a.size() > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + a.size());
	alignas(32) std::uint32_t values[64];
	for (std::size_t i = 0; i < a.size(); i += 64) {
		const auto count = std::min<std::size_t>(64, a.size() - i);
		std::size_t j = 0;
		for (; j + 8 <= count; j += 8)
			detail::store(values + j, detail::reduce(detail::times(detail::load_unaligned(a.data() + i + j), factor), Mint::mod()));
		for (; j < count; ++j) values[j] = blocked_ntt::detail::multiply_mod(a[i + j], scale);
		append_fps_words<true>(out, values, count);
	}
}

inline void native_inverse(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n) {
	const int lg = inverse_log(n);
	const std::size_t len = std::size_t(1) << lg, k = len / 2;
	const std::size_t c = std::min(input, n), scratch_words = std::max(len, inverse_scratch(k));
	Arena arena(Transform::words(lg) + Arena::footprint(len) + Arena::footprint(scratch_words) + Arena::footprint(k));
	const Transform t(arena, lg);
	auto a = arena.take(len), scratch = arena.take(scratch_words), g = arena.take(k);
	for (std::size_t i = 0; i < c; ++i) {
		const unsigned v = in[i].v;
		a[i] = std::min(v, v - Mint::mod());
	}
	const std::span<const std::uint32_t> f = a.first(c);
	const auto extend = [&](std::span<const std::uint32_t> current, std::span<std::uint32_t> to,
							std::span<std::uint32_t> transformed, std::span<std::uint32_t> work) {
		const std::size_t count = current.size();
		t.forward(current, 0, transformed);
		t.cyclic_product(f.first(std::min(2 * count, f.size())), 0, work, transformed, Half::kUpper);
		t.cyclic_product(work.subspan(count), count, work, transformed, Half::kUpper, 53925088);
		if (to.data() != work.data() + count) std::copy_n(work.begin() + std::ptrdiff_t(count), to.size(), to.begin());
	};
	const std::size_t target = std::min(n, k), base = std::min(target, detail::kInverseBase);
	detail::inverse_direct(f, g.first(base));
	for (std::size_t i = 0; i < base; ++i) g[i] = blocked_ntt::detail::multiply_mod(g[i], 932051910);
	for (std::size_t count = base; count < target; count *= 2) {
		const std::size_t size = 2 * count;
		extend(g.first(count), g.subspan(count, std::min(count, target - count)), scratch.first(size),
				scratch.subspan(Arena::footprint(size), size));
	}
	if (n <= k) return append_fps_words<true>(out, g.data(), n);
	extend(g, a.subspan(k, n - k), scratch.first(len), a);
	prepare_fps_output(out, n);
	append_fps_words<true>(out, g.data(), k);
	append_fps_words<true>(out, a.data() + k, n - k);
}
}

#pragma GCC pop_options
#endif

#line 1 "components/fps_sqrt_blocked.hpp"


#line 5 "components/fps_sqrt_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

namespace detail {

inline constexpr std::size_t kSqrtBase = 64;

inline void sqrt_direct(std::span<const std::uint32_t> f, std::uint32_t c, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	const std::uint32_t inverse = blocked_ntt::detail::power(multiply_mod(2, c), kP - 2);
	g[0] = c;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = i < f.size() ? f[i] : 0;
		for (std::size_t j = 1; j < i; ++j) sum += kP - std::uint64_t(g[j]) * g[i - j] % kP;
		g[i] = multiply_mod(std::uint32_t(sum % kP), inverse);
	}
}

inline Vec load_padded(std::span<const std::uint32_t> f, std::size_t at) {
	if (at + 8 <= f.size()) return load_unaligned(f.data() + at);
	alignas(32) std::uint32_t x[8] = {};
	if (at < f.size()) std::copy(f.begin() + std::ptrdiff_t(at), f.end(), x);
	return load(x);
}

inline Vec first_lanes(std::size_t count) {
	return _mm256_cmpgt_epi32(broadcast(std::uint32_t(std::min<std::size_t>(count, 8))),
							_mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
}
inline void store_first(std::uint32_t* p, Vec x, std::size_t count) {
	if (count >= 8) return store_unaligned(p, x);
	_mm256_maskstore_epi32(reinterpret_cast<int*>(p), first_lanes(count), x);
}
inline Vec load_first(const std::uint32_t* p, std::size_t count) {
	if (count >= 8) return load_unaligned(p);
	return _mm256_maskload_epi32(reinterpret_cast<const int*>(p), first_lanes(count));
}

[[gnu::always_inline]] inline Vec leaf_square(const Window& window) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2;
	#define CP_FPS_ASM_BLOCK(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r) \
h(m,b) c(n,b,d) c(m,b,a) h(n,b)\
c(l,b,e)\
h(40(%[w]),b)\
c(l,b,g)\
c(24(%[w]),b,i)\
f(e,a,a)\
h(44(%[w]),b)\
c(k,b,e)\
f(g,d,d)\
f(i,a,a)\
h(48(%[w]),b)\
c(k,b,g)\
c(16(%[w]),b,i)\
f(e,a,a)\
h(52(%[w]),b)\
c(j,b,e)\
f(g,d,d)\
f(i,a,a)\
h(56(%[w]),b)\
c(j,b,g)\
c(8(%[w]),b,i)\
f(e,a,a)\
h(60(%[w]),b)\
c(4(%[w]),b,e)\
f(g,d,d)\
f(i,a,a)\
f(e,a,a)\
f(d,d,d)\
c(%[ni],a,e)\
c(%[ni],d,g)\
c(%[p],e,e)\
c(%[p],g,g)\
f(e,a,a)\
f(g,d,d)\
q($32,a,a)\
o($0xaa,d,a,a)\
r(%[p2],a,e)\
"vpminud %[t0], %[even], %[even]"
	asm(
		CP_FPS_ASM_BLOCK(%[even],%[bi],CP_FPS_ASM_VPMULUDQ,%[odd],%[t0],CP_FPS_ASM_VPADDQ,%[t1],CP_FPS_ASM_VPBROADCASTD,%[t2],12(%[w]),20(%[w]),28(%[w]),32(%[w]),36(%[w]),CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VPSUBD)
#undef CP_FPS_ASM_BLOCK
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2)
		: [w] "r"(window.word), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)));
	return even;
}

struct InverseSquareBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t* roots;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* a;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[4]) const {
		const std::uint32_t* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			Vec f[4];
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) f[t] = leaf_square(window[j & 1][t]);
			inverse_h1(f, Group(inverse_roots, first + j));
			for (int t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline bool radix4(std::size_t nv) { return std::countr_zero(nv) % 2 == 0; }

template <class Bottom>
void subtrees(const Recursion<Bottom>& recursion, std::uint32_t* a, std::size_t nv) {
	if (!radix4(nv)) {
		recursion.visit(a, nv / 2, 0);
		return recursion.visit(a + 4 * nv, nv / 2, 1);
	}
	for (std::size_t q = 0; q < 4; ++q) recursion.visit(a + 2 * q * nv, nv / 4, q);
}

template <class Bottom>
void from_lower_source(const Transform& t, const Recursion<Bottom>& recursion, std::uint32_t* a, std::size_t nv,
					bool written = true) {
	const Source in(a, 4 * nv, 0);
	if (!radix4(nv)) return radix2_halves(recursion, in, a, nv, t.roots());
	if (!written) forward_top4(in, reinterpret_cast<Vec*>(a), nv / 4, t.roots());
	subtrees(recursion, a, nv);
}

inline void inverse_top(const Transform& t, Vec* v, std::size_t nv, Half output, std::uint32_t scale) {
	if (radix4(nv)) return inverse_top4(v, nv / 4, output, t.inverse_roots(), Factor(scale));
	inverse_top2(v, nv / 2, output, scale);
}

template <Half kOutput, class Visit>
[[gnu::always_inline]] inline void inverse_columns(const Transform& t, Vec* v, std::size_t nv, std::uint32_t scale,
												Visit visit) {
	constexpr bool kLower = kOutput != Half::kUpper, kUpper = kOutput != Half::kLower;
	const Factor s(scale);
	if (radix4(nv)) {
		const std::size_t h = nv / 4;
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec p0 = v[j], p1 = v[j + h], p2 = v[j + 2 * h], p3 = v[j + 3 * h];
			const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
			const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
			Vec lo[2] = {}, hi[2] = {};
			if constexpr (kLower) lo[0] = times(add(ab, cd), s), lo[1] = times(add(amb, cmd), s);
			if constexpr (kUpper) hi[0] = times(diff(ab, cd), s), hi[1] = times(diff(amb, cmd), s);
			visit(std::true_type{}, j, h, lo, hi);
		}
		return;
	}
	const std::size_t h = nv / 2;
	for (std::size_t j = 0; j < h; ++j) {
		Vec lo[1] = {}, hi[1] = {};
		if constexpr (kLower) lo[0] = times(add(v[j], v[j + h]), s);
		if constexpr (kUpper) hi[0] = times(diff(v[j], v[j + h]), s);
		visit(std::false_type{}, j, h, lo, hi);
	}
}

[[gnu::always_inline]] inline void forward_lower4(Vec* v, std::size_t j, std::size_t h, Vec a, Vec b, const Factor& z) {
	const Vec zb = times(b, z);
	v[j] = add(a, b), v[j + h] = diff(a, b), v[j + 2 * h] = add(a, zb), v[j + 3 * h] = diff(a, zb);
}

[[gnu::always_inline]] inline void forward_upper4(Vec* v, std::size_t j, std::size_t h, Vec c, Vec d, const Factor& z) {
	const Vec mc = _mm256_sub_epi32(broadcast(kP), c), zmd = times(_mm256_sub_epi32(broadcast(2 * kP), d), z);
	v[j] = add(c, d), v[j + h] = diff(c, d), v[j + 2 * h] = add(mc, zmd), v[j + 3 * h] = diff(mc, zmd);
}

inline Factor top_root(const Transform& t) { return Factor(t.roots()[1], t.roots()[9]); }

inline Vec* vectors(std::span<std::uint32_t> a) { return reinterpret_cast<Vec*>(a.data()); }

inline std::uint32_t product_scale(std::size_t n) { return kInverseScales[1][std::countr_zero(n)]; }

inline Vec half_residual(std::span<const std::uint32_t> f, std::size_t m, std::size_t at, Vec s) {
	const Factor half((kP + 1) / 2);
	const Vec fs = add(load_padded(f, at), load_padded(f, m + at));
	return reduce(times(_mm256_sub_epi32(add(s, broadcast(2 * kP)), fs), half), kP);
}

inline void half_residual(std::span<const std::uint32_t> f, std::size_t m, std::size_t first, const std::uint32_t* s,
						std::uint32_t* out, std::size_t count) {
	for (std::size_t i = 0; i < count; i += 8) store_first(out + i, half_residual(f, m, first + i, load(s + i)), count - i);
}

inline void sqrt_step(const Transform& t, std::span<const std::uint32_t> f, std::size_t m, std::uint32_t* g,
					std::uint32_t* h, std::span<std::uint32_t> b, std::span<std::uint32_t> ht) {
	const std::size_t half = m / 2, nv = m / 8;
	const std::span<std::uint32_t> gt = b.first(m), w = b.subspan(m, m), work = b.first(2 * m);
	const Factor z = top_root(t);
	const ProductBottom with_h{t.roots(), t.inverse_roots(), ht.data()};
	const std::uint32_t scale = product_scale(m), scale2 = product_scale(2 * m);
	t.forward(std::span<const std::uint32_t>(g, m), 0, gt);
	Vec* v = vectors(w);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseProductBottom{with_h, gt.data()}), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec*, const Vec* e) {
		if constexpr (radix4) forward_upper4(v, j, q, reduce(e[0], kP), e[1], z);
		else v[j] = e[0], v[j + q] = _mm256_sub_epi32(broadcast(2 * kP), e[0]);
	});
	subtrees(Recursion(t.roots(), t.inverse_roots(), with_h), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec*, const Vec* x) {
		store_unaligned(h + half + 8 * j, reduce(x[0], kP));
		if constexpr (radix4) store_unaligned(h + half + 8 * (j + q), reduce(x[1], kP));
	});
	t.forward(std::span<const std::uint32_t>(h, m), 0, ht);

	v = vectors(gt);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}), gt.data(),
			nv);
	inverse_columns<Half::kBoth>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec* lo, const Vec* hi) {
		if constexpr (radix4) {
			v[j] = half_residual(f, m, 8 * j, lo[0]), v[j + q] = half_residual(f, m, 8 * (j + q), lo[1]);
			v[j + 2 * q] = half_residual(f, m, half + 8 * j, hi[0]);
			v[j + 3 * q] = half_residual(f, m, half + 8 * (j + q), hi[1]);
		} else {
			forward_lower4(v, j, q, half_residual(f, m, 8 * j, lo[0]), half_residual(f, m, half + 8 * j, hi[0]), z);
		}
	});
	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_h), work.data(), 2 * nv);
	inverse_columns<Half::kLower>(t, vectors(work), 2 * nv, scale2,
								[&](auto radix4, std::size_t j, std::size_t q, const Vec* d, const Vec*) {
									store_unaligned(g + m + 8 * j, reduce(d[0], kP));
									if constexpr (radix4) store_unaligned(g + m + 8 * (j + q), reduce(d[1], kP));
								});
}

}

inline int sqrt_log(std::size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)) - 1);
}

inline std::size_t sqrt_scratch(std::size_t n) {
	if (n <= detail::kSqrtBase) return 0;
	return 3 * Arena::footprint(std::size_t(1) << sqrt_log(n));
}

inline void sqrt_steps(const Transform& t, std::span<const std::uint32_t> f, std::uint32_t c, std::span<std::uint32_t> a,
					std::span<std::uint32_t> b, std::span<std::uint32_t> ht) {
	using namespace detail;
	const std::size_t len = a.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0] || len < kSqrtBase) std::abort();
	sqrt_direct(f, c, a.first(kSqrtBase));
	std::uint32_t* const h = a.data() + len / 2;
	alignas(32) std::uint32_t h0[kSqrtBase / 2];
	inverse_direct(a.first(kSqrtBase / 2), h0);
	for (std::uint32_t& x : h0) x = x ? kP - x : 0;
	t.forward(h0, 0, ht.first(kSqrtBase));
	if (len > kSqrtBase) std::copy_n(h0, kSqrtBase / 2, h);
	for (std::size_t m = kSqrtBase; m < len; m *= 2) sqrt_step(t, f, m, a.data(), h, b.first(2 * m), ht.first(2 * m));
}

inline void sqrt_last_step(const Transform& t, std::span<const std::uint32_t> f, std::span<const std::uint32_t> gt,
						std::span<const std::uint32_t> ht, std::span<std::uint32_t> work, std::span<std::uint32_t> hi) {
	using namespace detail;
	const std::size_t m = gt.size(), half = m / 2, rest = hi.size(), nv = m / 8;
	if (rest == 0 || rest > m) std::abort();
	const std::uint32_t scale = product_scale(m);
	const Factor z = top_root(t);
	const ProductBottom with_g{t.roots(), t.inverse_roots(), gt.data()}, with_h{t.roots(), t.inverse_roots(), ht.data()};
	const Recursion product_h(t.roots(), t.inverse_roots(), with_h);
	Vec* v = vectors(work);

	const auto source = [&](auto radix4, std::size_t j, std::size_t q, auto x) {
		if constexpr (radix4) forward_lower4(v, j, q, x(0, j), x(1, j + q), z);
		else v[j] = x(0, j);
	};

	const auto each = [](auto radix4, std::size_t j, std::size_t q, auto put) {
		put(0, j);
		if constexpr (radix4) put(1, j + q);
	};
	const auto put = [&](std::uint32_t* out, std::size_t count, std::size_t p, Vec x) __attribute__((target("avx2"))) {
		if (8 * p < count) store_first(out + 8 * p, x, count - 8 * p);
	};

	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}),
			work.data(), nv);
	const bool split = rest > half;
	inverse_top(t, v, nv, split ? Half::kBoth : Half::kLower, scale);
	half_residual(f, m, 0, work.data(), work.data(), std::min(rest, half));
	const std::size_t count = split ? rest - half : 0;
	std::uint32_t* const upper = hi.data() + (rest - count);
	half_residual(f, m, half, work.data() + half, upper, count);
	from_lower_source(t, product_h, work.data(), nv, false);
	if (!split) {
		inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec* d, const Vec*) {
			each(radix4, j, q, [&](int k, std::size_t p) { put(hi.data(), rest, p, reduce(d[k], kP)); });
		});
		return;
	}

	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec* d, const Vec*) {
		source(radix4, j, q, [&](int k, std::size_t p) {
			const Vec d0 = reduce(d[k], kP);
			store_unaligned(hi.data() + 8 * p, d0);
			return d0;
		});
	});

	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_g), work.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec*, const Vec* x) {
		source(radix4, j, q, [&](int k, std::size_t p) {
			const std::size_t at = 8 * p;
			return canonical(add(x[k], load_first(upper + std::min(at, count), at < count ? count - at : 0)));
		});
	});
	from_lower_source(t, product_h, work.data(), nv);
	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, std::size_t j, std::size_t q, const Vec* d, const Vec*) {
		each(radix4, j, q, [&](int k, std::size_t p) { put(upper, count, p, reduce(d[k], kP)); });
	});
}

inline void sqrt(const Transform& t, std::span<const std::uint32_t> f, std::uint32_t c, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0]) std::abort();
	if (n <= kSqrtBase) return sqrt_direct(f, c, g);
	const std::size_t m = std::size_t(1) << sqrt_log(n), words = Arena::footprint(m);
	const std::span<std::uint32_t> b = scratch.first(m), ht = scratch.subspan(words, m), work = scratch.subspan(2 * words, m);
	sqrt_steps(t, f, c, g.first(m), b, ht);
	t.forward(g.first(m), 0, b);
	sqrt_last_step(t, f, b, ht, work, g.subspan(m));
}

}
namespace fps_detail::blocked_poly {
inline void native_sqrt(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, Mint constant) {
	const int lg = sqrt_log(n);
	const std::size_t c = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(c) + Arena::footprint(n) + Arena::footprint(sqrt_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(c), g = arena.take(n), scratch = arena.take(sqrt_scratch(n));
	for (std::size_t i = 0; i < c; ++i) {
		const unsigned v = in[i].v;
		f[i] = std::min(v, v - Mint::mod());
	}
	const std::uint32_t ct = blocked_ntt::detail::multiply_mod(std::min(constant.v, constant.v - Mint::mod()),
		blocked_ntt::detail::power(65536, Mint::mod() - 2));
	sqrt(t, f, ct, g, scratch);
	append_scaled(g, out, 65536);
}
}

#pragma GCC pop_options
#endif

#line 1 "components/fps_convolution.hpp"


#line 1 "components/fps_convolution_unbalanced.hpp"



#line 10 "components/fps_convolution_unbalanced.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::unbalanced {
using namespace blocked_poly;
inline bool fits(std::size_t n, std::size_t m) {
	const auto shorter = std::min(n, m), longer = std::max(n, m);
	return shorter > 32 && longer >= 32768 && shorter <= longer / 32;
}
inline std::vector<Mint> multiply(std::span<const Mint> a, std::span<const Mint> b) {
	const std::size_t len = 8 * std::bit_ceil(std::min(a.size(), b.size()));
	const auto& x = a.size() >= b.size() ? a : b;
	const auto& y = a.size() >= b.size() ? b : a;
	const std::size_t n = x.size(), m = y.size(), stride = len - m + 1;
	const int lg = std::countr_zero(len);
	Arena arena(Transform::words(lg) + 3 * Arena::footprint(len));
	const Transform t(arena, lg);
	auto bt = arena.take(len), in = arena.take(len), out = arena.take(len);
	for (std::size_t i = 0; i < m; ++i) in[i] = y[i].val();
	t.forward(in.first(m), 0, bt);
	std::vector<Mint> result;
	fps_detail::prepare_fps_output(result, n + m - 1);
	for (std::size_t p = 0; p < n; p += stride) {
		const std::size_t count = std::min(stride, n - p);
		for (std::size_t i = 0; i < count; ++i) {
			const unsigned v = x[p + i].v;
			in[i] = std::min(v, v - Mint::mod());
		}
		t.cyclic_product(in.first(count), 0, out, bt);
		const std::size_t overlap = p == 0 ? 0 : m - 1;
		for (std::size_t i = 0; i < overlap; ++i) {
			const unsigned v = result[p + i].v + out[i];
			result[p + i] = Mint::word(std::min(v, v - Mint::mod()));
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
inline unsigned canonical(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline bool same_eight(const Mint* p, unsigned value) {
	__m256i v;
	std::memcpy(&v, p, sizeof(v));
	v = _mm256_min_epu32(v, _mm256_sub_epi32(v, _mm256_set1_epi32(Mint::mod())));
	return _mm256_movemask_epi8(_mm256_cmpeq_epi32(v, _mm256_set1_epi32(value))) == -1;
}
inline bool remaining(std::span<const Mint> a, unsigned value) {
	std::size_t i = 8;
	for (; i + 8 <= a.size(); i += 8) if (!same_eight(a.data() + i, value)) return false;
	for (; i < a.size(); ++i) if (canonical(a[i]) != value) return false;
	return true;
}
[[gnu::noinline]] inline bool fits(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	if (std::min(a.size(), b.size()) < 4096) return false;
	const unsigned av = canonical(a[0]), bv = canonical(b[0]);
	if (!same_eight(a.data(), av) || !same_eight(b.data(), bv)) return false;
	return remaining(a, av) && (&a == &b || remaining(b, bv));
}
[[gnu::noinline]] inline std::vector<Mint> multiply(std::span<const Mint> a, std::span<const Mint> b) {
	const std::size_t n = std::max(a.size(), b.size()), m = std::min(a.size(), b.size()), size = n + m - 1;
	const unsigned c = canonical(Mint::word(canonical(a[0])) * Mint::word(canonical(b[0])));
	std::vector<Mint> result(size);
	if (c == 0) return result;
	alignas(32) unsigned first[8];
	for (unsigned i = 0; i < 8; ++i) first[i] = std::uint64_t(c) * (i + 1) % Mint::mod();
	__m256i value = _mm256_load_si256(reinterpret_cast<const __m256i*>(first));
	const auto step = _mm256_set1_epi32(std::uint64_t(c) * 8 % Mint::mod()), p = _mm256_set1_epi32(Mint::mod());
	const auto reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	std::size_t i = 0;
	for (; i + 8 <= m; i += 8) {
		std::memcpy(static_cast<void*>(result.data() + i), &value, sizeof(value));
		const auto reflected = _mm256_permutevar8x32_epi32(value, reverse);
		std::memcpy(static_cast<void*>(result.data() + size - i - 8), &reflected, sizeof(reflected));
		value = _mm256_add_epi32(value, step);
		value = _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
	}
	unsigned current = _mm256_extract_epi32(value, 0);
	for (; i < m; ++i) {
		result[i] = result[size - i - 1] = Mint::word(current);
		current += c;
		current = std::min(current, current - Mint::mod());
	}
	if (n > m + 1) std::fill(result.begin() + m, result.begin() + n - 1, Mint::word(std::uint64_t(c) * m % Mint::mod()));
	return result;
}
}
#pragma GCC pop_options
#endif

#line 15 "components/fps_convolution.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")

namespace fps_detail::multiply_detail {
namespace ntt::kernels {
using namespace blocked_ntt::kernels;
inline void forward_pair(Vec* a, Vec* b, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) \
R(%[P],z) R(%[P2],r) D((%[x]),c) m(c,G)\
D(32(%[x]),w)\
D((%[y]),c)\
m(c,L)\
D(32(%[y]),E)\
D(4(%[y]),c)\
m(c,M)\
D(36(%[y]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(H,c)\
m(J,d)\
m(N,f)\
m(O,g)\
q(H,w,i)\
q(J,w,t)\
m(I,l)\
q(4(%[a],%[h3]),w,e)\
q(4(%[b],%[h3]),w,j)\
m(K,n)\
m((%[a]),v)\
u(s,i,i)\
u(s,t,t)\
B(A,e,i,e)\
B(A,j,t,j)\
m((%[b]),t)\
q(I,w,i)\
p(z,e,e)\
p(z,j,j)\
p(G,c,c)\
p(G,d,d)\
u(s,i,i)\
p(G,l,l)\
k(e,c,e)\
k(j,d,j)\
p(G,n,n)\
k(r,f,d)\
C(d,f,d)\
k(r,g,f)\
q(K,w,c)\
C(f,g,f)\
q(4(%[a],%[h],2),w,g)\
u(s,c,c)\
B(A,g,i,g)\
q(4(%[b],%[h],2),w,i)\
p(z,g,g)\
B(A,i,c,i)\
o(r,d,c)\
o(e,d,d)\
k(e,c,e)\
p(z,i,i)\
k(g,l,g)\
o(r,f,l)\
o(j,f,f)\
k(j,l,j)\
u(s,d,l)\
q(E,d,c)\
q(E,l,l)\
p(L,d,d)\
k(i,n,i)\
u(s,e,n)\
q(F,n,n)\
u(s,c,c)\
B(A,l,c,l)\
q(F,e,c)\
p(M,e,e)\
p(z,l,l)\
u(s,c,c)\
B(A,n,c,n)\
k(l,d,l)\
u(s,f,d)\
q(E,f,c)\
q(E,d,d)\
p(z,n,n)\
p(L,f,f)\
u(s,c,c)\
B(A,d,c,d)\
k(n,e,n)\
u(s,j,e)\
q(F,j,c)\
q(F,e,e)\
p(z,d,d)\
p(M,j,j)\
u(s,c,c)\
B(A,e,c,e)\
k(d,f,d)\
k(r,v,f)\
k(r,t,c)\
C(f,v,f)\
C(c,t,c)\
p(z,e,e)\
o(g,f,t)\
k(g,f,g)\
o(i,c,f)\
k(i,c,i)\
k(r,t,c)\
o(r,g,v)\
C(c,t,c)\
C(v,g,v)\
k(e,j,e)\
k(r,f,j)\
o(r,i,g)\
C(j,f,j)\
o(r,c,f)\
o(r,v,t)\
C(g,i,g)\
o(l,c,c)\
k(l,f,l)\
o(n,v,v)\
o(r,j,f)\
o(r,g,i)\
k(n,t,n)\
o(d,j,j)\
m(c,(%[a]))\
k(d,f,d)\
o(e,g,g)\
k(e,i,e)\
m(l,N)\
m(v,I)\
m(n,H)\
m(j,(%[b]))\
m(d,O)\
m(g,K)\
m(e,J)\
Q(s,%[b])\
Q(s,%[a])\
S(%[end],%[a])\
T(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm1,%%ymm7,%%ymm2,%%ymm3,%%ymm4,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm6,CP_FPS_ASM_VMOVDQA,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,%%ymm5,CP_FPS_ASM_VPSRLQ,%%ymm10,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h],2),(%[b],%[h3]),(%[b],%[h],2),32(%[w]),64(%[w]),(%[a],%[h]),(%[b],%[h]),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void bottom_first(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R) \
H(%[P],p) H(%[P2],s) H(%[NI],%%ymm13) x(32(%[a]),k)\
m(D,A)\
m(D,e)\
m(E,h)\
x((%[a]),f)\
m(D,v)\
g(s,k,l)\
m(D,c)\
r(N,A,A)\
o(l,k,l)\
r(68(%[a]),e,e)\
q(N,h,h)\
g(s,f,k)\
m(E,i)\
r(Q,v,v)\
o(k,f,k)\
u(t,A,A)\
C(B,e,A,e)\
u(t,v,v)\
q(p,e,e)\
r(100(%[a]),c,c)\
g(e,h,e)\
z(s,l,h)\
z(e,k,A)\
g(e,k,e)\
m(F,k)\
q(Q,i,i)\
g(s,A,f)\
z(s,e,j)\
m(F,d)\
C(B,c,v,c)\
o(f,A,f)\
o(j,e,j)\
q(p,c,c)\
m((%[y]),e)\
g(c,i,c)\
m(G,i)\
z(c,l,l)\
g(c,h,c)\
m(G,h)\
u(t,l,A)\
u(t,c,v)\
m(M,n)\
r(k,l,k)\
r(d,A,d)\
q(e,l,e)\
u(t,k,k)\
C(B,d,k,d)\
z(s,f,k)\
q(p,d,d)\
r(i,c,i)\
z(s,j,l)\
r(h,v,h)\
q(n,c,n)\
g(d,e,d)\
u(t,i,i)\
z(d,f,f)\
g(d,k,d)\
C(B,h,i,h)\
m(I,i)\
g(s,f,k)\
q(p,h,h)\
m(I,e)\
o(k,f,k)\
m((%[w]),f)\
g(p,k,c)\
o(c,k,c)\
g(h,n,h)\
u(t,c,n)\
x(c,32(%[buf]))\
z(h,j,j)\
g(h,l,h)\
g(s,d,l)\
o(l,d,l)\
g(p,l,d)\
r(i,c,i)\
o(d,l,d)\
r(e,n,e)\
m(J,n)\
u(t,d,l)\
q(f,c,f)\
x(d,96(%[buf]))\
m(J,c)\
u(t,i,i)\
m(4(%[w]),k)\
C(B,e,i,e)\
q(p,e,e)\
g(e,f,e)\
g(p,e,f)\
r(n,d,n)\
o(f,e,f)\
g(s,j,e)\
r(c,l,c)\
x(f,(%[buf]))\
o(e,j,e)\
q(k,d,k)\
m(K,d)\
g(p,e,j)\
u(t,n,n)\
m(K,f)\
o(j,e,j)\
C(B,c,n,c)\
m(8(%[w]),n)\
u(t,j,e)\
q(p,c,c)\
x(j,160(%[buf]))\
g(c,k,c)\
g(p,c,k)\
o(k,c,k)\
g(s,h,c)\
r(d,j,d)\
x(k,64(%[buf]))\
o(c,h,c)\
r(f,e,f)\
g(p,c,e)\
q(n,j,n)\
m(L,j)\
o(e,c,e)\
u(t,d,d)\
m(L,c)\
u(t,e,h)\
C(B,f,d,f)\
x(e,224(%[buf]))\
m(12(%[w]),d)\
q(p,f,f)\
g(f,n,f)\
g(p,f,n)\
o(n,f,n)\
x((%[b]),f)\
r(j,e,j)\
x(n,128(%[buf]))\
x(32(%[b]),n)\
r(c,h,c)\
q(d,e,d)\
m(D,e)\
u(t,j,j)\
m(D,h)\
C(B,c,j,c)\
q(p,c,c)\
g(s,f,j)\
g(c,d,c)\
m(E,d)\
g(s,n,k)\
o(j,f,j)\
g(p,c,f)\
m(D,l)\
o(k,n,k)\
o(f,c,f)\
m(D,c)\
r(O,e,e)\
x(f,192(%[buf]))\
m(E,f)\
r(68(%[b]),h,h)\
u(t,e,e)\
C(B,h,e,h)\
q(p,h,h)\
q(O,d,d)\
r(R,l,l)\
r(100(%[b]),c,c)\
q(R,f,f)\
g(h,d,h)\
u(t,l,l)\
z(s,k,d)\
z(h,j,e)\
g(h,j,h)\
m(F,j)\
C(B,c,l,c)\
g(s,e,l)\
z(s,h,n)\
m(F,i)\
q(p,c,c)\
o(l,e,l)\
o(n,h,n)\
m((%[y]),h)\
g(c,f,c)\
m(G,f)\
z(c,k,k)\
g(c,d,c)\
m(G,d)\
u(t,k,e)\
u(t,c,v)\
m(M,A)\
r(j,k,j)\
r(i,e,i)\
q(h,k,h)\
u(t,j,j)\
C(B,i,j,i)\
z(s,l,j)\
r(f,c,f)\
q(p,i,i)\
r(d,v,d)\
z(s,n,v)\
q(A,c,A)\
u(t,f,f)\
g(i,h,i)\
C(B,d,f,d)\
z(i,l,l)\
g(i,j,i)\
q(p,d,d)\
g(s,l,j)\
g(s,i,f)\
o(j,l,j)\
o(f,i,f)\
g(p,j,i)\
g(p,f,l)\
g(d,A,d)\
o(i,j,i)\
o(l,f,l)\
z(d,n,n)\
g(d,v,d)\
x(i,256(%[buf]))\
g(s,n,i)\
g(s,d,v)\
x(l,288(%[buf]))\
o(i,n,i)\
o(v,d,v)\
g(p,i,d)\
g(p,v,n)\
o(d,i,d)\
o(n,v,n)\
x(d,320(%[buf]))\
x(n,352(%[buf]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm7,%%ymm10,%%ymm2,%%ymm4,CP_FPS_ASM_VPSUBD,%%ymm3,%%ymm8,%%ymm9,%%ymm0,%%ymm6,CP_FPS_ASM_VBROADCASTSS,%%ymm11,CP_FPS_ASM_VPMINUD,%%ymm15,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,CP_FPS_ASM_VPSRLQ,%%ymm5,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,%%ymm1,$170,CP_FPS_ASM_VPBLENDD,32(%[x]),(%[x]),32(%[y]),36(%[y]),CP_FPS_ASM_VPBROADCASTD,16(%[w]),20(%[w]),24(%[w]),28(%[w]),4(%[y]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

inline void bottom_last(void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) \
E(%[P],t) E(%[P2],s) E(%[NI],w) c(G,n)\
p(28(%[cur]),f)\
c(I,i)\
y(32(%[cur]),k)\
c(H,h)\
p(36(%[cur]),l)\
c(I,q)\
p(24(%[cur]),b)\
c(G,j)\
c(H,e)\
c(J,d)\
a(n,k,n)\
a(i,f,i)\
a(h,f,h)\
p(20(%[cur]),f)\
a(q,b,q)\
c(J,o)\
a(j,l,j)\
a(e,k,e)\
c(K,k)\
g(h,n,h)\
a(d,b,d)\
p(16(%[cur]),b)\
c(K,n)\
c(L,l)\
g(q,h,q)\
g(e,j,e)\
p(12(%[cur]),j)\
g(i,e,i)\
c(L,e)\
g(d,i,d)\
c(M,i)\
a(o,f,o)\
a(k,f,k)\
p(8(%[cur]),f)\
c(M,h)\
a(n,b,n)\
g(o,q,o)\
a(l,b,l)\
c(N,b)\
g(k,d,k)\
c(N,d)\
p(4(%[cur]),q)\
a(e,j,e)\
a(i,j,i)\
g(n,o,n)\
g(l,k,l)\
g(e,n,e)\
g(i,l,i)\
a(h,f,h)\
a(b,f,b)\
a(d,q,d)\
g(h,e,h)\
g(b,i,b)\
p(100(%[cur]),i)\
g(d,h,d)\
a(w,b,h)\
c(O,e)\
y(96(%[cur]),q)\
c(O,f)\
a(w,d,l)\
c(Q,n)\
a(t,h,h)\
p(92(%[cur]),k)\
c(Q,o)\
a(t,l,l)\
g(h,b,h)\
c(R,b)\
g(l,d,l)\
a(e,i,e)\
v(u,l,l)\
p(88(%[cur]),i)\
a(f,q,f)\
A(x,h,l,h)\
c(R,l)\
c(S,d)\
a(n,q,n)\
r(s,h,q)\
B(q,h,q)\
c(S,h)\
p(84(%[cur]),j)\
a(o,k,o)\
c(T,m)\
g(n,e,n)\
a(b,k,b)\
p(80(%[cur]),k)\
g(o,f,o)\
c(T,f)\
c(U,e)\
g(b,n,b)\
a(l,i,l)\
a(d,i,d)\
a(h,j,h)\
c(U,i)\
p(76(%[cur]),n)\
a(m,j,m)\
c(V,j)\
g(d,b,d)\
g(l,o,l)\
g(h,l,h)\
c(V,l)\
p(72(%[cur]),o)\
a(f,k,f)\
g(m,d,m)\
c(W,d)\
a(e,k,e)\
p(68(%[cur]),k)\
c(W,b)\
g(f,h,f)\
a(i,n,i)\
g(e,m,e)\
a(j,n,j)\
a(l,o,l)\
g(i,f,i)\
a(d,o,d)\
g(j,e,j)\
a(b,k,b)\
g(l,i,l)\
g(d,j,d)\
p(164(%[cur]),j)\
a(w,d,i)\
c(Y,k)\
g(b,l,b)\
y(160(%[cur]),l)\
a(w,b,e)\
c(Z,o)\
c(Y,f)\
a(t,i,i)\
p(156(%[cur]),n)\
a(t,e,e)\
c(Z,m)\
g(i,d,i)\
c(_0,d)\
g(e,b,e)\
a(k,j,k)\
v(u,e,e)\
c(_0,j)\
p(152(%[cur]),b)\
a(f,l,f)\
a(o,l,o)\
A(x,i,e,i)\
c(_1,e)\
r(s,i,l)\
a(m,n,m)\
B(l,i,l)\
p(148(%[cur]),i)\
g(o,k,o)\
c(_1,k)\
c(_2,h)\
a(d,n,d)\
p(144(%[cur]),n)\
g(m,f,m)\
c(_2,f)\
a(j,b,j)\
g(d,o,d)\
c(_3,o)\
a(e,b,e)\
p(140(%[cur]),b)\
g(j,m,j)\
c(_3,m)\
a(k,i,k)\
a(h,i,h)\
c(_4,i)\
g(e,d,e)\
p(136(%[cur]),d)\
a(f,n,f)\
g(k,j,k)\
g(h,e,h)\
c(_4,e)\
a(o,n,o)\
c(_5,n)\
g(f,k,f)\
p(132(%[cur]),k)\
c(_5,j)\
a(m,b,m)\
g(o,h,o)\
a(i,b,i)\
g(m,f,m)\
a(e,d,e)\
g(i,o,i)\
a(n,d,n)\
a(j,k,j)\
g(e,m,e)\
g(n,i,n)\
p(228(%[cur]),i)\
a(w,n,m)\
c(_6,k)\
g(j,e,j)\
c(_6,e)\
y(224(%[cur]),d)\
c(_7,o)\
a(w,j,f)\
a(t,m,m)\
p(220(%[cur]),b)\
c(_7,h)\
a(t,f,f)\
g(m,n,m)\
c(_9,n)\
a(k,i,k)\
g(f,j,f)\
a(e,d,e)\
v(u,f,f)\
p(216(%[cur]),j)\
a(o,d,o)\
A(x,m,f,m)\
c(_9,f)\
c(_10,d)\
r(s,m,i)\
a(h,b,h)\
B(i,m,i)\
p(212(%[cur]),m)\
g(o,k,o)\
c(_10,k)\
a(n,b,n)\
c(_11,b)\
g(h,e,h)\
p(208(%[cur]),e)\
g(n,o,n)\
c(_11,o)\
a(f,j,f)\
a(d,j,d)\
c(_12,j)\
a(k,m,k)\
g(f,h,f)\
c(_12,h)\
a(b,m,b)\
g(d,n,d)\
p(204(%[cur]),n)\
p(200(%[cur]),m)\
g(k,f,k)\
c(_13,f)\
a(o,e,o)\
g(b,d,b)\
c(_13,d)\
a(j,e,j)\
c(_14,e)\
g(o,k,o)\
c(_14,k)\
a(h,n,h)\
g(j,b,j)\
p(196(%[cur]),b)\
a(f,m,f)\
a(d,n,d)\
g(h,o,h)\
a(e,m,e)\
g(f,h,f)\
g(d,j,d)\
a(k,b,k)\
g(e,d,e)\
a(w,e,d)\
z(l,q,b)\
r(s,b,j)\
g(k,f,k)\
B(j,b,j)\
a(w,k,b)\
a(t,d,d)\
z(s,q,q)\
r(l,q,l)\
a(t,b,b)\
c(X,q)\
g(d,e,d)\
v(u,l,e)\
c(X,f)\
g(b,k,b)\
c((%[iy]),k)\
v(u,b,b)\
A(x,d,b,d)\
r(s,d,b)\
z(s,i,h)\
B(b,d,b)\
a(q,l,q)\
r(b,h,h)\
z(b,i,b)\
c(_8,i)\
c(_8,d)\
a(f,e,f)\
v(u,h,e)\
r(s,b,m)\
B(m,b,m)\
c(4(%[iy]),b)\
v(u,q,q)\
A(x,f,q,f)\
C(k,l,k)\
z(m,j,l)\
C(t,f,f)\
r(s,l,q)\
B(q,l,q)\
y(q,(%[out]))\
r(f,k,f)\
a(i,h,i)\
a(d,e,d)\
z(s,j,j)\
c(D,e)\
C(b,h,b)\
r(m,j,m)\
c(D,j)\
v(u,m,h)\
v(u,i,i)\
A(x,d,i,d)\
c(F,i)\
C(t,d,d)\
r(d,b,d)\
a(e,m,e)\
z(d,f,b)\
z(s,f,f)\
a(j,h,j)\
r(s,b,h)\
c(D,k)\
r(d,f,d)\
B(h,b,h)\
c(D,b)\
v(u,d,f)\
v(u,e,e)\
C(i,m,i)\
y(h,32(%[out]))\
c(F,h)\
A(x,j,e,j)\
C(t,j,j)\
r(j,i,j)\
y(j,64(%[out]))\
a(k,d,k)\
a(b,f,b)\
C(h,d,h)\
v(u,k,k)\
A(x,b,k,b)\
C(t,b,b)\
r(b,h,b)\
y(b,96(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm7,CP_FPS_ASM_VBROADCASTSS,%%ymm10,%%ymm9,%%ymm1,CP_FPS_ASM_VPADDQ,%%ymm4,%%ymm2,%%ymm8,%%ymm3,%%ymm5,%%ymm12,%%ymm0,%%ymm11,CP_FPS_ASM_VMOVDQU,%%ymm6,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm15,$32,CP_FPS_ASM_VPSRLQ,%%ymm13,$170,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VPMULLD,32(%[ix]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),256(%[cur]),260(%[cur]),264(%[cur]),268(%[cur]),272(%[cur]),276(%[cur]),280(%[cur]),284(%[cur]),288(%[cur]),292(%[cur]),296(%[cur]),300(%[cur]),304(%[cur]),308(%[cur]),312(%[cur]),316(%[cur]),32(%[iy]),320(%[cur]),324(%[cur]),328(%[cur]),332(%[cur]),336(%[cur]),340(%[cur]),344(%[cur]),348(%[cur]),352(%[cur]),356(%[cur]),36(%[iy]),360(%[cur]),364(%[cur]),368(%[cur]),372(%[cur]),376(%[cur]),380(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

inline void bottom_both(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w, void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31) \
M(%[P],t) M(%[P2],v) M(%[NI],G) C(32(%[a]),k)\
z(36(%[cur]),d)\
f(S,j)\
f(H,n)\
f(H,e)\
f(I,o)\
C(32(%[cur]),c)\
f(S,m)\
C((%[a]),s)\
q(v,k,h)\
u(h,k,h)\
f(T,k)\
g(j,d,j)\
g(_28,n,n)\
f(H,d)\
g(68(%[a]),e,e)\
z(28(%[cur]),i)\
f(T,l)\
g(m,c,m)\
B(x,n,n)\
A(_28,o,o)\
q(v,s,r)\
F(E,e,n,e)\
u(r,s,r)\
f(U,s)\
f(H,n)\
A(t,e,e)\
g(k,c,k)\
q(e,o,e)\
z(24(%[cur]),o)\
f(U,c)\
g(_30,d,d)\
g(l,i,l)\
p(k,j,k)\
f(I,j)\
g(s,i,s)\
g(100(%[a]),n,n)\
B(x,d,d)\
f(V,i)\
p(l,m,l)\
z(20(%[cur]),m)\
p(s,k,s)\
F(E,n,d,n)\
f(V,d)\
D(e,r,k)\
A(t,n,n)\
q(e,r,e)\
q(v,k,r)\
g(c,o,c)\
u(r,k,r)\
f(W,k)\
A(_30,j,j)\
g(i,o,i)\
p(c,l,c)\
z(16(%[cur]),l)\
D(v,e,o)\
u(o,e,o)\
f(W,e)\
q(n,j,n)\
f(X,j)\
g(d,m,d)\
p(i,s,i)\
D(v,h,s)\
D(n,h,h)\
q(n,s,n)\
f(X,s)\
g(k,m,k)\
z(12(%[cur]),m)\
p(d,c,d)\
f(K,c)\
p(k,i,k)\
f(Z,i)\
g(e,l,e)\
g(j,l,j)\
f(K,l)\
p(e,d,e)\
z(8(%[cur]),d)\
g(s,m,s)\
p(j,k,j)\
f(Z,k)\
g(c,h,c)\
g(i,m,i)\
p(s,e,s)\
f(_0,e)\
f((%[y]),m)\
B(x,c,c)\
p(i,j,i)\
z(4(%[cur]),j)\
g(k,d,k)\
p(k,s,k)\
g(e,d,e)\
A(m,h,m)\
f(_0,d)\
B(x,n,s)\
B(x,h,h)\
g(l,h,l)\
f(L,h)\
p(e,i,e)\
f(L,i)\
F(E,l,c,l)\
g(G,e,c)\
A(t,l,l)\
g(d,j,d)\
g(t,c,c)\
f(_27,j)\
q(l,m,l)\
g(h,n,h)\
p(d,k,d)\
p(c,e,c)\
g(i,s,i)\
z(100(%[cur]),s)\
g(G,d,e)\
f(_1,k)\
B(x,h,h)\
F(E,i,h,i)\
D(l,r,h)\
f(_1,m)\
g(t,e,e)\
A(t,i,i)\
D(v,r,r)\
A(j,n,j)\
q(l,r,l)\
C(96(%[cur]),r)\
f(_2,n)\
p(e,d,e)\
z(92(%[cur]),d)\
B(x,e,e)\
F(E,c,e,c)\
q(i,j,i)\
f(_2,j)\
D(v,o,e)\
g(k,s,k)\
D(i,o,o)\
q(i,e,i)\
f(_3,e)\
q(v,c,s)\
u(s,c,s)\
z(88(%[cur]),c)\
g(m,r,m)\
g(n,r,n)\
f(_3,r)\
g(j,d,j)\
g(e,d,e)\
p(n,k,n)\
q(v,h,k)\
f(_4,d)\
u(k,h,k)\
p(j,m,j)\
f(O,m)\
q(t,k,h)\
p(e,n,e)\
u(h,k,h)\
f(_4,k)\
z(84(%[cur]),n)\
C(h,32(%[buf]))\
g(r,c,r)\
p(r,j,r)\
f(_5,j)\
g(d,c,d)\
f(O,c)\
g(m,h,m)\
g(k,n,k)\
p(d,e,d)\
f(_5,e)\
B(x,m,m)\
p(k,r,k)\
z(80(%[cur]),r)\
g(j,n,j)\
f(_6,n)\
p(j,d,j)\
f((%[w]),d)\
g(e,r,e)\
g(n,r,n)\
z(76(%[cur]),r)\
p(e,k,e)\
f(_6,k)\
p(n,j,n)\
A(d,h,d)\
q(v,l,j)\
B(x,h,h)\
u(j,l,j)\
g(c,h,c)\
f(_7,h)\
z(72(%[cur]),l)\
F(E,c,m,c)\
q(t,j,m)\
u(m,j,m)\
A(t,c,c)\
f(_7,j)\
g(k,r,k)\
C(m,96(%[buf]))\
q(c,d,c)\
f(_8,d)\
g(h,r,h)\
p(k,e,k)\
f(Q,e)\
B(x,m,r)\
p(h,n,h)\
f(_8,n)\
g(j,l,j)\
g(d,l,d)\
p(j,k,j)\
z(68(%[cur]),k)\
q(t,c,l)\
g(e,m,e)\
u(l,c,l)\
f(Q,c)\
C(l,(%[buf]))\
f(4(%[w]),l)\
p(d,h,d)\
g(G,d,h)\
B(x,e,e)\
g(t,h,h)\
g(n,k,n)\
g(c,r,c)\
p(h,d,h)\
A(l,m,l)\
p(n,j,n)\
g(G,n,j)\
F(E,c,e,c)\
z(164(%[cur]),e)\
A(t,c,c)\
f(_10,m)\
g(t,j,j)\
q(v,o,d)\
f(_10,r)\
C(160(%[cur]),k)\
u(d,o,d)\
q(c,l,c)\
f(_11,l)\
p(j,n,j)\
q(t,d,n)\
q(t,c,o)\
B(x,j,j)\
u(n,d,n)\
u(o,c,o)\
f(_11,c)\
z(156(%[cur]),d)\
F(E,h,j,h)\
C(o,64(%[buf]))\
f(R,o)\
B(x,n,j)\
g(m,e,m)\
C(n,160(%[buf]))\
f(_12,e)\
g(r,k,r)\
g(l,k,l)\
z(152(%[cur]),k)\
g(c,d,c)\
p(l,m,l)\
g(o,n,o)\
f(_12,m)\
g(e,d,e)\
f(_13,d)\
p(c,r,c)\
q(v,h,r)\
u(r,h,r)\
B(x,o,o)\
f(R,h)\
p(e,l,e)\
f(_13,l)\
g(m,k,m)\
g(d,k,d)\
z(148(%[cur]),k)\
p(m,c,m)\
g(h,j,h)\
f(_14,j)\
f(8(%[w]),c)\
p(d,e,d)\
z(144(%[cur]),e)\
F(E,h,o,h)\
q(v,i,o)\
A(t,h,h)\
u(o,i,o)\
f(_14,i)\
g(l,k,l)\
g(j,k,j)\
A(c,n,c)\
f(_15,n)\
q(t,o,k)\
p(l,m,l)\
u(k,o,k)\
f(_15,o)\
z(140(%[cur]),m)\
p(j,d,j)\
q(h,c,h)\
C(k,224(%[buf]))\
B(x,k,c)\
f(_16,d)\
g(i,e,i)\
g(n,e,n)\
p(i,l,i)\
f(Y,l)\
f(Y,e)\
g(o,m,o)\
p(n,j,n)\
f(_16,j)\
g(d,m,d)\
z(136(%[cur]),m)\
p(o,i,o)\
f(_17,i)\
p(d,n,d)\
f(12(%[w]),n)\
g(e,c,e)\
g(l,k,l)\
q(t,h,c)\
u(c,h,c)\
f(_17,h)\
C(c,128(%[buf]))\
z(132(%[cur]),c)\
B(x,l,l)\
g(j,m,j)\
F(E,e,l,e)\
g(i,m,i)\
A(t,e,e)\
p(j,o,j)\
A(n,k,n)\
p(i,d,i)\
C((%[b]),d)\
g(G,i,k)\
g(h,c,h)\
q(e,n,e)\
q(t,e,n)\
g(t,k,k)\
u(n,e,n)\
p(h,j,h)\
C(32(%[b]),j)\
C(n,192(%[buf]))\
z(228(%[cur]),n)\
g(G,h,e)\
f(_18,c)\
p(k,i,k)\
f(H,i)\
q(v,d,o)\
C(224(%[cur]),m)\
g(t,e,e)\
u(o,d,o)\
f(_18,d)\
f(H,l)\
p(e,h,e)\
f(_19,h)\
B(x,e,e)\
F(E,k,e,k)\
q(v,j,e)\
g(c,n,c)\
u(e,j,e)\
f(_19,j)\
z(220(%[cur]),n)\
g(_29,i,i)\
g(d,m,d)\
g(68(%[b]),l,l)\
B(x,i,i)\
g(h,m,h)\
f(_21,m)\
F(E,l,i,l)\
z(216(%[cur]),i)\
A(t,l,l)\
g(j,n,j)\
p(h,c,h)\
f(_21,c)\
p(j,d,j)\
f(H,d)\
g(m,n,m)\
f(_22,n)\
g(c,i,c)\
p(m,h,m)\
f(H,h)\
p(c,j,c)\
g(_31,d,d)\
z(212(%[cur]),j)\
g(n,i,n)\
f(_22,i)\
B(x,d,d)\
p(n,m,n)\
g(100(%[b]),h,h)\
f(_23,m)\
F(E,h,d,h)\
f(I,d)\
A(t,h,h)\
g(i,j,i)\
p(i,c,i)\
g(m,j,m)\
f(_23,j)\
z(208(%[cur]),c)\
p(m,n,m)\
A(_31,d,d)\
q(v,k,n)\
u(n,k,n)\
f(_24,k)\
q(h,d,h)\
f(I,d)\
g(j,c,j)\
p(j,i,j)\
z(204(%[cur]),i)\
g(k,c,k)\
f(_24,c)\
A(_29,d,d)\
p(k,m,k)\
f(_25,m)\
q(l,d,l)\
D(v,e,d)\
D(h,e,e)\
q(h,d,h)\
z(200(%[cur]),d)\
g(c,i,c)\
g(m,i,m)\
p(c,j,c)\
f(_25,j)\
B(x,e,i)\
p(m,k,m)\
f(_26,k)\
g(j,d,j)\
g(k,d,k)\
p(j,c,j)\
f(K,c)\
f(K,d)\
p(k,m,k)\
z(196(%[cur]),m)\
g(d,i,d)\
g(c,e,c)\
f(_26,i)\
B(x,c,c)\
F(E,d,c,d)\
f((%[y]),c)\
g(i,m,i)\
q(l,o,m)\
D(l,o,l)\
g(G,k,o)\
A(t,d,d)\
p(i,j,i)\
D(v,m,j)\
g(t,o,o)\
A(c,e,c)\
u(j,m,j)\
B(x,h,m)\
g(G,i,e)\
p(o,k,o)\
q(d,c,d)\
f(L,c)\
q(v,l,k)\
g(t,e,e)\
u(k,l,k)\
f(L,l)\
p(e,i,e)\
D(r,s,i)\
B(x,e,e)\
F(E,o,e,o)\
q(v,i,e)\
D(v,s,s)\
u(e,i,e)\
q(v,o,i)\
u(i,o,i)\
q(r,s,r)\
g(c,h,c)\
f(_27,s)\
f(_9,o)\
g(l,m,l)\
f(_9,m)\
B(x,c,c)\
F(E,l,c,l)\
B(x,r,c)\
A(t,l,l)\
A(s,h,s)\
f((%[iy]),h)\
g(o,r,o)\
g(m,c,m)\
D(i,n,c)\
q(l,s,l)\
q(v,c,s)\
D(v,n,n)\
B(x,o,o)\
u(s,c,s)\
D(v,j,c)\
q(i,n,i)\
F(E,m,o,m)\
q(l,c,c)\
D(l,j,l)\
f(_20,j)\
f(_20,o)\
B(x,i,n)\
A(t,m,m)\
A(h,r,h)\
D(v,k,r)\
D(d,k,k)\
q(d,r,d)\
f(4(%[iy]),r)\
q(m,h,m)\
q(v,d,h)\
u(h,d,h)\
q(v,k,d)\
g(j,i,j)\
u(d,k,d)\
D(s,e,k)\
g(o,n,o)\
q(t,d,n)\
u(n,d,n)\
q(t,h,d)\
B(x,j,j)\
C(n,256(%[buf]))\
u(d,h,d)\
q(v,k,h)\
F(E,o,j,o)\
A(r,i,r)\
q(v,l,i)\
C(d,288(%[buf]))\
u(h,k,h)\
A(t,o,o)\
D(v,e,e)\
u(i,l,i)\
C(h,(%[out]))\
f(J,h)\
q(s,e,s)\
q(t,i,e)\
f(J,l)\
B(x,s,k)\
q(v,c,d)\
u(e,i,e)\
q(o,r,o)\
u(d,c,d)\
C(e,320(%[buf]))\
f(N,e)\
D(o,m,c)\
q(t,d,r)\
q(v,c,i)\
u(r,d,r)\
D(v,m,m)\
u(i,c,i)\
C(r,352(%[buf]))\
f(J,r)\
q(o,m,o)\
C(i,32(%[out]))\
f(J,i)\
B(x,o,m)\
g(h,s,h)\
f(N,c)\
g(l,k,l)\
B(x,h,h)\
A(e,s,e)\
F(E,l,h,l)\
A(t,l,l)\
g(r,o,r)\
g(i,m,i)\
q(l,e,l)\
B(x,r,r)\
A(c,o,c)\
C(l,64(%[out]))\
F(E,i,r,i)\
A(t,i,i)\
q(i,c,i)\
C(i,96(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm1,%%ymm4,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPMULUDQ,%%ymm9,%%ymm10,%%ymm2,%%ymm0,%%ymm11,%%ymm7,%%ymm3,%%ymm5,CP_FPS_ASM_VPADDQ,CP_FPS_ASM_VPSUBD,%%ymm12,%%ymm8,%%ymm15,CP_FPS_ASM_VPMINUD,%%ymm14,$32,CP_FPS_ASM_VMOVDQU,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VPBLENDD,%%ymm13,32(%[x]),(%[x]),32(%[ix]),32(%[y]),36(%[y]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),16(%[w]),20(%[w]),24(%[w]),256(%[cur]),260(%[cur]),264(%[cur]),268(%[cur]),272(%[cur]),276(%[cur]),28(%[w]),280(%[cur]),284(%[cur]),288(%[cur]),292(%[cur]),296(%[cur]),300(%[cur]),304(%[cur]),308(%[cur]),312(%[cur]),316(%[cur]),32(%[iy]),320(%[cur]),324(%[cur]),328(%[cur]),332(%[cur]),336(%[cur]),340(%[cur]),344(%[cur]),348(%[cur]),352(%[cur]),356(%[cur]),36(%[iy]),360(%[cur]),364(%[cur]),368(%[cur]),372(%[cur]),376(%[cur]),380(%[cur]),4(%[y]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [w] "r"(w), [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

}
namespace ntt {
inline constexpr int kMaxLog = 23;
namespace detail {
using namespace blocked_ntt::detail;
class Recursion {
public:
	Recursion(const std::uint32_t* roots, const std::uint32_t* inverse_roots) : r_(roots), ir_(inverse_roots) {}

	void visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
		switch (nv) {
		case 4: return tile<4>(a, b, k);
		case 16: return tile<16>(a, b, k);
		case 64: return tile<64>(a, b, k);
		case 256: return tile<256>(a, b, k);
		}
		const std::size_t h = nv / 4;
		forward(a, b, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + t * h, b + t * h, h, 4 * k + t);
		inverse(a, h, k);
	}

private:
	
	template <std::size_t NV>
	[[gnu::noinline]] void tile(Vec* a, Vec* b, std::size_t k) const {
		for (std::size_t h = NV / 4; h >= 4; h /= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) forward(a + j, b + j, h, g);
		bottom(a, b, NV, k * (NV / 4));
		for (std::size_t h = 4; h < NV; h *= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) inverse(a + j, h, g);
	}

	void forward(Vec* a, Vec* b, std::size_t h, std::size_t k) const {
		if (k == 0) {
			kernels::forward_identity(a, h, r_);
			kernels::forward_identity(b, h, r_);
			return;
		}
		const std::uint32_t *x = r_ + slot(k), *y = r_ + slot(2 * k);
		if (h == 4) return kernels::forward_pair(a, b, h, x, y);
		kernels::forward(a, h, x, y);
		kernels::forward(b, h, x, y);
	}

	void inverse(Vec* a, std::size_t h, std::size_t k) const {
		if (k == 0) return kernels::inverse_identity(a, h, ir_);
		kernels::inverse(a, h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	struct alignas(64) Leaves {
		std::uint32_t window[4][16];
		std::uint32_t coefficients[4][8];
	};

	void leaf_weights(std::size_t k, std::uint32_t* w) const {
		const std::uint32_t* y = r_ + slot(2 * k);
		w[0] = y[0], w[1] = kP - y[0], w[2] = y[1], w[3] = kP - y[1];
		w[4] = y[8], w[5] = ~y[8], w[6] = y[9], w[7] = ~y[9];
	}

	
	void bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {
		Leaves leaves[2]{};
		alignas(32) std::uint32_t w[2][8];
		leaf_weights(first, w[0]);
		kernels::bottom_first(a, b, &leaves[0], r_ + slot(first), r_ + slot(2 * first), w[0]);
		for (std::size_t j = 0, k = first; j < nv; j += 4, ++k) {
			const std::size_t cur = j / 4 % 2, next = cur ^ 1;
			const std::uint32_t *ix = ir_ + slot(k), *iy = ir_ + slot(2 * k);
			if (j + 4 == nv) {
				kernels::bottom_last(a + j, &leaves[cur], ix, iy);
				break;
			}
			leaf_weights(k + 1, w[next]);
			kernels::bottom_both(a + j + 4, b + j + 4, &leaves[next], r_ + slot(k + 1), r_ + slot(2 * k + 2), w[next],
								 a + j, &leaves[cur], ix, iy);
		}
	}

	const std::uint32_t *r_, *ir_;
};



inline void forward_radix2(Vec* f, std::size_t h, bool sparse) {
	if (sparse) {
		std::memcpy(f + h, f, h * sizeof(Vec));
		return;
	}
	for (std::size_t i = 0; i < h; ++i) {
		const Vec x = f[i], y = f[i + h];
		f[i] = reduce(add(x, y), 2 * kP);
		f[i + h] = reduce(diff(x, y), 2 * kP);
	}
}

inline void forward_radix4(Vec* f, std::size_t h, bool sparse, const std::uint32_t* roots) {
	if (!sparse) return kernels::forward_identity(f, h, roots);
	const Factor z(roots[1], roots[9]);
	for (std::size_t j = 0; j < h; ++j) {
		const Vec a = f[j], b = f[j + h], zb = multiply(b, z);
		f[j] = add(a, b), f[j + h] = diff(a, b), f[j + 2 * h] = add(a, zb), f[j + 3 * h] = diff(a, zb);
	}
}

inline void inverse_radix4(Vec* f, std::size_t h, const std::uint32_t* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(multiply(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = reduce(add(p0, p1), 2 * kP), cd = reduce(add(p2, p3), 2 * kP);
		const Vec amb = reduce(diff(p0, p1), 2 * kP), cmd = multiply(diff(p2, p3), z);
		f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

}
class Convolution {
public:
	Convolution(std::size_t n, std::size_t m) : Convolution(n, m, nullptr) {}
	Convolution(std::size_t n, std::size_t m, std::uint32_t* borrowed_a)
		: n_(n), m_(m), lg_(std::max(6, int(std::bit_width(n + m - 2)))) {
		if (n == 0 || m == 0 || lg_ > kMaxLog) std::abort();
		const std::size_t len = length(), words = (borrowed_a ? 1 : 2) * (len + kPadding) + 2 * detail::table_words(lg_) + (borrowed_a ? 1024 : 0);
		storage_ = allocate_fps_words(words);
		a_ = borrowed_a ? borrowed_a : storage_.get();
		const std::size_t color = ((reinterpret_cast<std::uintptr_t>(a_) >> 6) - (reinterpret_cast<std::uintptr_t>(storage_.get()) >> 6) + 1) & 63;
		b_ = storage_.get() + (borrowed_a ? 16 * color : len + kPadding);
		roots_ = b_ + len + kPadding;
		inverse_roots_ = roots_ + detail::table_words(lg_);
		std::fill(roots_, inverse_roots_ + detail::table_words(lg_), 0);
	}

	Convolution(const Convolution&) = delete;
	Convolution& operator=(const Convolution&) = delete;

	std::uint32_t* a() { return a_; }
	std::uint32_t* b() { return b_; }

	const std::uint32_t* multiply() {
		using namespace detail;
		const std::size_t len = length(), nv = len / 8;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		const Recursion recursion(roots_, inverse_roots_);
		const std::uint32_t scale = power(std::uint32_t(nv), kP - 2);
		auto* a = reinterpret_cast<Vec*>(a_);
		auto* b = reinterpret_cast<Vec*>(b_);
		const bool sparse_a = n_ <= len / 2, sparse_b = m_ <= len / 2;
		if (std::countr_zero(nv) % 2) {
			const std::size_t h = nv / 2;
			forward_radix2(a, h, sparse_a);
			forward_radix2(b, h, sparse_b);
			recursion.visit(a, b, h, 0);
			recursion.visit(a + h, b + h, h, 1);
			alignas(32) std::uint32_t s[16] = {};
			s[1] = scale;
			s[9] = quotient(scale);
			kernels::scale_radix2(a, h, s);
		} else {
			const std::size_t h = nv / 4;
			forward_radix4(a, h, sparse_a, roots_);
			forward_radix4(b, h, sparse_b, roots_);
			for (std::size_t t = 0; t < 4; ++t) recursion.visit(a + t * h, b + t * h, h, t);
			detail::inverse_radix4(a, h, inverse_roots_, Factor(scale));
		}
		return a_;
	}

private:
	static constexpr std::size_t kPadding = 16;

	std::size_t length() const { return std::size_t(1) << lg_; }

	std::size_t n_, m_;
	int lg_;
	FpsWords storage_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
};

}
namespace bottom_kernels {

using ntt::kernels::k2P;
using ntt::kernels::kNI;
using ntt::kernels::kP;

[[gnu::always_inline]] inline void first(const void* a, const void* b, void* buf, const void* x, const void* y) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W) \
N(%[P],q) N(%[P2],u) N(%[NI],%%ymm13) x(32(%[a]),j)\
m(B,l)\
m(B,n)\
m(H,f)\
x((%[a]),o)\
m(B,k)\
e(u,j,i)\
m(B,c)\
t(T,l,l)\
p(i,j,i)\
t(68(%[a]),n,n)\
s(T,f,f)\
e(u,o,j)\
m(H,g)\
t(V,k,k)\
p(j,o,j)\
w(v,l,l)\
A(z,n,l,n)\
w(v,k,k)\
s(q,n,n)\
t(100(%[a]),c,c)\
e(n,f,n)\
y(u,i,f)\
y(n,j,l)\
e(n,j,n)\
m(C,j)\
s(V,g,g)\
e(u,l,o)\
y(u,n,r)\
m(C,h)\
A(z,c,k,c)\
p(o,l,o)\
p(r,n,r)\
s(q,c,c)\
m(I,n)\
e(c,g,c)\
m(E,g)\
y(c,i,i)\
e(c,f,c)\
m(E,f)\
w(v,i,l)\
w(v,c,k)\
m(L,d)\
t(j,i,j)\
t(h,l,h)\
s(n,i,n)\
w(v,j,j)\
A(z,h,j,h)\
y(u,o,j)\
s(q,h,h)\
t(g,c,g)\
y(u,r,i)\
t(f,k,f)\
s(d,c,d)\
e(h,n,h)\
w(v,g,g)\
e(h,j,j)\
y(h,o,h)\
A(z,f,g,f)\
m(C,g)\
e(u,h,o)\
s(q,f,f)\
m(C,n)\
p(o,h,o)\
m(I,h)\
e(q,o,c)\
p(c,o,c)\
e(f,d,f)\
w(v,c,d)\
x(c,32(%[buf]))\
e(f,i,i)\
y(f,r,f)\
e(u,j,r)\
p(r,j,r)\
e(q,r,j)\
t(g,c,g)\
p(j,r,j)\
t(n,d,n)\
m(C,d)\
w(v,j,r)\
s(h,c,h)\
x(j,96(%[buf]))\
m(C,c)\
w(v,g,g)\
m(I,o)\
A(z,n,g,n)\
s(q,n,n)\
e(n,h,n)\
e(q,n,h)\
t(d,j,d)\
p(h,n,h)\
e(u,f,n)\
t(c,r,c)\
x(h,(%[buf]))\
p(n,f,n)\
s(o,j,o)\
e(q,n,j)\
w(v,d,d)\
m(E,f)\
p(j,n,j)\
A(z,c,d,c)\
m(E,d)\
w(v,j,n)\
s(q,c,c)\
x(j,160(%[buf]))\
m(L,h)\
e(c,o,c)\
e(q,c,o)\
p(o,c,o)\
e(o,q,o)\
e(u,i,c)\
t(f,j,f)\
x(o,64(%[buf]))\
p(c,i,c)\
t(d,n,d)\
e(q,c,n)\
s(h,j,h)\
m(E,j)\
p(n,c,n)\
w(v,f,f)\
m(E,c)\
w(v,n,i)\
A(z,d,f,d)\
x(n,224(%[buf]))\
m(L,f)\
s(q,d,d)\
e(d,h,d)\
e(q,d,h)\
p(h,d,h)\
t(j,n,j)\
x(h,128(%[buf]))\
x((%[b]),h)\
t(c,i,c)\
x(32(%[b]),i)\
s(f,n,f)\
w(v,j,j)\
m(B,n)\
A(z,c,j,c)\
m(B,j)\
s(q,c,c)\
e(c,f,c)\
e(u,h,f)\
e(q,c,d)\
m(H,o)\
e(u,i,r)\
p(f,h,f)\
p(d,c,d)\
m(B,c)\
p(r,i,r)\
e(d,q,d)\
m(B,i)\
t(U,n,n)\
x(d,192(%[buf]))\
m(H,d)\
t(68(%[b]),j,j)\
w(v,n,n)\
A(z,j,n,j)\
s(q,j,j)\
s(U,o,o)\
t(W,c,c)\
t(100(%[b]),i,i)\
s(W,d,d)\
e(j,o,j)\
w(v,c,c)\
y(u,r,o)\
y(j,f,n)\
e(j,f,j)\
m(C,f)\
A(z,i,c,i)\
e(u,n,c)\
y(u,j,h)\
m(C,g)\
s(q,i,i)\
p(c,n,c)\
p(h,j,h)\
m(I,j)\
e(i,d,i)\
m(E,d)\
y(i,r,r)\
e(i,o,i)\
m(E,o)\
w(v,r,n)\
w(v,i,k)\
m(L,l)\
t(f,r,f)\
t(g,n,g)\
s(j,r,j)\
w(v,f,f)\
A(z,g,f,g)\
y(u,c,f)\
t(d,i,d)\
s(q,g,g)\
t(o,k,o)\
y(u,h,k)\
s(l,i,l)\
w(v,d,d)\
e(g,j,g)\
A(z,o,d,o)\
y(g,c,c)\
e(g,f,g)\
s(q,o,o)\
e(u,c,f)\
e(u,g,d)\
p(f,c,f)\
p(d,g,d)\
e(q,f,g)\
e(q,d,c)\
e(o,l,o)\
p(g,f,g)\
p(c,d,c)\
y(o,h,h)\
e(o,k,o)\
x(g,256(%[buf]))\
x(128(%[a]),g)\
e(u,h,k)\
e(u,o,d)\
x(c,288(%[buf]))\
x(160(%[a]),c)\
p(k,h,k)\
p(d,o,d)\
m(D,o)\
e(q,k,h)\
e(q,d,f)\
m(D,l)\
p(h,k,h)\
p(f,d,f)\
m(K,d)\
x(h,320(%[buf]))\
x(f,352(%[buf]))\
m(D,f)\
e(u,g,h)\
m(D,k)\
e(u,c,j)\
p(h,g,h)\
t(O,o,o)\
p(j,c,j)\
m(K,c)\
t(196(%[a]),l,l)\
s(O,d,d)\
w(v,o,o)\
t(R,f,f)\
A(z,l,o,l)\
s(q,l,l)\
t(228(%[a]),k,k)\
w(v,f,f)\
e(l,d,l)\
s(R,c,c)\
y(u,j,d)\
y(l,h,o)\
e(l,h,l)\
m(F,h)\
A(z,k,f,k)\
e(u,o,f)\
y(u,l,g)\
m(F,i)\
s(q,k,k)\
p(f,o,f)\
p(g,l,g)\
m(M,l)\
e(k,c,k)\
m(G,c)\
y(k,j,j)\
e(k,d,k)\
m(G,d)\
w(v,j,o)\
w(v,k,r)\
m(J,n)\
t(h,j,h)\
t(i,o,i)\
s(l,j,l)\
w(v,h,h)\
A(z,i,h,i)\
y(u,f,h)\
t(c,k,c)\
s(q,i,i)\
t(d,r,d)\
y(u,g,r)\
s(n,k,n)\
w(v,c,c)\
e(i,l,i)\
A(z,d,c,d)\
e(i,h,h)\
y(i,f,i)\
s(q,d,d)\
m(F,f)\
e(u,i,c)\
m(F,l)\
p(c,i,c)\
m(M,i)\
e(q,c,k)\
e(d,n,d)\
p(k,c,k)\
e(d,r,r)\
y(d,g,d)\
w(v,k,g)\
x(k,416(%[buf]))\
e(u,h,c)\
p(c,h,c)\
e(q,c,h)\
p(h,c,h)\
t(f,k,f)\
m(F,c)\
w(v,h,n)\
t(l,g,l)\
x(h,480(%[buf]))\
m(F,g)\
s(i,k,i)\
m(M,k)\
w(v,f,f)\
A(z,l,f,l)\
s(q,l,l)\
e(l,i,l)\
t(c,h,c)\
e(q,l,i)\
t(g,n,g)\
p(i,l,i)\
e(u,d,l)\
s(k,h,k)\
x(i,384(%[buf]))\
p(l,d,l)\
w(v,c,c)\
m(G,d)\
e(q,l,i)\
A(z,g,c,g)\
m(G,c)\
p(i,l,i)\
s(q,g,g)\
m(J,l)\
w(v,i,h)\
x(i,544(%[buf]))\
e(g,k,g)\
e(q,g,k)\
p(k,g,k)\
e(k,q,k)\
e(u,r,g)\
t(d,i,d)\
x(k,448(%[buf]))\
p(g,r,g)\
t(c,h,c)\
e(q,g,h)\
s(l,i,l)\
m(G,i)\
p(h,g,h)\
w(v,d,d)\
m(G,g)\
w(v,h,r)\
A(z,c,d,c)\
x(h,608(%[buf]))\
m(J,d)\
s(q,c,c)\
e(c,l,c)\
e(q,c,l)\
p(l,c,l)\
t(i,h,i)\
x(l,512(%[buf]))\
x(128(%[b]),l)\
t(g,r,g)\
x(160(%[b]),r)\
s(d,h,d)\
w(v,i,i)\
m(D,h)\
A(z,g,i,g)\
m(D,i)\
s(q,g,g)\
e(g,d,g)\
e(u,l,d)\
e(q,g,c)\
m(K,k)\
e(u,r,n)\
p(d,l,d)\
p(c,g,c)\
m(D,g)\
p(n,r,n)\
e(c,q,c)\
m(D,r)\
t(Q,h,h)\
x(c,576(%[buf]))\
m(K,c)\
t(196(%[b]),i,i)\
w(v,h,h)\
A(z,i,h,i)\
s(q,i,i)\
s(Q,k,k)\
t(S,g,g)\
t(228(%[b]),r,r)\
s(S,c,c)\
e(i,k,i)\
w(v,g,g)\
y(u,n,k)\
y(i,d,h)\
e(i,d,i)\
m(F,d)\
A(z,r,g,r)\
e(u,h,g)\
y(u,i,l)\
m(F,f)\
s(q,r,r)\
p(g,h,g)\
p(l,i,l)\
m(M,i)\
e(r,c,r)\
m(G,c)\
y(r,n,n)\
e(r,k,r)\
m(G,k)\
w(v,n,h)\
w(v,r,j)\
m(J,o)\
t(d,n,d)\
t(f,h,f)\
s(i,n,i)\
w(v,d,d)\
A(z,f,d,f)\
y(u,g,d)\
t(c,r,c)\
s(q,f,f)\
t(k,j,k)\
y(u,l,j)\
s(o,r,o)\
w(v,c,c)\
e(f,i,f)\
A(z,k,c,k)\
y(f,g,g)\
e(f,d,f)\
s(q,k,k)\
e(u,g,d)\
e(u,f,c)\
p(d,g,d)\
p(c,f,c)\
e(q,d,f)\
e(q,c,g)\
e(k,o,k)\
p(f,d,f)\
p(g,c,g)\
y(k,l,l)\
e(k,j,k)\
x(f,640(%[buf]))\
e(u,l,f)\
e(u,k,j)\
x(g,672(%[buf]))\
p(f,l,f)\
p(j,k,j)\
e(q,f,k)\
e(q,j,l)\
p(k,f,k)\
p(l,j,l)\
x(k,704(%[buf]))\
x(l,736(%[buf]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm7,%%ymm11,CP_FPS_ASM_VPSUBD,%%ymm3,%%ymm8,%%ymm10,%%ymm6,%%ymm0,%%ymm5,%%ymm1,CP_FPS_ASM_VBROADCASTSS,%%ymm2,%%ymm4,CP_FPS_ASM_VPMINUD,%%ymm15,%%ymm9,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VPBLENDD,32(%[x]),32(%[y]),36(%[x]),36(%[y]),40(%[y]),44(%[y]),(%[x]),(%[y]),12(%[y]),4(%[x]),4(%[y]),8(%[y]),CP_FPS_ASM_VPBROADCASTD,192(%[a]),192(%[b]),224(%[a]),224(%[b]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

[[gnu::always_inline]] inline void last(void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39) \
C(%[P],q) C(%[P2],p) C(%[NI],t) k(256(%[cur]),l)\
k(260(%[cur]),c)\
k(268(%[cur]),n)\
k(264(%[cur]),e)\
a(X,c,g)\
a(36(%[cur]),l,i)\
a(Y,l,l)\
a(W,e,b)\
k(272(%[cur]),h)\
a(Y,c,c)\
a(N,n,j)\
a(X,e,e)\
k(276(%[cur]),f)\
a(W,n,n)\
d(g,l,g)\
d(c,i,c)\
d(b,g,b)\
k(280(%[cur]),g)\
d(j,b,j)\
d(e,c,e)\
d(n,e,n)\
a(N,h,e)\
a(L,h,h)\
k(284(%[cur]),c)\
a(E,f,b)\
a(L,f,f)\
d(e,n,e)\
d(h,j,h)\
a(_34,g,j)\
a(E,g,g)\
d(f,e,f)\
d(b,h,b)\
d(j,b,j)\
d(g,f,g)\
a(4(%[cur]),c,f)\
a(_34,c,c)\
d(f,j,f)\
d(c,g,c)\
k(288(%[cur]),g)\
a(t,f,j)\
a(t,c,b)\
k(292(%[cur]),h)\
a(q,j,j)\
a(q,b,b)\
d(j,f,j)\
d(b,c,b)\
k(296(%[cur]),c)\
s(r,j,j)\
a(100(%[cur]),g,f)\
a(_39,g,g)\
w(u,b,j,b)\
k(300(%[cur]),j)\
a(_39,h,e)\
o(p,b,n)\
a(_38,h,h)\
x(n,b,n)\
k(304(%[cur]),b)\
d(e,f,e)\
d(h,g,h)\
a(_37,c,g)\
a(_38,c,c)\
k(308(%[cur]),f)\
a(_36,j,i)\
a(_37,j,j)\
d(g,h,g)\
d(c,e,c)\
k(312(%[cur]),e)\
a(_35,b,h)\
a(_36,b,b)\
d(i,g,i)\
d(j,c,j)\
k(316(%[cur]),c)\
d(h,i,h)\
d(b,j,b)\
a(_35,f,j)\
a(_33,f,f)\
a(_32,e,i)\
d(f,h,f)\
a(_33,e,e)\
d(j,b,j)\
a(_32,c,b)\
a(68(%[cur]),c,c)\
d(i,f,i)\
d(e,j,e)\
d(c,i,c)\
d(b,e,b)\
k(320(%[cur]),e)\
a(t,c,i)\
a(t,b,j)\
k(324(%[cur]),f)\
a(q,i,i)\
a(q,j,j)\
d(j,b,j)\
d(i,c,i)\
k(328(%[cur]),c)\
s(r,i,i)\
a(M,e,b)\
a(164(%[cur]),e,e)\
w(u,j,i,j)\
k(332(%[cur]),i)\
a(K,f,h)\
o(p,j,g)\
a(M,f,f)\
x(g,j,g)\
k(336(%[cur]),j)\
d(h,b,h)\
d(f,e,f)\
a(J,c,e)\
a(K,c,c)\
k(340(%[cur]),b)\
a(I,i,l)\
a(J,i,i)\
d(e,h,e)\
d(c,f,c)\
k(344(%[cur]),f)\
a(I,j,h)\
a(H,j,j)\
d(l,e,l)\
d(i,c,i)\
k(348(%[cur]),c)\
d(h,i,h)\
d(j,l,j)\
a(G,b,l)\
a(H,b,b)\
a(F,f,i)\
a(G,f,f)\
d(b,h,b)\
d(l,j,l)\
a(132(%[cur]),c,j)\
a(F,c,c)\
d(i,l,i)\
d(f,b,f)\
d(j,i,j)\
d(c,f,c)\
k(352(%[cur]),f)\
a(t,j,i)\
a(t,c,b)\
k(356(%[cur]),l)\
a(q,i,i)\
a(q,b,b)\
d(i,j,i)\
d(b,c,b)\
k(360(%[cur]),c)\
s(r,i,i)\
a(V,f,j)\
a(228(%[cur]),f,f)\
w(u,b,i,b)\
k(364(%[cur]),i)\
a(U,l,h)\
o(p,b,e)\
a(V,l,l)\
x(e,b,e)\
k(368(%[cur]),b)\
d(h,j,h)\
d(l,f,l)\
a(T,c,f)\
a(U,c,c)\
k(372(%[cur]),j)\
a(T,i,m)\
a(S,i,i)\
d(c,l,c)\
d(f,h,f)\
k(376(%[cur]),h)\
a(R,b,l)\
d(i,f,i)\
a(S,b,b)\
d(m,c,m)\
k(380(%[cur]),c)\
d(l,i,l)\
d(b,m,b)\
a(Q,j,m)\
a(R,j,j)\
a(O,h,i)\
a(Q,h,h)\
d(m,l,m)\
d(j,b,j)\
a(196(%[cur]),c,b)\
a(O,c,c)\
d(i,m,i)\
d(h,j,h)\
d(b,i,b)\
d(c,h,c)\
a(t,b,h)\
a(t,c,i)\
v(g,n,j)\
o(p,j,m)\
x(m,j,m)\
a(q,h,h)\
a(q,i,i)\
v(p,n,n)\
k(Z,j)\
o(g,n,g)\
k(Z,n)\
s(r,g,l)\
d(h,b,h)\
d(i,c,i)\
s(r,h,h)\
k((%[iy]),c)\
w(u,i,h,i)\
o(p,i,h)\
v(p,e,b)\
x(h,i,h)\
o(h,b,b)\
v(h,e,h)\
k(_0,e)\
a(j,g,j)\
s(r,b,i)\
o(p,h,f)\
a(n,l,n)\
x(f,h,f)\
k(_0,h)\
k(4(%[iy]),l)\
s(r,j,j)\
y(c,g,c)\
w(u,n,j,n)\
v(f,m,j)\
y(q,n,n)\
o(p,j,g)\
x(g,j,g)\
z(g,(%[out]))\
a(e,b,e)\
o(n,c,n)\
a(h,i,h)\
v(p,m,m)\
k(A,i)\
y(l,b,l)\
o(f,m,f)\
s(r,e,e)\
k(A,m)\
s(r,f,b)\
w(u,h,e,h)\
k(D,e)\
y(q,h,h)\
o(h,l,h)\
v(h,n,l)\
v(p,n,n)\
a(i,f,i)\
o(p,l,c)\
k(A,g)\
o(h,n,h)\
a(m,b,m)\
x(c,l,c)\
k(A,l)\
s(r,h,b)\
y(e,f,e)\
z(c,32(%[out]))\
k(D,c)\
s(r,i,i)\
w(u,m,i,m)\
k(640(%[cur]),i)\
y(q,m,m)\
o(m,e,m)\
k(644(%[cur]),e)\
a(g,h,g)\
z(m,64(%[out]))\
a(l,b,l)\
y(c,h,c)\
k(648(%[cur]),h)\
s(r,g,g)\
a(_9,i,b)\
a(420(%[cur]),i,i)\
w(u,l,g,l)\
k(652(%[cur]),g)\
y(q,l,l)\
a(_8,e,m)\
a(_9,e,e)\
o(l,c,l)\
k(656(%[cur]),c)\
z(l,96(%[out]))\
a(_8,h,l)\
a(_7,h,h)\
d(e,i,e)\
d(m,b,m)\
k(660(%[cur]),b)\
a(_6,g,i)\
a(_7,g,g)\
d(h,m,h)\
d(l,e,l)\
k(664(%[cur]),e)\
d(i,h,i)\
d(g,l,g)\
a(_5,c,l)\
a(_6,c,c)\
k(668(%[cur]),h)\
a(_2,b,m)\
a(_5,b,b)\
d(l,i,l)\
d(c,g,c)\
a(_1,e,g)\
a(_2,e,e)\
d(m,l,m)\
d(b,c,b)\
a(388(%[cur]),h,c)\
a(_1,h,h)\
d(g,m,g)\
d(e,b,e)\
d(h,e,h)\
d(c,g,c)\
k(672(%[cur]),g)\
a(t,c,e)\
a(t,h,b)\
k(676(%[cur]),m)\
a(q,e,e)\
a(q,b,b)\
d(e,c,e)\
d(b,h,b)\
k(680(%[cur]),h)\
s(r,e,e)\
a(484(%[cur]),g,c)\
a(_17,g,g)\
w(u,b,e,b)\
k(684(%[cur]),e)\
a(_16,m,l)\
o(p,b,i)\
a(_17,m,m)\
x(i,b,i)\
k(688(%[cur]),b)\
d(l,g,l)\
d(m,c,m)\
a(_16,h,c)\
a(_15,h,h)\
k(692(%[cur]),g)\
a(_14,e,f)\
a(_15,e,e)\
d(c,m,c)\
d(h,l,h)\
k(696(%[cur]),l)\
a(_13,b,m)\
a(_14,b,b)\
d(f,h,f)\
d(e,c,e)\
k(700(%[cur]),c)\
d(m,f,m)\
d(b,e,b)\
a(_13,g,e)\
a(_12,g,g)\
a(_11,l,f)\
a(_12,l,l)\
d(g,m,g)\
d(e,b,e)\
a(452(%[cur]),c,b)\
a(_11,c,c)\
d(f,g,f)\
d(l,e,l)\
d(b,f,b)\
d(c,l,c)\
k(704(%[cur]),l)\
a(t,b,f)\
a(t,c,e)\
k(708(%[cur]),g)\
a(q,f,f)\
a(q,e,e)\
d(e,c,e)\
d(f,b,f)\
k(712(%[cur]),b)\
s(r,f,f)\
a(_24,l,c)\
a(548(%[cur]),l,l)\
w(u,e,f,e)\
k(716(%[cur]),f)\
a(_23,g,m)\
o(p,e,h)\
a(_24,g,g)\
x(h,e,h)\
k(720(%[cur]),e)\
d(m,c,m)\
d(g,l,g)\
a(_22,b,l)\
a(_23,b,b)\
k(724(%[cur]),c)\
a(_21,f,n)\
a(_22,f,f)\
d(l,m,l)\
d(b,g,b)\
k(728(%[cur]),g)\
a(_20,e,m)\
a(_21,e,e)\
d(n,l,n)\
d(f,b,f)\
k(732(%[cur]),b)\
d(m,n,m)\
d(e,f,e)\
a(_19,c,f)\
a(_20,c,c)\
a(_18,g,n)\
d(f,m,f)\
a(_19,g,g)\
d(c,e,c)\
a(516(%[cur]),b,e)\
a(_18,b,b)\
d(n,f,n)\
d(g,c,g)\
d(b,g,b)\
d(e,n,e)\
k(736(%[cur]),n)\
a(t,e,g)\
a(t,b,c)\
k(740(%[cur]),f)\
a(q,g,g)\
a(q,c,c)\
d(g,e,g)\
d(c,b,c)\
k(744(%[cur]),b)\
s(r,g,g)\
a(_31,n,e)\
a(612(%[cur]),n,n)\
w(u,c,g,c)\
k(748(%[cur]),g)\
a(_30,f,m)\
o(p,c,l)\
a(_31,f,f)\
x(l,c,l)\
k(752(%[cur]),c)\
d(m,e,m)\
d(f,n,f)\
a(_29,b,n)\
a(_30,b,b)\
k(756(%[cur]),e)\
a(_29,g,j)\
a(_28,g,g)\
d(n,m,n)\
d(b,f,b)\
k(760(%[cur]),f)\
a(_27,c,m)\
a(_28,c,c)\
d(g,n,g)\
d(j,b,j)\
k(764(%[cur]),b)\
d(m,g,m)\
d(c,j,c)\
a(_26,e,j)\
a(_27,e,e)\
a(_25,f,g)\
a(_26,f,f)\
d(j,m,j)\
d(e,c,e)\
a(580(%[cur]),b,c)\
a(_25,b,b)\
d(f,e,f)\
d(g,j,g)\
d(c,g,c)\
d(b,f,b)\
a(t,c,f)\
a(t,b,g)\
v(h,i,j)\
o(p,j,e)\
x(e,j,e)\
a(q,g,g)\
a(q,f,f)\
v(p,i,i)\
k(_4,j)\
o(h,i,h)\
k(_4,i)\
s(r,h,m)\
d(f,c,f)\
d(g,b,g)\
s(r,f,f)\
k(8(%[iy]),b)\
w(u,g,f,g)\
o(p,g,f)\
v(p,l,c)\
x(f,g,f)\
o(f,c,c)\
v(f,l,f)\
k(_10,l)\
a(j,h,j)\
s(r,c,g)\
o(p,f,n)\
a(i,m,i)\
x(n,f,n)\
k(_10,f)\
k(12(%[iy]),m)\
s(r,j,j)\
y(b,h,b)\
w(u,i,j,i)\
v(n,e,j)\
y(q,i,i)\
o(p,j,h)\
x(h,j,h)\
z(h,128(%[out]))\
a(l,c,l)\
o(i,b,i)\
a(f,g,f)\
v(p,e,e)\
k(B,g)\
y(m,c,m)\
o(n,e,n)\
s(r,l,l)\
k(B,e)\
s(r,n,c)\
w(u,f,l,f)\
k(_3,l)\
y(q,f,f)\
o(f,m,f)\
v(f,i,m)\
v(p,i,i)\
a(g,n,g)\
o(p,m,b)\
k(B,h)\
o(f,i,f)\
a(e,c,e)\
x(b,m,b)\
k(B,m)\
s(r,f,c)\
y(l,n,l)\
z(b,160(%[out]))\
k(_3,b)\
s(r,g,g)\
w(u,e,g,e)\
y(q,e,e)\
o(e,l,e)\
a(h,f,h)\
z(e,192(%[out]))\
a(m,c,m)\
y(b,f,b)\
s(r,h,h)\
w(u,m,h,m)\
y(q,m,m)\
o(m,b,m)\
z(m,224(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm6,%%ymm1,CP_FPS_ASM_VPADDQ,%%ymm3,%%ymm9,%%ymm4,%%ymm7,%%ymm5,%%ymm8,CP_FPS_ASM_VBROADCASTSS,%%ymm0,%%ymm10,%%ymm2,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm15,$32,CP_FPS_ASM_VPSRLQ,%%ymm13,$170,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VMOVDQA,32(%[ix]),36(%[ix]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),12(%[cur]),136(%[cur]),140(%[cur]),144(%[cur]),148(%[cur]),152(%[cur]),156(%[cur]),16(%[cur]),160(%[cur]),20(%[cur]),200(%[cur]),204(%[cur]),208(%[cur]),212(%[cur]),216(%[cur]),220(%[cur]),224(%[cur]),24(%[cur]),28(%[cur]),32(%[cur]),32(%[iy]),36(%[iy]),392(%[cur]),396(%[cur]),4(%[ix]),40(%[iy]),400(%[cur]),404(%[cur]),408(%[cur]),412(%[cur]),416(%[cur]),44(%[iy]),456(%[cur]),460(%[cur]),464(%[cur]),468(%[cur]),472(%[cur]),476(%[cur]),480(%[cur]),520(%[cur]),524(%[cur]),528(%[cur]),532(%[cur]),536(%[cur]),540(%[cur]),544(%[cur]),584(%[cur]),588(%[cur]),592(%[cur]),596(%[cur]),600(%[cur]),604(%[cur]),608(%[cur]),72(%[cur]),76(%[cur]),8(%[cur]),80(%[cur]),84(%[cur]),88(%[cur]),92(%[cur]),96(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

[[gnu::always_inline]] inline void both(const void* a, const void* b, void* buf, const void* x, const void* y, void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63) \
T(%[P],t) T(%[P2],v) T(%[NI],D) l(E,p)\
l(E,h)\
B(32(%[a]),m)\
l(K,g)\
l(E,d)\
l(256(%[cur]),n)\
l(E,j)\
l(K,k)\
B((%[a]),e)\
l(260(%[cur]),i)\
c(_61,p,p)\
c(100(%[a]),h,h)\
s(v,m,q)\
x(_61,g,g)\
c(_52,d,d)\
u(q,m,q)\
c(_18,n,m)\
c(36(%[cur]),n,n)\
y(w,p,p)\
l(264(%[cur]),f)\
C(A,h,p,h)\
c(68(%[a]),j,j)\
s(v,e,p)\
c(_17,i,o)\
c(_18,i,i)\
y(w,d,d)\
u(p,e,p)\
x(t,h,h)\
z(v,q,e)\
x(_52,k,k)\
C(A,j,d,j)\
l(268(%[cur]),d)\
r(o,m,o)\
r(i,n,i)\
x(t,j,j)\
l(F,n)\
s(h,g,h)\
l(272(%[cur]),g)\
z(h,q,q)\
s(h,e,h)\
l(F,e)\
y(w,q,m)\
s(j,k,j)\
c(_16,f,k)\
c(_17,f,f)\
r(k,o,k)\
r(f,i,f)\
c(_6,d,i)\
c(_16,d,d)\
l(276(%[cur]),o)\
c(n,q,n)\
r(i,k,i)\
r(d,f,d)\
c(_2,g,f)\
c(_6,g,g)\
l(L,k)\
c(e,m,e)\
z(j,p,m)\
s(j,p,j)\
l(280(%[cur]),p)\
y(w,n,n)\
r(f,i,f)\
r(g,d,g)\
l(H,d)\
l(H,i)\
C(A,e,n,e)\
l(284(%[cur]),n)\
x(t,e,e)\
x(k,q,k)\
c(V,o,q)\
c(_2,o,o)\
c(d,h,d)\
r(q,f,q)\
s(e,k,e)\
c(_56,p,k)\
c(V,p,p)\
c(4(%[cur]),n,f)\
c(_56,n,n)\
r(o,g,o)\
y(w,h,g)\
c(i,g,i)\
y(w,d,d)\
l(R,g)\
r(k,q,k)\
r(p,o,p)\
s(v,m,o)\
z(v,j,q)\
r(f,k,f)\
r(n,p,n)\
u(o,m,o)\
u(q,j,q)\
c(D,f,j)\
c(D,n,m)\
C(A,i,d,i)\
z(v,o,d)\
l(288(%[cur]),p)\
x(t,i,i)\
z(v,q,k)\
s(e,d,d)\
z(e,o,e)\
l(292(%[cur]),o)\
c(t,j,j)\
c(t,m,m)\
x(g,h,g)\
s(v,e,h)\
r(j,f,j)\
r(m,n,m)\
u(h,e,h)\
l(F,e)\
l(296(%[cur]),n)\
y(w,j,j)\
l(F,f)\
C(A,m,j,m)\
s(i,g,i)\
s(t,h,g)\
c(_63,p,j)\
c(100(%[cur]),p,p)\
s(i,k,k)\
z(i,q,i)\
l(300(%[cur]),q)\
u(g,h,g)\
c(_60,o,h)\
c(_63,o,o)\
B(g,32(%[buf]))\
r(h,j,h)\
r(o,p,o)\
c(e,g,e)\
s(v,m,p)\
l(L,j)\
u(p,m,p)\
l(304(%[cur]),m)\
y(w,e,e)\
x(j,g,j)\
y(w,g,g)\
c(f,g,f)\
c(_59,n,g)\
c(_60,n,n)\
r(g,h,g)\
C(A,f,e,f)\
l(308(%[cur]),e)\
c(_58,q,h)\
c(_59,q,q)\
r(n,o,n)\
x(t,f,f)\
s(v,d,o)\
u(o,d,o)\
l(312(%[cur]),d)\
r(h,g,h)\
c(_57,m,g)\
c(_58,m,m)\
r(q,n,q)\
s(f,j,f)\
l(F,j)\
l(F,n)\
r(g,h,g)\
r(m,q,m)\
s(t,o,q)\
s(t,f,h)\
u(q,o,q)\
u(h,f,h)\
l(316(%[cur]),f)\
c(_55,e,o)\
c(_57,e,e)\
B(h,(%[buf]))\
l(L,h)\
B(q,96(%[buf]))\
r(o,g,o)\
r(e,m,e)\
c(_54,d,m)\
c(_55,d,d)\
y(w,q,g)\
c(j,q,j)\
c(n,g,n)\
r(m,o,m)\
r(d,e,d)\
c(68(%[cur]),f,e)\
c(_54,f,f)\
y(w,j,j)\
s(v,i,o)\
C(A,n,j,n)\
u(o,i,o)\
x(h,q,h)\
x(t,n,n)\
l(320(%[cur]),q)\
r(e,m,e)\
r(f,d,f)\
s(t,o,d)\
c(D,e,m)\
c(D,f,i)\
u(d,o,d)\
l(324(%[cur]),o)\
l(H,j)\
y(w,d,g)\
s(n,h,n)\
B(d,160(%[buf]))\
l(H,h)\
c(t,m,m)\
c(t,i,i)\
r(m,e,m)\
r(i,f,i)\
l(328(%[cur]),f)\
s(t,n,e)\
y(w,m,m)\
u(e,n,e)\
c(_3,q,n)\
c(164(%[cur]),q,q)\
C(A,i,m,i)\
s(e,t,e)\
l(R,m)\
c(j,d,j)\
B(e,64(%[buf]))\
l(332(%[cur]),e)\
c(h,g,h)\
c(_1,o,g)\
c(_3,o,o)\
y(w,j,j)\
r(g,n,g)\
C(A,h,j,h)\
l(336(%[cur]),j)\
s(v,i,n)\
r(o,q,o)\
u(n,i,n)\
x(t,h,h)\
c(_0,f,i)\
c(_1,f,f)\
s(v,k,q)\
u(q,k,q)\
x(m,d,m)\
l(340(%[cur]),d)\
c(Z,e,k)\
c(_0,e,e)\
r(i,g,i)\
s(t,q,g)\
r(f,o,f)\
u(g,q,g)\
l(H,q)\
l(344(%[cur]),o)\
s(h,m,h)\
B(g,224(%[buf]))\
r(k,i,k)\
r(e,f,e)\
l(H,f)\
y(w,g,i)\
s(t,h,m)\
u(m,h,m)\
l(348(%[cur]),h)\
B(m,128(%[buf]))\
c(Y,j,m)\
c(Z,j,j)\
r(m,k,m)\
r(j,e,j)\
c(X,d,e)\
c(Y,d,d)\
l(R,k)\
c(q,g,q)\
r(e,m,e)\
r(d,j,d)\
c(f,i,f)\
c(W,o,i)\
c(X,o,o)\
c(132(%[cur]),h,j)\
y(w,q,q)\
c(W,h,h)\
B((%[b]),m)\
r(i,e,i)\
C(A,f,q,f)\
r(o,d,o)\
r(j,i,j)\
x(t,f,f)\
r(h,o,h)\
c(D,j,o)\
B(32(%[b]),i)\
l(352(%[cur]),d)\
c(D,h,q)\
x(k,g,k)\
l(E,g)\
c(t,o,o)\
c(t,q,q)\
l(E,e)\
s(f,k,f)\
l(356(%[cur]),k)\
r(o,j,o)\
s(v,m,j)\
r(q,h,q)\
y(w,o,o)\
u(j,m,j)\
s(t,f,m)\
l(360(%[cur]),h)\
C(A,q,o,q)\
l(K,o)\
u(m,f,m)\
c(_15,d,f)\
c(228(%[cur]),d,d)\
s(m,t,m)\
c(_53,g,g)\
B(m,192(%[buf]))\
s(v,q,m)\
u(m,q,m)\
l(364(%[cur]),q)\
c(68(%[b]),e,e)\
y(w,g,g)\
C(A,e,g,e)\
c(_12,k,g)\
c(_15,k,k)\
x(_53,o,o)\
x(t,e,e)\
r(g,f,g)\
r(k,d,k)\
l(368(%[cur]),d)\
c(_11,h,f)\
c(_12,h,h)\
s(e,o,e)\
c(_10,q,o)\
c(_11,q,q)\
r(f,g,f)\
l(E,g)\
r(h,k,h)\
r(o,f,o)\
l(372(%[cur]),f)\
l(E,k)\
r(q,h,q)\
l(K,h)\
c(_62,g,g)\
c(100(%[b]),k,k)\
x(_62,h,h)\
y(w,g,g)\
C(A,k,g,k)\
l(376(%[cur]),g)\
x(t,k,k)\
s(k,h,k)\
c(_9,d,h)\
c(_10,d,d)\
r(h,o,h)\
r(d,q,d)\
c(_8,f,q)\
c(_9,f,f)\
l(380(%[cur]),o)\
r(q,h,q)\
r(f,d,f)\
c(_7,g,d)\
c(_8,g,g)\
l(F,h)\
r(d,q,d)\
r(g,f,g)\
l(F,f)\
s(v,i,q)\
u(q,i,q)\
z(e,j,i)\
s(e,j,e)\
z(v,q,j)\
z(k,q,q)\
s(k,j,k)\
c(196(%[cur]),o,j)\
c(_7,o,o)\
r(j,d,j)\
r(o,g,o)\
c(h,q,h)\
l(L,g)\
y(w,q,d)\
c(f,d,f)\
c(D,j,d)\
y(w,h,h)\
c(t,d,d)\
C(A,f,h,f)\
c(D,o,h)\
x(t,f,f)\
c(t,h,h)\
r(d,j,d)\
x(g,q,g)\
l(H,q)\
l(H,j)\
y(w,d,d)\
r(h,o,h)\
s(f,g,f)\
z(v,p,g)\
y(w,k,o)\
C(A,h,d,h)\
s(n,g,g)\
z(n,p,n)\
l(R,p)\
s(v,i,d)\
u(d,i,d)\
l(_19,i)\
c(q,k,q)\
c(j,o,j)\
l(_19,o)\
y(w,q,q)\
C(A,j,q,j)\
x(p,k,p)\
s(v,h,k)\
y(w,g,q)\
u(k,h,k)\
c(i,g,i)\
x(t,j,j)\
l((%[iy]),h)\
c(o,q,o)\
y(w,i,i)\
s(j,p,j)\
s(v,n,p)\
z(v,m,q)\
z(k,m,m)\
u(p,n,p)\
z(v,e,n)\
s(k,q,k)\
u(n,e,n)\
l(_20,e)\
s(v,m,q)\
C(A,o,i,o)\
u(q,m,q)\
l(_20,m)\
y(w,k,i)\
x(t,o,o)\
x(h,g,h)\
z(v,d,g)\
z(f,d,d)\
s(f,g,f)\
l(4(%[iy]),g)\
s(o,h,o)\
z(v,n,h)\
z(j,n,n)\
s(j,h,j)\
s(v,d,h)\
c(e,k,e)\
u(h,d,h)\
s(v,f,d)\
c(m,i,m)\
u(d,f,d)\
s(v,n,f)\
z(q,p,i)\
u(f,n,f)\
s(v,j,n)\
y(w,e,e)\
u(n,j,n)\
s(t,h,j)\
C(A,m,e,m)\
x(g,k,g)\
u(j,h,j)\
s(t,d,h)\
x(t,m,m)\
z(v,p,p)\
s(t,f,k)\
s(v,i,e)\
B(j,256(%[buf]))\
B(128(%[a]),j)\
s(q,p,q)\
s(t,n,p)\
u(h,d,h)\
u(k,f,k)\
B(160(%[a]),f)\
y(w,q,d)\
u(e,i,e)\
u(p,n,p)\
B(h,288(%[buf]))\
l(G,h)\
l(N,n)\
s(m,g,m)\
B(e,(%[out]))\
l(G,e)\
l(N,g)\
z(m,o,i)\
B(k,320(%[buf]))\
l(U,k)\
B(p,352(%[buf]))\
z(v,o,o)\
s(v,i,p)\
s(m,o,m)\
u(p,i,p)\
l(Q,i)\
l(N,o)\
B(p,32(%[out]))\
s(v,j,p)\
u(p,j,p)\
s(v,f,j)\
c(_4,h,h)\
u(j,f,j)\
c(n,q,n)\
l(N,f)\
c(196(%[a]),e,e)\
c(g,d,g)\
l(G,d)\
x(k,q,k)\
l(G,q)\
y(w,h,h)\
y(w,n,n)\
C(A,e,h,e)\
x(_4,i,i)\
C(A,g,n,g)\
l(U,n)\
y(w,m,h)\
x(t,e,e)\
c(o,m,o)\
x(t,g,g)\
s(e,i,e)\
c(f,h,f)\
y(w,o,o)\
l(640(%[cur]),h)\
l(Q,i)\
c(_13,d,d)\
s(g,k,g)\
l(644(%[cur]),k)\
c(228(%[a]),q,q)\
B(g,64(%[out]))\
z(e,p,g)\
s(e,p,e)\
C(A,f,o,f)\
l(648(%[cur]),o)\
s(v,g,p)\
y(w,d,d)\
u(p,g,p)\
x(n,m,n)\
x(t,f,f)\
C(A,q,d,q)\
z(v,j,d)\
z(v,e,m)\
l(652(%[cur]),g)\
x(t,q,q)\
u(m,e,m)\
l(I,e)\
s(f,n,f)\
c(_29,h,n)\
c(420(%[cur]),h,h)\
B(f,96(%[out]))\
l(656(%[cur]),f)\
x(_13,i,i)\
s(q,i,q)\
c(_28,k,i)\
c(_29,k,k)\
z(q,j,j)\
s(q,d,q)\
c(_27,o,d)\
c(_28,o,o)\
r(i,n,i)\
r(k,h,k)\
c(e,j,e)\
l(660(%[cur]),h)\
c(_26,g,n)\
c(_27,g,g)\
r(d,i,d)\
r(o,k,o)\
l(I,k)\
y(w,j,i)\
r(n,d,n)\
y(w,e,e)\
l(664(%[cur]),d)\
r(g,o,g)\
c(_25,f,o)\
c(_26,f,f)\
r(o,n,o)\
r(f,g,f)\
l(J,g)\
l(J,n)\
c(k,i,k)\
l(668(%[cur]),i)\
C(A,k,e,k)\
c(_22,h,e)\
c(_25,h,h)\
x(t,k,k)\
r(e,o,e)\
r(h,f,h)\
c(g,q,g)\
c(_21,d,f)\
l(S,o)\
c(_22,d,d)\
r(f,e,f)\
y(w,g,g)\
c(388(%[cur]),i,e)\
c(_21,i,i)\
r(d,h,d)\
l(M,h)\
r(e,f,e)\
r(i,d,i)\
y(w,q,d)\
z(v,p,f)\
c(n,d,n)\
c(D,e,d)\
x(o,j,o)\
c(D,i,j)\
c(t,d,d)\
C(A,n,g,n)\
l(672(%[cur]),g)\
c(t,j,j)\
x(t,n,n)\
s(k,o,k)\
z(v,m,o)\
x(h,q,h)\
s(k,f,f)\
z(k,p,k)\
l(676(%[cur]),p)\
r(d,e,d)\
s(v,k,e)\
r(j,i,j)\
y(w,d,d)\
l(I,i)\
u(e,k,e)\
l(680(%[cur]),k)\
C(A,j,d,j)\
s(n,h,n)\
l(I,h)\
s(t,e,d)\
s(v,j,q)\
s(n,o,o)\
z(n,m,n)\
u(d,e,d)\
l(684(%[cur]),e)\
u(q,j,q)\
l(S,j)\
y(w,d,m)\
B(d,416(%[buf]))\
c(i,d,i)\
c(h,m,h)\
c(_37,g,m)\
c(484(%[cur]),g,g)\
y(w,i,i)\
x(j,d,j)\
l(688(%[cur]),d)\
C(A,h,i,h)\
c(_36,p,i)\
c(_37,p,p)\
x(t,h,h)\
r(i,m,i)\
r(p,g,p)\
c(_35,k,g)\
c(_36,k,k)\
l(692(%[cur]),m)\
s(h,j,h)\
c(_34,e,j)\
c(_35,e,e)\
r(g,i,g)\
r(k,p,k)\
s(v,f,p)\
s(t,h,i)\
u(p,f,p)\
u(i,h,i)\
l(696(%[cur]),h)\
c(_33,d,f)\
c(_34,d,d)\
r(j,g,j)\
r(e,k,e)\
B(i,384(%[buf]))\
l(I,i)\
s(t,p,k)\
l(700(%[cur]),g)\
u(k,p,k)\
r(f,j,f)\
l(I,j)\
l(S,p)\
B(k,480(%[buf]))\
r(d,e,d)\
y(w,k,e)\
c(i,k,i)\
c(j,e,j)\
x(p,k,p)\
c(_32,m,k)\
c(_33,m,m)\
y(w,i,i)\
c(_31,h,e)\
c(_32,h,h)\
C(A,j,i,j)\
c(452(%[cur]),g,i)\
c(_31,g,g)\
r(k,f,k)\
r(m,d,m)\
x(t,j,j)\
s(v,n,d)\
r(e,k,e)\
r(h,m,h)\
u(d,n,d)\
l(704(%[cur]),n)\
r(i,e,i)\
r(g,h,g)\
s(t,d,h)\
c(D,i,e)\
c(D,g,m)\
u(h,d,h)\
s(j,p,j)\
l(708(%[cur]),p)\
l(J,d)\
y(w,h,k)\
s(t,j,f)\
B(h,544(%[buf]))\
u(f,j,f)\
l(J,j)\
c(t,e,e)\
c(t,m,m)\
s(f,t,f)\
B(f,448(%[buf]))\
l(712(%[cur]),f)\
r(e,i,e)\
r(m,g,m)\
l(M,g)\
c(_44,n,i)\
c(548(%[cur]),n,n)\
y(w,e,e)\
C(A,m,e,m)\
l(716(%[cur]),e)\
c(d,h,d)\
c(j,k,j)\
c(_43,p,k)\
c(_44,p,p)\
y(w,d,d)\
r(k,i,k)\
C(A,j,d,j)\
l(720(%[cur]),d)\
c(_42,f,i)\
c(_43,f,f)\
r(p,n,p)\
x(g,h,g)\
s(v,m,h)\
u(h,m,h)\
s(v,o,m)\
x(t,j,j)\
l(724(%[cur]),n)\
r(i,k,i)\
u(m,o,m)\
c(_41,e,o)\
c(_42,e,e)\
r(f,p,f)\
s(t,m,p)\
l(J,k)\
u(p,m,p)\
s(j,g,j)\
l(728(%[cur]),g)\
l(J,m)\
B(p,608(%[buf]))\
r(o,i,o)\
r(e,f,e)\
y(w,p,f)\
s(t,j,i)\
u(i,j,i)\
l(732(%[cur]),j)\
B(i,512(%[buf]))\
c(_40,d,i)\
c(_41,d,d)\
r(i,o,i)\
r(d,e,d)\
c(_39,n,e)\
c(_40,n,n)\
l(M,o)\
c(k,p,k)\
c(m,f,m)\
c(_38,g,f)\
c(_39,g,g)\
r(e,i,e)\
r(n,d,n)\
c(516(%[cur]),j,d)\
c(_38,j,j)\
y(w,k,k)\
r(f,e,f)\
B(128(%[b]),e)\
C(A,m,k,m)\
r(g,n,g)\
x(t,m,m)\
r(d,f,d)\
r(j,g,j)\
B(160(%[b]),g)\
c(D,d,f)\
c(D,j,n)\
l(736(%[cur]),k)\
l(G,i)\
x(o,p,o)\
c(t,f,f)\
c(t,n,n)\
l(740(%[cur]),p)\
s(m,o,m)\
l(G,o)\
r(f,d,f)\
r(n,j,n)\
s(v,e,j)\
s(t,m,d)\
y(w,f,f)\
u(j,e,j)\
u(d,m,d)\
s(v,g,m)\
l(744(%[cur]),e)\
C(A,n,f,n)\
l(Q,f)\
u(m,g,m)\
s(d,t,d)\
c(_51,k,g)\
c(612(%[cur]),k,k)\
B(d,576(%[buf]))\
l(748(%[cur]),d)\
c(_5,i,i)\
c(196(%[b]),o,o)\
y(w,i,i)\
C(A,o,i,o)\
x(_5,f,f)\
c(_50,p,i)\
c(_51,p,p)\
x(t,o,o)\
r(i,g,i)\
l(752(%[cur]),g)\
r(p,k,p)\
s(o,f,o)\
c(_49,e,f)\
c(_50,e,e)\
c(_48,d,k)\
c(_49,d,d)\
r(f,i,f)\
r(e,p,e)\
l(G,p)\
l(G,i)\
r(k,f,k)\
r(d,e,d)\
l(756(%[cur]),e)\
l(Q,f)\
c(_14,p,p)\
c(228(%[b]),i,i)\
x(_14,f,f)\
y(w,p,p)\
C(A,i,p,i)\
l(760(%[cur]),p)\
x(t,i,i)\
s(i,f,i)\
c(_47,g,f)\
c(_48,g,g)\
r(f,k,f)\
r(g,d,g)\
c(_46,e,d)\
c(_47,e,e)\
l(764(%[cur]),k)\
r(d,f,d)\
r(e,g,e)\
c(_45,p,g)\
c(_46,p,p)\
s(v,n,f)\
u(f,n,f)\
l(I,n)\
r(g,d,g)\
r(p,e,p)\
l(I,e)\
z(v,m,d)\
z(i,m,m)\
s(i,d,i)\
y(w,m,d)\
c(n,m,n)\
c(e,d,e)\
c(580(%[cur]),k,d)\
c(_45,k,k)\
y(w,n,n)\
r(d,g,d)\
C(A,e,n,e)\
l(S,n)\
z(o,j,g)\
s(o,j,o)\
r(k,p,k)\
c(D,d,p)\
y(w,i,j)\
x(t,e,e)\
c(t,p,p)\
r(p,d,p)\
c(D,k,d)\
y(w,p,p)\
x(n,m,n)\
l(J,m)\
c(t,d,d)\
s(e,n,e)\
z(v,q,n)\
z(h,q,q)\
s(h,n,h)\
l(J,n)\
r(d,k,d)\
y(w,h,k)\
C(A,d,p,d)\
l(_24,p)\
c(m,i,m)\
y(w,m,m)\
c(n,j,n)\
l(_24,j)\
c(p,h,p)\
C(A,n,m,n)\
s(v,d,m)\
u(m,d,m)\
x(t,n,n)\
l(M,d)\
y(w,p,p)\
c(j,k,j)\
l(8(%[iy]),k)\
C(A,j,p,j)\
s(v,q,p)\
x(t,j,j)\
u(p,q,p)\
x(d,i,d)\
z(m,f,i)\
z(v,f,f)\
s(v,i,q)\
s(m,f,m)\
u(q,i,q)\
l(_30,i)\
s(v,g,f)\
u(f,g,f)\
s(n,d,n)\
l(_30,d)\
y(w,m,g)\
x(k,h,k)\
z(v,o,h)\
u(h,o,h)\
z(v,f,o)\
z(e,f,f)\
s(e,o,e)\
l(12(%[iy]),o)\
s(j,k,j)\
z(v,h,k)\
z(n,h,h)\
s(n,k,n)\
s(v,f,k)\
u(k,f,k)\
s(v,e,f)\
c(i,m,i)\
u(f,e,f)\
s(v,h,e)\
c(d,g,d)\
u(e,h,e)\
z(q,p,h)\
s(t,k,g)\
u(g,k,g)\
s(v,n,k)\
y(w,i,i)\
u(k,n,k)\
B(g,640(%[buf]))\
s(t,f,g)\
s(t,e,n)\
C(A,d,i,d)\
x(o,m,o)\
z(v,p,p)\
u(g,f,g)\
l(O,f)\
l(O,m)\
x(t,d,d)\
s(q,p,q)\
s(v,h,p)\
s(t,k,i)\
B(g,672(%[buf]))\
l(_23,g)\
u(n,e,n)\
u(p,h,p)\
u(i,k,i)\
z(v,j,k)\
y(w,q,h)\
B(p,128(%[out]))\
l(O,p)\
l(O,e)\
s(d,o,d)\
B(n,704(%[buf]))\
l(_23,n)\
s(d,k,k)\
z(d,j,d)\
B(i,736(%[buf]))\
y(w,k,i)\
s(v,d,j)\
u(j,d,j)\
B(j,160(%[out]))\
c(f,q,f)\
c(m,h,m)\
x(g,q,g)\
c(p,k,p)\
c(e,i,e)\
y(w,f,f)\
C(A,m,f,m)\
x(n,k,n)\
x(t,m,m)\
y(w,p,p)\
C(A,e,p,e)\
x(t,e,e)\
s(m,g,m)\
B(m,192(%[out]))\
s(e,n,e)\
B(e,224(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm4,%%ymm8,%%ymm11,%%ymm3,%%ymm1,%%ymm9,%%ymm6,%%ymm7,CP_FPS_ASM_VBROADCASTSS,%%ymm2,%%ymm5,%%ymm12,%%ymm0,%%ymm10,CP_FPS_ASM_VPADDQ,CP_FPS_ASM_VPSUBD,%%ymm15,CP_FPS_ASM_VPMINUD,%%ymm14,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPBLENDD,%%ymm13,32(%[x]),32(%[y]),36(%[x]),36(%[y]),40(%[y]),44(%[y]),(%[x]),(%[y]),12(%[y]),32(%[ix]),36(%[ix]),4(%[x]),4(%[y]),8(%[y]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),12(%[cur]),136(%[cur]),140(%[cur]),144(%[cur]),148(%[cur]),152(%[cur]),156(%[cur]),16(%[cur]),160(%[cur]),192(%[a]),192(%[b]),20(%[cur]),200(%[cur]),204(%[cur]),208(%[cur]),212(%[cur]),216(%[cur]),220(%[cur]),224(%[a]),224(%[b]),224(%[cur]),24(%[cur]),28(%[cur]),32(%[cur]),32(%[iy]),36(%[iy]),392(%[cur]),396(%[cur]),4(%[ix]),40(%[iy]),400(%[cur]),404(%[cur]),408(%[cur]),412(%[cur]),416(%[cur]),44(%[iy]),456(%[cur]),460(%[cur]),464(%[cur]),468(%[cur]),472(%[cur]),476(%[cur]),480(%[cur]),520(%[cur]),524(%[cur]),528(%[cur]),532(%[cur]),536(%[cur]),540(%[cur]),544(%[cur]),584(%[cur]),588(%[cur]),592(%[cur]),596(%[cur]),600(%[cur]),604(%[cur]),608(%[cur]),64(%[a]),64(%[b]),72(%[cur]),76(%[cur]),8(%[cur]),80(%[cur]),84(%[cur]),88(%[cur]),92(%[cur]),96(%[a]),96(%[b]),96(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

}
namespace top_kernels {

using ntt::kernels::k2P;
using ntt::kernels::kP;
using ntt::kernels::Vec;

inline void inverse_top(Vec* a, std::size_t q, const Vec* w) {
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],v) S(%[P2],j) ".p2align 5\n\t" "1:\n\t" \
A((%[b]),n)\
A(K,f)\
A((%[a]),g)\
A(J,t)\
o(j,n,e)\
o(I,n,n)\
c(I,e,e)\
o(j,f,d)\
c(j,n,r)\
o(H,f,f)\
z(u,e,h)\
y(L,e,m)\
c(H,d,d)\
o(j,g,k)\
y(L,h,h)\
z(u,d,s)\
y(M,d,i)\
o(j,t,p)\
y(M,s,s)\
x(128(%[w]),e,e)\
q(r,n,r)\
c(j,f,n)\
z(u,m,m)\
o(G,g,g)\
c(G,k,k)\
q(n,f,n)\
C(B,h,m,h)\
z(u,i,i)\
o(F,t,t)\
c(F,p,p)\
x(v,h,h)\
C(B,s,i,s)\
x(192(%[w]),d,d)\
z(u,g,i)\
x(v,s,s)\
y(D,g,m)\
z(u,k,f)\
z(u,t,l)\
y(D,i,i)\
y(D,f,f)\
y(D,l,l)\
c(h,e,h)\
x(E,g,g)\
z(u,p,e)\
y(Q,e,e)\
c(s,d,s)\
z(u,m,m)\
y(D,k,d)\
C(B,i,m,i)\
x(E,k,k)\
y(D,t,m)\
x(v,i,i)\
x(E,t,t)\
z(u,d,d)\
z(u,m,m)\
C(B,f,d,f)\
y(Q,p,d)\
x(64(%[w]),p,p)\
C(B,l,m,l)\
x(v,f,f)\
c(i,g,i)\
o(j,r,g)\
o(j,h,m)\
o(n,r,r)\
c(n,g,n)\
x(v,l,l)\
z(u,d,d)\
c(s,m,m)\
z(u,r,g)\
o(s,h,s)\
C(B,e,d,e)\
y(D,r,d)\
z(u,n,h)\
y(D,g,g)\
x(v,e,e)\
y(N,h,h)\
c(f,k,f)\
c(l,t,l)\
y(N,n,t)\
z(u,m,k)\
x(E,r,r)\
y(O,k,k)\
z(u,d,d)\
x(256(%[w]),n,n)\
c(e,p,e)\
C(B,g,d,g)\
y(O,m,d)\
o(l,i,p)\
z(u,t,t)\
c(l,i,l)\
x(v,g,g)\
x(320(%[w]),m,m)\
C(B,h,t,h)\
o(e,f,t)\
c(e,f,e)\
c(j,p,f)\
z(u,d,d)\
x(v,h,h)\
c(j,t,i)\
q(f,p,f)\
C(B,k,d,k)\
o(j,l,d)\
o(j,e,p)\
q(i,t,i)\
x(v,k,k)\
q(d,l,d)\
q(p,e,p)\
c(j,s,e)\
c(g,r,g)\
q(e,s,e)\
c(h,n,h)\
o(j,f,n)\
o(j,i,s)\
o(j,d,r)\
o(j,p,l)\
o(g,f,f)\
c(k,m,k)\
c(g,n,g)\
o(e,i,i)\
c(e,s,e)\
o(h,d,d)\
c(h,r,h)\
o(k,p,p)\
c(k,l,k)\
c(j,f,l)\
c(j,g,r)\
c(j,i,s)\
c(j,e,n)\
c(j,d,m)\
c(j,h,t)\
q(l,f,l)\
q(r,g,r)\
c(j,p,g)\
c(j,k,f)\
q(s,i,s)\
q(n,e,n)\
q(m,d,m)\
q(t,h,t)\
q(g,p,g)\
q(f,k,f)\
c(v,l,k)\
c(v,r,p)\
c(v,s,h)\
c(v,n,d)\
c(v,m,e)\
c(v,t,i)\
q(k,l,k)\
q(p,r,p)\
c(v,g,r)\
c(v,f,l)\
q(h,s,h)\
q(d,n,d)\
A(k,(%[a]))\
q(e,m,e)\
q(i,t,i)\
q(r,g,r)\
q(l,f,l)\
A(p,(%[b]))\
A(h,G)\
A(d,I)\
A(e,J)\
A(i,K)\
A(r,F)\
A(l,H)\
R(u,%[a])\
R(u,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm5,%%ymm4,%%ymm1,%%ymm2,%%ymm7,%%ymm11,%%ymm14,%%ymm9,%%ymm13,%%ymm8,%%ymm0,CP_FPS_ASM_VPADDD,%%ymm12,CP_FPS_ASM_VPMINUD,%%ymm6,%%ymm10,%%ymm3,$32,%%ymm15,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,$170,CP_FPS_ASM_VPBLENDD,32(%[w]),(%[w]),(%[a],%[h3]),(%[a],%[h]),(%[b],%[h3]),(%[b],%[h]),(%[a],%[h],2),(%[b],%[h],2),160(%[w]),224(%[w]),288(%[w]),352(%[w]),96(%[w]),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}

using ntt::detail::add;
using ntt::detail::diff;
using ntt::detail::Factor;
using ntt::detail::kP;
using ntt::detail::reduce;
using ntt::detail::slot;
using ntt::detail::Vec;

inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }

inline Vec diff_canonical(Vec x, Vec y) {
	return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y);
}



inline void forward_radix8(Vec* f, std::size_t q, const std::uint32_t* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (std::size_t j = 0; j < q; ++j) {
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



class Subtrees {
public:
	Subtrees(const std::uint32_t* roots, const std::uint32_t* inverse_roots) : r_(roots), ir_(inverse_roots) {}

	void visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
		switch (nv) {
		case 16: return tile<16>(a, b, k);
		case 64: return tile<64>(a, b, k);
		case 256: return tile<256>(a, b, k);
		}
		const std::size_t h = nv / 4;
		forward(a, b, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + t * h, b + t * h, h, 4 * k + t);
		inverse(a, h, k);
	}

private:
	template <std::size_t NV>
	[[gnu::noinline]] void tile(Vec* a, Vec* b, std::size_t k) const {
		for (std::size_t h = NV / 4; h >= 4; h /= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) forward(a + j, b + j, h, g);
		bottom(a, b, NV, k * (NV / 4));
		for (std::size_t h = 4; h < NV; h *= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) inverse(a + j, h, g);
	}

	void forward(Vec* a, Vec* b, std::size_t h, std::size_t k) const {
		if (k == 0) {
			ntt::kernels::forward_identity(a, h, r_);
			ntt::kernels::forward_identity(b, h, r_);
			return;
		}
		const std::uint32_t *x = r_ + slot(k), *y = r_ + slot(2 * k);
		if (h == 4) return ntt::kernels::forward_pair(a, b, h, x, y);
		ntt::kernels::forward(a, h, x, y);
		ntt::kernels::forward(b, h, x, y);
	}

	void inverse(Vec* a, std::size_t h, std::size_t k) const {
		if (k == 0) return ntt::kernels::inverse_identity(a, h, ir_);
		ntt::kernels::inverse(a, h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	struct alignas(64) Leaves {
		std::uint32_t window[4][16];
		std::uint32_t coefficients[4][8];
	};

	
	void bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {
		Leaves leaves[2][2]{};
		bottom_kernels::first(a, b, leaves[0], r_ + slot(first), r_ + slot(2 * first));
		for (std::size_t j = 0, k = first;; j += 8, k += 2) {
			const std::size_t cur = j / 8 % 2, next = cur ^ 1;
			const std::uint32_t *ix = ir_ + slot(k), *iy = ir_ + slot(2 * k);
			if (j + 8 == nv) return bottom_kernels::last(a + j, leaves[cur], ix, iy);
			bottom_kernels::both(a + j + 8, b + j + 8, leaves[next], r_ + slot(k + 2), r_ + slot(2 * k + 4), a + j,
								 leaves[cur], ix, iy);
		}
	}

	const std::uint32_t *r_, *ir_;
};


class Product {
public:
	static bool fits(std::size_t n, std::size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= std::size_t(1) << lg;
	}

	Product(std::size_t n, std::size_t m) : Product(n, m, nullptr) {}
	Product(std::size_t n, std::size_t m, std::uint32_t* borrowed_a) : lg_(log_length(n, m)) {
		const std::size_t len = length(), tables = ntt::detail::table_words(lg_);
		const std::size_t words = (borrowed_a ? 1 : 2) * (len + kPadding) + 2 * tables + (borrowed_a ? 1024 : 0);
		storage_ = allocate_fps_words(words);
		a_ = borrowed_a ? borrowed_a : storage_.get();
		const std::size_t color = ((reinterpret_cast<std::uintptr_t>(a_) >> 6) - (reinterpret_cast<std::uintptr_t>(storage_.get()) >> 6) + 1) & 63;
		b_ = storage_.get() + (borrowed_a ? 16 * color : len + kPadding);
		roots_ = b_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
		std::fill(roots_, inverse_roots_ + tables, 0);
	}

	Product(const Product&) = delete;
	Product& operator=(const Product&) = delete;

	std::uint32_t* a() { return a_; }
	std::uint32_t* b() { return b_; }


	
	const std::uint32_t* multiply() {
		using namespace ntt::detail;
		const std::size_t len = length(), nv = len / 8, q = nv / 8;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		auto* a = reinterpret_cast<Vec*>(a_);
		auto* b = reinterpret_cast<Vec*>(b_);
		forward_radix8(a, q, roots_);
		forward_radix8(b, q, roots_);

		const Subtrees subtrees(roots_, inverse_roots_);
		for (std::size_t c = 0; c < 8; ++c) subtrees.visit(a + c * q, b + c * q, q, c);
		const std::uint32_t s = power(std::uint32_t(nv), kP - 2);
		const std::uint32_t *z0 = inverse_roots_ + 1, *x1 = inverse_roots_ + slot(1), *y1 = inverse_roots_ + slot(2);
		const std::uint32_t twiddles[6] = {s, multiply_mod(s, *z0), multiply_mod(s, y1[0]), multiply_mod(s, y1[1]),
										   multiply_mod(s, *x1), *x1};
		alignas(32) Vec w[12];
		for (int i = 0; i < 6; ++i) w[2 * i] = broadcast(twiddles[i]), w[2 * i + 1] = broadcast(quotient(twiddles[i]));
		top_kernels::inverse_top(a, q, w);
		return a_;
	}

private:
	static constexpr std::size_t kPadding = 16;

	static int log_length(std::size_t n, std::size_t m) { return std::max(6, int(std::bit_width(n + m - 2))); }
	std::size_t length() const { return std::size_t(1) << lg_; }

	int lg_;
	FpsWords storage_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
};




inline void copy_input(const std::vector<Mint> &in, std::uint32_t *out, std::size_t len) {
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - Mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<Mint>& in, Mint* out) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = Mint::word(std::min(in[i].v, in[i].v - Mint::mod()));
}

template <class Plan>
inline std::vector<Mint> run_borrowed_as(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	const std::size_t len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr std::size_t alignment = 32;
	const std::size_t words = len + 16 + alignment / sizeof(Mint) - 1, size = a.size() + b.size() - 1;
	std::vector<Mint> result;
	prepare_fps_output(result, words);
	result.assign(words, Mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const std::size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(Mint);
	Mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<std::uint32_t*>(to));
	copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply();
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(Mint));
	result.resize(size);
	return result;
}

inline std::vector<Mint> run_borrowed(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<Mint> run(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<Mint> result;
	const std::size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}

inline std::vector<Mint> multiply(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	if (Product::fits(a.size(), b.size())) return run_borrowed(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}

}
#pragma GCC pop_options
#endif


#line 1 "components/fps_division_blocked.hpp"



#line 6 "components/fps_division_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

inline std::size_t quotient_size(std::size_t n, std::size_t m) { return n >= m ? n - m + 1 : 0; }
inline std::size_t remainder_size(std::size_t n, std::size_t m) { return std::min(n, m - 1); }

namespace division_detail {
using namespace detail;

inline constexpr std::size_t kDivisionTerms = 8;

inline constexpr std::size_t kLongDivision = std::size_t(1) << 12;

inline constexpr std::size_t kDirectRemainder = 64;

inline constexpr std::array<double, 26> kTransformNs = {0,    0,    0,    0,    0,    0,    0.45, 0.48, 0.51,
														0.55, 0.6,  0.65, 0.7,  0.76, 0.81, 0.88, 0.95, 1.02,
														1.09, 1.17, 1.23, 1.3,  1.37, 1.44, 1.51, 1.58};
inline constexpr double kLeafProductNs = 0.92, kDirectTermNs = 0.04, kFoldNs = 0.25;

inline std::size_t transform_length(std::size_t n) { return std::max<std::size_t>(64, std::bit_ceil(n)); }

inline double transform_ns(std::size_t len) {
	const auto lg = std::countr_zero(len);
	return std::size_t(lg) < kTransformNs.size() ? kTransformNs[lg] * double(len) : std::numeric_limits<double>::infinity();
}
inline double cyclic_ns(std::size_t len) { return 2 * transform_ns(len) + kLeafProductNs * double(len); }

inline double inverse_ns(std::size_t n) {
	double ns = 0;
	for (std::size_t k = kInverseBase; k < n; k *= 2) ns += transform_ns(2 * k) + 2 * cyclic_ns(2 * k);
	return ns;
}

struct QuotientPlan {
	std::size_t s = 0, blocks = 0, pad = 0, windows = 0, tail = 0;
	bool tails = false;
	double ns = std::numeric_limits<double>::infinity();

	QuotientPlan() = default;

	QuotientPlan(std::size_t k, std::size_t d, std::size_t block)
		: s(block), blocks((k + block - 1) / block), pad(blocks * block - k), tail(transform_length(2 * d)),
		  tails(d < block) {
		const std::size_t len = 2 * s;
		ns = inverse_ns(precision(k)) + transform_ns(len) + double(blocks) * cyclic_ns(len);
		if (blocks == 1) return;
		if (tails) {
			ns += transform_ns(tail) + double(blocks - 1) * cyclic_ns(tail);
			return;
		}
		const std::size_t reach = d / s + 1;
		windows = std::min(reach, blocks - 1);
		if (windows > kDivisionTerms) {
			ns = std::numeric_limits<double>::infinity();
			return;
		}
		double terms = 0;
		for (std::size_t j = 1; j < blocks; ++j) terms += double(std::min(j, reach));
		ns += double(windows + 2 * (blocks - 1)) * transform_ns(len) + terms * kLeafProductNs * double(len);
	}

	std::size_t precision(std::size_t k) const { return blocks == 1 ? k : s; }
};

enum class Remainder { kDirect, kWrap, kSplit, kTail };

struct RemainderPlan {
	Remainder method;
	std::size_t length = 0;
	double ns = std::numeric_limits<double>::infinity();

	RemainderPlan(Remainder how, std::size_t n, std::size_t k, std::size_t d, const QuotientPlan& quotient) : method(how) {
		const std::size_t wrap = transform_length(d);
		switch (how) {
			case Remainder::kDirect:
				if (k <= kDirectRemainder) ns = double(k) * double(d) * kDirectTermNs;
				break;
			case Remainder::kWrap:
				length = wrap;
				ns = cyclic_ns(length) + transform_ns(length) + double(n + k) * kFoldNs;
				break;
			case Remainder::kSplit:
				length = wrap / 2;
				if (length >= 64 && d > length && k + (d - length) <= length + 1)
					ns = transform_ns(length) + 2 * cyclic_ns(length) + double(n + d) * kFoldNs;
				break;
			case Remainder::kTail:
				if (!quotient.tails) break;
				ns = (quotient.blocks == 1 ? transform_ns(quotient.tail) : 0) + cyclic_ns(quotient.tail) + double(d) * kFoldNs;
				break;
		}
	}

	std::size_t buffers() const { return method == Remainder::kWrap ? 2 : method == Remainder::kSplit ? 3 : 0; }
};

struct DivisionPlan {
	std::size_t n, m, k, d;
	bool long_division;
	QuotientPlan quotient;
	RemainderPlan remainder;
	int lg = Transform::kMinLog;

	DivisionPlan(std::size_t n_, std::size_t m_)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(k * d <= kLongDivision),
		  remainder(Remainder::kWrap, n, k, d, quotient) {
		if (long_division) return;
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t s = 32;; s *= 2) {
			const QuotientPlan p(k, d, s);
			for (Remainder how : {Remainder::kDirect, Remainder::kWrap, Remainder::kSplit, Remainder::kTail}) {
				const RemainderPlan r(how, n, k, d, p);
				if (p.ns + r.ns < best) best = p.ns + r.ns, quotient = p, remainder = r;
			}
			if (s >= k || 4 * s > (std::size_t(1) << Transform::kMaxLog)) break;
		}
		set_log();
	}

	DivisionPlan(std::size_t n_, std::size_t m_, std::size_t s, Remainder how)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(false), quotient(k, d, s),
		  remainder(how, n, k, d, quotient) {
		set_log();
	}

	bool tails() const { return quotient.tails && (quotient.blocks > 1 || remainder.method == Remainder::kTail); }

	void set_log() { lg = std::countr_zero(std::max({2 * quotient.s, tails() ? quotient.tail : 0, remainder.length})); }

	std::size_t divisor() const {
		const QuotientPlan& p = quotient;
		return tails() ? m : std::min(m, std::max(p.precision(k), (p.windows + 1) * p.s));
	}

	std::size_t quotient_words() const {
		const QuotientPlan& p = quotient;
		const std::size_t len = 2 * p.s;
		std::size_t later = Arena::footprint(p.blocks * p.s + p.s);
		if (tails()) later += 2 * Arena::footprint(p.tail);
		if (p.windows) later += (2 * p.windows + 1) * Arena::footprint(len);
		return Arena::footprint(divisor()) + Arena::footprint(len) + std::max(inverse_scratch(p.precision(k)), later);
	}

	std::size_t scratch() const { return std::max(quotient_words(), remainder.buffers() * Arena::footprint(remainder.length)); }

	std::size_t words() const {
		return long_division ? Arena::footprint(n) : Transform::words(lg) + Arena::footprint(scratch());
	}
};

class Bump {
public:
	explicit Bump(std::span<std::uint32_t> memory) : memory_(memory) {}

	std::span<std::uint32_t> take(std::size_t n) {
		if (used_ > memory_.size() || Arena::footprint(n) > memory_.size() - used_) throw std::bad_alloc();
		const std::span<std::uint32_t> s = memory_.subspan(used_, n);
		used_ += Arena::footprint(n);
		return s;
	}

	std::span<std::uint32_t> rest() const { return memory_.subspan(used_); }

private:
	std::span<std::uint32_t> memory_;
	std::size_t used_ = 0;
};

inline Vec montgomery_reduce(Vec even, Vec odd) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

inline std::uint32_t input_word(std::uint32_t x) { return x; }
inline std::uint32_t input_word(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec load_any(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline Vec load_any(const Mint* p) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	Vec value;
	std::memcpy(&value, p, sizeof(value));
	return reduce(value, kP);
}
inline void store_any(std::uint32_t* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }
inline Vec reversed(Vec x) { return _mm256_permutevar8x32_epi32(x, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }

inline void append_words(std::vector<Mint>& out, const std::uint32_t* values, std::size_t n, std::uint32_t factor = 1) {
	const Factor scale(factor);
	std::size_t i = 0;
	for (; i + 8 <= n; i += 8) {
		Vec word = load_any(values + i);
		if (factor != 1) word = times(word, scale);
		Mint block[8];
		static_assert(sizeof(block) == sizeof(word) && std::is_trivially_copyable_v<Mint>);
		std::memcpy(static_cast<void*>(block), &word, sizeof(block));
		out.insert(out.end(), block, block + 8);
	}
	for (; i < n; ++i) out.emplace_back(Mint::word(factor == 1 ? values[i] : blocked_ntt::detail::multiply_mod(values[i], factor)));
}

inline void append_division_words(std::vector<Mint>& out, const std::uint32_t* values, std::size_t n, std::uint32_t factor = 1) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	const Factor scale(factor);
	std::array<Mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if (factor == 1) std::memcpy(static_cast<void*>(batch.data()), values + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (std::size_t j = 0; j < 64; j += 8) {
				const Vec word = times(load_any(values + i + j), scale);
				std::memcpy(static_cast<void*>(batch.data() + j), &word, sizeof(word));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	if (i < n) append_words(out, values + i, n - i, factor);
}

template<class T>
inline void subtract(const T* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) store_any(out + i, difference(load_any(a + i), load_any(b + i)));
	for (; i < count; ++i) out[i] = (input_word(a[i]) + kP - b[i]) % kP;
}

template<class T>
inline void reverse_difference(std::span<T> a, std::ptrdiff_t top, std::uint32_t* out, std::size_t count,
							   const std::uint32_t* sub = nullptr) {
	const std::ptrdiff_t size = std::ptrdiff_t(a.size());
	const auto value = [&](std::size_t i) {
		const std::ptrdiff_t p = top - std::ptrdiff_t(i);
		const std::uint32_t x = p >= 0 && p < size ? input_word(a[std::size_t(p)]) : 0;
		return sub ? (x + kP - sub[i]) % kP : x;
	};
	std::size_t i = top >= size ? std::min(count, std::size_t(top - size + 1)) : 0;
	if (sub) {
		for (std::size_t j = 0; j < i; ++j) out[j] = value(j);
	} else {
		std::fill_n(out, i, 0);
	}
	for (; i + 8 <= count && top - std::ptrdiff_t(i) >= 7; i += 8) {
		const Vec x = reversed(load_any(a.data() + (top - std::ptrdiff_t(i) - 7)));
		store_any(out + i, sub ? difference(x, load_any(sub + i)) : x);
	}
	for (; i < count; ++i) out[i] = value(i);
}

template<class T>
inline void fold(std::span<T> a, std::span<std::uint32_t> out) {
	const std::size_t len = out.size();
	std::fill(out.begin(), out.end(), 0);
	for (std::size_t from = 0; from < a.size(); from += len) {
		const std::size_t count = std::min(len, a.size() - from), full = count / 8 * 8;
		const T* x = a.data() + from;
		for (std::size_t j = 0; j < full; j += 8) {
			store(out.data() + j, reduce(add(load(out.data() + j), load_any(x + j)), kP));
		}
		for (std::size_t j = full; j < count; ++j) out[j] = (out[j] + input_word(x[j])) % kP;
	}
}

template<class T>
inline void long_division(std::span<const T> f, std::span<const std::uint32_t> g, std::span<std::uint32_t> q,
						  std::span<std::uint32_t> r, std::span<std::uint32_t> rest) {
	using blocked_ntt::detail::multiply_mod;
	const std::size_t d = g.size() - 1;
	if constexpr (std::is_same_v<T, std::uint32_t>) std::copy(f.begin(), f.end(), rest.begin());
	else for (std::size_t i = 0; i < f.size(); ++i) rest[i] = input_word(f[i]);
	const std::uint32_t lead = blocked_ntt::detail::power(g[d], kP - 2);
	for (std::size_t i = q.size(); i-- > 0;) {
		const std::uint32_t c = multiply_mod(rest[i + d], lead);
		q[i] = c;
		for (std::size_t t = 0; t < d; ++t) rest[i + t] = (rest[i + t] + kP - multiply_mod(c, g[t])) % kP;
	}
	std::copy_n(rest.begin(), r.size(), r.begin());
}

template<class T>
inline void remainder_direct(std::span<const T> f, std::span<const std::uint32_t> g,
							 std::span<const std::uint32_t> q, std::span<std::uint32_t> r) {
	const std::size_t d = r.size(), k = q.size();
	Vec factor[kDirectRemainder];
	for (std::size_t i = 0; i < k; ++i) factor[i] = _mm256_set1_epi64x(std::int64_t((std::uint64_t(q[i]) << 32) % kP));
	const auto scalar = [&](std::size_t j) {
		std::uint64_t sum = 0;
		for (std::size_t i = 0; i <= std::min(j, k - 1); ++i) sum = (sum + std::uint64_t(q[i]) * g[j - i]) % kP;
		r[j] = std::uint32_t((input_word(f[j]) + kP - sum) % kP);
	};
	std::size_t j = 0;
	for (; j < std::min(d, k); ++j) scalar(j);
	for (; j + 8 <= d; j += 8) {
		Vec total = load_any(f.data() + j);
		for (std::size_t i = 0; i < k; i += 4) {
			Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
			for (std::size_t t = i; t < std::min(k, i + 4); ++t) {
				const Vec x = load_any(g.data() + (j - t));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, factor[t]));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), factor[t]));
			}
			total = reduce(_mm256_sub_epi32(add(total, broadcast(2 * kP)), montgomery_reduce(even, odd)), 2 * kP);
		}
		store_any(r.data() + j, reduce(total, kP));
	}
	for (; j < d; ++j) scalar(j);
}

template<class T>
inline void remainder_wrap(const Transform& t, std::span<const T> f, std::span<const std::uint32_t> g,
						   std::span<const std::uint32_t> q, std::span<std::uint32_t> r, std::span<std::uint32_t> a,
						   std::span<std::uint32_t> b) {
	const std::size_t len = a.size();
	if (q.size() <= len) {
		t.forward(q, 0, a);
	} else {
		fold(q, a);
		t.forward(a);
	}
	std::span<const std::uint32_t> gl = g;
	if (g.size() > len) {
		std::copy_n(g.begin(), len, b.begin());
		b[0] = (b[0] + g[len]) % kP;
		gl = b;
	}
	t.cyclic_product(gl, 0, b, a);
	fold(f, a);
	subtract(a.data(), b.data(), r.data(), r.size());
}

template<class T>
inline void remainder_split(const Transform& t, std::span<const T> f, std::span<const std::uint32_t> g,
							std::span<const std::uint32_t> q, std::span<std::uint32_t> r, std::span<std::uint32_t> a,
							std::span<std::uint32_t> b, std::span<std::uint32_t> c) {
	const std::size_t len = a.size(), k = q.size(), e = r.size() - len;
	t.forward(q, 0, a);
	t.cyclic_product(g.subspan(len + 1 - k, k + e - 1), 0, c, a);
	subtract(f.data() + len, c.data() + (k - 1), r.data() + len, e);
	fold(g, b);
	t.cyclic_product(b, a);
	fold(f, c);
	subtract(c.data(), b.data(), r.data(), len);
	subtract(r.data(), r.data() + len, r.data(), e);
}

template <std::size_t K>
void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, std::span<std::uint32_t> out) {
	InverseProductSumBottom<K> bottom{};
	for (std::size_t i = 0; i < K; ++i)
		bottom.terms[i] = {{t.roots(), t.inverse_roots(), pairs[i].b.data()}, pairs[i].a.data()};
	t.inverse_with(bottom, out, Half::kUpper);
}

inline void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, std::span<std::uint32_t> out) {
	switch (pairs.size()) {
		case 1:
		case 2:
		case 3: return t.inverse_product_sum(pairs, out, Half::kUpper);
		case 4: return residual_sum<4>(t, pairs, out);
		case 5: return residual_sum<5>(t, pairs, out);
		case 6: return residual_sum<6>(t, pairs, out);
		case 7: return residual_sum<7>(t, pairs, out);
		case 8: return residual_sum<8>(t, pairs, out);
		default: std::abort();
	}
}

template<class T>
inline void subtract_reversed(const T* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {
	std::size_t j = 0;
	for (; j + 8 <= count; j += 8) store_any(out + j, difference(load_any(a + j), reversed(load_any(b + count - 8 - j))));
	for (; j < count; ++j) out[j] = (input_word(a[j]) + kP - b[count - 1 - j]) % kP;
}

template<class T>
inline void divide_blocks(const Transform& t, const DivisionPlan& plan, std::span<const T> f,
						  std::span<const std::uint32_t> g, std::span<std::uint32_t> q, std::span<std::uint32_t> r,
						  std::span<std::uint32_t> scratch) {
	const QuotientPlan& p = plan.quotient;
	const std::size_t k = q.size(), s = p.s, len = 2 * s, d = plan.d;
	const std::ptrdiff_t top = std::ptrdiff_t(f.size() - 1 + p.pad);
	Bump bump(scratch);
	const std::span<std::uint32_t> G = bump.take(plan.divisor()), ht = bump.take(len);
	std::reverse_copy(g.end() - std::ptrdiff_t(G.size()), g.end(), G.begin());
	const std::size_t precision = p.precision(k);
	inverse(t, G.first(std::min(G.size(), precision)), ht.first(precision), bump.rest());
	t.forward(ht.first(precision), 0, ht);

	const std::span<std::uint32_t> qs = bump.take(p.blocks * s + s);
	std::span<std::uint32_t> gt, mp, work;
	std::array<std::span<std::uint32_t>, kDivisionTerms> windows{}, ring{};
	if (plan.tails()) {
		gt = bump.take(p.tail), mp = bump.take(p.tail);
		t.forward(G, 0, gt);
	}
	if (p.windows) {
		work = bump.take(len);
		for (std::size_t i = 0; i < p.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(G.subspan(i * s, std::min(len, G.size() - i * s)), 0, windows[i]);
		}
	}

	const auto tail_product = [&](std::size_t j) {
		t.cyclic_product(qs.subspan(j * s - d, d), p.tail - 2 * d, mp, gt, Half::kUpper);
		return mp.data() + (p.tail - d);
	};
	for (std::size_t j = 0; j < p.blocks; ++j) {
		std::uint32_t* const block = qs.data() + j * s;
		const std::ptrdiff_t from = top - std::ptrdiff_t(j * s);
		if (j == 0) {
			reverse_difference(f, from, block, s);
		} else if (p.tails) {
			reverse_difference(f, from, block, d, tail_product(j));
			reverse_difference(f, from - std::ptrdiff_t(d), block + d, s - d);
		} else {
			const std::size_t terms = std::min(j, d / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (std::size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % p.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			reverse_difference(f, from, block, s, work.data() + s);
		}
		t.cyclic_product(qs.subspan(j * s, s), 0, qs.subspan(j * s, len), ht, Half::kLower);
		if (p.windows && j + 1 < p.blocks) t.forward(qs.subspan(j * s, s), 0, ring[j % p.windows]);

		reverse_difference(qs.first((j + 1) * s), std::ptrdiff_t((j + 1) * s - 1), q.data() + (p.blocks - 1 - j) * s,
						   j == 0 ? s - p.pad : s);
	}

	if (plan.remainder.method == Remainder::kTail) subtract_reversed(f.data(), tail_product(p.blocks), r.data(), d);
}

template<class T>
inline void divide(Arena& arena, const DivisionPlan& plan, std::span<const T> f, std::span<const std::uint32_t> g,
				   std::span<std::uint32_t> q, std::span<std::uint32_t> r) {
	if (plan.long_division) return long_division(f, g, q, r, arena.take(f.size()));
	const Transform t(arena, plan.lg);
	const std::span<std::uint32_t> scratch = arena.take(plan.scratch());
	divide_blocks(t, plan, f, g, q, r, scratch);
	Bump bump(scratch);
	const std::size_t len = plan.remainder.length;
	switch (plan.remainder.method) {
		case Remainder::kDirect: return remainder_direct(f, g, q, r);
		case Remainder::kWrap: return remainder_wrap(t, f, g, q, r, bump.take(len), bump.take(len));
		case Remainder::kSplit: return remainder_split(t, f, g, q, r, bump.take(len), bump.take(len), bump.take(len));
		case Remainder::kTail: return;
	}
}

}

inline std::size_t divide_words(std::size_t n, std::size_t m) { return division_detail::DivisionPlan(n, m).words(); }

inline void divide(Arena& arena, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
				   std::span<std::uint32_t> q, std::span<std::uint32_t> r) {
	const division_detail::DivisionPlan plan(f.size(), g.size());
	if (plan.k == 0) {
		std::copy(f.begin(), f.end(), r.begin());
		return;
	}
	division_detail::divide(arena, plan, f, g, q, r);
}

}


namespace fps_detail::blocked_poly {
inline void native_divide(const Mint* a, std::size_t n, const Mint* b, std::size_t m,
						  std::vector<Mint>& quotient, std::vector<Mint>& remainder) {
	const division_detail::DivisionPlan plan(n, m);
	const std::size_t k = quotient_size(n, m), d = remainder_size(n, m);
	Arena arena(plan.words() + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));
	const std::span<const Mint> f(a, n);
	auto g = arena.take(m), q = arena.take(k), r = arena.take(d);
	for (std::size_t i = 0; i < m; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	division_detail::divide(arena, plan, f, g, q, r);
	prepare_fps_output(quotient, k), prepare_fps_output(remainder, d);
	division_detail::append_division_words(quotient, q.data(), q.size(), blocked_ntt::detail::kR);
	division_detail::append_division_words(remainder, r.data(), r.size());
}

inline void native_div_series(const Mint* a, std::size_t input, const Mint* b, std::size_t divisor,
							  std::vector<Mint>& out, std::size_t n) {
	using namespace division_detail;
	const std::size_t count = std::min(divisor, n), degree = count - 1;
	QuotientPlan plan;
	for (std::size_t s = 32;; s *= 2) {
		QuotientPlan candidate(n, degree, s);
		if (candidate.ns < plan.ns) plan = candidate;
		if (s >= n || 4 * s > (std::size_t(1) << Transform::kMaxLog)) break;
	}
	const std::size_t s = plan.s, len = 2 * s;
	const bool tails = plan.tails && plan.blocks > 1;
	const std::size_t precision = plan.precision(n), numerator = std::min(input, n);
	const int lg = std::countr_zero(std::max(len, tails ? plan.tail : 0));
	const std::size_t later = Arena::footprint(plan.blocks * s + s) +
		(tails ? 2 * Arena::footprint(plan.tail) : 0) +
		(plan.windows ? (2 * plan.windows + 1) * Arena::footprint(len) : 0);
	Arena arena(Transform::words(lg) + Arena::footprint(numerator) + Arena::footprint(count) +
		Arena::footprint(len) + Arena::footprint(std::max(inverse_scratch(precision), later)));
	const Transform t(arena, lg);
	auto f = arena.take(numerator), g = arena.take(count), ht = arena.take(len);
	for (std::size_t i = 0; i < numerator; ++i) f[i] = std::min(a[i].v, a[i].v - Mint::mod());
	for (std::size_t i = 0; i < count; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	auto scratch = arena.take(std::max(inverse_scratch(precision), later));
	inverse(t, g.first(std::min(count, precision)), ht.first(precision), scratch);
	t.forward(ht.first(precision), 0, ht);
	Bump bump(scratch);
	auto qs = bump.take(plan.blocks * s + s);
	std::span<std::uint32_t> gt, mp, work;
	std::array<std::span<std::uint32_t>, kDivisionTerms> windows{}, ring{};
	if (tails) {
		gt = bump.take(plan.tail), mp = bump.take(plan.tail);
		t.forward(g, 0, gt);
	}
	if (plan.windows) {
		work = bump.take(len);
		for (std::size_t i = 0; i < plan.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(g.subspan(i * s, std::min(len, count - i * s)), 0, windows[i]);
		}
	}
	prepare_fps_output(out, n);
	for (std::size_t j = 0; j < plan.blocks; ++j) {
		auto block = qs.subspan(j * s, len);
		const std::size_t from = j * s, available = from < numerator ? std::min(s, numerator - from) : 0;
		if (available) std::copy_n(f.data() + from, available, block.data());
		std::fill(block.begin() + available, block.begin() + s, 0u);
		if (j && tails) {
			t.cyclic_product(qs.subspan(j * s - degree, degree), plan.tail - 2 * degree, mp, gt, Half::kUpper);
			subtract(block.data(), mp.data() + plan.tail - degree, block.data(), degree);
		} else if (j) {
			const std::size_t terms = std::min(j, degree / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (std::size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % plan.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			subtract(block.data(), work.data() + s, block.data(), s);
		}
		t.cyclic_product(block.first(s), 0, block, ht, Half::kLower);
		if (plan.windows && j + 1 < plan.blocks) t.forward(block.first(s), 0, ring[j % plan.windows]);
		append_division_words(out, block.data(), std::min(s, n - from), blocked_ntt::detail::kR);
	}
}
}

#pragma GCC pop_options
#endif

#line 24 "fps/fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <class T = fps_mint>
struct FormalPowerSeries : std::vector<T>, private fps_detail::Operations<T> {
	static_assert(T::mod() == 998244353, "FormalPowerSeries supports only mod 998244353");
	using W = FormalPowerSeries<fps_detail::Mint>;
	using base = std::vector<T>;
	using F = FormalPowerSeries;
	using base::base;

	FormalPowerSeries() = default;
	FormalPowerSeries(base v) : base(std::move(v)) {}
	F &operator=(base v) {
		base::operator=(std::move(v));
		return *this;
	}
	F &operator=(std::initializer_list<T> v) {
		base::operator=(v);
		return *this;
	}

	static F multiply(const F &a, const F &b) {
		if (a.empty() || b.empty()) return {};
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			W x = import_work(a);
			return export_work(&a == &b ? W::multiply(x, x) : W::multiply(x, import_work(b)));
		}
		if (std::min(a.size(), b.size()) <= 32) {
			F r(a.size() + b.size() - 1);
			const F &small = a.size() < b.size() ? a : b;
			const F &large = a.size() < b.size() ? b : a;
			if constexpr (std::is_same_v<T, fps_detail::Mint>) {
				for (int j = 0; j < static_cast<int>(small.size()); ++j)
					fps_detail::NTT::add_scaled(words(r) + j, words(large), large.size(), small[j].val());
			} else
				for (int j = 0; j < static_cast<int>(small.size()); ++j)
					for (int i = 0; i < static_cast<int>(large.size()); ++i) r[i + j] += large[i] * small[j];
			return r;
		}
		int n = 1, size = a.size() + b.size() - 1;
		while (n < size) n <<= 1;
		assert(n <= (1 << 23));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (fps_detail::NTT::vectorized()) return F(fps_detail::multiply_detail::multiply(a, b));
		}
#endif
		F x = a, y;
		x.resize(n);
		if (&a != &b) {
			y = b;
			y.resize(n);
		}
		if (n >= 64 && &a != &b) {
			auto &t = transform(n / 16);
			t.convolution(words(x), words(y), n);
			x.resize(size);
			t.scale(words(x), size, T::mod() - (T::mod() - 1) / (n / 8));
			return x;
		}
		ntt(x);
		if (&a == &b) pointwise(x, x, x, n);
		else {
			ntt(y);
			pointwise(x, x, y, n);
		}
		intt(x);
		x.resize(size);
		return x;
	}

	F operator-() const {
		F r = *this;
		for (T &x : r) x = -x;
		return r;
	}
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
		return *this;
	}
	F &operator+=(T x) {
		if (this->empty()) this->resize(1);
		(*this)[0] += x;
		return *this;
	}
	F &operator-=(T x) { return *this += -x; }
	F &operator*=(const F &g) { return *this = multiply(*this, g); }
	F &operator*=(T x) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			scale_words(*this, x);
			return *this;
		}
		for (T &a : *this) a *= x;
		return *this;
	}
	F &operator/=(const F &g) { return *this = div_series(g); }
	F &operator%=(const F &g) { return *this = divmod(g).second; }
	F &operator/=(T x) { return *this *= x.inv(); }

	F &operator<<=(int d) {
		assert(d >= 0);
		const int n = static_cast<int>(this->size());
		if (d >= n) return *this = F(n);
		this->insert(this->begin(), d, T(0));
		this->resize(n);
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		const int n = static_cast<int>(this->size());
		if (d >= n) return *this = F(n);
		this->erase(this->begin(), this->begin() + d);
		this->resize(n);
		return *this;
	}

	F operator+(const F &g) const { return F(*this) += g; }
	F operator-(const F &g) const { return F(*this) -= g; }
	F operator*(const F &g) const { return multiply(*this, g); }
	F operator/(const F &g) const { return div_series(g); }
	F operator%(const F &g) const { return divmod(g).second; }
	F operator+(T x) const {
		F r(*this);
		r += x;
		return r;
	}
	F operator-(T x) const {
		F r(*this);
		r -= x;
		return r;
	}
	F operator*(T x) const {
		F r(*this);
		r *= x;
		return r;
	}
	F operator/(T x) const {
		F r(*this);
		r /= x;
		return r;
	}
	F operator<<(int d) const { return F(*this) <<= d; }
	F operator>>(int d) const { return F(*this) >>= d; }

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min<int>(n, this->size()));
	}

	F div_series(const F &g, int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).div_series(import_work(g), deg));
		if (deg < 0) deg = this->size();
		assert(deg >= 0);
		if (!deg) return {};
		assert(!g.empty() && g[0] != T(0));
		if (this->empty()) return F(deg);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 129 && deg <= (1 << 23) && g.size() >= 64 && fps_detail::NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_div_series(this->data(), this->size(), g.data(), g.size(), r, deg);
				return r;
			}
		}
#endif
		if (deg <= 128 || g.size() < 64) {
			F r = multiply(pre(deg), g.inv(deg)).pre(deg);
			r.resize(deg);
			return r;
		}
		int size = 1;
		while (size < deg) size <<= 1;
		if (deg < size - size / 4) size >>= 1;
		const int block = size / 32, length = 2 * block, pitch = length + 16, count = (deg + block - 1) / block;
		F inverse = g.inv(block), r = multiply(pre(block), inverse).pre(block);
		r.resize(deg);
		inverse.resize(length);
		ntt_half(inverse);
		const Word norm = T::mod() - (T::mod() - 1) / length;
		transform(1).scale(words(inverse), length, norm);
		auto storage = fps_detail::allocate_fps_words((2 * count - 1) * pitch);
		Word *denominator = storage.get(), *quotient = denominator + count * pitch;
		for (int i = 0; i < count; ++i) {
			Word *h = denominator + i * pitch;
			const int available = std::max(0, std::min<int>(block, static_cast<int>(g.size()) - i * block));
			if (available) std::copy_n(words(g) + i * block, available, h);
			std::fill(h + available, h + block, 0u);
			ntt_half(h, length);
		}
		O::join_windows_strided(denominator, length, count, pitch);
		base error(length);
		for (int i = 1; i < count; ++i) {
			const int offset = i * block, n = std::min(block, deg - offset);
			Word *q = quotient + (i - 1) * pitch;
			std::copy_n(words(r) + offset - block, block, q);
			ntt_half(q, length);
			fps_detail::NTT::scale_normalize(q, length, norm);

			for (int j = 0; j < i; j += 7) {
				const Word *a[7], *b[7];
				int terms = std::min(7, i - j);
				for (int k = 0; k < terms; ++k) {
					a[k] = quotient + (j + k) * pitch;
					b[k] = denominator + (i - j - k) * pitch;
				}
				auto update = [&](auto number) {
					constexpr int count = decltype(number)::value;
					if (j == 0) O::template accumulate_products<count, true>(words(error), a, b, length);
					else O::template accumulate_products<count, false>(words(error), a, b, length);
				};
				if (terms == 7) update(std::integral_constant<int, 7>{});
				else if (terms == 6) update(std::integral_constant<int, 6>{});
				else if (terms == 5) update(std::integral_constant<int, 5>{});
				else if (terms == 4) update(std::integral_constant<int, 4>{});
				else if (terms == 3) update(std::integral_constant<int, 3>{});
				else if (terms == 2) update(std::integral_constant<int, 2>{});
				else update(std::integral_constant<int, 1>{});
			}
			ntt_inverse(error);
			for (int j = 0; j < block; ++j) error[j] = (offset + j < static_cast<int>(this->size()) ? (*this)[offset + j] : T(0)) - error[j];
			ntt_half(error);
			pointwise(error, error, inverse, length);
			ntt_inverse(error);
			std::copy_n(error.begin(), n, r.begin() + offset);
		}
		return r;
	}

	F inv(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).inv(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] != T(0));
		F g{(*this)[0].inv()};
		auto terms = sparse_terms(deg, 32);
		int stride = terms.size() <= 32 ? prepare_sparse(terms) : 1;
		if (terms.size() <= 32 && (deg <= 256 || terms.size() <= 12ULL * stride)) {
			g.resize(deg);
			if (terms.size() == 1 && stride == 1) {
				std::fill(g.begin(), g.end(), g[0]);
				geometric_scale(g, T(1), -terms[0].second * g[0]);
				return g;
			}
			for (long long i = stride; stride && i < deg; i += stride) g[i] = -sparse_dot(terms, g, static_cast<int>(i)) * g[0];
			return g;
		}

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 512 && deg <= (1 << 23) && fps_detail::NTT::vectorized()) {
				F result;
				const int block = 1 << (std::bit_width(static_cast<unsigned>(deg)) - 1), tail = deg - block;
				if (tail > 0 && tail <= 96) {
					fps_detail::blocked_poly::native_inverse(this->data(), this->size(), result, block);
					result.resize(deg);
					for (int i = block; i < deg; ++i)
						result[i] = -result[0] * dot(this->data() + 1, result.data() + i - 1, std::min<int>(i, this->size() - 1));
					return result;
				}
				fps_detail::blocked_poly::native_inverse(this->data(), this->size(), result, deg);
				return result;
			}
		}
#endif

		if (deg > 256 && this->size() >= 64) {
#if defined(__x86_64__) || defined(__i386__)
			if (deg < 1024 || !fps_detail::NTT::vectorized()) return F{1}.div_series(*this, deg);
#else
			return F{1}.div_series(*this, deg);
#endif
		}
		g.resize(std::min(deg, 32));
		for (int i = 1; i < static_cast<int>(g.size()); ++i)
			g[i] =
				-g[0] * dot(this->data() + 1, g.data() + i - 1, std::min<int>(i, this->size() - 1));
#if defined(__x86_64__) || defined(__i386__)
		fps_detail::FpsWords storage;
		Word *a = nullptr, *b = nullptr;
		if (deg > 96 && fps_detail::NTT::vectorized()) {
			int capacity = 64;
			while (capacity < deg && capacity < (1 << 23)) capacity <<= 1;
			storage = fps_detail::allocate_fps_words(2 * capacity);
			a = storage.get();
			b = a + capacity;
			g.reserve(capacity);
			transform(std::max(32, capacity / 16));
		}
#endif
		while (static_cast<int>(g.size()) < deg) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (m >= 32 && (deg - m <= 32 || (m >= 512 && deg - m <= 96))) {
				g.resize(deg);
				for (int i = m; i < deg; ++i) {
					const int count = std::min<int>(i, this->size() - 1);
					g[i] = -g[0] * dot(this->data() + 1, g.data() + i - 1, count);
				}
				break;
			}
#if defined(__x86_64__) || defined(__i386__)
			if (m >= 64 && n <= (1 << 23) && fps_detail::NTT::vectorized()) {
				int lo = std::min<int>(n, this->size());
				std::copy_n(words(*this), lo, a);
				std::fill(a + lo, a + n, Word(0));
				std::copy_n(words(g), m, b);
				transform(std::max(1, n / 16)).inverse_newton8(a, b, n);
				g.resize(n);
				std::copy_n(a + m, m, words(g) + m);

			} else
#endif
			{
				base f = this->pre(n), r = g;
				f.resize(n);
				r.resize(n);
				ntt(f);
				ntt_half(r);
				pointwise(f, f, r, n);
				ntt_inverse(f);
				std::fill(f.begin(), f.begin() + m, T(0));
				ntt(f);
				pointwise(f, f, r, n);
				ntt_inverse(f);
				const T inv = T::raw(T::mod() - (T::mod() - 1) / n);
				const T scale = -inv * inv;
				g.resize(n);
				transform(1).scale(words(f) + m, m, scale.val());
				std::copy_n(f.begin() + m, m, g.begin() + m);
			}
		}
		g.resize(deg);
		return g;
	}

	F sqrt(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).sqrt(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		int last = static_cast<int>(std::min<std::size_t>(deg, this->size())), first = 0;
		while (first < last && (*this)[first] == T(0)) ++first;
		if (first == last) return F(deg);
		if (first & 1) return {};
		const T constant = mod_sqrt((*this)[first]);
		if (constant == T(0)) return {};
		while ((*this)[last - 1] == T(0)) --last;
		if (first + 1 == last) {
			F result(deg);
			result[first / 2] = constant;
			return result;
		}
		const int shift = first / 2, target = deg - shift;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (target >= 256 && target <= (1 << 23) && fps_detail::NTT::vectorized()) {
				F result;
				fps_detail::blocked_poly::native_sqrt(this->data() + first, last - first, result, target, constant);
				result.insert(result.begin(), shift, T(0));
				return result;
			}
		}
#endif
		F f(this->begin() + first, this->begin() + last);
		f.resize(target);
		const T half = T(2).inv(), scale = half / constant;
		F g(std::min(target, 32));
		g[0] = constant;
		for (int i = 1; i < static_cast<int>(g.size()); ++i) {
			T sum = 0;
			for (int j = 1; j < i; ++j) sum += g[j] * g[i - j];
			g[i] = (f[i] - sum) * scale;
		}
		if (target > 256) {
			int size = 1;
			while (size < target) size <<= 1;
			if (target < size - size / 4) size >>= 1;
			const int block = size / 32, length = 2 * block, pitch = length + 16, count = (target + block - 1) / block;
			g = f.pre(block).sqrt(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word half_norm = T::mod() - (T::mod() - 1) / (2 * length);
			const int levels = __builtin_ctz((unsigned int)length);
			T root_norm = T::raw(T::mod() - (T::mod() - 1) / (1 << (levels / 2)));
			if (levels & 1) root_norm *= T::raw(441024591);
			transform(1).scale(words(inverse), length, half_norm);
			auto storage = fps_detail::allocate_fps_words((count - 1) * pitch);
			Word *cache = storage.get();
			base error(length);
			base zero(length);
			g.resize(target);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, target - offset);
				Word *q = cache + (i - 1) * pitch;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, root_norm.val());

				for (int j = 0; j < (i + 1) / 2; j += 6) {
					const Word *a[6], *b[6], *c[6];
					int terms = std::min(6, (i + 1) / 2 - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * pitch;
						b[k] = (j + k ? cache + (i - j - k) * pitch : words(zero));
						c[k] = cache + (i - j - k - 1) * pitch;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						if (j == 0) {
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, true, true>(words(error) + block, a, b, c, block, true);
						} else {
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error), a, b, c, block, false);
							fps_detail::NTT::template accumulate_wide<count, false, true>(words(error) + block, a, b, c, block, true);
						}
					};
					if (terms == 6) update(std::integral_constant<int, 6>{});
					else if (terms == 5) update(std::integral_constant<int, 5>{});
					else if (terms == 4) update(std::integral_constant<int, 4>{});
					else if (terms == 3) update(std::integral_constant<int, 3>{});
					else if (terms == 2) update(std::integral_constant<int, 2>{});
					else update(std::integral_constant<int, 1>{});
				}
				fps_detail::NTT::square_fold(words(error), cache + (i / 2) * pitch, block, i & 1);
				ntt_inverse(error);
				for (int j = 0; j < block; ++j) error[j] = (offset + j < target ? f[offset + j] : T(0)) - error[j];
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				std::copy_n(error.begin(), n, g.begin() + offset);
			}
			g.insert(g.begin(), shift, T(0));
			return g;
		}
		F h;
		base cache;
		if (static_cast<int>(g.size()) < target) {
			h = g.pre(16).inv(16);
			cache = h;
			cache.resize(32);
			ntt(cache);
		}
		while (static_cast<int>(g.size()) < target) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (target - m <= 32) {
				g.resize(target);
				for (int i = m; i < target; ++i)
					g[i] = (f[i] - dot(g.data() + 1, g.data() + i - 1, i - 1)) * scale;
				break;
			}
			assert(n <= (1 << 23));
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			pointwise(error, a, a, n);
			ntt_inverse(error);
			std::fill(error.begin(), error.begin() + m, T(0));
			for (int i = m; i < n; ++i) error[i] = (i < target ? f[i] : T(0)) - error[i] * in;
			ntt(error);
			pointwise(error, error, cache, n);
			F::scale_words(error, half);
			ntt_inverse(error);
			g.resize(std::min(n, target));
			for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
		}
		g.resize(target);
		g.insert(g.begin(), shift, T(0));
		return g;
	}

	T eval(T x) const {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) return fps_detail::horner(this->data(), this->size(), x);
		T ans = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) ans = ans * x + *it;
		return ans;
	}

	void onemul(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this *= T(1) + c;
			return;
		}
		for (int i = (static_cast<int>(this->size())) - 1; i >= d; --i) (*this)[i] += (*this)[i - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this /= T(1) + c;
			return;
		}
		for (int i = d; i < static_cast<int>(this->size()); ++i) (*this)[i] -= (*this)[i - d] * c;
	}

	F diff() const {
		const int n = static_cast<int>(this->size());
		F r(n);
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::weighted(words(r), words(*this) + 1, n - 1, 1);
		} else
			for (int i = 1; i < n; ++i) r[i - 1] = (*this)[i] * i;
		return r;
	}
	F integral() const {
		const int n = static_cast<int>(this->size());
		assert(unsigned(n) < unsigned(T::mod()));
		F r(n);
		const auto &inv = inverses(n);
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n > 1) fps_detail::NTT::product(words(r) + 1, words(*this), words(inv) + 1, n - 1);
		} else
			for (int i = 1; i < n; ++i) r[i] = (*this)[i - 1] * inv[i];
		return r;
	}

	F log(int deg = -1) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).log(deg));
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(!this->empty() && (*this)[0] == T(1));
		return log_derivative(deg, true);
	}

	F exp(int deg = -1) const {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) return exp_impl<false>(deg);
		else return export_work(import_work(*this).exp(deg));
	}

	F pow(long long k) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) return export_work(import_work(*this).pow(k));
		assert(k >= 0);
		const int n = static_cast<int>(this->size());
		if (n == 0) return {};
		if (k == 0) {
			F r(n);
			r[0] = 1;
			return r;
		}
		if (k == 1) return *this;
		if (k <= 4) {
			F square = multiply(*this, *this).pre(n);
			return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
		}
		int first = 0;
		while (first < n && (*this)[first] == T(0)) ++first;
		if (first == n || first > (n - 1) / k) return F(n);
		const int shift = first * k;
		const int len = n - shift;
		const T lead = (*this)[first];
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (len >= 256 && len <= (1 << 24) && fps_detail::NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_power(this->data() + first, this->size() - first, r, len, T(k).val(), lead.pow(k));
				r.insert(r.begin(), shift, T(0));
				return r;
			}
		}
#endif
		F f(this->begin() + first, this->end());
		f.resize(len);
		f = f.log_derivative(len);
		f *= T(k);
		f = f.template exp_impl<true>(len);
		f *= lead.pow(k);
		F r(n);
		for (int i = 0; i < len; ++i) r[shift + i] = f[i];
		return r;
	}

	std::pair<F, F> divmod(const F &divisor) const {
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
			auto [q, r] = import_work(*this).divmod(import_work(divisor));
			return {export_work(q), export_work(r)};
		}
		int n = static_cast<int>(this->size()), m = static_cast<int>(divisor.size());
		while (n && (*this)[n - 1] == T(0)) --n;
		while (m && divisor[m - 1] == T(0)) --m;
		assert(m);
		if (n < m) return {{}, pre(n)};
		const int qn = n - m + 1;

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (m > 2 && (std::size_t)m * qn > 4096 && n <= (1 << 23) && fps_detail::NTT::vectorized()) {
				F q, r;
				fps_detail::blocked_poly::native_divide(this->data(), n, divisor.data(), m, q, r);
				q.trim(), r.trim();
				return {std::move(q), std::move(r)};
			}
		}
#endif
		F a = pre(n);
		if (m <= 2 || (std::size_t)m * qn <= 4096) {
			F q(qn);
			const T inv = divisor[m - 1].inv();
			for (int i = qn - 1; i >= 0; --i) {
				q[i] = a[i + m - 1] * inv;
				for (int j = 0; j < m; ++j) a[i + j] -= q[i] * divisor[j];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F ra = a, rb(divisor.begin(), divisor.begin() + m);
		std::reverse(ra.begin(), ra.end());
		std::reverse(rb.begin(), rb.end());
		F q = ra.pre(qn).div_series(rb, qn);
		std::reverse(q.begin(), q.end());
		std::reverse(rb.begin(), rb.end());
		int cyclic = 1;
		while (cyclic < std::max(m, qn)) cyclic <<= 1;
		if (std::min(m - 1, qn) > 32 && cyclic < m + std::min(m - 1, qn) - 2) {
			assert(cyclic <= (1 << 23));
			F product = rb, quotient = q;
			product.resize(cyclic);
			quotient.resize(cyclic);
			auto &t = transform(cyclic / 16);
			t.convolution(words(product), words(quotient), cyclic);
			t.scale(words(product), m - 1, T::mod() - (T::mod() - 1) / (cyclic / 8));
			for (int i = 0; i < m - 1; ++i) {
				a[i] -= product[i];
				if (i + cyclic < static_cast<int>(a.size())) a[i] += a[i + cyclic];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		for (int i = 0; i < std::min<int>(a.size(), product.size()); ++i) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

	private:
	using O = fps_detail::Operations<T>;
	using O::import_work;
	using O::export_work;
	using O::transform;
	using O::words;
	using O::transform_inplace;
	using O::ntt;
	using O::ntt_half;
	using O::ntt_inverse;
	using O::scale_words;
	using O::geometric_scale;
	using O::batch_inverse;
	using O::pointwise;
	using O::inverse_extend;
	using O::dot;
	using O::prepare_sparse;
	using O::sparse_dot;
	using O::intt;
	using O::mod_sqrt;
	using O::inverse_ntt_roots;
	using O::ntt_roots;
	using O::inverses;

	using Word = typename O::Word;

	F log_derivative(int deg, bool integrate = false) const {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 256 && deg <= (1 << 24) && this->size() >= 64 && fps_detail::NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_log(this->data(), this->size(), r, deg, integrate);
				return r;
			}
		}
#endif
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 256 && deg <= (1 << 24) && this->size() >= 64 && fps_detail::NTT::vectorized()) {
				int k = 32;
				while (k * 4 < deg - 1) k *= 2;
				const int n = 2 * k, count = (deg - 1 + k - 1) / k, pitch = n + 16;
				F h = inv(k), r(deg);
				fps_detail::LogTransform t{transform(n / 16)};
				auto memory = fps_detail::allocate_fps_words((count + 2) * pitch);
				Word *ht = memory.get(), *window = ht + pitch, *q = window + (count - 1) * pitch, *work = q + pitch;
				std::copy_n(words(h), k, ht);
				const Word norm = T::mod() - (T::mod() - 1) / (n / 8);
				t.forward(ht, n, norm, true);
				for (int j = 1; j < count; ++j) {
					Word *w = window + (j - 1) * pitch;
					const int first = (j - 1) * k, c = std::max(0, std::min<int>(n, static_cast<int>(this->size()) - first));
					if (c) std::copy_n(words(*this) + first, c, w);
					std::fill(w + c, w + n, 0u);
					t.forward(w, n);
				}
				for (int j = 0; j < count; ++j) {
					const int first = j * k, c = std::min(k, deg - 1 - first);
					if (j) {
						const Word *aa[] = {window + (j - 1) * pitch, window}, *bb[] = {q, work};
						if (count == 4 && j == 2)
							t.toeplitz(work, window + 2 * pitch, window, window + pitch, q, work, n);
						else if (count == 4 && j == 3) {
							const Word *a[] = {window}, *b[] = {work};
							t.products(work, a, b, 1, n, window + 2 * pitch);
						} else {
							t.products(work, aa, bb, j, n);
						}
						t.inverse(work, n);
					}
					const int available = std::max(0, std::min<int>(c, static_cast<int>(this->size()) - first - 1));
					if (available) fps_detail::NTT::weighted(work, words(*this) + first + 1, available, first + 1);
					std::fill(work + available, work + k, 0u);
					if (j)
						for (int i = 0; i < k; ++i) work[i] = fps_detail::NTT::sub(work[i], work[k + i]);
					t.forward(work, n, 1, true);
					const Word *aa[] = {work}, *bb[] = {ht};
					t.products(work, aa, bb, 1, n);
					t.inverse(work, n);
					if (integrate)
						fps_detail::LogTransform::integrate(words(r) + first + 1, work, work + k, first + 1, c);
					else
						std::copy_n(work, c, words(r) + first + 1);
					if (j + 1 < count) {
						Word *cache = j == 0 ? q : work;
						if (j == 0) std::copy_n(work, k, cache);
						t.forward(cache, n, norm, true);
					}
				}
				return r;
			}
		}
#endif

		F derivative(std::min<int>(deg, this->size()));
		if (derivative.size() > 1) fps_detail::NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		if (integrate) {
			const auto &iv = inverses(deg - 1);
			fps_detail::NTT::product(words(r), words(r), words(iv), deg);
		}
		return r;
	}

	template <bool differentiated>
	F exp_impl(int deg) const {
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		auto terms = sparse_terms(deg, 64);
		int stride = terms.size() <= 64 ? prepare_sparse(terms) : 1;
		if (terms.size() <= 64 && (deg <= 256 || terms.size() <= 24ULL * stride)) {
			F g(deg);
			g[0] = 1;
			if (terms.size() == 1 && stride == 1 && deg >= 256) {
				base fact(deg);
				fps_detail::factorials(words(fact), words(g), deg);
				geometric_scale(g, T(1), terms[0].second);
				return g;
			}
			if (!stride) return g;
			const auto &iv = inverses((deg - 1) / stride);
			T factor = differentiated ? T(stride).inv() : T(1);
			for (auto &[j, x] : terms) x *= differentiated ? factor : T(j / stride);
			prepare_sparse(terms);
			for (long long i = stride; i < deg; i += stride) g[i] = sparse_dot(terms, g, static_cast<int>(i)) * iv[i / stride];
			return g;
		}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (deg >= 256 && deg <= (1 << 24) && fps_detail::NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_exp(this->data(), this->size(), r, deg, differentiated);
				return r;
			}
		}
#endif
		const auto &iv = inverses(deg);
		const int initial = std::min(deg, 32);
		F g(initial);
		g[0] = 1;
		for (int i = 1; i < initial; ++i) {
			T sum = 0;
			for (int j = 1; j <= i && j < static_cast<int>(this->size()); ++j)
				sum += (differentiated ? (*this)[j] : T(j) * (*this)[j]) * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		if (deg > 256) {
			int size = 1;
			while (size < deg) size <<= 1;
			if (deg < size - size / 4) size >>= 1;
			const int block = size / 32, length = 2 * block, pitch = length + 16, count = (deg + block - 1) / block;
			g = pre(block).template exp_impl<differentiated>(block);
			base inverse = g.inv(block);
			inverse.resize(length);
			ntt_half(inverse);
			const Word norm = T::mod() - (T::mod() - 1) / length;
			transform(1).scale(words(inverse), length, norm);
			auto storage = fps_detail::allocate_fps_words((2 * count - 1) * pitch);
			Word *derivative = storage.get(), *cache = derivative + count * pitch;
			for (int i = 0; i < count; ++i) {
				Word *d = derivative + i * pitch;
				const int offset = i * block;
				const int available = std::max(0, std::min<int>(block, std::min<int>(deg, this->size()) - offset));
				if (available) {
					if constexpr (differentiated) std::copy_n(words(*this) + offset, available, d);
					else fps_detail::NTT::weighted(d, words(*this) + offset, available, offset);
				}
				std::fill(d + available, d + block, 0u);
				ntt_half(d, length);
			}
			O::join_windows_strided(derivative, length, count, pitch);
			base error(length);
			g.resize(deg);
			for (int i = 1; i < count; ++i) {
				int offset = i * block, n = std::min(block, deg - offset);
				Word *q = cache + (i - 1) * pitch;
				std::copy_n(words(g) + offset - block, block, q);
				ntt_half(q, length);
				fps_detail::NTT::scale_normalize(q, length, norm);
				for (int j = 0; j < i; j += 7) {
					const Word *a[7], *b[7];
					int terms = std::min(7, i - j);
					for (int k = 0; k < terms; ++k) {
						a[k] = cache + (j + k) * pitch;
						b[k] = derivative + (i - j - k) * pitch;
					}
					auto update = [&](auto number) {
						constexpr int count = decltype(number)::value;
						if (j == 0) O::template accumulate_products<count, true>(words(error), a, b, length);
						else O::template accumulate_products<count, false>(words(error), a, b, length);
					};
					if (terms == 7) update(std::integral_constant<int, 7>{});
					else if (terms == 6) update(std::integral_constant<int, 6>{});
					else if (terms == 5) update(std::integral_constant<int, 5>{});
					else if (terms == 4) update(std::integral_constant<int, 4>{});
					else if (terms == 3) update(std::integral_constant<int, 3>{});
					else if (terms == 2) update(std::integral_constant<int, 2>{});
					else update(std::integral_constant<int, 1>{});
				}
				ntt_inverse(error);
				std::fill(error.begin() + block, error.end(), T(0));
				ntt_half(error);
				pointwise(error, error, inverse, length);
				ntt_inverse(error);
				fps_detail::NTT::product(words(error), words(error), words(iv) + offset, n);
				std::fill(error.begin() + n, error.end(), T(0));
				ntt_half(error);
				fps_detail::NTT::product(words(error), words(error), cache, length);
				ntt_inverse(error);
				std::copy_n(error.begin(), n, g.begin() + offset);
			}
			return g;
		}
		F h = g.pre(16).inv(16);
		base cache = h;
		cache.resize(32);
		ntt(cache);
		while (static_cast<int>(g.size()) < deg) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (deg - m <= 32) {
				base derivative(std::min<int>(deg, this->size()));
				for (int i = 1; i < static_cast<int>(derivative.size()); ++i) derivative[i] = differentiated ? (*this)[i] : T(i) * (*this)[i];
				derivative.resize(std::max<int>(1, derivative.size()));
				g.resize(deg);
				for (int i = m; i < deg; ++i) {
					const int count = std::min<int>(i, derivative.size() - 1);
					g[i] = dot(derivative.data() + 1, g.data() + i - 1, count) * iv[i];
				}
				break;
			}
			assert(n <= (1 << 23));
			const T im = T::raw(T::mod() - (T::mod() - 1) / m);
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, b(m), error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			std::fill(b.begin(), b.end(), T(0));
			for (int i = 1; i < m && i < static_cast<int>(this->size()); ++i) b[i - 1] = differentiated ? (*this)[i] : T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			for (int i = 0; i < m - 1; ++i) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			for (int i = (std::min(n, deg)) - 1; i >= m; --i)
				error[i] = (i < static_cast<int>(this->size()) ? (*this)[i] * (differentiated ? iv[i] : T(1)) : T(0)) + error[i - 1] * in * iv[i];
			std::fill(error.begin(), error.begin() + m, T(0));
			std::fill(error.begin() + std::min(n, deg), error.end(), T(0));
			ntt(error);
			pointwise(error, error, a, n);
			ntt_inverse(error);
			g.resize(std::min(n, deg));
			for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
		}
		return g;
	}

	std::vector<std::pair<int, T>> sparse_terms(int deg, int limit) const {
		std::vector<std::pair<int, T>> terms;
		for (int i = 1; i < std::min<int>(deg, this->size()) && static_cast<int>(terms.size()) <= limit; ++i)
			if ((*this)[i] != T(0)) terms.emplace_back(i, (*this)[i]);
		return terms;
	}

	void trim() { O::trim(*this); }

};

using fps = FormalPowerSeries<fps_mint>;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "components/fps_bostan.hpp"



#line 5 "components/fps_bostan.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::bostan_detail {
inline void inverse_twisted(const NTT& plan, const NTT::U* in, NTT::U* out, const NTT::U* powers, int n) {
	using N = NTT;
	if (n == 8) {
		auto z = plan.small8<4, true>(plan.small8<2, true>(plan.small8<1, true>(N::load8(in), 0), 0), 0);
		N::store8(out, N::mul8(z, N::load8(powers)));
		return;
	}
	for (int j = 0; j < n; j += 8) {
		auto z = N::load8(in + j);
		z = plan.small8<4, true>(plan.small8<2, true>(plan.small8<1, true>(z, j / 8), j / 8), j / 8);
		N::store8(out + j, z);
	}
	int h = 8;
	for (; 4 * h < n; h *= 4) plan.radix4_8<true>(out, h, n);
	if (4 * h == n) {
		const auto im = _mm256_set1_epi32(plan.inverse_roots[1]);
		const auto iq = _mm256_set1_epi32(plan.inverse_root_quotients[1]);
		for (int i = 0; i < h; i += 8) {
			const auto x0 = N::load8(out + i), x1 = N::load8(out + h + i);
			const auto x2 = N::load8(out + 2 * h + i), x3 = N::load8(out + 3 * h + i);
			const auto u = N::add8(x0, x1), v = N::sub8(x0, x1), s = N::add8(x2, x3);
			const auto t = N::fixed_mul8(N::sub8(x2, x3), im, iq);
			N::store8(out + i, N::mul8(N::add8(u, s), N::load8(powers + i)));
			N::store8(out + h + i, N::mul8(N::add8(v, t), N::load8(powers + h + i)));
			N::store8(out + 2 * h + i, N::mul8(N::sub8(u, s), N::load8(powers + 2 * h + i)));
			N::store8(out + 3 * h + i, N::mul8(N::sub8(v, t), N::load8(powers + 3 * h + i)));
		}
	} else {
		for (int i = 0; i < h; i += 8) {
			const auto x = N::load8(out + i), y = N::load8(out + h + i);
			N::store8(out + i, N::mul8(N::add8(x, y), N::load8(powers + i)));
			N::store8(out + h + i, N::mul8(N::sub8(x, y), N::load8(powers + h + i)));
		}
	}
}
}

#pragma GCC pop_options
#endif

#line 6 "fps/bostan_mori.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
T bostan_mori(FormalPowerSeries<T> p, FormalPowerSeries<T> q, long long k) {
	using F = FormalPowerSeries<T>;
	using base = std::vector<T>;
	using O = fps_detail::Operations<T>;
	using Word = typename fps_detail::Operations<T>::Word;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		return T::raw(bostan_mori<fps_detail::Mint>(O::import_work(p), O::import_work(q), k).val());
	} else {
		assert(k >= 0 && !q.empty() && q[0] != T(0));
		O::trim(p);
		O::trim(q);
		if (p.empty()) return 0;
		auto coefficient = [](const base &p, const base &q, int k) {
			F inverse(q.begin(), q.begin() + std::min<int>(k + 1, q.size()));
			inverse = inverse.inv(k + 1);
			return O::dot(p.data(), inverse.data() + k, std::min<int>(k + 1, p.size()));
		};
		if (k < 32) return coefficient(p, q, k);
		T answer = 0;
		if (p.size() >= q.size()) {
			auto [quotient, remainder] = p.divmod(q);
			if (k < (long long)quotient.size()) answer = quotient[k];
			p = std::move(remainder);
		}
		if (p.empty()) return answer;
		if (q.size() == 2) return answer + p[0] / q[0] * (-q[1] / q[0]).pow(k);
		const int d = static_cast<int>(q.size()) - 1;
		if (k < d) return answer + coefficient(p, q, k);
		int n = 1;
		while (n < d) n <<= 1;
		assert(n <= (1 << 22));
		const T half = T(2).inv(), inv = T(n).inv();
		T correction = 1;
		const T step = T(3).pow((T::mod() - 1) / (2 * n));
		base powers(n), twists(n);
		auto storage = fps_detail::allocate_fps_words(4 * std::size_t(n));
		Word *a = storage.get(), *b = a + 2 * n;
		std::copy_n(O::words(p), p.size(), a);
		std::fill(a + p.size(), a + 2 * n, 0u);
		std::copy_n(O::words(q), q.size(), b);
		std::fill(b + q.size(), b + 2 * n, 0u);
		auto &plan = O::transform(n);
		for (int i = 0; i < n; ++i) O::words(twists)[i] = plan.inverse_root_quotients[i] * (0u - T::mod());
		T lead = q.back();
		std::fill(powers.begin(), powers.end(), T(1));
		O::geometric_scale(powers, inv, step);
		plan.template fft<false>(a, 2 * n);
		plan.template fft<false>(b, 2 * n);
		auto doubling = [&](Word *v, T leading) {
			Word *odd = v + n;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
			if (n >= 8 && fps_detail::NTT::vectorized()) {
				fps_detail::bostan_detail::inverse_twisted(plan, v, odd, O::words(powers), n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2) * leading).v);
			} else
#endif
			{
				std::copy_n(v, n, odd);
				plan.template fft<true>(odd, n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
				fps_detail::NTT::product(odd, odd, O::words(powers), n);
			}
			plan.template fft<false>(odd, n);
		};
		while (k) {
			if (k & 1) fps_detail::NTT::reduce_pairs<true>(a, b, O::words(twists), n);
			else fps_detail::NTT::reduce_pairs<false>(a, b, O::words(twists), n);
			correction *= half;
			lead = (d & 1 ? -lead : lead) * lead;
			k >>= 1;
			if (k < d) break;
			doubling(a, T(0));
			doubling(b, d == n ? lead : T(0));
		}
		if (k) {
			base aa(n), bb(n);
			std::copy_n(a, n, O::words(aa));
			std::copy_n(b, n, O::words(bb));
			O::intt(aa);
			O::intt(bb);
			if (d == n) bb[0] -= lead;
			return answer + correction * coefficient(aa, bb, k);
		}
		T numerator = 0, denominator = 0;
		for (int i = 0; i < n; ++i) numerator += T::word(a[i]), denominator += T::word(b[i]);
		if (d == n) denominator -= T(n) * lead;
		return answer + correction * numerator / denominator;
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 5 "fps/linear_recurrence.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
T linear_recurrence(const std::vector<T> &initial, const std::vector<T> &coefficients, long long k) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	assert(k >= 0 && initial.size() == coefficients.size());
	const int d = static_cast<int>(initial.size());
	if (k < d) return initial[k];
	int order = d;
	auto zero = [](T x) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) return x == T(0);
		else return x.val() == 0;
	};
	while (order && zero(coefficients[order - 1])) --order;
	if (order == 0) return 0;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		typename O::W a(order), c(order);
		for (int i = 0; i < order; ++i) {
			a[i] = fps_detail::Mint::raw(initial[d - order + i].val());
			c[i] = fps_detail::Mint::raw(coefficients[i].val());
		}
		return T::raw(linear_recurrence<fps_detail::Mint>(a, c, k - d + order).val());
	}
	if (order == 1) return initial[d - 1] * coefficients[0].pow(k - d + 1);
	F q(order + 1);
	q[0] = 1;
	for (int i = 0; i < order; ++i) q[i + 1] = -coefficients[i];
	F p = F::multiply(F(initial.end() - order, initial.end()), q).pre(order);
	return bostan_mori(std::move(p), std::move(q), k - d + order);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "test/fps_io.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 15 "components/fast_io.hpp"
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


#line 1 "components/fps_io.hpp"


#line 1 "components/fast_io_bulk.hpp"



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


#line 7 "components/fps_io.hpp"
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_io_fields {

namespace detail {

struct alignas(32) Lanes {
	std::uint32_t lane[8];
};

constexpr Lanes all(std::uint32_t x) { return {{x, x, x, x, x, x, x, x}}; }
constexpr Lanes all16(std::uint32_t x) { return all(x * 0x10001); }

constexpr int Z = 0x80;

constexpr Lanes shuffle(const int (&b)[16]) {
	Lanes t{};
	for (int i = 0; i < 32; ++i) t.lane[i / 4] |= std::uint32_t(b[i % 16] & 0xFF) << 8 * (i % 4);
	return t;
}

struct Constants {
	Lanes div10 = all(429496730);
	Lanes div10e5 = all(1407374884);
	Lanes pack = all(1 - 10000 * 65536u);
	Lanes ten = all(10);
	Lanes tail = all(' ' << 8 | '0');
	Lanes by100 = all16(5243), hundred = all16(100);
	Lanes by10 = all16(6554);
	Lanes tens_out = all16(2559);
	Lanes ones = all(~0u), zero = all(0x30303030), space = all(0x20202020);

	Lanes chunk0[2] = {shuffle({0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, Z, Z, Z, Z, Z, Z})};
	Lanes chunk1[3] = {shuffle({14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, 4, 5, Z, Z, Z, Z, Z, Z, Z, Z, 8, 9, Z, Z}),
					shuffle({Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9})};
	Lanes chunk2[3] = {shuffle({10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, 12, 13, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7})};
	Lanes chunk3[3] = {shuffle({0, 1, Z, Z, Z, Z, Z, Z, Z, Z, 4, 5, Z, Z, Z, Z}),
					shuffle({Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3})};
	Lanes chunk4[2] = {shuffle({4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z}),
					shuffle({Z, Z, Z, Z, 8, 9, Z, Z, Z, Z, Z, Z, Z, Z, 12, 13})};
};

inline constexpr Constants kConstants{};

[[gnu::always_inline]] inline __m256i load(const Lanes& l) {
	return _mm256_load_si256(reinterpret_cast<const __m256i*>(&l));
}

[[gnu::always_inline]] inline __m256i shuffle(__m256i x, const Lanes& control) {
	return _mm256_shuffle_epi8(x, load(control));
}

struct Divided {
	__m256i halves, tail;
};

[[gnu::always_inline]] inline Divided divide(__m256i v, __m256i odd, const Constants& k) {
	const __m256i w = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10)), 32),
										_mm256_mul_epu32(odd, load(k.div10)), 0xAA);
	const __m256i h = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10e5)), 47),
										_mm256_srli_epi64(_mm256_mul_epu32(odd, load(k.div10e5)), 15), 0xAA);
	const __m256i halves = _mm256_add_epi32(_mm256_slli_epi32(w, 16), _mm256_mullo_epi32(h, load(k.pack)));
	const __m256i tail = _mm256_sub_epi32(_mm256_add_epi32(v, load(k.tail)), _mm256_mullo_epi32(w, load(k.ten)));
	return {halves, tail};
}

[[gnu::always_inline]] inline __m256i two_digits(__m256i z, const Constants& k) {
	const __m256i tens = _mm256_mulhi_epu16(z, load(k.by10));
	return _mm256_sub_epi16(_mm256_slli_epi16(z, 8), _mm256_mullo_epi16(tens, load(k.tens_out)));
}

[[gnu::always_inline]] inline __m256i text(__m256i digits, const Constants& k) {
	const __m256i upto = _mm256_xor_si256(digits, _mm256_add_epi64(digits, load(k.ones)));
	return _mm256_blendv_epi8(_mm256_or_si256(digits, load(k.zero)), load(k.space), upto);
}

[[gnu::always_inline]] inline void digits(const Divided& d, __m256i& lo, __m256i& hi, const Constants& k) {
	const __m256i hundreds = _mm256_srli_epi16(_mm256_mulhi_epu16(d.halves, load(k.by100)), 3);
	const __m256i rest = _mm256_sub_epi16(d.halves, _mm256_mullo_epi16(hundreds, load(k.hundred)));
	lo = text(two_digits(_mm256_unpacklo_epi16(hundreds, rest), k), k);
	hi = text(two_digits(_mm256_unpackhi_epi16(hundreds, rest), k), k);
}

[[gnu::always_inline]] inline void store(char* p, const Divided& lo, const Divided& hi, const Constants& k) {
	__m256i d01, d23, d45, d67;
	const auto either = [&](__m256i a, __m256i b) __attribute__((target("avx2"))) { return _mm256_or_si256(a, b); };
	digits(lo, d01, d23, k);
	digits(hi, d45, d67, k);
	const __m256i c0 = either(shuffle(d01, k.chunk0[0]), shuffle(lo.tail, k.chunk0[1]));
	const __m256i c1 = either(either(shuffle(d01, k.chunk1[0]), shuffle(lo.tail, k.chunk1[1])), shuffle(d23, k.chunk1[2]));
	const __m256i c2 = either(either(shuffle(d23, k.chunk2[0]), shuffle(lo.tail, k.chunk2[1])), shuffle(d45, k.chunk2[2]));
	const __m256i c3 = either(either(shuffle(hi.tail, k.chunk3[0]), shuffle(d45, k.chunk3[1])), shuffle(d67, k.chunk3[2]));
	const __m256i c4 = either(shuffle(d67, k.chunk4[0]), shuffle(hi.tail, k.chunk4[1]));

	const auto whole = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p + 64 + 16 * c), chunk); };
	const auto low = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm_store_si128(reinterpret_cast<__m128i*>(p + 16 * c), _mm256_castsi256_si128(chunk)); };
	whole(4, c4), whole(3, c3), whole(2, c2), whole(1, c1), whole(0, c0);
	low(0, c0), low(1, c1), low(2, c2), low(3, c3), low(4, c4);
}

[[gnu::always_inline]] inline void divide16(const std::uint32_t* x, Divided& lo, Divided& hi, const Constants& k) {
	const auto at = [x](int i) __attribute__((target("avx2"))) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(x + i)); };
	const auto both = [x](int i) __attribute__((target("avx2"))) { return _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i*>(x + i))); };
	const __m256i l = _mm256_blend_epi32(at(0), both(8), 0xF0), h = _mm256_blend_epi32(at(8), both(4), 0x0F);
	lo = divide(l, _mm256_srli_epi64(l, 32), k);
	hi = divide(h, _mm256_srli_epi64(h, 32), k);
}

inline void format(char* p, const std::uint32_t* x, std::size_t count, const Constants& k) {
	Divided dl, dh;
	divide16(x, dl, dh, k);
	for (std::size_t i = 16; i < count; i += 16, p += 160) {
		Divided nl, nh;
		divide16(x + i, nl, nh, k);
		store(p, dl, dh, k);
		dl = nl, dh = nh;
	}
	store(p, dl, dh, k);
}

}

inline constexpr std::size_t kBlock = 25600;
inline constexpr std::size_t kTextBytes = 10 * kBlock;

}

namespace fps_detail {
[[gnu::always_inline]] inline __m256i io_values(__m256i x) {
	const auto p = _mm256_set1_epi32(Mint::mod()), ni = _mm256_set1_epi32(998244351);
	auto even = _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffULL));
	auto odd = _mm256_srli_epi64(x, 32);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	const auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
	return _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
}

inline void read_coefficients(FastInput &input, std::vector<Mint> &out, std::size_t n) {
	if (!n) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps input");
	prepare_fps_output(out, out.size() + n);
	fast_io_detail::BulkInput::read_blocks(input, n, [&](const unsigned *values, std::size_t count) {
		append_fps_words<false>(out, values, count);
	});
}

inline void write_coefficients(FastOutput &output, const Mint *in, std::size_t n) {
	alignas(32) unsigned values[25600];
	alignas(32) char text[256000];
	const auto &k = fps_io_fields::detail::kConstants;
	for (std::size_t i = 0; i < n; i += 25600) {
		const auto count = std::min<std::size_t>(25600, n - i), full = count / 16 * 16;
		std::size_t j = 0;
		for (; j + 8 <= count; j += 8) {
			alignas(32) unsigned words[8];
			std::memcpy(words, in + i + j, sizeof(words));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(values + j), io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words))));
		}
		for (; j < count; ++j) values[j] = in[i + j].val();
		if (full) fps_io_fields::detail::format(text, values, full, k);
		if (full < count) {
			alignas(32) unsigned rest[16] = {};
			alignas(16) char last[160];
			std::copy_n(values + full, count - full, rest);
			fps_io_fields::detail::format(last, rest, 16, k);
			std::memcpy(text + 10 * full, last, 10 * (count - full));
		}
		if (i + count == n) text[10 * count - 1] = '\n';
		fast_io_detail::BulkOutput::write(output, std::string_view(text, 10 * count));
	}
	if (!n) output.write('\n');
}
}
#pragma GCC pop_options
#endif

#line 7 "test/fps_io.hpp"

inline fps read_fps(FastInput &input, int n) {
	fps result;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::read_coefficients(input, result, n);
		return result;
	}
#endif
	result.resize(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_fps(FastOutput &output, const fps &f) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::write_coefficients(output, f.data(), f.size());
		return;
	}
#endif
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}


#line 4 "test/fps_linear_recurrence.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0; long long k = 0; input.read(d); input.read(k);
	auto a = read_fps(input, d), c = read_fps(input, d);
	output.println(linear_recurrence<mint>(a, c, k).val());
}
