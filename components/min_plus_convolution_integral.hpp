#ifndef MIN_PLUS_CONVOLUTION_INTEGRAL_HPP
#define MIN_PLUS_CONVOLUTION_INTEGRAL_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace min_plus_convolution_integral_detail {
template <class T>
inline constexpr bool supported =
#if defined(__SIZEOF_INT128__)
	std::is_integral_v<T> && sizeof(T) <= 8 && !std::is_same_v<T, bool>;
#else
	false;
#endif

inline bool available() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(MIN_PLUS_CONVOLUTION_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
struct Kernel {
	using U = std::uint32_t;
	static constexpr int group = 16, pad = 24;
	__attribute__((target("avx2"))) static U minimum(__m256i value) {
		auto x = _mm_min_epu32(_mm256_castsi256_si128(value), _mm256_extracti128_si256(value, 1));
		x = _mm_min_epu32(x, _mm_shuffle_epi32(x, 0x4e));
		x = _mm_min_epu32(x, _mm_shuffle_epi32(x, 0xb1));
		return U(_mm_cvtsi128_si32(x));
	}
	__attribute__((target("avx2"))) static void take(const U *a, const U *b, int row, int column, __m256i indices,
		__m256i &best, __m256i &best_column) {
		const auto reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + row - column - 7));
		const auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + column));
		const auto value = _mm256_add_epi32(y, _mm256_permutevar8x32_epi32(x, reverse));
		const auto low = _mm256_min_epu32(best, value);
		best_column = _mm256_blendv_epi8(indices, best_column, _mm256_cmpeq_epi32(low, best));
		best = low;
	}
	__attribute__((target("avx2"))) static int argmin(const U *a, const U *b, int row, int low, int high) {
		if (high - low < 8) {
			int best = low;
			U value = a[row - low] + b[low];
			for (int i = low + 1; i <= high; ++i) {
				U next = a[row - i] + b[i];
				if (next < value) best = i, value = next;
			}
			return best;
		}
		const auto lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		auto column = _mm256_add_epi32(_mm256_set1_epi32(low), lanes);
		auto best = _mm256_set1_epi32(-1), best_column = column;
		int last = high - 7;
		for (int i = low; i < last; i += 8) {
			take(a, b, row, i, column, best, best_column);
			column = _mm256_add_epi32(column, _mm256_set1_epi32(8));
		}
		column = _mm256_add_epi32(_mm256_set1_epi32(last), lanes);
		take(a, b, row, last, column, best, best_column);
		auto mask = _mm256_cmpeq_epi32(best, _mm256_set1_epi32(std::bit_cast<int>(minimum(best))));
		return int(minimum(_mm256_or_si256(best_column, _mm256_xor_si256(mask, _mm256_set1_epi32(-1)))));
	}
	__attribute__((target("avx2"))) static bool multiply(const U *a, const U *b, int n, int m, U *result,
		std::size_t capacity, std::uint64_t budget) {
		if (n <= 0 || m <= 0 || n > std::numeric_limits<int>::max() - m - 2 * pad) return false;
		int length = n + m - 1, groups = (length + group - 1) / group;
		int size = std::bit_ceil(unsigned(groups));
		if (capacity < std::size_t(groups) * group) return false;
		std::vector<U> opt(std::size_t(size) + 1, U(m - 1));
		opt[0] = 0;
		for (int step = size / 2; step; step /= 2)
			for (int t = step; t < groups; t += 2 * step) {
				int row = t * group;
				int low = std::max(int(opt[t - step]), std::max(0, row + 1 - n));
				int high = std::min(int(opt[t + step]), row);
				std::uint64_t work = std::uint64_t(high - low) + 1;
				if (work > budget) return false;
				budget -= work;
				opt[t] = U(argmin(a, b, row, low, high));
			}
		for (int t = 0; t < groups; ++t) {
			int row = t * group;
			int low = std::max(int(opt[t]), std::max(0, row + 1 - n));
			int high = std::min(int(opt[t + 1]), row + group - 1);
			auto x = _mm256_set1_epi32(-1), y = x;
			for (int j = low; j <= high; ++j) {
				auto value = _mm256_set1_epi32(int(b[j]));
				x = _mm256_min_epu32(x, _mm256_add_epi32(value, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + row - j))));
				y = _mm256_min_epu32(y, _mm256_add_epi32(value, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + row - j + 8))));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(result + row), x);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(result + row + 8), y);
		}
		return true;
	}
};
#endif

