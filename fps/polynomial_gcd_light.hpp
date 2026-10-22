#ifndef CP_FPS_POLYNOMIAL_GCD_LIGHT_HPP
#define CP_FPS_POLYNOMIAL_GCD_LIGHT_HPP

#include "../components/polynomial_half_gcd_light.hpp"
#include <optional>

namespace light_fps_algorithms {
template<class T>
struct PolynomialExtendedGCD {
	LightFormalPowerSeries<T> gcd, x, y;
};

template<class T>
LightFormalPowerSeries<T> polynomial_gcd(LightFormalPowerSeries<T> a, LightFormalPowerSeries<T> b) {
	a.shrink(), b.shrink();
	fps_light_detail::PolynomialHalfGCD<T>::reduce(a, b);
	if (!a.empty()) a /= a.back();
	return a;
}

template<class T>
PolynomialExtendedGCD<T> polynomial_extended_gcd(LightFormalPowerSeries<T> a, LightFormalPowerSeries<T> b) {
	a.shrink(), b.shrink();
	if (a.empty() && b.empty()) return {};
	using H = fps_light_detail::PolynomialHalfGCD<T>;
	typename H::Matrix matrix;
	H::reduce(a, b, &matrix);
	T inverse = a.back().inv();
	a *= inverse, matrix.a[0] *= inverse, matrix.a[1] *= inverse;
	return {std::move(a), std::move(matrix.a[0]), std::move(matrix.a[1])};
}

template<class T>
std::optional<LightFormalPowerSeries<T>> polynomial_inv_mod(LightFormalPowerSeries<T> a, LightFormalPowerSeries<T> modulus) {
	modulus.shrink();
	assert(!modulus.empty());
	if (modulus.size() == 1) return LightFormalPowerSeries<T>{};
	a = a.divmod(modulus).second;
	auto result = polynomial_extended_gcd(std::move(a), modulus);
	if (result.gcd.size() != 1) return std::nullopt;
	return result.x.divmod(modulus).second;
}
}

#endif
