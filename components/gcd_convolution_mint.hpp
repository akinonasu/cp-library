#ifndef GCD_CONVOLUTION_MINT_HPP
#define GCD_CONVOLUTION_MINT_HPP

#include "divisor_zeta.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace mint_detail { struct Mint; }

namespace gcd_convolution_mint_detail {

#if defined(__x86_64__) || defined(__i386__)
template <bool inverse, class T>
__attribute__((target("avx2")))
void prime_pass(T *a, T *b, std::size_t n, const std::vector<std::size_t> &ps, std::size_t count) {
	const __m256i mod = _mm256_set1_epi32(2 * T::mod());
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto scalar = [&](std::size_t i) {
			if constexpr (inverse) a[i] -= a[i * p];
			else a[i] += a[i * p], b[i] += b[i * p];
		};
		auto gather = [&](const T *s) __attribute__((target("avx2"))) {
			return _mm256_setr_epi32(s[0].v, s[p].v, s[2 * p].v, s[3 * p].v,
									s[4 * p].v, s[5 * p].v, s[6 * p].v, s[7 * p].v);
		};
		auto vector = [&](std::size_t i) __attribute__((target("avx2"))) {
			__m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)), y = gather(a + i * p);
			if constexpr (inverse) {
				x = _mm256_sub_epi32(x, y);
				x = _mm256_min_epu32(x, _mm256_add_epi32(x, mod));
			} else {
				x = _mm256_add_epi32(x, y);
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, mod));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
			if constexpr (!inverse) {
				x = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), gather(b + i * p));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, mod));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(b + i), x);
			}
		};
		if constexpr (inverse) {
			std::size_t i = 1;
			for (; i <= end && i < 8; ++i) scalar(i);
			for (; i + 7 <= end; i += 8) vector(i);
			for (; i <= end; ++i) scalar(i);
		} else {
			std::size_t i = end;
			for (; i >= 15; i -= 8) vector(i - 7);
			for (; i; --i) scalar(i);
		}
	}
}
#endif

template <bool inverse, class T>
void transform(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps, std::size_t count) {
#if defined(__x86_64__) || defined(__i386__)
	if (__builtin_cpu_supports("avx2")) {
		prime_pass<inverse>(a.data(), b.data(), a.size(), ps, count);
		return;
	}
#endif
	divisor_zeta_detail::for_each<inverse, true>(a.size(), ps, count, [&](std::size_t to, std::size_t from) {
		if constexpr (inverse) a[to] -= a[from];
		else a[to] += a[from], b[to] += b[from];
	});
}

template <bool inverse, class T>
void large_multiples(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps,
					 std::size_t first) {
	if (first == ps.size()) return;
	constexpr std::uint64_t twice = 2 * T::mod();
	const std::size_t n = a.size() - 1;
	for (std::size_t i = 1; i <= n / ps[first]; ++i) {
		const auto end = std::upper_bound(ps.begin() + first, ps.end(), n / i);
		std::uint64_t x = inverse ? 0 : a[i].v, y = inverse ? a[i].v : b[i].v;
		for (auto p = ps.begin() + first; p != end; ++p) {
			x += a[i * *p].v;
			if constexpr (!inverse) y += b[i * *p].v;
		}
		if constexpr (inverse) a[i] = T::word(y % twice) - T::word(x % twice);
		else {
			a[i] = T::word(x % twice), b[i] = T::word(y % twice);
		}
	}
}

template <class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	const std::size_t n = a.size() - 1;
	std::size_t cutoff = std::sqrt(n);
	while (cutoff + 1 <= n / (cutoff + 1)) ++cutoff;
	while (cutoff && cutoff > n / cutoff) --cutoff;
	const std::size_t first = std::upper_bound(ps.begin(), ps.end(), cutoff) - ps.begin();
	transform<false>(a, b, ps, first);
	large_multiples<false>(a, b, ps, first);
	for (std::size_t i = 1; i <= n; ++i) a[i] *= b[i];
	transform<true>(a, b, ps, first);
	large_multiples<true>(a, b, ps, first);
}

}

#endif
