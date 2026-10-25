#ifndef MIN_PLUS_CONVOLUTION_ENVELOPE_HPP
#define MIN_PLUS_CONVOLUTION_ENVELOPE_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <memory>
#include <span>
#include <type_traits>
#include <cstddef>
#include <vector>
#include <bit>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace min_plus_convolution_envelope_detail {
struct Envelope {
	struct Entry { int offset, end; std::uint32_t bias; };
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	template <int step>
	__attribute__((target("avx2"))) static int window(std::span<const std::uint32_t> a, const Entry &x, const Entry &y, int t) {
		constexpr int shift = step == 1 ? 0 : -7;
		auto u = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(a.data() + x.offset + step * t + shift)), _mm256_set1_epi32(int(x.bias)));
		auto v = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(a.data() + y.offset + step * t + shift)), _mm256_set1_epi32(int(y.bias)));
		auto sign = _mm256_set1_epi32(std::numeric_limits<std::int32_t>::min());
		unsigned loses = unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(_mm256_xor_si256(u, sign), _mm256_xor_si256(v, sign)))));
		if constexpr (step == 1) return std::countr_zero(loses | 0x100u);
		else return std::countl_zero((loses << 24) | 0x800000u);
	}
#endif
	template <int step>
	static int crossing(std::span<const std::uint32_t> a, const Entry &x, const Entry &y, int low, int high, [[maybe_unused]] bool simd) {
		while (high - low > 8) {
			int mid = low + (high - low) / 2;
			(a[x.offset + step * mid] + x.bias <= a[y.offset + step * mid] + y.bias ? low : high) = mid;
		}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		int p = x.offset + step * low, q = y.offset + step * low, n = int(a.size());
		if (simd && (step == 1 ? std::max(p, q) < n - 7 : std::min(p, q) >= 7)) return low + window<step>(a, x, y, low) - 1;
#endif
		while (high - low > 1) {
			int mid = low + (high - low) / 2;
			(a[x.offset + step * mid] + x.bias <= a[y.offset + step * mid] + y.bias ? low : high) = mid;
		}
		return low;
	}
	template <int step>
	static void sweep(std::span<const std::uint32_t> a, std::span<const std::uint32_t> b, std::vector<std::uint32_t> &c,
		int first, int entering, int count, std::vector<Entry> &stack, bool simd) {
		int n = int(a.size());
		stack.clear();
		auto value = [&](const Entry &e, int t) { return a[e.offset + step * t] + e.bias; };
		auto take = [&] { auto e = stack.back(); stack.pop_back(); return e; };
		Entry top{first - entering, n - 1, b[entering]};
		c[first] = std::min(c[first], value(top, 0));
		int t = 1;
		for (; t < count; ++t) {
			while (top.end < t) top = take();
			std::uint32_t best = value(top, t);
			int column = entering + step * t;
			Entry next{first - column, 0, b[column]};
			if (std::uint32_t v = value(next, t); v <= best) {
				int low = t;
				for (;;) {
					if (value(next, top.end) > value(top, top.end)) {
						next.end = crossing<step>(a, next, top, low, top.end, simd);
						stack.push_back(top);
						break;
					}
					low = top.end;
					if (stack.empty()) { next.end = n - 1; break; }
					top = take();
				}
				top = next;
				best = v;
			}
			int row = first + step * t;
			c[row] = std::min(c[row], best);
		}
		stack.push_back(top);
		while (t < n) {
			Entry e = take();
			if (e.end < t) continue;
			int last = e.end, lo = first + step * t, hi = first + step * last;
			if (hi < lo) std::swap(lo, hi);
			int column = first - e.offset;
			for (int row = lo; row <= hi; ++row) c[row] = std::min(c[row], a[row - column] + e.bias);
			t = last + 1;
		}
	}
	static std::vector<std::uint32_t> run(std::span<const std::uint32_t> a, std::span<const std::uint32_t> b) {
		int n = int(a.size()), m = int(b.size());
		std::vector<std::uint32_t> c(n + m - 1, std::numeric_limits<std::uint32_t>::max());
		std::vector<Entry> stack;
		stack.reserve(std::min(n, m));
		bool simd = false;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(MIN_PLUS_CONVOLUTION_DISABLE_AVX2)
		simd = __builtin_cpu_supports("avx2");
#endif
		for (int j = 0; j < m; j += n) {
			int end = std::min(m, j + n);
			sweep<1>(a, b, c, j, j, end - j, stack, simd);
			sweep<-1>(a, b, c, end + n - 2, end - 1, end - j, stack, simd);
		}
		return c;
	}
};

template <class T, bool minimum>
std::optional<std::vector<T>> multiply(const std::vector<T> &a, const std::vector<T> &b) {
#if defined(__SIZEOF_INT128__)
	static_assert(std::is_integral_v<T> && sizeof(T) <= 8 && !std::is_same_v<T, bool>);
	using W = __int128_t;
	if (a.empty() || b.empty()) return std::vector<T>{};
	if (a.size() > std::size_t(std::numeric_limits<int>::max()) || b.size() > std::size_t(std::numeric_limits<int>::max()) - a.size()) return std::nullopt;
	int n = int(a.size()), m = int(b.size());
	int low = 0, high = n - 1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (minimum ? a[mid] >= a[mid + 1] : a[mid] <= a[mid + 1]) high = mid;
		else low = mid + 1;
	}
	T extreme = a[low], endpoint = minimum ? std::min(a.front(), a.back()) : std::max(a.front(), a.back());
	W amin = minimum ? W(endpoint) : W(extreme), amax = minimum ? W(extreme) : W(endpoint);
	T bmin_value = b.front(), bmax_value = b.front();
	for (T value : b) bmin_value = std::min(bmin_value, value), bmax_value = std::max(bmax_value, value);
	W bmin = W(bmin_value), bmax = W(bmax_value);
	if (amin + bmin < W(std::numeric_limits<T>::lowest()) || amax + bmax > W(std::numeric_limits<T>::max()) || amax - amin + bmax - bmin > W(std::numeric_limits<std::uint32_t>::max())) return std::nullopt;
	W abase = minimum ? amin : amax, bbase = minimum ? bmin : bmax, offset = abase + bbase;
	auto x = std::make_unique_for_overwrite<std::uint32_t[]>(n);
	auto y = std::make_unique_for_overwrite<std::uint32_t[]>(m);
	for (int i = 0; i < n; ++i) x[i] = std::uint32_t(minimum ? W(a[i]) - abase : abase - W(a[i]));
	for (int i = 0; i < m; ++i) y[i] = std::uint32_t(minimum ? W(b[i]) - bbase : bbase - W(b[i]));
	auto c = Envelope::run({x.get(), std::size_t(n)}, {y.get(), std::size_t(m)});
	std::vector<T> result(c.size());
	for (std::size_t i = 0; i < c.size(); ++i) result[i] = T(minimum ? offset + c[i] : offset - c[i]);
	return result;
#else
	return std::nullopt;
#endif
}

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
