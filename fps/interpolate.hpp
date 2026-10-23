#ifndef CP_FPS_INTERPOLATE_HPP
#define CP_FPS_INTERPOLATE_HPP

#include "fps.hpp"
#include "../components/polynomial_product_tree.hpp"
#include "../components/fps_interpolation.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> interpolate(const std::vector<T> &xs, const std::vector<T> &ys) {
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, mint>) return O::export_work(interpolate<mint>(O::import_work(xs), O::import_work(ys)));
	assert(xs.size() == ys.size());
	const int n = static_cast<int>(xs.size());
	if (n == 0) return {};
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if constexpr (std::is_same_v<T, mint>) {
		if (n >= 33 && n <= (1 << 23) && fps_detail::NTT::vectorized())
			return FormalPowerSeries<T>(fps_detail::evaluation_poly::native_interpolate(xs, ys));
	}
#endif

	return polynomial_detail::ProductTree<FormalPowerSeries<T>>(xs).interpolate(ys);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