template <class T, bool Minimum>
std::optional<std::vector<T>> multiply(const std::vector<T> &a, const std::vector<T> &b) {
#if defined(__SIZEOF_INT128__)
	static_assert(supported<T>);
	using W = __int128_t;
	constexpr W lowest = W(std::numeric_limits<T>::lowest()), highest = W(std::numeric_limits<T>::max());
	if (a.empty() || b.empty()) return std::vector<T>{};
	if (a.size() > std::size_t(std::numeric_limits<int>::max()) || b.size() > std::size_t(std::numeric_limits<int>::max()) - a.size()) return std::nullopt;
	int n = int(a.size()), m = int(b.size()), length = n + m - 1;
	std::vector<T> result;
	result.reserve(length);
	if (n == 1 || m == 1) {
		for (int i = 0; i < length; ++i) {
			W value = W(a[n == 1 ? 0 : i]) + W(b[m == 1 ? 0 : i]);
			if (value < lowest || value > highest) return std::nullopt;
			result.push_back(T(value));
		}
		return result;
	}
	int low = 0, high = n - 1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (Minimum ? a[mid] <= a[mid + 1] : a[mid] >= a[mid + 1]) high = mid;
		else low = mid + 1;
	}
	const T *extreme = a.data() + low;
	const T *endpoint = a.data() + ((Minimum ? a.front() < a.back() : a.back() < a.front()) ? n - 1 : 0);
	const T *amin = Minimum ? extreme : endpoint, *amax = Minimum ? endpoint : extreme;
	const auto [bmin, bmax] = std::minmax_element(b.begin(), b.end());
	if (W(*amin) + W(*bmin) < lowest || W(*amax) + W(*bmax) > highest) return std::nullopt;
	bool convex = true;
	W previous = W(b[1]) - W(b[0]);
	for (int i = 2; i < m; ++i) {
		W next = W(b[i]) - W(b[i - 1]);
		if (Minimum ? next < previous : previous < next) { convex = false; break; }
		previous = next;
	}
	if (convex) {
		auto merge = [&]<class Difference>() {
			int i = 0, j = 0;
			result.push_back(T(W(a[0]) + W(b[0])));
			Difference left = Difference(W(a[1]) - W(a[0])), right = Difference(W(b[1]) - W(b[0]));
			while (i + 1 < n && j + 1 < m) {
				if (Minimum ? left < right : right < left) {
					++i;
					if (i + 1 < n) left = Difference(W(a[i + 1]) - W(a[i]));
				} else {
					++j;
					if (j + 1 < m) right = Difference(W(b[j + 1]) - W(b[j]));
				}
				result.push_back(T(W(a[i]) + W(b[j])));
			}
			while (++i < n) result.push_back(T(W(a[i]) + W(b[j])));
			--i;
			while (++j < m) result.push_back(T(W(a[i]) + W(b[j])));
		};
		if (W(*amax) - W(*amin) <= std::numeric_limits<std::int64_t>::max()
			&& W(*bmax) - W(*bmin) <= std::numeric_limits<std::int64_t>::max()) merge.template operator()<std::int64_t>();
		else merge.template operator()<W>();
		return result;
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (length < 256 || !available() || n > std::numeric_limits<int>::max() - m - 2 * Kernel::pad) return std::nullopt;
	W arange = W(*amax) - W(*amin), brange = W(*bmax) - W(*bmin);
	if (arange + 2 * brange >= W(std::numeric_limits<std::uint32_t>::max())) return std::nullopt;
	std::size_t capacity = std::vector<std::uint32_t>{}.max_size();
	std::size_t groups = (std::size_t(length) + Kernel::group - 1) / Kernel::group;
	if (capacity < 2 * Kernel::pad || std::size_t(n) > capacity - 2 * Kernel::pad || std::size_t(m) > capacity || groups > capacity / Kernel::group) return std::nullopt;
	W abase = Minimum ? W(*amin) : W(*amax), bbase = Minimum ? W(*bmin) : W(*bmax), offset = abase + bbase;
	std::vector<std::uint32_t> x(std::size_t(n) + 2 * Kernel::pad, std::numeric_limits<std::uint32_t>::max() - std::uint32_t(brange)), y(m), values(groups * Kernel::group);
	for (int i = 0; i < n; ++i) x[Kernel::pad + i] = std::uint32_t(Minimum ? W(a[i]) - abase : abase - W(a[i]));
	for (int i = 0; i < m; ++i) y[i] = std::uint32_t(Minimum ? W(b[i]) - bbase : bbase - W(b[i]));
	if (!Kernel::multiply(x.data() + Kernel::pad, y.data(), n, m, values.data(), values.size(), std::uint64_t(n + m) * 16)) return std::nullopt;
	for (int i = 0; i < length; ++i) result.push_back(T(Minimum ? offset + values[i] : offset - values[i]));
	return result;
#endif
#endif
	return std::nullopt;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
