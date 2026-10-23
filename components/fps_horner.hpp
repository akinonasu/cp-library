#ifndef CP_COMPONENTS_FPS_HORNER_HPP
#define CP_COMPONENTS_FPS_HORNER_HPP

#include "mint_arithmetic.hpp"
#include <array>
#include <cstring>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
using mint_detail::Ops;
namespace horner_detail {
template<int K, class T>
inline T scalar(const T* a, std::size_t n, T x) {
	static_assert(K > 0);
	T step = 1;
	for (int i = 0; i < K; ++i) step *= x;
	std::array<T, K> sum{};
	std::size_t groups = n / K, tail = n % K;
	for (std::size_t i = 0; i < tail; ++i) sum[i] = a[groups * K + i];
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) sum[i] = sum[i] * step + a[groups * K + i];
	}
	T result = 0;
	for (int i = K; i-- > 0;) result = result * x + sum[i];
	return result;
}

#if defined(__x86_64__) || defined(__i386__)
template<int K>
__attribute__((target("avx2"))) inline mint vector(const mint* a, std::size_t n, mint x) {
	static_assert(K == 1 || K == 2 || K == 4);
	using V = Ops::I8;
	static_assert(std::is_trivially_copyable_v<mint> && sizeof(mint) == sizeof(unsigned));
	mint step = x;
	for (int i = 0; i < 3; ++i) step *= step;
	const unsigned x8 = step.val();
	for (int i = 1; i < K; i *= 2) step *= step;
	const unsigned value = step.val();
	const V factor = _mm256_set1_epi32(value), quotient = _mm256_set1_epi32(Ops::quotient(value));
	V sum[K];
	for (auto& v : sum) v = _mm256_setzero_si256();
	constexpr std::size_t width = 8 * K;
	std::size_t groups = n / width, tail = n % width;
	if (tail) {
		alignas(32) mint block[width]{};
		std::memcpy(static_cast<void*>(block), a + groups * width, tail * sizeof(mint));
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], block + 8 * i, sizeof(V));
	} else if (groups) {
		--groups;
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], a + groups * width + 8 * i, sizeof(V));
	}
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) {
			V coefficient;
			std::memcpy(&coefficient, a + groups * width + 8 * i, sizeof(V));
			sum[i] = Ops::add8(value == 1 ? sum[i] : Ops::fixed_mul8(sum[i], factor, quotient), coefficient);
		}
	}
	const V step8 = _mm256_set1_epi32(x8), quotient8 = _mm256_set1_epi32(Ops::quotient(x8));
	V joined = sum[K - 1];
	for (int i = K - 1; i-- > 0;) joined = Ops::add8(Ops::fixed_mul8(joined, step8, quotient8), sum[i]);
	alignas(32) mint word[8];
	std::memcpy(static_cast<void*>(word), &joined, sizeof(joined));
	mint result = 0;
	for (int i = 8; i-- > 0;) result = result * x + word[i];
	return result;
}
#endif
}

inline mint horner(const mint* a, std::size_t n, mint x) {
	if (!n) return mint(0);
	if (x == mint(0)) return a[0];
#if defined(__x86_64__) || defined(__i386__)
	if (n >= 32 && Ops::vectorized()) {
		if (n < 64) return horner_detail::vector<1>(a, n, x);
		return horner_detail::vector<4>(a, n, x);
	}
#endif
	if (n >= 32) return horner_detail::scalar<8>(a, n, x);
	mint result = 0;
	while (n) result = result * x + a[--n];
	return result;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
