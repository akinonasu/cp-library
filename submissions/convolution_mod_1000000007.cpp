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
template <std::uint32_t mod> struct Vector;

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
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + half));
			auto r = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			auto q = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + j));
			y = V::multiply(y, r, q);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), V::add(x, y));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + half), V::subtract(x, y));
		}
}
#endif

template <std::uint32_t mod>
std::uint32_t shoup(std::uint32_t value, std::uint32_t factor, std::uint32_t quotient) {
	std::uint32_t q = std::uint64_t(value) * quotient >> 32;
	std::uint32_t result = value * factor - q * mod;
	return result < mod ? result : result - mod;
}


template <std::uint32_t mod>
void forward_stage(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], y = a[start + j + half];
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			std::uint32_t difference = x >= y ? x - y : x + mod - y;
			a[start + j + half] = shoup<mod>(difference, roots[j], quotients[j]);
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void forward_stage_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + half));
			auto r = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + j));
			auto q = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + j));
			auto difference = V::multiply(V::subtract(x, y), r, q);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), V::add(x, y));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + half), difference);
		}
}
#endif

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
struct Vector {
	static constexpr std::uint32_t range = mod < (std::uint32_t(1) << 30) ? 2 * mod : mod;
	__attribute__((target("avx2"), always_inline))
	static inline __m256i add(__m256i x, __m256i y) {
		auto sum = _mm256_add_epi32(x, y);
		return _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i subtract(__m256i x, __m256i y) {
		auto difference = _mm256_sub_epi32(x, y);
		return _mm256_min_epu32(difference, _mm256_add_epi32(difference, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i multiply(__m256i x, __m256i factor, __m256i quotient) {
		auto even = _mm256_mul_epu32(x, quotient);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(quotient, 32));
		auto q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
		auto result = _mm256_sub_epi32(_mm256_mullo_epi32(x, factor), _mm256_mullo_epi32(q, _mm256_set1_epi32(mod)));
		if constexpr (range == 2 * mod) return result;
		return _mm256_min_epu32(result, _mm256_sub_epi32(result, _mm256_set1_epi32(mod)));
	}
};

template <std::uint32_t mod, bool inverse>
__attribute__((target("avx2")))
void leaf_avx2(std::uint32_t *a, std::size_t n, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	auto r4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(roots + 4)));
	auto q4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(quotients + 4)));
	auto r2 = _mm256_setr_epi32(roots[2], roots[3], roots[2], roots[3], roots[2], roots[3], roots[2], roots[3]);
	auto q2 = _mm256_setr_epi32(quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3]);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		if constexpr (!inverse) {
			auto y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r4, q4), 0xf0);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
		} else {
			auto y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
			x = _mm256_blend_epi32(x, V::multiply(x, r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xcc);
			x = _mm256_blend_epi32(x, V::multiply(x, r4, q4), 0xf0);
			y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xf0);
		}
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
}

template <std::uint32_t mod, bool inverse>
__attribute__((target("avx2")))
void pair_avx2(std::uint32_t *a, std::size_t n, std::size_t h, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 4 * h)
		for (std::size_t j = 0; j < h; j += 8) {
			auto x0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
			auto x1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + h));
			auto x2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + 2 * h));
			auto x3 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j + 3 * h));
			auto r0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + 2 * h + j));
			auto q0 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + 2 * h + j));
			auto r1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + 3 * h + j));
			auto q1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + 3 * h + j));
			auto r2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(roots + h + j));
			auto q2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(quotients + h + j));
			if constexpr (!inverse) {
				auto y0 = V::add(x0, x2), y1 = V::add(x1, x3);
				auto y2 = V::multiply(V::subtract(x0, x2), r0, q0);
				auto y3 = V::multiply(V::subtract(x1, x3), r1, q1);
				x0 = V::add(y0, y1); x1 = V::multiply(V::subtract(y0, y1), r2, q2);
				x2 = V::add(y2, y3); x3 = V::multiply(V::subtract(y2, y3), r2, q2);
			} else {
				x1 = V::multiply(x1, r2, q2); x3 = V::multiply(x3, r2, q2);
				auto y0 = V::add(x0, x1), y1 = V::subtract(x0, x1);
				auto y2 = V::multiply(V::add(x2, x3), r0, q0);
				auto y3 = V::multiply(V::subtract(x2, x3), r1, q1);
				x0 = V::add(y0, y2); x2 = V::subtract(y0, y2);
				x1 = V::add(y1, y3); x3 = V::subtract(y1, y3);
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + h), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + 2 * h), x2);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + start + j + 3 * h), x3);
		}
}

