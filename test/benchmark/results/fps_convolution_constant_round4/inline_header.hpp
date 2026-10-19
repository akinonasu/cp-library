#ifndef CP_COMPONENTS_FPS_CONVOLUTION_CONSTANT_HPP
#define CP_COMPONENTS_FPS_CONVOLUTION_CONSTANT_HPP

#include "fps_blocked.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::constant {
inline unsigned canonical(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline bool same_eight(const Mint* p, unsigned value) {
	__m256i v;
	std::memcpy(&v, p, sizeof(v));
	v = _mm256_min_epu32(v, _mm256_sub_epi32(v, _mm256_set1_epi32(Mint::mod())));
	return _mm256_movemask_epi8(_mm256_cmpeq_epi32(v, _mm256_set1_epi32(value))) == -1;
}
inline bool remaining(std::span<const Mint> a, unsigned value) {
	std::size_t i = 8;
	for (; i + 8 <= a.size(); i += 8) if (!same_eight(a.data() + i, value)) return false;
	for (; i < a.size(); ++i) if (canonical(a[i]) != value) return false;
	return true;
}
inline bool fits(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	if (std::min(a.size(), b.size()) < 4096) return false;
	const unsigned av = canonical(a[0]), bv = canonical(b[0]);
	if (!same_eight(a.data(), av) || !same_eight(b.data(), bv)) return false;
	return remaining(a, av) && (&a == &b || remaining(b, bv));
}
inline std::vector<Mint> multiply(std::span<const Mint> a, std::span<const Mint> b) {
	const std::size_t n = std::max(a.size(), b.size()), m = std::min(a.size(), b.size()), size = n + m - 1;
	const unsigned c = canonical(Mint::word(canonical(a[0])) * Mint::word(canonical(b[0])));
	std::vector<Mint> result(size);
	if (c == 0) return result;
	alignas(32) unsigned first[8];
	for (unsigned i = 0; i < 8; ++i) first[i] = std::uint64_t(c) * (i + 1) % Mint::mod();
	__m256i value = _mm256_load_si256(reinterpret_cast<const __m256i*>(first));
	const auto step = _mm256_set1_epi32(std::uint64_t(c) * 8 % Mint::mod()), p = _mm256_set1_epi32(Mint::mod());
	const auto reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	std::size_t i = 0;
	for (; i + 8 <= m; i += 8) {
		std::memcpy(static_cast<void*>(result.data() + i), &value, sizeof(value));
		const auto reflected = _mm256_permutevar8x32_epi32(value, reverse);
		std::memcpy(static_cast<void*>(result.data() + size - i - 8), &reflected, sizeof(reflected));
		value = _mm256_add_epi32(value, step);
		value = _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
	}
	unsigned current = _mm256_extract_epi32(value, 0);
	for (; i < m; ++i) {
		result[i] = result[size - i - 1] = Mint::word(current);
		current += c;
		current = std::min(current, current - Mint::mod());
	}
	if (n > m + 1) std::fill(result.begin() + m, result.begin() + n - 1, Mint::word(std::uint64_t(c) * m % Mint::mod()));
	return result;
}
}
#pragma GCC pop_options
#endif
#endif
