#ifndef INTEGER_ARITHMETIC_HPP
#define INTEGER_ARITHMETIC_HPP

#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace integer_arithmetic_detail {

template <class T>
inline constexpr bool integer = (std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>) && !std::is_same_v<T, bool>;

template <class T>
constexpr T add(const T &a, const T &b) {
	if constexpr (integer<T>) {
		T result;
		if (__builtin_add_overflow(a, b, &result)) throw std::overflow_error("integer addition");
		return result;
	} else return a + b;
}

template <class T>
constexpr T multiply(const T &a, const T &b) {
	if constexpr (integer<T>) {
		T result;
		if (__builtin_mul_overflow(a, b, &result)) throw std::overflow_error("integer multiplication");
		return result;
	} else return a * b;
}

}

#endif
