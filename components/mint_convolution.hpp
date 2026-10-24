#ifndef MINT_CONVOLUTION_HPP
#define MINT_CONVOLUTION_HPP

#include "ntt.hpp"
#include "fps_convolution.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_convolution_detail {
inline std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b) {
	using NTT = fps_detail::NTT;
	using Word = NTT::U;
	if (a.empty() || b.empty()) return {};
	auto words = [](auto &v) { return reinterpret_cast<Word *>(v.data()); };
	auto input = [](const auto &v) { return reinterpret_cast<const Word *>(v.data()); };
	if (std::min(a.size(), b.size()) <= 32) {
		std::vector<mint> result(a.size() + b.size() - 1);
		const auto &small = a.size() < b.size() ? a : b;
		const auto &large = a.size() < b.size() ? b : a;
		for (int j = 0; j < static_cast<int>(small.size()); ++j)
			NTT::add_scaled(words(result) + j, input(large), large.size(), small[j].val());
		return result;
	}
	int n = 1, size = a.size() + b.size() - 1;
	while (n < size) n <<= 1;
	assert(n <= (1 << 23));
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (NTT::vectorized()) return fps_detail::multiply_detail::multiply(a, b);
#endif
	std::vector<mint> x = a, y;
	x.resize(n);
	if (&a != &b) y = b, y.resize(n);
	auto &plan = fps_detail::ntt_plan(std::max(1, n / 2));
	plan.fft<false>(words(x), n);
	if (&a == &b) NTT::product(words(x), input(x), input(x), n);
	else {
		plan.fft<false>(words(y), n);
		NTT::product(words(x), input(x), input(y), n);
	}
	plan.fft<true>(words(x), n);
	plan.scale(words(x), n, mint(n).inv().val());
	x.resize(size);
	return x;
}
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
