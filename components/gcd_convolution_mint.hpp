#ifndef GCD_CONVOLUTION_MINT_HPP
#define GCD_CONVOLUTION_MINT_HPP

#include "divisor_zeta.hpp"
#include <algorithm>
#include <bit>
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
__attribute__((target("avx2"), always_inline)) inline
void update8(T *a, T *b, std::size_t i, std::size_t p) {
	const __m256i mod = _mm256_set1_epi32(2 * T::mod());
	auto gather = [&](const T *s) __attribute__((target("avx2"))) {
		return _mm256_setr_epi32(s[0].v, s[p].v, s[2 * p].v, s[3 * p].v,
								s[4 * p].v, s[5 * p].v, s[6 * p].v, s[7 * p].v);
	};
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
}

template <bool inverse, class T>
__attribute__((target("avx2")))
void prime_pass(T *a, T *b, std::size_t n, const std::vector<std::size_t> &ps, std::size_t count) {
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto scalar = [&](std::size_t i) {
			if constexpr (inverse) a[i] -= a[i * p];
			else a[i] += a[i * p], b[i] += b[i * p];
		};
		if constexpr (inverse) {
			std::size_t i = 1;
			for (; i <= end && i < 8; ++i) scalar(i);
			for (; i + 7 <= end; i += 8) update8<inverse>(a, b, i, p);
			for (; i <= end; ++i) scalar(i);
		} else {
			std::size_t i = end;
			for (; i >= 15; i -= 8) update8<inverse>(a, b, i - 7, p);
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

#if defined(__x86_64__) || defined(__i386__)
template <bool inverse, class T>
void partial_transform(std::vector<T> &a, std::vector<T> &b, const divisor_zeta_detail::Stage &stage, std::uint32_t last) {
	constexpr std::uint64_t twice = 2 * T::mod();
	const auto &ds = stage.divisors;
	auto evaluate = [&](std::uint32_t i) {
		std::uint32_t limit = last / i;
		std::uint64_t x = 0, y = 0, xx = 0, yy = 0;
		std::size_t j = 0;
		for (; j + 1 < ds.size() && ds[j + 1] <= limit; j += 2) {
			std::size_t p = std::size_t(i) * ds[j], q = std::size_t(i) * ds[j + 1];
			x += a[p].v, xx += a[q].v;
			if constexpr (!inverse) y += b[p].v, yy += b[q].v;
		}
		if (j < ds.size() && ds[j] <= limit) {
			std::size_t p = std::size_t(i) * ds[j];
			x += a[p].v;
			if constexpr (!inverse) y += b[p].v;
		}
		if constexpr (inverse) a[i] -= T::word(std::uint32_t((x + xx) % twice));
		else {
			a[i] = T::word(std::uint32_t((x + xx + a[i].v) % twice));
			b[i] = T::word(std::uint32_t((y + yy + b[i].v) % twice));
		}
	};
	std::uint32_t end = last / ds[0];
	if constexpr (inverse) for (std::uint32_t i = end; i; --i) evaluate(i);
	else for (std::uint32_t i = 1; i <= end; ++i) evaluate(i);
}

template <bool inverse, class T>
__attribute__((target("avx2")))
void stage_pass(T *a, T *b, std::uint32_t first, std::uint32_t last, std::uint32_t p) {
	std::uint32_t i = first;
	for (; i + 7 <= last; i += 8) update8<inverse>(a, b, i, p);
	for (; i <= last; ++i) {
		if constexpr (inverse) a[i] -= a[std::size_t(i) * p];
		else a[i] += a[std::size_t(i) * p], b[i] += b[std::size_t(i) * p];
	}
}

template <bool inverse, class T>
void stage_transform(std::vector<T> &a, std::vector<T> &b, const divisor_zeta_detail::Stage &stage,
					 std::uint32_t block, std::uint32_t split) {
	constexpr std::uint64_t twice = 2 * T::mod();
	const auto &ds = stage.divisors;
	if (ds.empty()) return;
	std::uint32_t n = std::uint32_t(a.size() - 1);
	std::size_t large = std::size_t(std::upper_bound(ds.begin(), ds.end(), split) - ds.begin());
	std::uint64_t large_first = large < ds.size() ? ds[large] : std::uint64_t(n) + 1;
	std::vector<std::size_t> cursor(std::size_t(n / large_first) + 1, large);
	std::vector<std::uint32_t> next(large);
	if constexpr (!inverse) {
		std::uint32_t first_last = std::min(n, block);
		partial_transform<false>(a, b, stage, first_last);
		for (std::size_t j = 0; j < large; ++j) next[j] = first_last / ds[j] + 1;
		for (std::uint32_t i = 1; i <= first_last / large_first; ++i)
			cursor[i] = std::size_t(std::upper_bound(ds.begin() + large, ds.end(), first_last / i) - ds.begin());
		for (std::uint64_t first = std::uint64_t(first_last) + 1; first <= n; first += block) {
			std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
			for (std::size_t j = 0; j < large; ++j) {
				std::uint32_t end = last / ds[j];
				stage_pass<false>(a.data(), b.data(), next[j], end, ds[j]);
				next[j] = end + 1;
			}
			for (std::uint32_t i = 1; i <= last / large_first; ++i) {
				std::uint32_t limit = last / i;
				std::size_t j = cursor[i];
				std::uint64_t x = a[i].v, y = b[i].v, xx = 0, yy = 0;
				for (; j + 1 < ds.size() && ds[j + 1] <= limit; j += 2) {
					std::size_t p = std::size_t(i) * ds[j], q = std::size_t(i) * ds[j + 1];
					x += a[p].v, y += b[p].v, xx += a[q].v, yy += b[q].v;
				}
				if (j < ds.size() && ds[j] <= limit) {
					std::size_t p = std::size_t(i) * ds[j++];
					x += a[p].v, y += b[p].v;
				}
				a[i] = T::word(std::uint32_t((x + xx) % twice));
				b[i] = T::word(std::uint32_t((y + yy) % twice));
				cursor[i] = j;
			}
		}
	} else {
		for (std::size_t j = 0; j < large; ++j) next[j] = n / ds[j];
		for (std::uint32_t i = 1; i < cursor.size(); ++i)
			cursor[i] = std::size_t(std::upper_bound(ds.begin() + large, ds.end(), n / i) - ds.begin());
		for (std::uint64_t first = std::uint64_t(n / block) * block + 1; first > block; first -= block) {
			if (first > n) continue;
			std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
			for (std::size_t j = 0; j < large; ++j) {
				std::uint32_t start = std::uint32_t((first + ds[j] - 1) / ds[j]);
				stage_pass<true>(a.data(), b.data(), start, next[j], ds[j]);
				next[j] = start - 1;
			}
			for (std::uint32_t i = 1; i <= last / large_first; ++i) {
				std::uint32_t limit = std::uint32_t((first + i - 1) / i);
				std::size_t j = cursor[i];
				std::uint64_t x = 0, xx = 0;
				for (; j >= large + 2 && ds[j - 2] >= limit; j -= 2)
					x += a[std::size_t(i) * ds[j - 1]].v, xx += a[std::size_t(i) * ds[j - 2]].v;
				if (j > large && ds[j - 1] >= limit) x += a[std::size_t(i) * ds[--j]].v;
				a[i] -= T::word(std::uint32_t((x + xx) % twice));
				cursor[i] = j;
			}
		}
		partial_transform<true>(a, b, stage, std::min(n, block));
	}
}

template <class T>
void blocked_multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	std::uint32_t n = std::uint32_t(a.size() - 1);
	if (!n) return;
	std::size_t small;
	auto stages = divisor_zeta_detail::make_plan(n, ps, small);
	std::uint32_t block = std::max(32768u, std::uint32_t(1) << ((std::bit_width(n) + 1) / 2));
	std::uint32_t split = std::max(4096u, std::uint32_t((std::uint64_t(n) + block - 1) / block));
	transform<false>(a, b, ps, small);
	for (const auto &stage : stages) stage_transform<false>(a, b, stage, block, split);
	for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
	for (auto it = stages.rbegin(); it != stages.rend(); ++it) stage_transform<true>(a, b, *it, block, split);
	transform<true>(a, b, ps, small);
}
#endif

template <class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	const std::size_t n = a.size() - 1;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= (std::size_t(1) << 19) && __builtin_cpu_supports("avx2")) {
		blocked_multiply(a, b, ps);
		return;
	}
#endif
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
