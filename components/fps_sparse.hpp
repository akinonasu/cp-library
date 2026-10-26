#ifndef FPS_SPARSE_HPP
#define FPS_SPARSE_HPP

#include "mint_arithmetic.hpp"
#include <array>
#include <cstdint>
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#include "fps_sparse_core.hpp"

namespace fps_detail {
struct SparseSeries : SparseBase<SparseSeries> {
	struct Tap { int degree; std::uint32_t constant, slope; };
	using O = mint_detail::Ops;
	using U = O::U;
	static_assert(12ULL * (O::mod - 1) * (O::mod - 1) + 0xffffffffULL * O::mod < ~0ULL);
	static U canonical(U x) { return x < O::mod ? x : x - O::mod; }
#if defined(__x86_64__) || defined(__i386__)
	using V = O::I8;
	__attribute__((target("avx2"))) static V canonical8(V x) {
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(O::twice)));
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(O::mod)));
	}
	__attribute__((target("avx2"))) static V reduce8(V even, V odd) {
		return canonical8(O::reduce8(even, odd));
	}
	__attribute__((target("avx2"), always_inline)) static inline V triangular(V x, const U *f) {
		const V lower = _mm256_permute2x128_si256(x, x, 0x08);
		V shifted[8] = {x, _mm256_alignr_epi8(x, lower, 12), _mm256_alignr_epi8(x, lower, 8), _mm256_alignr_epi8(x, lower, 4),
			lower, _mm256_slli_si256(lower, 4), _mm256_slli_si256(lower, 8), _mm256_slli_si256(lower, 12)};
		V even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
		for (int j = 1; j < 8; ++j) if (f[j]) {
			V factor = _mm256_set1_epi32(f[j]);
			even = _mm256_add_epi64(even, _mm256_mul_epu32(shifted[j], factor));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(shifted[j], 32), factor));
		}
		return canonical8(O::add8(x, reduce8(even, odd)));
	}
	template <Kind kind>
	__attribute__((target("avx2"), always_inline)) static inline void block(U *out, int i, const Tap *taps, int near, int count, const U *inv, const U *p, const U *q, V *coefficients, const U *steps) {
		V total = _mm256_setzero_si256();
		const V lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		for (int first = 0; first < count; first += 12) {
			V ce = _mm256_setzero_si256(), co = ce;
			for (int j = first; j < std::min(count, first + 12); ++j) {
				V x = O::load8(out + i - taps[j].degree);
				if (j < near) x = _mm256_and_si256(x, _mm256_cmpgt_epi32(_mm256_set1_epi32(taps[j].degree), lanes));
				V c;
				if constexpr (kind == Kind::power) c = coefficients[j];
				else c = _mm256_set1_epi32(taps[j].constant);
				ce = _mm256_add_epi64(ce, _mm256_mul_epu32(x, c));
				co = _mm256_add_epi64(co, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), kind == Kind::power ? _mm256_srli_epi64(c, 32) : c));
				if constexpr (kind == Kind::power) {
					V next = _mm256_add_epi32(c, _mm256_set1_epi32(steps[j]));
					coefficients[j] = _mm256_min_epu32(next, _mm256_sub_epi32(next, _mm256_set1_epi32(O::mod)));
				}
			}
			total = O::add8(total, reduce8(ce, co));
		}
		V value = canonical8(total);
		if constexpr (kind == Kind::logarithm) value = O::add8(value, O::load8(out + i));
		if constexpr (kind == Kind::power || kind == Kind::exponential) {
			if (near) value = triangular(canonical8(value), q);
			value = O::mul8(value, O::load8(inv + i));
		}
		if (near) value = triangular(canonical8(value), p);
		O::store8(out + i, canonical8(value));
	}
	template <Kind kind>
	__attribute__((target("avx2"))) static int near_blocks(U *out, int i, int end, const Tap *taps, int count, const U *inv, const U *p, const U *q) {
		const int width = taps[count - 1].degree;
		if constexpr (kind == Kind::logarithm) if (i == width) {
			mint value = mint::word(out[i]);
			for (int j = 0; j < count; ++j) value += mint::word(taps[j].constant) * mint::word(out[i - taps[j].degree]);
			out[i++] = canonical(value.v);
		}
		std::array<mint, 8> constant{}, slope{};
		for (int j = 0; j < count; ++j) {
			constant[taps[j].degree] = mint::word(taps[j].constant);
			slope[taps[j].degree] = mint::word(taps[j].slope);
		}
		V coefficients[7], steps[7];
		for (int j = 0; j < width; ++j) {
			std::array<std::uint32_t, 8> first{}, step{};
			for (int t = 0; t < 8; ++t) {
				mint a = 0, b = 0;
				for (int u = 0; u <= t && u + j < width; ++u) {
					const int d = u + j + 1;
					mint factor = mint::word((kind == Kind::power || kind == Kind::exponential ? q : p)[t - u]);
					a += factor * (constant[d] + mint(u) * slope[d]);
					if constexpr (kind == Kind::power) b += factor * slope[d];
				}
				first[t] = canonical((a + mint(i) * b).v);
				if constexpr (kind == Kind::power) step[t] = canonical((mint(8) * b).v);
			}
			coefficients[j] = O::load8(first.data());
			if constexpr (kind == Kind::power) steps[j] = O::load8(step.data());
		}
		for (; end - i >= 8; i += 8) {
			V even = _mm256_setzero_si256(), odd = even;
			for (int j = 0; j < width; ++j) {
				V x = _mm256_set1_epi32(out[i - 1 - j]), c = coefficients[j];
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, c));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(x, _mm256_srli_epi64(c, 32)));
				if constexpr (kind == Kind::power) {
					V next = _mm256_add_epi32(c, steps[j]);
					coefficients[j] = _mm256_min_epu32(next, _mm256_sub_epi32(next, _mm256_set1_epi32(O::mod)));
				}
			}
			V value = reduce8(even, odd);
			if constexpr (kind == Kind::power || kind == Kind::exponential) value = triangular(canonical8(O::mul8(value, O::load8(inv + i))), p);
			O::store8(out + i, value);
		}
		return i;
	}
	template <Kind kind>
	__attribute__((target("avx2"))) static int blocks(U *out, int i, int end, const Tap *taps, int near, int count, const U *inv, const U *p, const U *q) {
		if (end - i < 8) return i;
		if (count == near && count && end - i >= 256) return near_blocks<kind>(out, i, end, taps, count, inv, p, q);
		V coefficients[64];
		U steps[64];
		if constexpr (kind == Kind::power) {
			const V offsets = _mm256_setr_epi32(0, O::one, O::in(2), O::in(3), O::in(4), O::in(5), O::in(6), O::in(7));
			const V index = O::add8(_mm256_set1_epi32(O::in(i)), offsets);
			for (int j = 0; j < count; ++j) {
				coefficients[j] = canonical8(O::add8(_mm256_set1_epi32(taps[j].constant), O::mul8(index, _mm256_set1_epi32(taps[j].slope))));
				steps[j] = canonical(O::mul(O::in(8), taps[j].slope));
			}
		}
		for (; end - i >= 8; i += 8) block<kind>(out, i, taps, near, count, inv, p, q, coefficients, steps);
		return i;
	}
	__attribute__((target("avx2"))) static int scale8(U *out, const U *factors, int n) {
		int i = 1;
		for (; n - i >= 8; i += 8) O::store8(out + i, O::mul8(O::load8(out + i), O::load8(factors + i)));
		return i;
	}
	template <bool exponential = false>
	__attribute__((target("avx2"))) static void geometric8(U *out, int n, U initial, U ratio, const U *inv = nullptr) {
		std::uint32_t first[8];
		U step = O::one;
		first[0] = initial;
		for (int j = 0; j < 8; ++j) {
			step = O::mul(step, ratio);
			if (j) {
				first[j] = O::mul(first[j - 1], ratio);
				if constexpr (exponential) first[j] = O::mul(first[j], inv[j]);
			}
		}
		V value = O::load8(first), multiplier = _mm256_set1_epi32(step);
		O::store8(out, value);
		int i = 8;
		for (; n - i >= 8; i += 8) {
			value = O::mul8(value, multiplier);
			if constexpr (exponential) {
				V factors[4];
#pragma GCC unroll 4
				for (int j = 0; j < 4; ++j) factors[j] = O::mul8(O::load8(inv + i - 2 * j), O::load8(inv + i - 2 * j - 1));
				value = O::mul8(value, O::mul8(O::mul8(factors[0], factors[1]), O::mul8(factors[2], factors[3])));
			}
			O::store8(out + i, value);
		}
		for (; i < n; ++i) {
			out[i] = O::mul(out[i - 1], ratio);
			if constexpr (exponential) out[i] = O::mul(out[i], inv[i]);
		}
	}

