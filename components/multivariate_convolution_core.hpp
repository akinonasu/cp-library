#ifndef MULTIVARIATE_CONVOLUTION_CORE_HPP
#define MULTIVARIATE_CONVOLUTION_CORE_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <vector>
#include "mint_simd.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_convolution_detail {
struct Core : mint_detail::SimdOps {
	using Indices = std::vector<int>;
	using Row = std::vector<mint>;
	using Rows = std::vector<Row>;
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
	[[gnu::target("avx2")]] static I8 load(const mint *p) { return load8(reinterpret_cast<const U *>(p)); }
	[[gnu::target("avx2")]] static void store(mint *p, I8 v) { store8(reinterpret_cast<U *>(p), v); }
	template<int Tables>
	__attribute__((target("avx2"), always_inline)) static inline I8 lookup(const mint *table, I8 index) {
		auto low = _mm256_permutevar8x32_epi32(load(table), index);
		if constexpr (Tables == 1) return low;
		auto pick = [index](I8 x, I8 y, int bit) __attribute__((always_inline, target("avx2"))) {
			auto mask = _mm256_castsi256_ps(_mm256_slli_epi32(index, 31 - bit));
			return _mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), mask));
		};
		low = pick(low, _mm256_permutevar8x32_epi32(load(table + 8), index), 3);
		if constexpr (Tables == 2) return low;
		auto high = pick(_mm256_permutevar8x32_epi32(load(table + 16), index),
			_mm256_permutevar8x32_epi32(load(table + 24), index), 3);
		return pick(low, high, 4);
	}
	template<bool Sum, int Tables>
	[[gnu::target("avx2")]] static int apply8(mint *out, const mint *in, const unsigned char *rank, const mint *weights, int count, mint *other, const mint *input) {
		int i = 0;
		for (; i + 8 <= count; i += 8) {
			auto indices = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(rank + i)));
			auto w = lookup<Tables>(weights, indices), value = mul8(load(in + i), w);
			if constexpr (Sum) value = add8(load(out + i), value);
			else store(other + i, mul8(load(input + i), w));
			store(out + i, value);
		}
		return i;
	}
	template<int Count>
	[[gnu::target("avx2")]] static void row8(Row &out, const Rows &a, const Rows &b, int row, const Indices &parts) {
		const auto p = _mm256_set1_epi32(mod), two = _mm256_set1_epi32(twice);
		for (std::size_t i = 0; i < out.size(); i += 8) {
			auto sum = _mm256_setzero_si256();
			for (std::size_t j = 0; j < (Count ? Count : parts.size()); j += 8) {
				auto even = _mm256_setzero_si256(), odd = even;
				for (std::size_t k = 0; k < (Count ? Count : std::min(std::size_t(8), parts.size() - j)); ++k) {
					auto x = load(a[parts[j + k]].data() + i), y = load(b[row - parts[j + k]].data() + i);
					if constexpr (!Count || Count >= 4) {
						x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
						y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
					}
					even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
					odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
				}
				auto value = reduce8(even, odd);
				value = _mm256_min_epu32(value, _mm256_sub_epi32(value, two));
				if constexpr (Count) sum = value;
				else sum = add8(sum, value);
			}
			store(out.data() + i, sum);
		}
	}
