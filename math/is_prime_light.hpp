#ifndef IS_PRIME_LIGHT_HPP
#define IS_PRIME_LIGHT_HPP

#include "../components/is_prime_core.hpp"

inline bool light_is_prime(std::uint64_t n) {
	return is_prime_detail::check(n, [](const auto &m) { return is_prime_detail::prime(m); });
}

#endif
