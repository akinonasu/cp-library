#ifndef CONVOLUTION_MOD_HPP
#define CONVOLUTION_MOD_HPP

#include "../components/ntt_mod.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

namespace convolution_mod_detail {
template <std::uint32_t mod, std::uint32_t root, class T>
std::vector<std::uint32_t> multiply(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t factor = 1) {
	std::size_t length = a.size() + b.size() - 1, n = std::bit_ceil(length);
	std::vector<std::uint32_t> x(n), y(n);
	auto reduce = [](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	for (std::size_t i = 0; i < a.size(); ++i) x[i] = reduce(a[i]);
	for (std::size_t i = 0; i < b.size(); ++i) y[i] = reduce(b[i]);
	if (std::min(a.size(), b.size()) <= 32) {
		std::vector<std::uint32_t> result(length);
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) result[i + j] = (result[i + j] + std::uint64_t(x[i]) * y[j]) % mod;
		if (factor != 1) for (auto &v : result) v = std::uint64_t(v) * factor % mod;
		return result;
	}
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::Plan<mod, root> plan(n, vectorized);
	ntt_mod_detail::forward(x, plan, vectorized);
	if (&a == &b) y = x;
	else ntt_mod_detail::forward(y, plan, vectorized);
	ntt_mod_detail::pointwise<mod>(x, y, vectorized);
	ntt_mod_detail::backward(x, plan, vectorized);
	std::reverse(x.begin() + 1, x.end());
	std::uint32_t scale = (std::uint64_t(1) << 32) % mod * ntt_mod_detail::power<mod>(n, mod - 2) % mod * factor % mod;
	ntt_mod_detail::scale<mod>(x, scale, vectorized);
	x.resize(length);
	return x;
}
inline constexpr std::uint32_t p = 754974721, q = 1224736769, r = 2013265921;
inline constexpr __uint128_t modulus = __uint128_t(p) * q * r;
struct Reconstruction { std::uint64_t low; std::uint32_t high; };
template <std::uint32_t P = p, std::uint32_t Q = q, std::uint32_t R = r>
inline Reconstruction reconstruct(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	static const std::uint32_t inverse_p = ntt_mod_detail::power<Q>(P, Q - 2);
	static const std::uint32_t inverse_pq = ntt_mod_detail::power<R>(std::uint64_t(P) * Q % R, R - 2);
	std::uint64_t x = std::uint64_t(b + Q - a) * inverse_p % Q;
	std::uint64_t low = a + std::uint64_t(P) * x;
	std::uint64_t y = std::uint64_t(c + R - low % R) * inverse_pq % R;
	return {low, std::uint32_t(y)};
}
inline __uint128_t restore(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	auto [low, high] = reconstruct(a, b, c);
	return low + __uint128_t(p) * q * high;
}
#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t P, std::uint32_t Q, std::uint32_t R>
__attribute__((target("avx2")))
void combine(std::uint32_t *a, const std::uint32_t *b, const std::uint32_t *c, std::size_t n) {
	static_assert(P > 256 && Q > 256 && R > 256 && P + std::uint64_t(Q) + R < (std::uint64_t(1) << 32));
	constexpr std::uint32_t mod = 1000000007;
	constexpr auto form = [](std::uint64_t x) { return std::uint32_t(x % mod * ((std::uint64_t(1) << 32) % mod) % mod); };
	constexpr std::uint32_t place[3] = {form(std::uint64_t(Q) * R), form(std::uint64_t(P) * R), form(std::uint64_t(P) * Q)};
	constexpr std::uint32_t wrap = form(mod - std::uint64_t(P) * Q % mod * R % mod);
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
		auto q = std::uint32_t(sum) * ntt_mod_detail::negative_inverse<mod>;
		std::uint32_t value = (sum + std::uint64_t(q) * mod) >> 32;
		a[i] = value < mod ? value : value - mod;
	}
}
#endif
template <std::uint32_t P, std::uint32_t PR, std::uint32_t Q, std::uint32_t QR, std::uint32_t R, std::uint32_t RR>
std::vector<std::uint32_t> modular(const std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, std::uint32_t mod) {
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

#endif
