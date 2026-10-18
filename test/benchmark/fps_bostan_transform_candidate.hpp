#ifndef CP_COMPONENTS_FPS_BOSTAN_TRANSFORM_HPP
#define CP_COMPONENTS_FPS_BOSTAN_TRANSFORM_HPP

#include "ntt.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {

struct BostanTransform {
	using U = NTT::U;
	const NTT &t;
#if defined(__x86_64__) || defined(__i386__)
	template <bool inverse>
	__attribute__((target("avx2"))) void radix4(U *a, int stride, int length, int first = 0) const {
		const NTT::V &w = inverse ? t.inverse_roots : t.roots;
		const NTT::V &q = inverse ? t.inverse_root_quotients : t.root_quotients;
		t.prepare_cubes(first + length / (4 * stride));
		const NTT::V &w3 = inverse ? t.inverse_cube_roots : t.cube_roots;
		const NTT::V &q3s = inverse ? t.inverse_cube_quotients : t.cube_quotients;
		NTT::I8 p = _mm256_set1_epi32(NTT::twice);
		NTT::I8 im = _mm256_set1_epi32(w[1]), iq = _mm256_set1_epi32(q[1]);
		for (int j = 0, k = first; j < length; j += 4 * stride, ++k) {
			NTT::I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			NTT::I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			NTT::I8 r3 = _mm256_set1_epi32(w3[k]), q3 = _mm256_set1_epi32(q3s[k]);
			for (int i = 0; i < stride; i += 8) {
				NTT::I8 x0 = NTT::load8(a + j + i), x1 = NTT::load8(a + j + stride + i);
				NTT::I8 x2 = NTT::load8(a + j + 2 * stride + i), x3 = NTT::load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					NTT::I8 u = NTT::add8(x0, x1), v = NTT::sub8(x0, x1);
					NTT::I8 s = NTT::add8(x2, x3);
					NTT::I8 t = NTT::fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x2, p), x3), im, iq);
					x0 = NTT::add8(u, s);
					x1 = _mm256_add_epi32(v, t);
					x2 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
					if (k) {
						x1 = NTT::fixed_mul8(x1, r0, q0);
						x2 = NTT::fixed_mul8(x2, r, rq);
						x3 = NTT::fixed_mul8(x3, r3, q3);
					} else {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
				} else {
					x0 = _mm256_min_epu32(x0, _mm256_sub_epi32(x0, p));
					if (!k) {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
					if (k) {
						x1 = NTT::fixed_mul8(x1, r0, q0);
						x2 = NTT::fixed_mul8(x2, r, rq);
						x3 = NTT::fixed_mul8(x3, r3, q3);
					}
					NTT::I8 u = NTT::add8(x0, x2), v = NTT::sub8(x0, x2), s = NTT::add8(x1, x3);
					NTT::I8 t = NTT::fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x1, p), x3), im, iq);
					x0 = _mm256_add_epi32(u, s);
					x1 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x2 = _mm256_add_epi32(v, t);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
				}
				NTT::store8(a + j + i, x0);
				NTT::store8(a + j + stride + i, x1);
				NTT::store8(a + j + 2 * stride + i, x2);
				NTT::store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	__attribute__((target("avx2"))) void forward8(U *a, int n) const {
		using I = NTT::I8;
		int h = n / 2;
		for (; h >= 16 && 2 * h > 2048; h >>= 2) t.radix4_8<false>(a, h / 2, n);
		const int tile = 2 * h;
		for (int first = 0; first < n; first += tile) {
			int stride = h;
			for (; stride >= 16; stride >>= 2) radix4<false>(a + first, stride / 2, tile, first / (2 * stride));
			if (stride == 8) {
				for (int j = first; j < first + tile; j += 8) {
					I x = NTT::load8(a + j), p = _mm256_set1_epi32(NTT::twice);
					NTT::store8(a + j, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
				}
				t.stage8<false>(a + first, 8, tile, first / 16);
			}
			for (int j = first; j < first + tile; j += 8) {
				I z = NTT::load8(a + j), p = _mm256_set1_epi32(NTT::twice);
				z = _mm256_min_epu32(z, _mm256_sub_epi32(z, p));
				z = t.small8<1, false>(t.small8<2, false>(t.small8<4, false>(z, j / 8), j / 8), j / 8);
				NTT::store8(a + j, z);
			}
		}
	}
	__attribute__((target("avx2"))) void inverse_twisted8(U *a, int n, const U *twist, U correction) const {
		using I = NTT::I8;
		int tile = std::min(n, 2048);
		for (int first = 0; first < n; first += tile) {
			for (int j = first; j < first + tile; j += 8) {
				I z = NTT::load8(a + j);
				z = t.small8<4, true>(t.small8<2, true>(t.small8<1, true>(z, j / 8), j / 8), j / 8);
				NTT::store8(a + j, z);
			}
			for (int h = 8; 4 * h <= tile && 4 * h < n; h <<= 2) radix4<true>(a + first, h, tile, first / (4 * h));
		}
		int h = 8;
		while (4 * h <= tile && 4 * h < n) h <<= 2;
		for (; 4 * h < n; h <<= 2) t.radix4_8<true>(a, h, n);
		if (4 * h == n) {
			I p = _mm256_set1_epi32(NTT::twice);
			I imag = _mm256_set1_epi32(t.inverse_roots[1]);
			I iq = _mm256_set1_epi32(t.inverse_root_quotients[1]);
			for (int i = 0; i < h; i += 8) {
				I x0 = NTT::load8(a + i), x1 = NTT::load8(a + h + i);
				I x2 = NTT::load8(a + 2 * h + i), x3 = NTT::load8(a + 3 * h + i);
				I u = NTT::add8(x0, x1), v = NTT::sub8(x0, x1), s = NTT::add8(x2, x3);
				I z = NTT::fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x2, p), x3), imag, iq);
				x0 = NTT::add8(u, s);
				x1 = NTT::add8(v, z);
				x2 = NTT::sub8(u, s);
				x3 = NTT::sub8(v, z);
				if (i == 0) x0 = NTT::sub8(x0, _mm256_setr_epi32(correction, 0, 0, 0, 0, 0, 0, 0));
				NTT::store8(a + i, NTT::mul8(x0, NTT::load8(twist + i)));
				NTT::store8(a + h + i, NTT::mul8(x1, NTT::load8(twist + h + i)));
				NTT::store8(a + 2 * h + i, NTT::mul8(x2, NTT::load8(twist + 2 * h + i)));
				NTT::store8(a + 3 * h + i, NTT::mul8(x3, NTT::load8(twist + 3 * h + i)));
			}
		} else {
			for (int i = 0; i < h; i += 8) {
				I x = NTT::load8(a + i), y = NTT::load8(a + h + i);
				I u = NTT::add8(x, y), v = NTT::sub8(x, y);
				if (i == 0) u = NTT::sub8(u, _mm256_setr_epi32(correction, 0, 0, 0, 0, 0, 0, 0));
				NTT::store8(a + i, NTT::mul8(u, NTT::load8(twist + i)));
				NTT::store8(a + h + i, NTT::mul8(v, NTT::load8(twist + h + i)));
			}
		}
	}
#endif
	void forward(U *a, int n) const {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 32 && NTT::vectorized()) {
			forward8(a, n);
			return;
		}
#endif
		t.fft<false>(a, n);
	}
	void inverse_twisted(U *a, int n, const U *twist, U correction) const {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 16 && NTT::vectorized()) {
			inverse_twisted8(a, n, twist, correction);
			return;
		}
#endif
		t.fft<true>(a, n);
		a[0] = NTT::sub(a[0], correction);
		NTT::product(a, a, twist, n);
	}
};

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
