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
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) return T::raw(linear_recurrence<fps_detail::Mint>(O::import_work(initial), O::import_work(coefficients), k).val());
	assert(k >= 0 && initial.size() == coefficients.size());
	const int d = static_cast<int>(initial.size());
	if (k < d) return initial[k];
	if (d == 0) return 0;
	F q(d + 1);
	q[0] = 1;
	for (int i = 0; i < d; ++i) q[i + 1] = -coefficients[i];
	O::trim(q);
	if (q.size() == 1) return 0;
	F p = F::multiply(F(initial), q).pre(d);
	return bostan_mori(std::move(p), std::move(q), k);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
