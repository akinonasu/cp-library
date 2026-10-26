#line 1 "test/binomial_prefix_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/prefix_sum_of_binomial_coefficient"
#line 1 "math/binomial_prefix_sum.hpp"


#line 1 "components/binomial_prefix_core.hpp"



#line 1 "math/mint.hpp"


#include <cassert>
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


#line 1 "components/mint_factorials.hpp"



#line 1 "components/mint_arithmetic.hpp"



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


#line 5 "components/mint_factorials.hpp"

namespace mint_detail {

#if defined(__x86_64__) || defined(__i386__)

__attribute__((target("avx2"))) inline void factorial_transpose(Ops::I8 (&x)[8]) {
	Ops::I8 a[8], b[8];
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
__attribute__((target("avx2"))) inline void factorial_chain(Ops::U* out, int chunk, Ops::I8 value, Ops::I8 factor) {
	using I = Ops::I8;
	I step = _mm256_set1_epi32(Ops::one);
	I order = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	for (int j = 0; j < chunk; j += 8) {
		I block[8];
		for (int k = 0; k < 8; ++k) {
			block[k] = value;
			value = Ops::mul8(value, factor);
			if constexpr (reverse) factor = Ops::sub8(factor, step);
			else factor = Ops::add8(factor, step);
		}
		factorial_transpose(block);
		for (int k = 0; k < 8; ++k) {
			if constexpr (reverse) Ops::store8(out + (k + 1) * chunk - j - 8, _mm256_permutevar8x32_epi32(block[k], order));
			else Ops::store8(out + k * chunk + j, block[k]);
		}
	}
}

__attribute__((target("avx2"))) inline void factorials8(Ops::U* fact, Ops::U* invfact, int n) {
	using I = Ops::I8;
	using U = Ops::U;
	const int chunk = n / 256 * 8, covered = chunk * 32;
	alignas(32) U factor[32], products[32], starts[33], inverse[33];
	I p[4], x[4], step = _mm256_set1_epi32(Ops::one);
	for (int i = 0; i < 32; ++i) factor[i] = Ops::in(i * chunk + 1);
	for (int k = 0; k < 4; ++k) p[k] = _mm256_set1_epi32(Ops::one), x[k] = Ops::load8(factor + 8 * k);
	for (int i = 0; i < chunk; ++i) {
		for (int k = 0; k < 4; ++k) p[k] = Ops::mul8(p[k], x[k]), x[k] = Ops::add8(x[k], step);
	}
	for (int k = 0; k < 4; ++k) Ops::store8(products + 8 * k, p[k]);
	starts[0] = Ops::one;
	for (int i = 0; i < 32; ++i) starts[i + 1] = Ops::mul(starts[i], products[i]);
	U last = starts[32];
	for (int i = covered; i < n; ++i) fact[i] = last, last = Ops::mul(last, Ops::in(i + 1));
	last = mint::word(last).inv().v;
	for (int i = n - 1; i >= covered; --i) last = Ops::mul(last, Ops::in(i + 1)), invfact[i] = last;
	inverse[32] = last;
	for (int i = 31; i >= 0; --i) inverse[i] = Ops::mul(inverse[i + 1], products[i]);
	for (int k = 0; k < 4; ++k) {
		factorial_chain<false>(fact + 8 * k * chunk, chunk, Ops::load8(starts + 8 * k), Ops::load8(factor + 8 * k));
		alignas(32) U values[8], factors[8];
		for (int j = 0; j < 8; ++j) {
			const int end = (8 * k + j + 1) * chunk;
			values[j] = Ops::mul(inverse[8 * k + j + 1], Ops::in(end));
			factors[j] = Ops::in(end - 1);
		}
		factorial_chain<true>(invfact + 8 * k * chunk, chunk, Ops::load8(values), Ops::load8(factors));
	}
}
#endif

inline void factorials(Ops::U* fact, Ops::U* invfact, int n) {
	if (!n) return;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 256 && Ops::vectorized()) {
		factorials8(fact, invfact, n);
		return;
	}
#endif
	Ops::U value = Ops::one;
	for (int i = 0; i < n; ++i) fact[i] = value, value = Ops::mul(value, Ops::in(i + 1));
	value = mint::word(fact[n - 1]).inv().v;
	for (int i = n - 1; i >= 0; --i) invfact[i] = value, value = Ops::mul(value, Ops::in(i));
}

template<class T> requires std::is_same_v<T, mint>
inline void factorials(T* fact, T* invfact, int n) {
	assert(0 <= n && unsigned(n) <= mint::mod());
	static_assert(sizeof(mint) == sizeof(Ops::U) && std::is_trivially_copyable_v<mint>);
	if (unsigned(n) == mint::mod()) {
		factorials(fact, invfact, n - 1);
		fact[n - 1] = -fact[n - 2];
		invfact[n - 1] = -invfact[n - 2];
		return;
	}
	factorials(reinterpret_cast<Ops::U*>(fact), reinterpret_cast<Ops::U*>(invfact), n);
}

}


