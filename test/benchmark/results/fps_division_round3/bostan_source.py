from pathlib import Path
w=Path('/tmp/fps-division-round3/bostan_candidate')
s='''#ifndef CP_COMPONENTS_FPS_BOSTAN_HPP
#define CP_COMPONENTS_FPS_BOSTAN_HPP

#include "ntt.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("avx2")
#pragma GCC optimize("O3")

namespace fps_detail::bostan_detail {
inline void inverse_twisted(const NTT& plan, const NTT::U* in, NTT::U* out, const NTT::U* powers, int n) {
	using N = NTT;
	if (n == 8) {
		auto z = plan.small8<4, true>(plan.small8<2, true>(plan.small8<1, true>(N::load8(in), 0), 0), 0);
		N::store8(out, N::mul8(z, N::load8(powers)));
		return;
	}
	for (int j = 0; j < n; j += 8) {
		auto z = N::load8(in + j);
		z = plan.small8<4, true>(plan.small8<2, true>(plan.small8<1, true>(z, j / 8), j / 8), j / 8);
		N::store8(out + j, z);
	}
	int h = 8;
	for (; 4 * h < n; h *= 4) plan.radix4_8<true>(out, h, n);
	if (4 * h == n) {
		const auto im = _mm256_set1_epi32(plan.inverse_roots[1]);
		const auto iq = _mm256_set1_epi32(plan.inverse_root_quotients[1]);
		for (int i = 0; i < h; i += 8) {
			const auto x0 = N::load8(out + i), x1 = N::load8(out + h + i);
			const auto x2 = N::load8(out + 2 * h + i), x3 = N::load8(out + 3 * h + i);
			const auto u = N::add8(x0, x1), v = N::sub8(x0, x1), s = N::add8(x2, x3);
			const auto t = N::fixed_mul8(N::sub8(x2, x3), im, iq);
			N::store8(out + i, N::mul8(N::add8(u, s), N::load8(powers + i)));
			N::store8(out + h + i, N::mul8(N::add8(v, t), N::load8(powers + h + i)));
			N::store8(out + 2 * h + i, N::mul8(N::sub8(u, s), N::load8(powers + 2 * h + i)));
			N::store8(out + 3 * h + i, N::mul8(N::sub8(v, t), N::load8(powers + 3 * h + i)));
		}
	} else {
		for (int i = 0; i < h; i += 8) {
			const auto x = N::load8(out + i), y = N::load8(out + h + i);
			N::store8(out + i, N::mul8(N::add8(x, y), N::load8(powers + i)));
			N::store8(out + h + i, N::mul8(N::sub8(x, y), N::load8(powers + h + i)));
		}
	}
}
}

#pragma GCC pop_options
#endif
#endif
'''
(w/'components/fps_bostan.hpp').write_text(s)
p=w/'fps/bostan_mori.hpp';s=p.read_text().replace('#include "fps.hpp"','#include "fps.hpp"\n#include "../components/fps_bostan.hpp"')
a=s.index('\t\t\tstd::copy_n(v, n, odd);');b=s.index('\n\t\t\tplan.template fft<false>(odd, n);',a)
s=s[:a]+'''#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
			if (n >= 8 && fps_detail::NTT::vectorized()) {
				fps_detail::bostan_detail::inverse_twisted(plan, v, odd, O::words(powers), n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2) * leading).v);
			} else
#endif
			{
				std::copy_n(v, n, odd);
				plan.template fft<true>(odd, n);
				odd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);
				fps_detail::NTT::product(odd, odd, O::words(powers), n);
			}'''+s[b:]
p.write_text(s)
