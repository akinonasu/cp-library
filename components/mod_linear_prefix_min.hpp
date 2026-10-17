#ifndef MOD_LINEAR_PREFIX_MIN_HPP
#define MOD_LINEAR_PREFIX_MIN_HPP

#include <cstdint>
#include <type_traits>

namespace mod_linear_detail {
template <class U, class Emit>
void prefix_min(U n, U m, U a, U b, Emit &&emit) {
	using W = std::conditional_t<(sizeof(U) <= 4), std::uint64_t, __uint128_t>;
	bool first = true;
	if (a && b && n > 1) {
		U p = m - a, q = a, sp = 1, sq = 1, x = 0, remaining = n - 1;
		for (;;) {
			if (p <= b) {
				U k = b / p;
				bool final = W(k) * sp > remaining;
				if (final) k = remaining / sp;
				if (k) {
					if (first) emit(U(0), b, sp, p, U(k + 1));
					else emit(U(x + sp), U(b - p), sp, p, k);
					first = false;
					x += k * sp;
					b -= k * p;
					remaining -= k * sp;
				}
				if (final || !b) break;
			}
			if (sp > remaining || sq > remaining - sp) break;
			if (p > q) {
				U k = (p - b - 1) / q + 1, d = k * q;
				if (d == p) break;
				if (d > p) --k;
				sp += k * sq;
				if (sp > remaining) break;
				p -= k * q;
			} else {
				U k = q / p;
				q %= p;
				if (!q) break;
				sq += k * sp;
			}
		}
	}
	if (first) emit(U(0), b, U(0), U(0), U(1));
}
}

#endif
