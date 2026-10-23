#ifndef CP_FPS_COMPOSITIONAL_INVERSE_HPP
#define CP_FPS_COMPOSITIONAL_INVERSE_HPP

#include "fps.hpp"
#include "../components/fps_reversion.hpp"
#include "../components/fps_composition_generic.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> compositional_inverse(const FormalPowerSeries<T> &f, int deg = -1) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if (deg < 0) deg = static_cast<int>(f.size());
	assert(deg >= 0 && (f.empty() || f[0] == T(0)));
	if (deg == 0) return {};
	if (deg == 1) return F(1);
	assert(f.size() >= 2 && f[1] != T(0));
	if constexpr (!std::is_same_v<T, mint>)
		return O::export_work(compositional_inverse<mint>(O::import_work(f), deg));
	else {
		T w = f[1].inv();
		int length = std::min<int>(deg, f.size());
		int first = 2;
		while (first < length && f[first] == T(0)) ++first;
		if (first == length) {
			F result(deg);
			result[1] = w;
			return result;
		}
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
		if (deg >= 64 && deg <= (1 << 21) && fps_detail::NTT::vectorized()) {
			F result;
			fps_detail::composition_poly::native_reversion(f.data(), f.size(), result, deg);
			return result;
		}
#endif
		return fps_detail::composition_fallback::compositional_inverse(f, deg);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
