#ifndef CP_FPS_LINEAR_RECURRENCE_HPP
#define CP_FPS_LINEAR_RECURRENCE_HPP

#include "bostan_mori.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
T linear_recurrence(const std::vector<T> &initial, const std::vector<T> &coefficients, long long k) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	assert(k >= 0 && initial.size() == coefficients.size());
	const int d = static_cast<int>(initial.size());
	if (k < d) return initial[k];
	int order = d;
	auto zero = [](T x) {
		if constexpr (std::is_same_v<T, mint>) return x == T(0);
		else return x.val() == 0;
	};
	while (order && zero(coefficients[order - 1])) --order;
	if (order == 0) return 0;
	if constexpr (!std::is_same_v<T, mint>) {
		typename O::W a(order), c(order);
		for (int i = 0; i < order; ++i) {
			a[i] = mint::raw(initial[d - order + i].val());
			c[i] = mint::raw(coefficients[i].val());
		}
		return T::raw(linear_recurrence<mint>(a, c, k - d + order).val());
	}
	if (order == 1) return initial[d - 1] * coefficients[0].pow(k - d + 1);
	F q(order + 1);
	q[0] = 1;
	for (int i = 0; i < order; ++i) q[i + 1] = -coefficients[i];
	F p = F::multiply(F(initial.end() - order, initial.end()), q).pre(order);
	return bostan_mori(std::move(p), std::move(q), k - d + order);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