#endif
	template <bool exponential = false>
	static void geometric(Series &out, mint initial, mint ratio, const mint *inv = nullptr) {
#if defined(__x86_64__) || defined(__i386__)
		if (out.size() >= 8 && O::vectorized()) {
			geometric8<exponential>(reinterpret_cast<U *>(out.data()), out.size(), initial.v, ratio.v, reinterpret_cast<const U *>(inv));
			return;
		}
#endif
		for (int i = 0; i < int(out.size()); ++i) {
			out[i] = initial;
			initial *= ratio;
			if constexpr (exponential) if (i + 1 < int(out.size())) initial *= inv[i + 1];
		}
	}
	static void scale(Series &out, const mint *factors) {
		int i = 1;
#if defined(__x86_64__) || defined(__i386__)
		if (O::vectorized()) i = scale8(reinterpret_cast<U *>(out.data()), reinterpret_cast<const U *>(factors), out.size());
#endif
		for (; i < int(out.size()); ++i) out[i] *= factors[i];
	}
	template <Kind kind>
	static Series solve(const Terms &terms, int n, mint initial, mint exponent, const mint *inverses) {
		std::vector<Tap> taps;
		Series result(n);
		U *out = reinterpret_cast<U *>(result.data());
		const U *inv = reinterpret_cast<const U *>(inverses);
		for (auto [j, x] : terms) {
			mint c = 0, s = 0;
			if constexpr (kind == Kind::inverse || kind == Kind::logarithm) c = -x;
			if constexpr (kind == Kind::exponential) c = x * mint(j);
			if constexpr (kind == Kind::power) c = (exponent + mint(1)) * mint(j) * x, s = -x;
			taps.push_back({j, canonical(c.v), canonical(s.v)});
			if constexpr (kind == Kind::logarithm) out[j] = canonical((x * mint(j)).v);
		}
		std::array<std::uint32_t, 8> p{}, q{};
		p[0] = q[0] = O::one;
		if (n >= 8) for (int i = 1; i < 8; ++i) {
			mint a = 0, b = 0;
			for (const auto &tap : taps) {
				int j = tap.degree;
				if (j > i) break;
				mint c = mint::word(tap.constant), s = mint::word(tap.slope);
				a += (c + mint(i) * s) * mint::word(p[i - j]);
				b += (-c + mint(i - j) * s) * mint::word(q[i - j]);
			}
			if constexpr (kind == Kind::power || kind == Kind::exponential) a *= inverses[i], b *= inverses[i];
			p[i] = canonical(a.v), q[i] = canonical(b.v);
		}
		out[0] = canonical(initial.v);
		int active = 0, i = 1;
		[[maybe_unused]] int near = 0;
		while (i < n) {
			while (active < int(taps.size()) && taps[active].degree <= i) {
				near += taps[active].degree < 8;
				++active;
			}
			int end = active < int(taps.size()) ? taps[active].degree : n;
			if (!active) { i = end; continue; }
#if defined(__x86_64__) || defined(__i386__)
			if (O::vectorized()) i = blocks<kind>(out, i, end, taps.data(), near, active, inv, p.data(), q.data());
#endif
			for (; i < end; ++i) {
				U x = 0;
				for (int first = 0; first < active; first += 12) {
					std::uint64_t total = 0;
					for (int j = first; j < std::min(active, first + 12); ++j) {
						U c = taps[j].constant;
						if constexpr (kind == Kind::power) c = canonical(O::add(c, O::mul(O::in(i), taps[j].slope)));
						total += std::uint64_t(c) * out[i - taps[j].degree];
					}
					x = O::add(x, O::reduce(total) % O::mod);
				}
				if constexpr (kind == Kind::logarithm) x = O::add(x, out[i]);
				if constexpr (kind == Kind::power || kind == Kind::exponential) x = O::mul(x, inv[i]);
				out[i] = canonical(x);
			}
		}
		if constexpr (kind == Kind::logarithm) scale(result, inverses);
		return result;
	}

};
}
#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
#endif
