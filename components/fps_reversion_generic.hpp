#ifndef CP_COMPONENTS_FPS_REVERSION_GENERIC_HPP
#define CP_COMPONENTS_FPS_REVERSION_GENERIC_HPP

#include "fps_projection_generic.hpp"

namespace fps_detail::composition_fallback {
template<class F>
F power_projection(const F &f, int n, int deg = -1) {
	assert(0 <= n && n < (1 << 21));
	F weights(n + 1);
	weights[n] = 1;
	return fps_detail::composition_fallback::power_projection(f, weights, deg);
}


template<class F>
F compositional_inverse(const F &f, int deg = -1) {
	using T = typename F::value_type;
	if (deg < 0) deg = int(f.size());
	assert(0 <= deg && deg <= (1 << 21) && (f.empty() || f[0] == T(0)));
	if (deg <= 1) return F(deg);
	assert(f.size() >= 2 && f[1] != T(0));
	F projected = fps_detail::composition_fallback::power_projection(f, deg - 1), h(deg - 1);
	std::vector<T> inverse(deg, T(1));
	for (int i = 2; i < deg; ++i) inverse[i] = -T(T::mod() / i) * inverse[T::mod() % i];
	for (int i = 1; i < deg; ++i) h[deg - 1 - i] = projected[i] * inverse[i];
	h /= h[0];
	h = (h.log() * -inverse[deg - 1]).exp() / f[1];
	h.insert(h.begin(), T(0));
	return h;
}
}

#endif