#endif

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void build_roots_avx2(std::uint32_t *roots, std::uint32_t *quotients, std::size_t half, std::uint32_t step) {
	std::uint32_t initial[8], w = 1;
	for (int i = 0; i < 8; ++i) {
		initial[i] = w;
		w = std::uint64_t(w) * step % mod;
	}
	auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(initial));
	auto factor = _mm256_set1_epi32(w), quotient = _mm256_set1_epi32((std::uint64_t(w) << 32) / mod);
	for (std::size_t j = 0; j < half; j += 8) {
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(mod)));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(roots + j), x);
		for (int k = 0; k < 8; ++k) quotients[j + k] = (std::uint64_t(roots[j + k]) << 32) / mod;
		x = Vector<mod>::multiply(x, factor, quotient);
	}
}
#endif

template <std::uint32_t mod, std::uint32_t root>
struct Plan {
	std::vector<std::uint32_t> roots, quotients;
	explicit Plan(std::size_t n, [[maybe_unused]] bool vectorized = false) : roots(n), quotients(n) {
		static_assert(mod > 2 && mod < (std::uint32_t(1) << 31));
		assert(std::has_single_bit(n) && (mod - 1) % n == 0);
		if (n == 1) return;
		std::size_t half = n / 2;
		std::uint32_t step = power<mod>(root, (mod - 1) / n), w = 1;
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized && half >= 8) build_roots_avx2<mod>(roots.data() + half, quotients.data() + half, half, step);
		else
#endif
		for (std::size_t j = 0; j < half; ++j) {
			roots[half + j] = w;
			quotients[half + j] = (std::uint64_t(w) << 32) / mod;
			w = std::uint64_t(w) * step % mod;
		}
		for (half /= 2; half; half /= 2)
			for (std::size_t j = 0; j < half; ++j) {
				roots[half + j] = roots[2 * half + 2 * j];
				quotients[half + j] = quotients[2 * half + 2 * j];
			}
	}
};

template <std::uint32_t mod, std::uint32_t root>
void forward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = a.size() / 2;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		for (; half >= 16; half /= 4) pair_avx2<mod, false>(a.data(), a.size(), half / 2, plan.roots.data(), plan.quotients.data());
		if (half == 8) forward_stage_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		leaf_avx2<mod, false>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		return;
	}
#endif
	for (; half; half /= 2) forward_stage<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

template <std::uint32_t mod, std::uint32_t root>
void backward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = 1;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		leaf_avx2<mod, true>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		half = 8;

		for (; 2 * half < a.size(); half *= 4) pair_avx2<mod, true>(a.data(), a.size(), half, plan.roots.data(), plan.quotients.data());
		if (half < a.size()) butterfly_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		return;
	}
#endif
	for (; half < a.size(); half *= 2) butterfly<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void scale_avx2(std::uint32_t *a, std::size_t n, std::uint32_t factor, std::uint32_t quotient) {
	auto r = _mm256_set1_epi32(factor), q = _mm256_set1_epi32(quotient);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = Vector<mod>::multiply(x, r, q);
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), _mm256_min_epu32(y, _mm256_sub_epi32(y, _mm256_set1_epi32(mod))));
	}
}
#endif

template <std::uint32_t mod>
void scale(std::vector<std::uint32_t> &a, std::uint32_t factor, [[maybe_unused]] bool vectorized) {
	std::uint32_t quotient = (std::uint64_t(factor) << 32) / mod;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		scale_avx2<mod>(a.data(), a.size(), factor, quotient);
		return;
	}
#endif
	for (auto &x : a) x = shoup<mod>(x, factor, quotient);
}

