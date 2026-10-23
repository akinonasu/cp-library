#ifndef BITWISE_CONVOLUTION_MODULAR_HPP
#define BITWISE_CONVOLUTION_MODULAR_HPP

#include <algorithm>
#include <cstddef>
#include <vector>
#include <type_traits>
#include "mint_arithmetic.hpp"

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

#endif
