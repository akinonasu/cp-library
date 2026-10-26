#ifndef CP_FPS_LIGHT_BACKEND_HPP
#define CP_FPS_LIGHT_BACKEND_HPP

#include "fps_light_core.hpp"
#include "polynomial_product_tree.hpp"

namespace fps_light_detail {
template<class T>
using ProductTree = polynomial_detail::ProductTree<LightFormalPowerSeries<T>>;
}

namespace light_fps_algorithms {
template<class T>
LightFormalPowerSeries<T> taylor_shift(LightFormalPowerSeries<T> f, std::type_identity_t<T> c) {
	using F = LightFormalPowerSeries<T>;
	const std::size_t n = f.size();
	if (n <= 1 || c == T(0)) return f;
	assert(n < T::mod());
	F factorial(n, T(1)), inverse(n, T(1)), a(n), b(n);
	for (std::size_t i = 1; i < n; ++i) factorial[i] = factorial[i - 1] * T(i);
	inverse[n - 1] = factorial[n - 1].inv();
	for (std::size_t i = n - 1; i; --i) inverse[i - 1] = inverse[i] * T(i);
	T power = 1;
	for (std::size_t i = 0; i < n; ++i) {
		a[n - 1 - i] = f[i] * factorial[i];
		b[i] = power * inverse[i];
		power *= c;
	}
	a = F::multiply(a, b);
	for (std::size_t i = 0; i < n; ++i) f[i] = a[n - 1 - i] * inverse[i];
	return f;
}

template<class T>
T bostan_mori(LightFormalPowerSeries<T> p, LightFormalPowerSeries<T> q, long long k) {
	using F = LightFormalPowerSeries<T>;
	assert(k >= 0 && !q.empty() && q[0] != T(0));
	p.shrink();
	q.shrink();
	while (k && !p.empty() && q.size() > 1) {
		F opposite = q;
		for (std::size_t i = 1; i < opposite.size(); i += 2) opposite[i] = -opposite[i];
		F numerator = F::multiply(p, opposite), denominator = F::multiply(q, opposite);
		p.resize((numerator.size() + 1 - std::size_t(k & 1)) / 2);
		for (std::size_t i = 0; i < p.size(); ++i) p[i] = numerator[2 * i + std::size_t(k & 1)];
		q.resize((denominator.size() + 1) / 2);
		for (std::size_t i = 0; i < q.size(); ++i) q[i] = denominator[2 * i];
		p.shrink();
		q.shrink();
		k >>= 1;
	}
	return static_cast<unsigned long long>(k) < p.size() ? p[std::size_t(k)] / q[0] : T(0);
}

template<class T>
T linear_recurrence(const std::vector<T> &initial, const std::vector<T> &coefficients, long long k) {
	using F = LightFormalPowerSeries<T>;
	assert(k >= 0 && initial.size() == coefficients.size());
	const std::size_t n = initial.size();
	if (static_cast<unsigned long long>(k) < n) return initial[std::size_t(k)];
	if (!n) return T(0);
	F q(n + 1);
	q[0] = T(1);
	for (std::size_t i = 0; i < n; ++i) q[i + 1] = -coefficients[i];
	F p = F::multiply(F(initial.begin(), initial.end()), q).pre(static_cast<int>(n));
	return bostan_mori(std::move(p), std::move(q), k);
}

template<class T>
std::vector<T> multipoint_eval(const LightFormalPowerSeries<T> &f, const std::vector<T> &xs) {
	if (f.size() <= 32 || xs.size() <= 32) {
		std::vector<T> result;
		result.reserve(xs.size());
		for (T x : xs) result.push_back(f.eval(x));
		return result;
	}
	return fps_light_detail::ProductTree<T>(xs).evaluate(f);
}

template<class T>
LightFormalPowerSeries<T> interpolate(const std::vector<T> &xs, const std::vector<T> &ys) {
	assert(xs.size() == ys.size());
	return fps_light_detail::ProductTree<T>(xs).interpolate(ys);
}
}

#endif
