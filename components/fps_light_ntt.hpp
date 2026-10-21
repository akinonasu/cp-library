#ifndef FPS_LIGHT_NTT_HPP
#define FPS_LIGHT_NTT_HPP

#include "../math/mint.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace fps_light_detail {
struct Roots {
	std::vector<mint> forward{0, 1}, inverse{0, 1};
	void prepare(std::size_t n) {
		std::size_t begin = forward.size();
		if (begin >= n) return;
		forward.resize(n), inverse.resize(n);
		for (std::size_t h = begin; h < n; h *= 2) {
			mint step = mint(3).pow((mint::mod() - 1) / (h * 2)), back = step.inv();
			forward[h] = inverse[h] = 1;
			for (std::size_t j = 1; j < h; ++j) {
				forward[h + j] = forward[h + j - 1] * step;
				inverse[h + j] = inverse[h + j - 1] * back;
			}
		}
	}
};

template<bool Inverse>
void scalar_stage(mint *a, std::size_t n, std::size_t h, const mint *roots) {
	for (std::size_t start = 0; start < n; start += h * 2)
		for (std::size_t j = 0; j < h; ++j) {
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

template<bool Inverse>
__attribute__((target("avx2")))
void vector_stage(mint *a, std::size_t n, std::size_t h, const mint *roots) {
	const auto p = _mm256_set1_epi32(2 * mint::mod());
	for (std::size_t start = 0; start < n; start += h * 2)
		for (std::size_t j = 0; j < h; j += 8) {
			auto *first = reinterpret_cast<__m256i *>(a + start + j);
			auto *last = reinterpret_cast<__m256i *>(a + start + j + h);
			auto x = _mm256_loadu_si256(first), y = _mm256_loadu_si256(last);
			auto w = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			if constexpr (Inverse) y = product(y, w);
			auto sum = _mm256_add_epi32(x, y), difference = _mm256_sub_epi32(x, y);
			sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, p));
			difference = _mm256_min_epu32(difference, _mm256_add_epi32(difference, p));
			if constexpr (!Inverse) difference = product(difference, w);
			_mm256_storeu_si256(first, sum);
			_mm256_storeu_si256(last, difference);
		}
}
#endif

template<bool Inverse>
void stage(mint *a, std::size_t n, std::size_t h, const mint *roots, [[maybe_unused]] bool vectorized) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (vectorized && h >= 8) return vector_stage<Inverse>(a, n, h, roots);
#endif
	scalar_stage<Inverse>(a, n, h, roots);
}

inline void transform(std::vector<mint> &a, bool inverse = false) {
	std::size_t n = a.size();
	assert(std::has_single_bit(n) && n <= (std::size_t(1) << 23));
	static Roots roots;
	roots.prepare(n);
	bool vectorized = false;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	vectorized = __builtin_cpu_supports("avx2");
#endif
	if (inverse) {
		for (std::size_t h = 1; h < n; h *= 2) stage<true>(a.data(), n, h, roots.inverse.data() + h, vectorized);
		mint scale = mint(n).inv();
		for (auto &x : a) x *= scale;
	} else {
		for (std::size_t h = n / 2; h; h /= 2) stage<false>(a.data(), n, h, roots.forward.data() + h, vectorized);
	}
}

inline std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b) {
	if (a.empty() || b.empty()) return {};
	[[maybe_unused]] constexpr std::size_t limit = std::size_t(1) << 23;
	assert(a.size() <= limit && b.size() <= limit && a.size() + b.size() - 1 <= limit);
	std::size_t length = a.size() + b.size() - 1;
	if (std::min(a.size(), b.size()) <= 32) {
		std::vector<mint> result(length);
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) result[i + j] += a[i] * b[j];
		return result;
	}
	std::size_t n = std::bit_ceil(length);
	std::vector<mint> x = a;
	x.resize(n);
	transform(x);
	if (&a == &b) {
		for (auto &v : x) v *= v;
	} else {
		std::vector<mint> y = b;
		y.resize(n);
		transform(y);
		for (std::size_t i = 0; i < n; ++i) x[i] *= y[i];
	}
	transform(x, true);
	x.resize(length);
	return x;
}
}

#endif
