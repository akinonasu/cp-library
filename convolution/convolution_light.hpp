#ifndef CONVOLUTION_LIGHT_HPP
#define CONVOLUTION_LIGHT_HPP

#include "../fps/fps_light.hpp"
#include "convolution_fft.hpp"
#include "convolution_mod.hpp"
#include "convolution_ring.hpp"
#include <complex>
#include <limits>
#include <type_traits>
#include <vector>

namespace light_convolution {
namespace detail {
template<class T> struct IsComplex : std::false_type {};
template<class T> struct IsComplex<std::complex<T>> : std::true_type {};
}

template<class T>
std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			if constexpr (std::is_same_v<T, mint>) return fps_light_detail::multiply(a, b);
			light_fps x(a.size()), y;
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = mint::raw(a[i].val());
			if (&a != &b) {
				y.resize(b.size());
				for (std::size_t i = 0; i < b.size(); ++i) y[i] = mint::raw(b[i].val());
			}
			auto values = &a == &b ? light_fps::multiply(x, x) : light_fps::multiply(x, y);
			std::vector<T> result;
			result.reserve(values.size());
			for (auto value : values) result.push_back(T::raw(value.val()));
			return result;
		} else {
			std::vector<std::uint32_t> x(a.size()), y(b.size());
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
			for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
			auto values = convolution_mod(x, y, T::mod());
			std::vector<T> result;
			result.reserve(values.size());
			for (auto value : values) result.push_back(T::raw(value));
			return result;
		}
	} else if constexpr (std::numeric_limits<T>::is_integer && sizeof(T) <= 8) return convolution_integer(a, b);
	else if constexpr (std::is_floating_point_v<T> || detail::IsComplex<T>::value) return convolution_fft(a, b);
	else return convolution_ring(a, b);
}
}

#endif