#line 7 "components/binomial_prefix_core.hpp"
#include <cstdint>
#include <limits>
#line 10 "components/binomial_prefix_core.hpp"
#include <vector>

namespace binomial_prefix_detail {
template<class Mint> struct Table {
	std::vector<Mint> factorial, inverse_factorial, power2;
	Mint half;
	explicit Table(int n = 0) { build(n); }
	void build(int n) {
		assert(0 <= n && n < std::numeric_limits<int>::max() && std::uint64_t(n) < Mint::mod() && Mint::mod() > 1);
		factorial.resize(n + 1), inverse_factorial.resize(n + 1), power2.resize(n + 1);
		if constexpr (std::is_same_v<Mint, mint>) {
			mint_detail::factorials(factorial.data(), inverse_factorial.data(), n + 1);
			power2[0] = Mint(1);
			for (int i = 1; i <= n; ++i) power2[i] = power2[i - 1] + power2[i - 1];
		} else {
			factorial[0] = power2[0] = Mint(1);
			for (int i = 1; i <= n; ++i) factorial[i] = factorial[i - 1] * Mint(i), power2[i] = power2[i - 1] + power2[i - 1];
			inverse_factorial[n] = factorial[n].inv();
			for (int i = n; i > 0; --i) inverse_factorial[i - 1] = inverse_factorial[i] * Mint(i);
		}
		half = Mint((std::uint64_t(Mint::mod()) + 1) / 2);
	}
	Mint binom(int n, int k) const {
		if (k < 0 || k > n) return Mint(0);
		return factorial[n] * inverse_factorial[k] * inverse_factorial[n - k];
	}
	Mint center(int n) const {
		if (!n) return Mint(0);
		return n & 1 ? power2[n - 1] : (power2[n] - binom(n, n / 2)) * half;
	}
	Mint range(int n, int l, int r) const {
		assert(0 <= l && l <= r && r <= n + 1);
		if constexpr (std::is_same_v<Mint, mint>) {
			__uint128_t sum = 0;
			for (int i = l; i < r; ++i) sum += std::uint64_t(inverse_factorial[i].v) * inverse_factorial[n - i].v;
			return factorial[n] * Mint::word(Mint::reduce(std::uint64_t(sum % Mint::mod())));
		} else {
			Mint sum(0);
			for (int i = l; i < r; ++i) sum += inverse_factorial[i] * inverse_factorial[n - i];
			return factorial[n] * sum;
		}
	}
};
}


#line 1 "components/binomial_prefix_online.hpp"



#line 1 "components/mint_inner_product.hpp"



#line 6 "components/mint_inner_product.hpp"
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#endif