inline void bit_reverse(std::vector<std::uint32_t> &a) {
	for (std::size_t i = 1, j = 0; i < a.size(); ++i) {
		std::size_t bit = a.size() >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
}

template <std::uint32_t mod, std::uint32_t root>
void transform(std::vector<std::uint32_t> &a, bool inverse, bool vectorized) {
	Plan<mod, root> plan(a.size(), vectorized);
	if (inverse) {
		bit_reverse(a);
		backward(a, plan, vectorized);
		std::reverse(a.begin() + 1, a.end());
		scale<mod>(a, mod - (mod - 1) / a.size(), vectorized);
	} else {
		forward(a, plan, vectorized);
		if constexpr (mod < (std::uint32_t(1) << 30)) for (auto &x : a) if (x >= mod) x -= mod;
		bit_reverse(a);
	}
}

template <std::uint32_t mod>
constexpr std::uint32_t negative_inverse = [] {
	std::uint32_t value = mod;
	for (int i = 0; i < 5; ++i) value *= 2 - mod * value;
	return 0 - value;
}();

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
__attribute__((target("avx2")))
void pointwise_avx2(std::uint32_t *a, const std::uint32_t *b, std::size_t n) {
	auto p = _mm256_set1_epi32(mod), inverse = _mm256_set1_epi32(negative_inverse<mod>);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto even = _mm256_mul_epu32(x, y);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
		auto qe = _mm256_mul_epu32(even, inverse), qo = _mm256_mul_epu32(odd, inverse);
		even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(qe, p)), 32);
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(qo, p));
		x = _mm256_blend_epi32(even, odd, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	}
}
#endif

template <std::uint32_t mod>
void pointwise(std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, [[maybe_unused]] bool vectorized) {
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		pointwise_avx2<mod>(a.data(), b.data(), a.size());
		return;
	}
#endif
	for (std::size_t i = 0; i < a.size(); ++i) {
		std::uint64_t product = std::uint64_t(a[i]) * b[i];
		std::uint32_t q = std::uint32_t(product) * negative_inverse<mod>;
		std::uint32_t value = (product + std::uint64_t(q) * mod) >> 32;
		a[i] = value < mod ? value : value - mod;
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
#include <utility>
#line 12 "convolution/convolution_mod.hpp"

namespace convolution_mod_detail {
using Word = std::uint32_t;
template <Word mod, Word root, class T>
std::vector<Word> multiply(const std::vector<T> &a, const std::vector<T> &b, Word factor = 1) {
	const auto &small = a.size() <= b.size() ? a : b;
	const auto &large = a.size() <= b.size() ? b : a;
	std::size_t length = a.size() + b.size() - 1;
	bool blocked = small.size() > 32 && large.size() / small.size() >= 8, square = &a == &b;
	std::size_t n = std::bit_ceil(blocked ? 2 * small.size() - 1 : length), block = n - small.size() + 1;
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
template <Word P, Word PR, Word Q, Word QR, Word R, Word RR>
std::vector<Word> modular(const std::vector<Word> &a, const std::vector<Word> &b, Word mod) {
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


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 6 "components/fast_io_bulk.hpp"
#include <span>
#line 8 "components/fast_io_bulk.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		if (a < 1 || a > 10 || b <= a + 1 || b - a - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		const auto window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	static std::uint32_t one(const char *&p, bool &bad) {
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		bad |= value > 0xffffffffULL;
		p += n + 1;
		return std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				const auto g0 = two(p[0], invalid), g1 = two(p[1], invalid), g2 = two(p[2], invalid), g3 = two(p[3], invalid);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, _mm256_set1_epi32(42)),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, _mm256_set1_epi32(42)), _mm256_cmpgt_epi32(low, _mm256_set1_epi32(94967295))));
				invalid = _mm256_or_si256(invalid, overflow);
				const auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

namespace fast_io_detail {
template <class Emit, std::size_t... I>
void emit_record(Emit &emit, const std::uint32_t *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "test/convolution_mod.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n);
	input.read(m);
	std::vector<std::uint32_t> a(n), b(m);
	for (auto *values : {&a, &b}) {
		std::size_t offset = 0;
		read_unsigned_blocks(input, values->size(), [&](const std::uint32_t *block, std::size_t count) {
			std::copy(block, block + count, values->data() + offset);
			offset += count;
		});
	}
	auto result = convolution_mod(a, b, 1000000007);
	write_unsigned_records<1>(output, result.size(), [&](std::size_t i) {
		return std::array<std::uint32_t, 1>{result[i]};
	});
}
