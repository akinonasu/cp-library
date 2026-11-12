#ifndef CONVOLUTION_MOD_CORE_HPP
#define CONVOLUTION_MOD_CORE_HPP

#include "ntt_mod_core.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace convolution_mod_detail {
using Word = std::uint32_t;
struct PortableBackend {
	static constexpr bool accelerated = false, specialized = false;
	struct Storage { Word *get() const { return nullptr; } };
	template <Word... Mods> static Storage workspace(std::size_t, std::size_t) { return {}; }
};
#ifndef CP_CONVOLUTION_MOD_BACKEND
#define CP_CONVOLUTION_MOD_BACKEND 1
using DefaultBackend = PortableBackend;
#endif
template <class Backend, Word mod, Word root, class T>
std::vector<Word> multiply_as(const std::vector<T> &a, const std::vector<T> &b, Word factor = 1, Word *workspace = nullptr) {
	const auto &small = a.size() <= b.size() ? a : b;
	const auto &large = a.size() <= b.size() ? b : a;
	std::size_t length = a.size() + b.size() - 1;
	bool blocked = small.size() > 32 && large.size() / small.size() >= 8, square = &a == &b;
	std::size_t n = std::bit_ceil(blocked ? 2 * small.size() - 1 : length), block = n - small.size() + 1;
	(void)workspace;
	if constexpr (Backend::accelerated && mod < (Word(1) << 30))
		if (Backend::enabled(small.size(), n)) return Backend::template multiply<mod, root>(a, b, factor, workspace);
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
template <class Backend, Word P, Word PR, Word Q, Word QR, Word R, Word RR>
std::vector<Word> modular_as(const std::vector<Word> &a, const std::vector<Word> &b, Word mod) {
	auto storage = Backend::template workspace<P, Q, R>(a.size(), b.size());
	Word *workspace = storage.get();
#if defined(__x86_64__) || defined(__i386__)
	if constexpr (P + std::uint64_t(Q) + R < (std::uint64_t(1) << 32)) {
		if (mod == 1000000007 && __uint128_t(std::min(a.size(), b.size())) * (mod - 1) * (mod - 1) * 4 < __uint128_t(P) * Q * R && __builtin_cpu_supports("avx2")) {
			auto x = multiply_as<Backend, P, PR>(a, b, ntt_mod_detail::power<P>(std::uint64_t(Q) * R % P, P - 2), workspace);
			auto y = multiply_as<Backend, Q, QR>(a, b, ntt_mod_detail::power<Q>(std::uint64_t(P) * R % Q, Q - 2), workspace);
			auto z = multiply_as<Backend, R, RR>(a, b, ntt_mod_detail::power<R>(std::uint64_t(P) * Q % R, R - 2), workspace);
			combine<P, Q, R>(x.data(), y.data(), z.data(), x.size());
			return x;
		}
	}
#endif
	auto x = multiply_as<Backend, P, PR>(a, b, 1, workspace);
	auto y = multiply_as<Backend, Q, QR>(a, b, 1, workspace);
	auto z = multiply_as<Backend, R, RR>(a, b, 1, workspace);
	std::uint64_t pq_mod = std::uint64_t(P) * Q % mod;
	for (std::size_t i = 0; i < x.size(); ++i) {
		auto [low, high] = reconstruct<P, Q, R>(x[i], y[i], z[i]);
		x[i] = (low % mod + pq_mod * high) % mod;
	}
	return x;
}

template <class T, class Backend = DefaultBackend>
std::vector<T> convolution_integer(const std::vector<T> &a, const std::vector<T> &b) {
	static_assert(std::numeric_limits<T>::is_integer && sizeof(T) <= 8);
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() - 1 <= std::size_t(1) << 24);
	auto x = multiply_as<Backend, p, 11>(a, b);
	auto y = multiply_as<Backend, q, 3>(a, b);
	auto z = multiply_as<Backend, r, 31>(a, b);
	std::vector<T> result(x.size());
	for (std::size_t i = 0; i < x.size(); ++i) {
		auto value = restore(x[i], y[i], z[i]);
		if constexpr (std::numeric_limits<T>::is_signed) {
			__int128_t v = value > modulus / 2 ? __int128_t(value) - __int128_t(modulus) : __int128_t(value);
			assert(std::numeric_limits<T>::lowest() <= v && v <= std::numeric_limits<T>::max());
			result[i] = T(v);
		} else {
			assert(value <= std::numeric_limits<T>::max());
			result[i] = T(value);
		}
	}
	return result;
}

template <class T, class Backend = DefaultBackend>
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
	if (mod == 998244353 && x.size() + other.size() - 1 <= std::size_t(1) << 23) return multiply_as<Backend, 998244353, 3>(x, other);
	constexpr std::uint32_t P = 167772161, Q = 469762049, R = 754974721;
	const __uint128_t bound = __uint128_t(std::min(x.size(), other.size())) * (mod - 1) * (mod - 1);
	if constexpr (Backend::specialized)
		if (Backend::fits(mod, bound)) return Backend::convolution(x, other, mod, bound);
	if (bound < __uint128_t(P) * Q * R)
		return modular_as<Backend, P, 3, Q, 3, R, 11>(x, other, mod);
	return modular_as<Backend, p, 11, q, 3, r, 31>(x, other, mod);
}

}

using convolution_mod_detail::convolution_integer;
using convolution_mod_detail::convolution_mod;

#endif