namespace mint_detail {
template<bool Reverse> inline __uint128_t inner_product_scalar(const mint* a, const mint* b, int size) {
	__uint128_t sum = 0;
	int i = 0;
	for (; i <= size - 4; i += 4) {
		std::uint64_t part = std::uint64_t(a[i].v) * b[Reverse ? -i : i].v
			+ std::uint64_t(a[i + 1].v) * b[Reverse ? -i - 1 : i + 1].v
			+ std::uint64_t(a[i + 2].v) * b[Reverse ? -i - 2 : i + 2].v
			+ std::uint64_t(a[i + 3].v) * b[Reverse ? -i - 3 : i + 3].v;
		sum += part;
	}
	for (; i < size; ++i) sum += std::uint64_t(a[i].v) * b[Reverse ? -i : i].v;
	return sum;
}

#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
inline bool inner_product_has_avx2() {
#if defined(__AVX2__)
	return true;
#else
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#endif
}

template<bool Reverse> __attribute__((target("avx2")))
__uint128_t inner_product_avx2(const mint* a, const mint* b, int size) {
	const auto order = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	const auto zero = _mm256_setzero_si256();
	auto total = zero, carry = zero;
	int i = 0;
	for (; i <= size - 8; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + (Reverse ? -i - 7 : i)));
		if constexpr (Reverse) y = _mm256_permutevar8x32_epi32(y, order);
		auto even = _mm256_mul_epu32(x, y);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
		auto next = _mm256_add_epi64(total, _mm256_add_epi64(even, odd));
		carry = _mm256_sub_epi64(carry, _mm256_cmpgt_epi64(zero, _mm256_andnot_si256(next, total)));
		total = next;
	}
	alignas(32) std::uint64_t low[4], high[4];
	_mm256_store_si256(reinterpret_cast<__m256i*>(low), total);
	_mm256_store_si256(reinterpret_cast<__m256i*>(high), carry);
	__uint128_t sum = 0;
	if (i < size) sum = inner_product_scalar<Reverse>(a + i, b + (Reverse ? -i : i), size - i);
	for (int j = 0; j < 4; ++j) sum += (__uint128_t(high[j]) << 64) + low[j];
	return sum;
}
#endif

template<bool Reverse = false> mint inner_product(const mint* a, const mint* b, int size) {
	assert(size >= 0);
	__uint128_t sum;
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
	if (size >= 64 && inner_product_has_avx2()) sum = inner_product_avx2<Reverse>(a, b, size);
	else
#endif
		sum = inner_product_scalar<Reverse>(a, b, size);
	return mint::word(mint::reduce(std::uint64_t(sum % mint::mod())));
}
}


#line 5 "components/binomial_prefix_online.hpp"
#include <algorithm>
#line 7 "components/binomial_prefix_online.hpp"
#include <cmath>
#include <cstddef>
#line 12 "components/binomial_prefix_online.hpp"

