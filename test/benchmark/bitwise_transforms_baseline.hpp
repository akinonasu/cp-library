#ifndef BITWISE_TRANSFORMS_BASELINE_HPP
#define BITWISE_TRANSFORMS_BASELINE_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace bitwise_transforms_baseline {
template <bool inverse = false, bool superset = false, class T>
void bitwise_zeta(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	for (std::size_t half = 1; half < a.size(); half *= 2)
		for (std::size_t start = 0; start < a.size(); start += 2 * half)
			for (std::size_t j = 0; j < half; ++j) {
				T &to = a[start + j + (superset ? 0 : half)];
				const T &from = a[start + j + (superset ? half : 0)];
				if constexpr (inverse) to -= from;
				else to += from;
			}
}

template <class T>
void walsh_hadamard(std::vector<T> &a) {
	assert(a.empty() || std::has_single_bit(a.size()));
	for (std::size_t half = 1; half < a.size(); half *= 2)
		for (std::size_t start = 0; start < a.size(); start += 2 * half)
			for (std::size_t j = 0; j < half; ++j) {
				T x = a[start + j], y = a[start + j + half];
				a[start + j] = x + y;
				a[start + j + half] = x - y;
			}
}

}

#endif