#endif
	template<bool Sum>
	static void apply(mint *out, const mint *in, const unsigned char *rank, const mint *weights, int count, [[maybe_unused]] int colors, mint *other = nullptr, const mint *input = nullptr) {
		int i = 0;
#if defined(__x86_64__) || defined(__i386__)
		if (available()) {
			if (colors <= 8) i = apply8<Sum, 1>(out, in, rank, weights, count, other, input);
			else if (colors <= 16) i = apply8<Sum, 2>(out, in, rank, weights, count, other, input);
			else i = apply8<Sum, 4>(out, in, rank, weights, count, other, input);
		}
#endif
		for (; i < count; ++i) {
			if constexpr (Sum) out[i] += in[i] * weights[rank[i]];
			else out[i] = in[i] * weights[rank[i]], other[i] = input[i] * weights[rank[i]];
		}
	}
	static void row(Row &out, const Rows &a, const Rows &b, int index, const Indices &parts) {
#if defined(__x86_64__) || defined(__i386__)
		if (out.size() >= 8 && available()) {
			using Kernel = void (*)(Row &, const Rows &, const Rows &, int, const Indices &);
			static constexpr Kernel kernels[] = {row8<0>, row8<1>, row8<2>, row8<3>, row8<4>, row8<5>, row8<6>, row8<7>, row8<8>};
			return kernels[parts.size() <= 8 ? parts.size() : 0](out, a, b, index, parts);
		}
#endif
		std::fill(out.begin(), out.end(), mint(0));
		for (int p : parts)
			for (std::size_t i = 0; i < out.size(); ++i) out[i] += a[p][i] * b[index - p][i];
	}
	template <class Transform, class Product>
	static Row multiply(const Row &a, const Row &b, Indices dims, Transform transform, Product product) {
		int n = a.size();
		Indices order(dims.size());
		for (int j = 0; j < int(order.size()); ++j) order[j] = j;
		std::stable_sort(order.begin(), order.end(), [&](int x, int y) { return dims[x] > dims[y]; });
		Indices sorted;
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
		Indices permutation;
		Row f, g, result(n);
		if (run != n) {
			Indices target(dims.size());
			for (int j = 0, s = 1; j < int(dims.size()); s *= sorted[j++]) target[order[j]] = s;
			Indices digit(dims.size());
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
		Indices rank_digits(split);
		for (int i = 0, rank = 0; i < inner; ++i) {
			ranks[i] = rank;
			for (int j = 0; j + 1 < split && ++rank_digits[j] == sorted[j]; ++j) {
				rank_digits[j] = 0;
				if (++rank == m) rank = 0;
			}
		}
		std::vector<Indices> parts(outer);
		for (int e = 0; e < outer; ++e)
			for (int x = 0; x <= e; ++x) {
				bool valid = true;
				for (int j = split, u = x, v = e; j < int(sorted.size()); u /= sorted[j], v /= sorted[j++])
					if (u % sorted[j] > v % sorted[j]) { valid = false; break; }
				if (valid) parts[e].push_back(x);
			}
		Rows x(outer, Row(outer == 1 ? inner : length)), y = x;
		Row z(outer == 1 ? 0 : length), weights(32), inverse(32);
		mint root = mint(3).pow((mod - 1) / m), scale = mint(m).inv();
		for (int s = 0; s < m; ++s) {
			mint step = root.pow(s), back = step.inv();
			weights[0] = 1, inverse[0] = scale;
			for (int j = 1; j < m; ++j) weights[j] = weights[j - 1] * step, inverse[j] = inverse[j - 1] * back;
			for (int row = 0; row < outer; ++row) {
				std::fill(x[row].begin() + inner, x[row].end(), mint(0));
				std::fill(y[row].begin() + inner, y[row].end(), mint(0));
				if (s == 0) {
					std::copy_n(input_f.begin() + row * inner, inner, x[row].begin());
					std::copy_n(input_g.begin() + row * inner, inner, y[row].begin());
				} else apply<false>(x[row].data(), input_f.data() + row * inner, ranks.data(), weights.data(), inner, m, y[row].data(), input_g.data() + row * inner);
				if (outer != 1) transform(x[row], false), transform(y[row], false);
			}
			for (int row = outer; row-- > 0;) {
				if (outer == 1) {
					const auto &values = product(x[0], y[0]);
					apply<true>(result.data() + row * inner, values.data(), ranks.data(), inverse.data(), inner, m);
				} else {
					Core::row(z, x, y, row, parts[row]), transform(z, true);
					apply<true>(result.data() + row * inner, z.data(), ranks.data(), inverse.data(), inner, m);
				}
			}
		}
		if (run == n) return result;
		Row output(n);
		for (int i = 0; i < n; i += run) std::copy_n(result.begin() + permutation[i / run], run, output.begin() + i);
		return output;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
