#ifndef GCD_CONVOLUTION_MINT_HPP
#define GCD_CONVOLUTION_MINT_HPP

#include "divisor_zeta.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mint_detail { struct Mint; }

namespace gcd_convolution_mint_detail {

template <bool inverse, class T>
void large_multiples(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps,
					 std::size_t first, const std::vector<std::size_t> &products, const std::vector<std::size_t> &squares) {
	if (first == ps.size()) return;
	constexpr std::uint64_t twice = 2 * T::mod();
	const std::size_t n = a.size() - 1;
	for (std::size_t i = 1; i <= n / ps[first]; ++i) {
		const auto end = std::upper_bound(ps.begin() + first, ps.end(), n / i);
		std::uint64_t x = inverse ? 0 : a[i].v, y = inverse ? a[i].v : b[i].v;
		for (auto p = ps.begin() + first; p != end; ++p) {
			x += a[i * *p].v;
			if constexpr (!inverse) y += b[i * *p].v;
		}
		for (auto p : products) {
			if (p > n / i) break;
			if constexpr (inverse) y += a[i * p].v;
			else x += a[i * p].v, y += b[i * p].v;
		}
		if constexpr (inverse) a[i] = T::word(y % twice) - T::word(x % twice);
		else {
			for (auto p : squares) {
				if (p > n / i) break;
				x += a[i * p].v, y += b[i * p].v;
			}
			a[i] = T::word(x % twice), b[i] = T::word(y % twice);
		}
	}
}

template <class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	const std::size_t n = a.size() - 1;
	std::size_t cutoff = std::sqrt(n) / 2;
	while (cutoff + 1 <= n / (cutoff + 1) / (cutoff + 1)) ++cutoff;
	const std::size_t first = std::upper_bound(ps.begin(), ps.end(), cutoff) - ps.begin();
	std::vector<std::size_t> products, squares;
	for (std::size_t j = first; j < ps.size() && ps[j] <= n / ps[j]; ++j) {
		squares.push_back(ps[j] * ps[j]);
		for (std::size_t k = j + 1; k < ps.size() && ps[k] <= n / ps[j]; ++k) products.push_back(ps[j] * ps[k]);
	}
	std::sort(products.begin(), products.end());
	divisor_zeta_detail::for_each<false, true>(a.size(), ps, first, [&](std::size_t to, std::size_t from) {
		a[to] += a[from], b[to] += b[from];
	});
	large_multiples<false>(a, b, ps, first, products, squares);
	for (std::size_t i = 1; i <= n; ++i) a[i] *= b[i];
	divisor_zeta_detail::for_each<true, true>(a.size(), ps, first, [&](std::size_t to, std::size_t from) { a[to] -= a[from]; });
	large_multiples<true>(a, b, ps, first, products, squares);
}

}

#endif
