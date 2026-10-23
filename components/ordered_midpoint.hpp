#ifndef ORDERED_MIDPOINT_HPP
#define ORDERED_MIDPOINT_HPP

#include <type_traits>

template<class T>
constexpr T ordered_midpoint(T l, T r) {
	static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	return T(U(l) + U(U(r) - U(l)) / 2);
}

#endif
