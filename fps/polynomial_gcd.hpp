#ifndef CP_FPS_POLYNOMIAL_GCD_HPP
#define CP_FPS_POLYNOMIAL_GCD_HPP

#include "fps.hpp"
#include "../components/polynomial_half_gcd.hpp"
#include <optional>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
struct PolynomialExtendedGCD {
	FormalPowerSeries<T> gcd, x, y;
};

template<class T>
FormalPowerSeries<T> polynomial_gcd(FormalPowerSeries<T> a, FormalPowerSeries<T> b) {
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>)
		return O::export_work(polynomial_gcd(O::import_work(a), O::import_work(b)));
	else {
		O::trim(a), O::trim(b);
		if (a.size() < b.size()) a.swap(b);
		if (!b.empty() && a.size() == b.size()) {
			auto r = a.divmod(b).second;
			a = std::move(b), b = std::move(r);
		}
		if (!b.empty()) {
			auto matrix = H::jump(a, b, int(a.size()) - 1, true);
			if (matrix.shift == int(a.size()) - 1) return {T(1)};
			a = H::apply_first(matrix, a, b);
		}
		if (!a.empty()) a /= a.back();
		return a;
	}
}

template<class T>
PolynomialExtendedGCD<T> polynomial_extended_gcd(FormalPowerSeries<T> a, FormalPowerSeries<T> b) {
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		auto result = polynomial_extended_gcd(O::import_work(a), O::import_work(b));
		return {O::export_work(result.gcd), O::export_work(result.x), O::export_work(result.y)};
	} else {
		O::trim(a), O::trim(b);
		if (a.empty() && b.empty()) return {};
		typename H::Matrix matrix;
		while (!b.empty() && a.size() <= b.size()) {
			auto [q, r] = a.divmod(b);
			H::step(matrix, q), a = std::move(b), b = std::move(r);
		}
		if (!b.empty()) {
			auto part = H::jump(a, b, int(a.size()) - 1, true);
			a = H::apply_first(part, a, b);
			matrix = H::multiply(part, matrix, true);
		}
		T inverse = a.back().inv();
		a *= inverse, matrix.a[0] *= inverse, matrix.a[1] *= inverse;
		return {std::move(a), std::move(matrix.a[0]), std::move(matrix.a[1])};
	}
}

template<class T>
std::optional<FormalPowerSeries<T>> polynomial_inv_mod(FormalPowerSeries<T> a, FormalPowerSeries<T> modulus) {
	fps_detail::Operations<T>::trim(modulus);
	assert(!modulus.empty());
	if (modulus.size() == 1) return FormalPowerSeries<T>{};
	using H = fps_detail::PolynomialHalfGCD;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) {
		auto result = polynomial_inv_mod(O::import_work(a), O::import_work(modulus));
		if (!result) return std::nullopt;
		return O::export_work(*result);
	} else {
		if (a.size() >= modulus.size()) a = a.divmod(modulus).second;
		O::trim(a);
		if (a.empty()) return std::nullopt;
		auto matrix = H::jump(modulus, a, int(modulus.size()) - 1, true);
		if (matrix.shift != int(modulus.size()) - 1) return std::nullopt;
		T constant = (matrix.a[0].empty() ? T(0) : matrix.a[0][0] * modulus[0]) +
			(matrix.a[1].empty() ? T(0) : matrix.a[1][0] * a[0]);
		matrix.a[1] /= constant;
		return std::move(matrix.a[1]);
	}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
