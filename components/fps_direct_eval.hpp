#ifndef CP_COMPONENTS_FPS_DIRECT_EVAL_HPP
#define CP_COMPONENTS_FPS_DIRECT_EVAL_HPP

#include "mint_arithmetic.hpp"
#include <algorithm>
#include <cstddef>

namespace fps_detail {
using mint_detail::Ops;

#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx2"))) inline void evaluate_direct(Ops::U *dst, const Ops::U *f,
		std::size_t n, const Ops::U *points, std::size_t m) {
	using I = Ops::I8;
	for (std::size_t i = 0; i < m; i += 32) {
		alignas(32) Ops::U tail[32] = {}, out[32];
		const std::size_t count = std::min<std::size_t>(32, m - i);
		if (count < 8) {
			for (std::size_t k = 0; k < count; ++k) {
				mint value = 0, x = mint::word(points[i + k]);
				for (std::size_t j = n; j-- > 0;) value = value * x + mint::word(f[j]);
				dst[i + k] = value.v;
			}
			break;
		}
		const Ops::U *xs = points + i;
		if (count < 32) {
			std::copy_n(xs, count, tail);
			xs = tail;
		}
		I x0 = Ops::load8(xs), x1 = Ops::load8(xs + 8);
		I x2 = Ops::load8(xs + 16), x3 = Ops::load8(xs + 24);
		I a0 = _mm256_setzero_si256(), a1 = a0, a2 = a0, a3 = a0;
		for (std::size_t j = n; j-- > 0;) {
			I c = _mm256_set1_epi32(f[j]);
			a0 = Ops::add8(Ops::mul8(a0, x0), c);
			a1 = Ops::add8(Ops::mul8(a1, x1), c);
			a2 = Ops::add8(Ops::mul8(a2, x2), c);
			a3 = Ops::add8(Ops::mul8(a3, x3), c);
		}
		Ops::U *output = count == 32 ? dst + i : out;
		Ops::store8(output, a0);
		Ops::store8(output + 8, a1);
		Ops::store8(output + 16, a2);
		Ops::store8(output + 24, a3);
		if (count < 32) std::copy_n(out, count, dst + i);
	}
}
#endif

}

#endif
