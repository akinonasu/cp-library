#ifndef CP_COMPONENTS_FPS_FACTORIALS_HPP
#define CP_COMPONENTS_FPS_FACTORIALS_HPP

#include "ntt.hpp"

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
	last = mint::word(last).inv().v;
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
	value = mint::word(fact[n - 1]).inv().v;
	for (int i = n - 1; i >= 0; --i) invfact[i] = value, value = NTT::mul(value, NTT::in(i));
}

template<class T> requires std::is_same_v<T, mint>
inline void factorials(T* fact, T* invfact, int n) {
	assert(0 <= n && unsigned(n) <= mint::mod());
	static_assert(sizeof(mint) == sizeof(NTT::U) && std::is_trivially_copyable_v<mint>);
	if (unsigned(n) == mint::mod()) {
		factorials(fact, invfact, n - 1);
		fact[n - 1] = -fact[n - 2];
		invfact[n - 1] = -invfact[n - 2];
		return;
	}
	factorials(reinterpret_cast<NTT::U*>(fact), reinterpret_cast<NTT::U*>(invfact), n);
}

}

#endif
