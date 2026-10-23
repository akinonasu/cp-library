#ifndef INTEGER_DIVISION_HPP
#define INTEGER_DIVISION_HPP

#include "../../components/integer_arithmetic.hpp"
#include <limits>
#include <stdexcept>
#include <type_traits>

template <class T, class U>
	requires (integer_arithmetic_detail::integer<T> && integer_arithmetic_detail::integer<U> && std::numeric_limits<T>::is_signed == std::numeric_limits<U>::is_signed)
constexpr std::common_type_t<T, U> floor_div(T a, U b) {
	using R = std::common_type_t<T, U>;
	R x = a, y = b;
	if (!y) throw std::invalid_argument("floor_div divisor");
	if constexpr (std::numeric_limits<R>::is_signed) {
		if (x == std::numeric_limits<R>::min() && y == -1) throw std::overflow_error("floor_div quotient");
		return x / y - (x % y != 0 && ((x < 0) != (y < 0)));
	} else return x / y;
}

template <class T, class U>
	requires (integer_arithmetic_detail::integer<T> && integer_arithmetic_detail::integer<U> && std::numeric_limits<T>::is_signed == std::numeric_limits<U>::is_signed)
constexpr std::common_type_t<T, U> ceil_div(T a, U b) {
	using R = std::common_type_t<T, U>;
	R x = a, y = b;
	if (!y) throw std::invalid_argument("ceil_div divisor");
	if constexpr (std::numeric_limits<R>::is_signed) {
		if (x == std::numeric_limits<R>::min() && y == -1) throw std::overflow_error("ceil_div quotient");
		return x / y + (x % y != 0 && ((x < 0) == (y < 0)));
	} else return x / y + (x % y != 0);
}

#endif
