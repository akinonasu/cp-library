#ifndef FPS_LIGHT_NTT_HPP
#define FPS_LIGHT_NTT_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>
#include "../math/mint.hpp"

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_light_detail {
using std::size_t;
using Series = std::vector<mint>;
static_assert(sizeof(mint) == sizeof(unsigned) && std::is_trivially_copyable_v<mint>);
inline bool simd_available() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	return __builtin_cpu_supports("avx2");
#else
	return false;
#endif
}
template<bool Inverse>
void scalar_stage(mint *a, size_t n, size_t h, const mint *roots) {
	for (size_t start = 0; start < n; start += h * 2)
		for (size_t j = 0; j < h; ++j) {
			mint x = a[start + j], y = a[start + j + h];
			if constexpr (Inverse) y *= roots[j];
			a[start + j] = x + y;
			a[start + j + h] = x - y;
			if constexpr (!Inverse) a[start + j + h] *= roots[j];
		}
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2")))
inline __m256i product(__m256i x, __m256i y) {
	const auto p = _mm256_set1_epi32(mint::mod());
	auto even = _mm256_mul_epu32(x, y);
	auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
	auto q = _mm256_mullo_epi32(_mm256_mullo_epi32(x, y), _mm256_set1_epi32(998244351));
	even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(q, p)), 32);
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(q, 32), p));
	return _mm256_blend_epi32(even, odd, 0xaa);
}

__attribute__((target("avx2")))
inline __m256i fixed_product(__m256i x, __m256i w, __m256i q) {
	auto even = _mm256_mul_epu32(x, q);
	auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(q, 32));
	q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w), _mm256_mullo_epi32(q, _mm256_set1_epi32(mint::mod())));
}

__attribute__((target("avx2")))
inline void pointwise_avx2(mint *a, const mint *b, size_t n) {
	size_t i = 0;
	for (; i + 8 <= n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), product(x, y));
	}
	for (; i < n; ++i) a[i] *= b[i];
}

__attribute__((target("avx2")))
inline void scale_avx2(mint *a, size_t n, mint factor) {
	size_t i = 0;
	unsigned value = factor.val();
	const auto f = _mm256_set1_epi32(value);
	const auto q = _mm256_set1_epi32((static_cast<unsigned long long>(value) << 32) / mint::mod());
	for (; i + 8 <= n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), fixed_product(x, f, q));
	}
	for (; i < n; ++i) a[i] *= factor;
}

__attribute__((target("avx2")))
inline __m256i sum_vec(__m256i x, __m256i y) {
	auto z = _mm256_add_epi32(x, y);
	return _mm256_min_epu32(z, _mm256_sub_epi32(z, _mm256_set1_epi32(2 * mint::mod())));
}

__attribute__((target("avx2")))
inline __m256i diff_vec(__m256i x, __m256i y) {
	auto z = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(z, _mm256_add_epi32(z, _mm256_set1_epi32(2 * mint::mod())));
}

template<int Half, bool Inverse>
__attribute__((target("avx2"), always_inline))
inline __m256i leaf_stage(__m256i x, const unsigned *w, const unsigned *q) {
	constexpr int mask = Half == 4 ? 0xf0 : Half == 2 ? 0xcc : 0xaa;
	__m256i factor{}, quotient{};
	if constexpr (Half == 4) {
		factor = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(w + 4)));
		quotient = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(q + 4)));
	}
	if constexpr (Half == 2) {
		factor = _mm256_broadcastq_epi64(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(w + 2)));
		quotient = _mm256_broadcastq_epi64(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(q + 2)));
	}
	if constexpr (Inverse && Half > 1) x = _mm256_blend_epi32(x, fixed_product(x, factor, quotient), mask);
	__m256i y;
	if constexpr (Half == 4) y = _mm256_permute2x128_si256(x, x, 1);
	if constexpr (Half == 2) y = _mm256_shuffle_epi32(x, 0x4e);
	if constexpr (Half == 1) y = _mm256_shuffle_epi32(x, 0xb1);
	auto high = diff_vec(y, x);
	if constexpr (!Inverse && Half > 1) high = fixed_product(high, factor, quotient);
	return _mm256_blend_epi32(sum_vec(x, y), high, mask);
}

template<bool Inverse>
__attribute__((target("avx2")))
void leaf8(mint *a, size_t n, const mint *roots) {
	alignas(32) unsigned values[8], quotients[8];
	for (int i = 0; i < 8; ++i) {
		values[i] = roots[i].val();
		quotients[i] = (static_cast<unsigned long long>(values[i]) << 32) / mint::mod();
	}
	for (size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		x = leaf_stage<Inverse ? 1 : 4, Inverse>(x, values, quotients);
		x = leaf_stage<2, Inverse>(x, values, quotients);
		x = leaf_stage<Inverse ? 4 : 1, Inverse>(x, values, quotients);
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
}

__attribute__((target("avx2")))
inline void expand_roots(mint *out, const mint *previous, size_t n, mint step) {
	const auto factor = _mm256_set1_epi32(step.v);
	for (size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(previous + i));
		auto y = product(x, factor);
		auto low = _mm256_unpacklo_epi32(x, y), high = _mm256_unpackhi_epi32(x, y);
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(out + 2 * i), _mm256_permute2x128_si256(low, high, 0x20));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(out + 2 * i + 8), _mm256_permute2x128_si256(low, high, 0x31));
	}
}

