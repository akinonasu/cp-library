#ifndef CYCLIC_CONVOLUTION_HPP
#define CYCLIC_CONVOLUTION_HPP

#include <cassert>
#include <cstddef>
#include <vector>

#include "../components/convolution_dispatch.hpp"

template <bool negative = false, class T>
std::vector<T> cyclic_convolution(const std::vector<T> &a, const std::vector<T> &b, std::size_t n) {
	assert(n > 0);
	auto fold = [n](const std::vector<T> &v) {
		std::vector<T> result(n, T(0));
		for (std::size_t i = 0; i < v.size(); ++i) {
			if constexpr (negative) {
				if ((i / n) & 1) result[i % n] -= v[i];
				else result[i % n] += v[i];
			} else result[i % n] += v[i];
		}
		return result;
	};
	auto x = fold(a), y = fold(b);
	auto product = convolution(x, y);
	x.assign(n, T(0));
	for (std::size_t i = 0; i < product.size(); ++i) {
		if constexpr (negative) {
			if (i >= n) x[i - n] -= product[i];
			else x[i] += product[i];
		} else x[i % n] += product[i];
	}
	return x;
}

#endif
