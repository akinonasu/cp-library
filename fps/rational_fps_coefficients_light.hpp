#ifndef CP_FPS_RATIONAL_FPS_COEFFICIENTS_LIGHT_HPP
#define CP_FPS_RATIONAL_FPS_COEFFICIENTS_LIGHT_HPP

#include "fps_light.hpp"
#include "../components/rational_fps_coefficients.hpp"

namespace fps_light_detail {
struct CoefficientRangeTransform {
	static void forward(light_fps &f) { transform(f); }
	static void inverse(light_fps &f) { transform(f, true); }
	static void product(light_fps &f, const light_fps &g) { pointwise(f, g); }
};
}

namespace light_fps_algorithms {
template <class T>
LightFormalPowerSeries<T> rational_fps_coefficients(LightFormalPowerSeries<T> p, LightFormalPowerSeries<T> q, long long l, long long r) {
	if constexpr (!std::is_same_v<T, mint>) {
		light_fps a(p.size()), b(q.size());
		for (std::size_t i = 0; i < p.size(); ++i) a[i] = mint::raw(p[i].val());
		for (std::size_t i = 0; i < q.size(); ++i) b[i] = mint::raw(q[i].val());
		a = rational_fps_coefficients(std::move(a), std::move(b), l, r);
		LightFormalPowerSeries<T> result(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) result[i] = T::raw(a[i].val());
		return result;
	} else return rational_fps_detail::Coefficients<light_fps, fps_light_detail::CoefficientRangeTransform>::get(std::move(p), std::move(q), l, r);
}
}

#endif
