#ifndef MULTIVARIATE_CONVOLUTION_CORE_HPP
#define MULTIVARIATE_CONVOLUTION_CORE_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>
#include "mint_arithmetic.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_convolution_detail {
struct Core : mint_detail::Ops {
	static int modulus(int k) {
		for (int m : {1, 2, 4, 7, 8, 14, 16, 17, 28, 32})
			if (m >= k) return m;
		return int(std::bit_ceil(unsigned(k)));
	}
	static bool available() {
#if (defined(__x86_64__) || defined(__i386__)) && !defined(MULTIVARIATE_CONVOLUTION_DISABLE_AVX2)
		return vectorized();
#else
		return false;
#endif
	}
#if defined(__x86_64__) || defined(__i386__)
	template<int Tables>
	__attribute__((target("avx2"), always_inline)) static inline I8 lookup8(const U *table, I8 index) {
		auto low = _mm256_permutevar8x32_epi32(load8(table), index);
		if constexpr (Tables == 1) return low;
		auto pick = [index](I8 x, I8 y, int bit) __attribute__((always_inline, target("avx2"))) {
			auto mask = _mm256_castsi256_ps(_mm256_slli_epi32(index, 31 - bit));
			return _mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), mask));
		};
		low = pick(low, _mm256_permutevar8x32_epi32(load8(table + 8), index), 3);
		if constexpr (Tables == 2) return low;
		auto high = pick(_mm256_permutevar8x32_epi32(load8(table + 16), index),
			_mm256_permutevar8x32_epi32(load8(table + 24), index), 3);
		return pick(low, high, 4);
	}
	template<int Tables>
	__attribute__((target("avx2"))) static int weigh8(mint *x, mint *y, const mint *f, const mint *g, const unsigned char *rank, const mint *weights, int count) {
		int i = 0;
		for (; i + 8 <= count; i += 8) {
			auto indices = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(rank + i)));
			auto w = lookup8<Tables>(reinterpret_cast<const U *>(weights), indices);
			store8(reinterpret_cast<U *>(x + i), mul8(load8(reinterpret_cast<const U *>(f + i)), w));
			store8(reinterpret_cast<U *>(y + i), mul8(load8(reinterpret_cast<const U *>(g + i)), w));
		}
		return i;
	}
	template<int Tables>
	__attribute__((target("avx2"))) static int accumulate8(mint *out, const mint *z, const unsigned char *rank, const mint *weights, int count) {
		int i = 0;
		for (; i + 8 <= count; i += 8) {
			auto indices = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(rank + i)));
			auto w = lookup8<Tables>(reinterpret_cast<const U *>(weights), indices);
			auto value = mul8(load8(reinterpret_cast<const U *>(z + i)), w);
			store8(reinterpret_cast<U *>(out + i), add8(load8(reinterpret_cast<const U *>(out + i)), value));
		}
		return i;
	}

	template<int Count>
	__attribute__((target("avx2"))) static void short_row8(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int row, const std::vector<int> &parts) {
		const auto p = _mm256_set1_epi32(mod), two = _mm256_set1_epi32(twice);
		for (std::size_t i = 0; i < out.size(); i += 8) {
			auto even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
			for (int k = 0; k < Count; ++k) {
				auto x = load8(reinterpret_cast<const U *>(a[parts[k]].data() + i));
				auto y = load8(reinterpret_cast<const U *>(b[row - parts[k]].data() + i));
				if constexpr (Count >= 4) {
					x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
					y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
				}
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			auto value = reduce8(even, odd);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, two));
			store8(reinterpret_cast<U *>(out.data() + i), value);
		}
	}

	__attribute__((target("avx2"))) static void row8(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int row, const std::vector<int> &parts) {
		const auto p = _mm256_set1_epi32(mod), r = _mm256_set1_epi32(998244351), two = _mm256_set1_epi32(twice);
		for (std::size_t i = 0; i < out.size(); i += 8) {
			auto sum = _mm256_setzero_si256();
			for (std::size_t j = 0; j < parts.size(); j += 8) {
				auto even = _mm256_setzero_si256(), odd = even;
				for (std::size_t k = j; k < std::min(j + 8, parts.size()); ++k) {
					auto x = load8(reinterpret_cast<const U *>(a[parts[k]].data() + i));
					auto y = load8(reinterpret_cast<const U *>(b[row - parts[k]].data() + i));
					x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
					y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
					even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
					odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
				}
				even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, r), p));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, r), p));
				auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
				value = _mm256_min_epu32(value, _mm256_sub_epi32(value, two));
				sum = add8(sum, value);
			}
			store8(reinterpret_cast<U *>(out.data() + i), sum);
		}
	}
