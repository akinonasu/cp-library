#ifndef GEOMETRIC_SUM_HPP
#define GEOMETRIC_SUM_HPP

#include "../components/integer_arithmetic.hpp"
#include <limits>
#include <stdexcept>

template <class T, class N>
	requires integer_arithmetic_detail::integer<N>
constexpr T geometric_sum(T ratio, N n) {
	using integer_arithmetic_detail::add;
	using integer_arithmetic_detail::multiply;
	if constexpr (std::numeric_limits<N>::is_signed) if (n < 0) throw std::invalid_argument("geometric_sum length");
	T sum = T(0), power = T(1), block = T(1);
	while (n) {
		if (n & 1) sum = add(sum, multiply(power, block));
		if (n == 1) break;
		if (n & 1) power = multiply(power, ratio);
		block = add(block, multiply(ratio, block));
		if (n >= 4) ratio = multiply(ratio, ratio);
		n >>= 1;
	}
	return sum;
}

template <class T, class N>
	requires integer_arithmetic_detail::integer<N>
constexpr T geometric_sum(T first, T ratio, N n) {
	return integer_arithmetic_detail::multiply(first, geometric_sum(ratio, n));
}

#endif
