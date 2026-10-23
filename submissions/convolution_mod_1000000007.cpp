#line 1 "test/convolution_mod.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod_1000000007"
#line 1 "convolution/convolution_mod.hpp"



#line 1 "components/ntt_mod.hpp"



#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace ntt_mod_detail {
template <std::uint32_t mod>
std::uint32_t power(std::uint32_t a, std::uint32_t k) {
	std::uint32_t result = 1;
	for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
		if (k & 1) result = std::uint64_t(result) * a % mod;
	return result;
}

template <std::uint32_t mod>
void butterfly(std::uint32_t *a, std::size_t n, std::size_t half,
			   const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], v = a[start + j + half];
			std::uint32_t q = std::uint64_t(v) * quotients[j] >> 32;
			std::uint32_t y = v * roots[j] - q * mod;
			if (y >= mod) y -= mod;
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			a[start + j + half] = x >= y ? x - y : x + mod - y;
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void butterfly_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
					const std::uint32_t *roots, const std::uint32_t *quotients) {
	const auto p = _mm256_set1_epi32(mod);
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + half));
			auto r = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			auto q = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + j));
			auto even = _mm256_mul_epu32(y, q);
			auto odd = _mm256_mul_epu32(_mm256_srli_epi64(y, 32), _mm256_srli_epi64(q, 32));
			q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			y = _mm256_sub_epi32(_mm256_mullo_epi32(y, r), _mm256_mullo_epi32(q, p));
			y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			auto sum = _mm256_add_epi32(x, y), difference = _mm256_sub_epi32(x, y);
			sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, p));
			difference = _mm256_min_epu32(difference, _mm256_add_epi32(difference, p));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), sum);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + half), difference);
		}
}
#endif

template <std::uint32_t mod, std::uint32_t root>
void transform(std::vector<std::uint32_t> &a, bool inverse, [[maybe_unused]] bool vectorized) {
	static_assert(mod > 2 && mod < (std::uint32_t(1) << 31));
	std::size_t n = a.size();
	assert(std::has_single_bit(n) && (mod - 1) % n == 0);
	for (std::size_t i = 1, j = 0; i < n; ++i) {
		std::size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	std::vector<std::uint32_t> roots(n > 16 ? n / 2 : 0), quotients(roots.size());
	for (std::size_t width = 2; width <= n; width *= 2) {
		std::uint32_t step = ntt_mod_detail::power<mod>(root, (mod - 1) / width);
		if (inverse) step = ntt_mod_detail::power<mod>(step, mod - 2);
		if (n <= 16) {
			for (std::size_t start = 0; start < n; start += width) {
				std::uint32_t w = 1;
				for (std::size_t j = 0; j < width / 2; ++j) {
					std::uint32_t x = a[start + j], y = std::uint64_t(a[start + j + width / 2]) * w % mod;
					a[start + j] = x + y < mod ? x + y : x + y - mod;
					a[start + j + width / 2] = x >= y ? x - y : x + mod - y;
					w = std::uint64_t(w) * step % mod;
				}
			}
			continue;
		}
		std::uint32_t w = 1;
		for (std::size_t j = 0; j < width / 2; ++j) {
			roots[j] = w;
			quotients[j] = (std::uint64_t(w) << 32) / mod;
			w = std::uint64_t(w) * step % mod;
		}
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized && width >= 16) ntt_mod_detail::butterfly_avx2<mod>(a.data(), n, width / 2, roots.data(), quotients.data());
		else
#endif
		ntt_mod_detail::butterfly<mod>(a.data(), n, width / 2, roots.data(), quotients.data());
	}
	if (inverse) {
		std::uint32_t inv = ntt_mod_detail::power<mod>(n, mod - 2);
		for (auto &x : a) x = std::uint64_t(x) * inv % mod;
	}
}
}

template <std::uint32_t mod, std::uint32_t root>
void ntt_mod(std::vector<std::uint32_t> &a, bool inverse = false) {
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::transform<mod, root>(a, inverse, vectorized);
}


#line 9 "convolution/convolution_mod.hpp"
#include <limits>
#line 11 "convolution/convolution_mod.hpp"

namespace convolution_mod_detail {
template <std::uint32_t mod, std::uint32_t root, class T>
std::vector<std::uint32_t> multiply(const std::vector<T> &a, const std::vector<T> &b) {
	std::size_t length = a.size() + b.size() - 1, n = std::bit_ceil(length);
	std::vector<std::uint32_t> x(n), y(n);
	auto reduce = [](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	for (std::size_t i = 0; i < a.size(); ++i) x[i] = reduce(a[i]);
	for (std::size_t i = 0; i < b.size(); ++i) y[i] = reduce(b[i]);
	ntt_mod<mod, root>(x, false);
	if (&a == &b) y = x;
	else ntt_mod<mod, root>(y, false);
	for (std::size_t i = 0; i < n; ++i) x[i] = std::uint64_t(x[i]) * y[i] % mod;
	ntt_mod<mod, root>(x, true);
	x.resize(length);
	return x;
}
inline constexpr std::uint32_t p = 754974721, q = 1224736769, r = 2013265921;
inline constexpr __uint128_t modulus = __uint128_t(p) * q * r;
inline __uint128_t restore(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	static const std::uint32_t inverse_p = ntt_mod_detail::power<q>(p, q - 2);
	static const std::uint32_t inverse_pq = ntt_mod_detail::power<r>(std::uint64_t(p) * q % r, r - 2);
	std::uint64_t x = std::uint64_t(b + q - a) * inverse_p % q;
	std::uint64_t low = a + std::uint64_t(p) * x;
	std::uint64_t y = std::uint64_t(c + r - low % r) * inverse_pq % r;
	return low + __uint128_t(p) * q * y;
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
	std::vector<std::uint32_t> x(a.size()), y(&a == &b ? 0 : b.size());
	auto reduce = [=](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	std::transform(a.begin(), a.end(), x.begin(), reduce);
	if (&a != &b) std::transform(b.begin(), b.end(), y.begin(), reduce);
	const auto &other = &a == &b ? x : y;
	if (mod == 998244353 && x.size() + other.size() - 1 <= std::size_t(1) << 23) return convolution_mod_detail::multiply<998244353, 3>(x, other);
	auto p = convolution_mod_detail::multiply<convolution_mod_detail::p, 11>(x, other);
	auto q = convolution_mod_detail::multiply<convolution_mod_detail::q, 3>(x, other);
	auto r = convolution_mod_detail::multiply<convolution_mod_detail::r, 31>(x, other);
	for (std::size_t i = 0; i < p.size(); ++i) p[i] = convolution_mod_detail::restore(p[i], q[i], r[i]) % mod;
	return p;
}


#line 4 "test/convolution_mod.test.cpp"
#include <iostream>
#line 6 "test/convolution_mod.test.cpp"

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	std::cin >> n >> m;
	std::vector<std::uint32_t> a(n), b(m);
	for (auto &x : a) std::cin >> x;
	for (auto &x : b) std::cin >> x;
	for (auto x : convolution_mod(a, b, 1000000007)) std::cout << x << ' ';
	std::cout << '\n';
}