namespace binomial_prefix_detail {
template<class Mint, class Table> class Online {
	Table table_;
	int max_n_ = 0, block_ = 1;
	std::vector<Mint> scaled_factorial_, data_;
	void check([[maybe_unused]] int n) const { assert(0 <= n && n <= max_n_); }
	static std::size_t offset(int row) { return std::size_t(row) * (std::size_t(row) + 1) / 2; }
	template<bool Reverse> static Mint dot(const Mint* a, const Mint* b, int size) {
		if constexpr (std::is_same_v<Mint, mint>) {
			return mint_detail::inner_product<Reverse>(a, b, size);
		} else {
			Mint sum(0);
			for (int i = 0; i < size; ++i) sum += a[i] * b[Reverse ? -i : i];
			return sum;
		}
	}
	Mint range(int n, int l, int r) const {
		return table_.factorial[n] * dot<true>(table_.inverse_factorial.data() + l, table_.inverse_factorial.data() + n - l, r - l);
	}

  public:
	explicit Online(int max_n = 0, int block_size = 0) { build(max_n, block_size); }
	void build(int max_n, int block_size = 0) {
		assert(max_n >= 0 && block_size >= 0);
		table_.build(max_n);
		max_n_ = max_n;
		block_ = block_size ? block_size : std::max(1, int(std::sqrt(double(max_n) + 1)));
		if (max_n < 2) { scaled_factorial_.clear(); data_.clear(); return; }
		scaled_factorial_.resize(std::size_t(max_n) + 1);
		Mint inverse_power = table_.power2[max_n].inv() * table_.half;
		for (int i = max_n; i >= 0; --i) {
			scaled_factorial_[i] = table_.factorial[i] * inverse_power;
			inverse_power += inverse_power;
		}
		int rows = max_n / block_;
		data_.resize(offset(rows) + std::size_t(rows) + 1);
		for (int row = 0; row <= rows; ++row) {
			int n = row * block_;
			Mint factor = scaled_factorial_[n] + scaled_factorial_[n], sum(0);
			auto base = offset(row);
			for (int column = 0; column <= row / 2; ++column) {
				int k = column * block_;
				if (column) sum += dot<true>(table_.inverse_factorial.data() + k - block_, table_.inverse_factorial.data() + n - k + block_, block_);
				Mint value = sum * factor;
				data_[base + row - column] = Mint(1) - value;
				data_[base + column] = value + table_.inverse_factorial[k] * table_.inverse_factorial[n - k] * factor;
			}
		}
	}
	int max_n() const { return max_n_; }
	int block_size() const { return block_; }
	std::size_t table_size() const { return data_.size(); }
	Mint query(int n, std::int64_t k) const {
		check(n);
		if (k <= 0) return Mint(0);
		if (k >= std::int64_t(n) + 1) return table_.power2[n];
		bool reflect = k > (n + 1) / 2;
		int m = reflect ? n + 1 - int(k) : int(k);
		Mint value(1);
		if (m != 1) {
			if (m == (n + 1) / 2) value = table_.center(n);
			else {
				int row = std::min(int((std::int64_t(n) + block_ / 2) / block_), max_n_ / block_);
				int column = std::min(int((std::int64_t(m - 1) + block_ / 2) / block_), row);
				int start = row * block_, anchor = column * block_;
				int center = (n + 1) / 2;
				auto distance = std::int64_t(std::abs(n - start)) + std::abs(m - 1 - anchor);
				if (std::min(m - 1, center - m) <= distance) {
					value = m - 1 <= center - m ? Mint(1) + range(n, 1, m) : table_.center(n) - range(n, m, center);
				} else {
					value = data_[offset(row) + column];
					if (start < n) value -= table_.inverse_factorial[anchor] * dot<false>(scaled_factorial_.data() + start, table_.inverse_factorial.data() + start - anchor, n - start);
					else if (n < start) {
						int first = std::max(n, anchor);
						value += table_.inverse_factorial[anchor] * dot<false>(scaled_factorial_.data() + first, table_.inverse_factorial.data() + first - anchor, start - first);
					}
					value *= table_.power2[n];
					if (anchor < m - 1) value += range(n, anchor + 1, m);
					else if (m - 1 < anchor) value -= range(n, m, std::min(anchor, n) + 1);
				}
			}
		}
		return reflect ? table_.power2[n] - value : value;
	}
	Mint prod(int n, std::int64_t l, std::int64_t r) const { assert(l <= r); return query(n, r) - query(n, l); }
};
}


#line 5 "math/binomial_prefix_sum.hpp"

template<class Mint = mint> class BinomialPrefixSum : public binomial_prefix_detail::Online<Mint, binomial_prefix_detail::Table<Mint>> {
	using Base = binomial_prefix_detail::Online<Mint, binomial_prefix_detail::Table<Mint>>;

  public:
	explicit BinomialPrefixSum(int max_n = 0, int block_size = 0) : Base(max_n, block_size) {}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
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


#line 4 "test/binomial_prefix_sum.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	BinomialPrefixSum<> prefix(200000);
	while (t--) {
		int n = 0, k = 0;
		input.read(n);
		input.read(k);
		output.println(prefix.query(n, std::int64_t(k) + 1).val());
	}
}