template<bool Inverse>
__attribute__((target("avx2")))
void vector_stage(mint *a, size_t n, size_t h, const mint *roots) {
	for (size_t start = 0; start < n; start += h * 2)
		for (size_t j = 0; j < h; j += 8) {
			auto *first = reinterpret_cast<__m256i *>(a + start + j);
			auto *last = reinterpret_cast<__m256i *>(a + start + j + h);
			auto x = _mm256_loadu_si256(first), y = _mm256_loadu_si256(last);
			auto w = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			if constexpr (Inverse) y = product(y, w);
			auto sum = sum_vec(x, y), difference = diff_vec(x, y);
			if constexpr (!Inverse) difference = product(difference, w);
			_mm256_storeu_si256(first, sum);
			_mm256_storeu_si256(last, difference);
		}
}
#endif

struct Roots {
	Series forward{0, 1}, inverse{0, 1};
	void prepare(size_t n) {
		size_t begin = forward.size();
		if (begin >= n) return;
		forward.resize(n), inverse.resize(n);
		for (size_t h = begin; h < n; h *= 2) {
			mint step = mint(3).pow((mint::mod() - 1) / (h * 2));
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
			if (simd_available() && h >= 16) expand_roots(forward.data() + h, forward.data() + h / 2, h / 2, step);
			else
#endif
			for (size_t j = 0; j < h / 2; ++j) {
				forward[h + 2 * j] = forward[h / 2 + j];
				forward[h + 2 * j + 1] = forward[h / 2 + j] * step;
			}
			inverse[h] = 1;
			for (size_t j = 1; j < h; ++j) inverse[h + j] = -forward[2 * h - j];
		}
	}
};

inline void pointwise(Series &a, const Series &b, [[maybe_unused]] bool vectorized = simd_available()) {
	assert(a.size() == b.size());
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (vectorized) return pointwise_avx2(a.data(), b.data(), a.size());
#endif
	for (size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
}

inline void scale(Series &a, mint factor, [[maybe_unused]] bool vectorized = simd_available()) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (vectorized) return scale_avx2(a.data(), a.size(), factor);
#endif
	for (auto &x : a) x *= factor;
}

template<bool Inverse>
void stage(mint *a, size_t n, size_t h, const mint *roots, [[maybe_unused]] bool vectorized) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (vectorized && h >= 8) return vector_stage<Inverse>(a, n, h, roots);
#endif
	scalar_stage<Inverse>(a, n, h, roots);
}

inline void transform(Series &a, bool inverse = false) {
	size_t n = a.size();
	assert(std::has_single_bit(n) && n <= (size_t(1) << 23));
	static Roots roots;
	roots.prepare(n);
	bool vectorized = simd_available();
	if (inverse) {
		size_t h = 1;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (vectorized && n >= 8) leaf8<true>(a.data(), n, roots.inverse.data()), h = 8;
#endif
		for (; h < n; h *= 2) stage<true>(a.data(), n, h, roots.inverse.data() + h, vectorized);
		scale(a, mint(n).inv(), vectorized);
	} else {
		size_t h = n / 2;
		for (; h >= 8; h /= 2) stage<false>(a.data(), n, h, roots.forward.data() + h, vectorized);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (vectorized && n >= 8) return leaf8<false>(a.data(), n, roots.forward.data());
#endif
		for (; h; h /= 2) stage<false>(a.data(), n, h, roots.forward.data() + h, vectorized);
	}
}

inline Series multiply(const Series &a, const Series &b) {
	if (a.empty() || b.empty()) return {};
	[[maybe_unused]] constexpr size_t limit = size_t(1) << 23;
	assert(a.size() <= limit && b.size() <= limit && a.size() + b.size() - 1 <= limit);
	size_t length = a.size() + b.size() - 1;
	if (std::min(a.size(), b.size()) <= 32) {
		Series result(length);
		for (size_t i = 0; i < a.size(); ++i)
			for (size_t j = 0; j < b.size(); ++j) result[i + j] += a[i] * b[j];
		return result;
	}
	size_t n = std::bit_ceil(length);
	Series x = a;
	x.resize(n);
	transform(x);
	if (&a == &b) {
		pointwise(x, x);
	} else {
		Series y = b;
		y.resize(n);
		transform(y);
		pointwise(x, y);
	}
	transform(x, true);
	x.resize(length);
	return x;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
