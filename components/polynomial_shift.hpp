#ifndef CP_COMPONENTS_POLYNOMIAL_SHIFT_HPP
#define CP_COMPONENTS_POLYNOMIAL_SHIFT_HPP

#include <algorithm>
#include <cassert>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace polynomial_detail {
template<bool Left, class F>
F &shift_fixed(F &a, int distance) {
	assert(distance >= 0);
	using T = typename F::value_type;
	if constexpr (Left) std::fill(a.begin(), std::shift_right(a.begin(), a.end(), distance), T(0));
	else std::fill(std::shift_left(a.begin(), a.end(), distance), a.end(), T(0));
	return a;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
