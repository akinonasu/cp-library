#ifndef DIVISOR_ZETA_HPP
#define DIVISOR_ZETA_HPP

#include "../math/number_theory/prime_sieve.hpp"
#include <cstddef>
#include <vector>

namespace divisor_zeta_detail {

template <bool inverse, bool multiples, class F>
void for_each(std::size_t n, const std::vector<std::size_t> &ps, std::size_t count, F update) {
	if (!n) return;
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto apply = [&](std::size_t i) { update(multiples ? i : i * p, multiples ? i * p : i); };
		if constexpr (inverse == multiples) {
			std::size_t i = 1;
			for (; i + 3 <= end; i += 4) apply(i), apply(i + 1), apply(i + 2), apply(i + 3);
			for (; i <= end; ++i) apply(i);
		} else {
			std::size_t i = end;
			for (; i >= 4; i -= 4) apply(i), apply(i - 1), apply(i - 2), apply(i - 3);
			for (; i; --i) apply(i);
		}
	}
}

}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a, const std::vector<std::size_t> &ps) {
	divisor_zeta_detail::for_each<inverse, multiples>(a.size(), ps, ps.size(), [&](std::size_t to, std::size_t from) {
		if constexpr (inverse) a[to] -= a[from];
		else a[to] += a[from];
	});
}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a) { divisor_zeta<inverse, multiples>(a, prime_sieve(a.size())); }

#endif
