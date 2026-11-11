#ifndef CP_COMPONENTS_FPS_COMPOSITION_LIGHT_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_LIGHT_HPP

#include "../fps/fps_light.hpp"
#include "fps_composition_generic.hpp"

namespace light_fps_algorithms {
template<class T>
Series<T> compose(const Series<T> &outer, const Series<T> &inner, int deg = -1) {
	assert(deg <= (1 << 21) && (deg >= 0 || outer.size() <= (std::size_t(1) << 21)));
	return fps_detail::composition_fallback::compose(outer, inner, deg);
}

template<class T>
Series<T> power_projection(const Series<T> &f, const Series<T> &weights, int deg = -1) {
	return fps_detail::composition_fallback::power_projection(f, weights, deg);
}

template<class T>
Series<T> power_projection(const Series<T> &f, int n, int deg = -1) {
	return fps_detail::composition_fallback::power_projection(f, n, deg);
}

template<class T>
Series<T> compositional_inverse(const Series<T> &f, int deg = -1) {
	return fps_detail::composition_fallback::compositional_inverse(f, deg);
}
}

#ifdef CP_FPS_LIGHT_API
#ifndef CP_FPS_COMPOSE_API
#define CP_FPS_COMPOSE_API 1
using light_fps_algorithms::compose;
#endif
#ifndef CP_FPS_PROJECT_API
#define CP_FPS_PROJECT_API 1
using light_fps_algorithms::power_projection;
#endif
#ifndef CP_FPS_REVERT_API
#define CP_FPS_REVERT_API 1
using light_fps_algorithms::compositional_inverse;
#endif
#endif

#endif
