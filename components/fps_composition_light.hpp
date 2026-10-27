#ifndef CP_COMPONENTS_FPS_COMPOSITION_LIGHT_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_LIGHT_HPP

#include "../fps/fps_light.hpp"
#include "fps_composition_generic.hpp"

namespace light_fps_algorithms {
template<class T>
LightFormalPowerSeries<T> compose(const LightFormalPowerSeries<T> &outer, const LightFormalPowerSeries<T> &inner, int deg = -1) {
	assert(deg <= (1 << 21) && (deg >= 0 || outer.size() <= (std::size_t(1) << 21)));
	return fps_detail::composition_fallback::compose(outer, inner, deg);
}

template<class T>
LightFormalPowerSeries<T> power_projection(const LightFormalPowerSeries<T> &f, const LightFormalPowerSeries<T> &weights, int deg = -1) {
	return fps_detail::composition_fallback::power_projection(f, weights, deg);
}

template<class T>
LightFormalPowerSeries<T> power_projection(const LightFormalPowerSeries<T> &f, int n, int deg = -1) {
	return fps_detail::composition_fallback::power_projection(f, n, deg);
}

template<class T>
LightFormalPowerSeries<T> compositional_inverse(const LightFormalPowerSeries<T> &f, int deg = -1) {
	return fps_detail::composition_fallback::compositional_inverse(f, deg);
}
}

#endif
