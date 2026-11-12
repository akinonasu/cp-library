#ifndef MULTIVARIATE_CONVOLUTION_PRODUCT_HPP
#define MULTIVARIATE_CONVOLUTION_PRODUCT_HPP

#include "mint_convolution.hpp"
#include <bit>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace multivariate_convolution_detail {
struct Product {
	std::vector<mint> output, fallback;
#if CP_GCC_LINUX64
	std::unique_ptr<fps_detail::multiply_detail::Product> plan;
	mint *borrowed = nullptr;
#endif
	std::span<const mint> operator()(const std::vector<mint> &a, const std::vector<mint> &b) {
#if CP_GCC_LINUX64
		using namespace fps_detail::multiply_detail;
		if (a.size() > 32 && fps_detail::NTT::vectorized() && !constant::fits(a, b)) {
			const std::size_t len = std::bit_ceil(a.size() + b.size() - 1);
			if (!plan) {
				fps_detail::prepare_fps_output(output, len + 23);
				output.resize(len + 23);
				const auto address = reinterpret_cast<std::uintptr_t>(output.data());
				const std::size_t offset = ((32 - (address & 31)) & 31) / sizeof(mint);
				borrowed = output.data() + offset;
				plan = std::make_unique<fps_detail::multiply_detail::Product>(a.size(), b.size(), reinterpret_cast<fps_detail::Word *>(borrowed));
			}
			copy_input_mint(a, borrowed);
			copy_input(b, plan->b(), len);
			plan->multiply();
			return {borrowed, a.size() + b.size() - 1};
		}
#endif
		fallback = mint_convolution_detail::multiply(a, b);
		return fallback;
	}
};
}

#endif
