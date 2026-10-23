#ifndef SYMMETRIC_GROUP_CONVOLUTION_HPP
#define SYMMETRIC_GROUP_CONVOLUTION_HPP

#include "../components/symmetric_group_fft.hpp"
#include "../components/matrix_arithmetic.hpp"
#include <stdexcept>
#include <vector>

template <class Mint>
std::vector<Mint> symmetric_group_convolution(const SymmetricGroupFFT<Mint> &fft, const std::vector<Mint> &a, const std::vector<Mint> &b) {
	auto x = fft.forward(a), y = fft.forward(b);
	for (const auto &block : fft.blocks()) {
		int d = block.dimension, offset = block.offset, length = d * d;
		std::vector<Mint> left(x.begin() + offset, x.begin() + offset + length);
		std::vector<Mint> right(y.begin() + offset, y.begin() + offset + length);
		auto product = matrix_detail::multiply(left, right, d, d, d);
		std::copy(product.begin(), product.end(), x.begin() + offset);
	}
	return fft.inverse(x);
}

template <class Mint>
std::vector<Mint> symmetric_group_convolution(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	int n = 0, size = 1;
	while (std::size_t(size) < a.size() && n < 10) size *= ++n;
	if (a.size() != std::size_t(size) || b.size() != a.size()) throw std::invalid_argument("symmetric_group_convolution size");
	return symmetric_group_convolution(SymmetricGroupFFT<Mint>(n), a, b);
}

#endif