#endif
	static void row(std::vector<mint> &out, const std::vector<std::vector<mint>> &a, const std::vector<std::vector<mint>> &b, int index, const std::vector<int> &parts) {
#if defined(__x86_64__) || defined(__i386__)
		if (out.size() >= 8 && available()) {
			switch (parts.size()) {
			case 1: return short_row8<1>(out, a, b, index, parts);
			case 2: return short_row8<2>(out, a, b, index, parts);
			case 3: return short_row8<3>(out, a, b, index, parts);
			case 4: return short_row8<4>(out, a, b, index, parts);
			case 5: return short_row8<5>(out, a, b, index, parts);
			case 6: return short_row8<6>(out, a, b, index, parts);
			case 7: return short_row8<7>(out, a, b, index, parts);
			case 8: return short_row8<8>(out, a, b, index, parts);
			default: return row8(out, a, b, index, parts);
			}
		}
#endif
		std::fill(out.begin(), out.end(), mint(0));
		for (int p : parts)
			for (std::size_t i = 0; i < out.size(); ++i) out[i] += a[p][i] * b[index - p][i];
	}
	template <class Transform, class Product>
	static std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b, std::vector<int> dims, Transform transform, Product product) {
		int n = a.size();
		std::vector<int> order(dims.size());
		for (int j = 0; j < int(order.size()); ++j) order[j] = j;
		std::stable_sort(order.begin(), order.end(), [&](int x, int y) { return dims[x] > dims[y]; });
		std::vector<int> sorted;
		for (int j : order) sorted.push_back(dims[j]);
		int split = sorted.size(), inner = n, outer = 1;
		double best = 1e300, pairs = 1;
		for (int k = int(sorted.size()), q = 1, size = n; k; size /= sorted[--k], q *= sorted[k]) {
			int length = std::bit_ceil(unsigned(2 * size - 1));
			double cost = modulus(k) * length * (3.0 * q * std::bit_width(unsigned(length)) + pairs * 2.5);
			if (cost < best) best = cost, split = k, inner = size, outer = q;
			pairs *= sorted[k - 1] * (sorted[k - 1] + 1.0) / 2;
			if (q > 128 / sorted[k - 1]) break;
		}
		std::sort(order.begin(), order.begin() + split);
		std::sort(order.begin() + split, order.end());
		for (int j = 0; j < int(dims.size()); ++j) sorted[j] = dims[order[j]];
		int prefix = 0, run = 1;
		while (prefix < int(dims.size()) && order[prefix] == prefix) run *= dims[prefix++];
		std::vector<int> permutation;
		std::vector<mint> f, g, result(n);
		if (run != n) {
			std::vector<int> target(dims.size());
			for (int j = 0, s = 1; j < int(dims.size()); s *= sorted[j++]) target[order[j]] = s;
			std::vector<int> digit(dims.size());
			f.resize(n), g.resize(n), permutation.resize(n / run);
			for (int i = 0, p = 0; i < n; i += run) {
				permutation[i / run] = p;
				std::copy_n(a.begin() + i, run, f.begin() + p);
				std::copy_n(b.begin() + i, run, g.begin() + p);
				for (int j = prefix; j < int(dims.size()); ++j) {
					p += target[j];
					if (++digit[j] < dims[j]) break;
					digit[j] = 0, p -= target[j] * dims[j];
				}
			}
		}
		const auto &input_f = run == n ? a : f, &input_g = run == n ? b : g;
		int m = modulus(split), length = std::bit_ceil(unsigned(2 * inner - 1));
		std::vector<unsigned char> ranks(inner);
		std::vector<int> rank_digits(split);
		for (int i = 0, rank = 0; i < inner; ++i) {
			ranks[i] = rank;
			for (int j = 0; j + 1 < split && ++rank_digits[j] == sorted[j]; ++j) {
				rank_digits[j] = 0;
				if (++rank == m) rank = 0;
			}
		}
		std::vector<std::vector<int>> parts(outer);
		for (int e = 0; e < outer; ++e)
			for (int x = 0; x <= e; ++x) {
				bool valid = true;
				for (int j = split, u = x, v = e; j < int(sorted.size()); u /= sorted[j], v /= sorted[j++])
					if (u % sorted[j] > v % sorted[j]) { valid = false; break; }
				if (valid) parts[e].push_back(x);
			}
		std::vector<std::vector<mint>> x(outer, std::vector<mint>(outer == 1 ? inner : length)), y = x;
		std::vector<mint> z(length), weights(32), inverse(32);
		mint root = mint(3).pow((mod - 1) / m), scale = mint(m).inv();
		for (int s = 0; s < m; ++s) {
			mint step = root.pow(s), back = step.inv();
			weights[0] = 1, inverse[0] = scale;
			for (int j = 1; j < m; ++j) weights[j] = weights[j - 1] * step, inverse[j] = inverse[j - 1] * back;
			for (int row = 0; row < outer; ++row) {
				std::fill(x[row].begin() + inner, x[row].end(), mint(0));
				std::fill(y[row].begin() + inner, y[row].end(), mint(0));
				int i = 0;
				if (s == 0) {
					std::copy_n(input_f.begin() + row * inner, inner, x[row].begin());
					std::copy_n(input_g.begin() + row * inner, inner, y[row].begin());
					i = inner;
				}
#if defined(__x86_64__) || defined(__i386__)
				else if (available()) {
					if (m <= 8) i = weigh8<1>(x[row].data(), y[row].data(), input_f.data() + row * inner, input_g.data() + row * inner, ranks.data(), weights.data(), inner);
					else if (m <= 16) i = weigh8<2>(x[row].data(), y[row].data(), input_f.data() + row * inner, input_g.data() + row * inner, ranks.data(), weights.data(), inner);
					else i = weigh8<4>(x[row].data(), y[row].data(), input_f.data() + row * inner, input_g.data() + row * inner, ranks.data(), weights.data(), inner);
				}
#endif
				for (; i < inner; ++i) x[row][i] = input_f[row * inner + i] * weights[ranks[i]], y[row][i] = input_g[row * inner + i] * weights[ranks[i]];
				if (outer != 1) transform(x[row], false), transform(y[row], false);
			}
			for (int row = outer; row-- > 0;) {
				if (outer == 1) z = product(x[0], y[0]);
				else Core::row(z, x, y, row, parts[row]), transform(z, true);
				int i = 0;
#if defined(__x86_64__) || defined(__i386__)
				if (available()) {
					if (m <= 8) i = accumulate8<1>(result.data() + row * inner, z.data(), ranks.data(), inverse.data(), inner);
					else if (m <= 16) i = accumulate8<2>(result.data() + row * inner, z.data(), ranks.data(), inverse.data(), inner);
					else i = accumulate8<4>(result.data() + row * inner, z.data(), ranks.data(), inverse.data(), inner);
				}
#endif
				for (; i < inner; ++i) result[row * inner + i] += z[i] * inverse[ranks[i]];
			}
		}
		if (run == n) return result;
		std::vector<mint> output(n);
		for (int i = 0; i < n; i += run) std::copy_n(result.begin() + permutation[i / run], run, output.begin() + i);
		return output;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
