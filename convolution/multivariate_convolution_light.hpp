#ifndef MULTIVARIATE_CONVOLUTION_LIGHT_HPP
#define MULTIVARIATE_CONVOLUTION_LIGHT_HPP

#include <cassert>
#include <cstddef>
#include <vector>
#include <type_traits>
#include "../components/multivariate_convolution_core.hpp"

#include "convolution_light.hpp"

namespace light_convolution {
template <class T>
std::vector<T> multivariate_convolution(const std::vector<T> &a, const std::vector<T> &b, const std::vector<int> &dims) {
	using Size = std::size_t;
	Size n = 1;
	std::vector<Size> bases;
	for (int d : dims) {
		assert(d > 0 && n <= Size(-1) / d);
		if (d == 1) continue;
		n *= d;
		bases.push_back(n);
	}
	assert(a.size() == n && b.size() == n);
	if (bases.empty()) return {a[0] * b[0]};
	if (bases.size() == 1) {
		auto res = light_convolution::convolution(a, b);
		res.resize(n);
		return res;
	}
	int k = bases.size();

	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			assert(n <= Size(1) << 22);
			std::vector<int> shape;
			for (int d : dims) if (d > 1) shape.push_back(d);
			auto mul = [&](const std::vector<mint> &x, const std::vector<mint> &y) {
				auto ntt = [](std::vector<mint> &v, bool inv) { fps_light_detail::transform(v, inv); };
				return multivariate_convolution_detail::Core::multiply(x, y, shape, ntt, fps_light_detail::multiply);
			};
			if constexpr (std::is_same_v<T, mint>) return mul(a, b);
			std::vector<mint> x(n), y(n);
			for (Size i = 0; i < n; ++i) x[i] = mint::raw(a[i].val()), y[i] = mint::raw(b[i].val());
			auto prod = mul(x, y);
			std::vector<T> res;
			res.reserve(n);
			for (auto v : prod) res.push_back(T::raw(v.val()));
			return res;
		}
	}
	std::vector<int> rank(n);
	for (Size i = 0; i < n; ++i) {
		Size value = 0;
		for (Size base : bases) value += i / base;
		rank[i] = value % k;
	}
	Size step = 2 * k - 1;
	assert(n <= Size(-1) / step / 2);
	std::vector<T> x(n * step, T(0)), y(n * step, T(0));
	for (Size i = 0; i < n; ++i) x[i * step + rank[i]] = a[i], y[i * step + rank[i]] = b[i];
	auto prod = light_convolution::convolution(x, y);
	std::vector<T> res(n, T(0));
	for (Size i = 0; i < n; ++i) {
		res[i] = prod[i * step + rank[i]];
		if (rank[i] + k < int(step)) res[i] += prod[i * step + rank[i] + k];
	}
	return res;
}

}

#ifndef MULTIVARIATE_CONVOLUTION_HPP
#define MULTIVARIATE_CONVOLUTION_HPP 1
using light_convolution::multivariate_convolution;
#endif

#endif
