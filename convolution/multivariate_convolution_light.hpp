#ifndef MULTIVARIATE_CONVOLUTION_LIGHT_HPP
#define MULTIVARIATE_CONVOLUTION_LIGHT_HPP

#include <cassert>
#include <cstddef>
#include <vector>
#include "../components/multivariate_convolution_core.hpp"

#include "convolution_light.hpp"

namespace light_convolution {
template <class T>
std::vector<T> multivariate_convolution(const std::vector<T> &a, const std::vector<T> &b, const std::vector<int> &dimensions) {
	std::size_t size = 1;
	std::vector<std::size_t> bases;
	for (int d : dimensions) {
		assert(d > 0 && size <= std::size_t(-1) / d);
		if (d == 1) continue;
		size *= d;
		bases.push_back(size);
	}
	assert(a.size() == size && b.size() == size);
	if (bases.empty()) return {a[0] * b[0]};
	if (bases.size() == 1) {
		auto result = light_convolution::convolution(a, b);
		result.resize(size);
		return result;
	}
	int colors = bases.size();

	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			assert(size <= std::size_t(1) << 22);
			std::vector<mint> x(size), y(size);
			std::vector<int> dims;
			for (int d : dimensions) if (d > 1) dims.push_back(d);
			for (std::size_t i = 0; i < size; ++i) x[i] = mint::raw(a[i].val()), y[i] = mint::raw(b[i].val());
			auto transform = [](std::vector<mint> &v, bool inverse) { fps_light_detail::transform(v, inverse); };
			auto product = multivariate_convolution_detail::Core::multiply(x, y, dims, transform, fps_light_detail::multiply);
			std::vector<T> result;
			result.reserve(size);
			for (auto v : product) result.push_back(T::raw(v.val()));
			return result;
		}
	}
	std::vector<int> rank(size);
	for (std::size_t i = 0; i < size; ++i) {
		std::size_t value = 0;
		for (std::size_t base : bases) value += i / base;
		rank[i] = value % colors;
	}
	std::size_t stride = 2 * colors - 1;
	assert(size <= std::size_t(-1) / stride / 2);
	std::vector<T> x(size * stride, T(0)), y(size * stride, T(0));
	for (std::size_t i = 0; i < size; ++i) x[i * stride + rank[i]] = a[i], y[i * stride + rank[i]] = b[i];
	auto product = light_convolution::convolution(x, y);
	std::vector<T> result(size, T(0));
	for (std::size_t i = 0; i < size; ++i) {
		result[i] = product[i * stride + rank[i]];
		if (rank[i] + colors < int(stride)) result[i] += product[i * stride + rank[i] + colors];
	}
	return result;
}

}

#endif
