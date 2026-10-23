#ifndef CP_FPS_RATIONAL_FPS_COEFFICIENTS_HPP
#define CP_FPS_RATIONAL_FPS_COEFFICIENTS_HPP

#include "fps.hpp"
#include "../components/rational_fps_coefficients.hpp"

namespace fps_detail {
struct CoefficientRangeTransform {
	using O = Operations<mint>;
	static void forward(fps &f) { O::ntt(f); }
	static void inverse(fps &f) { O::intt(f); }
	static void product(fps &f, const fps &g) { O::pointwise(f, f, g, int(f.size())); }
};
}

template <class T>
FormalPowerSeries<T> rational_fps_coefficients(FormalPowerSeries<T> p, FormalPowerSeries<T> q, long long l, long long r) {
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, mint>)
		return O::export_work(rational_fps_coefficients(O::import_work(p), O::import_work(q), l, r));
	else return rational_fps_detail::Coefficients<fps, fps_detail::CoefficientRangeTransform>::get(std::move(p), std::move(q), l, r);
}

#endif
