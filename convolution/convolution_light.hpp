#ifndef CONVOLUTION_LIGHT_HPP
#define CONVOLUTION_LIGHT_HPP

#include "../components/fps_light_ntt.hpp"
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
		if constexpr (std::is_same_v<T, mint>) return fps_light_detail::multiply(a, b);
		using M = std::conditional_t<T::mod() == 998244353, mint, std::uint32_t>;
		auto convert = [](const std::vector<T> &input) {
			std::vector<M> result;
			result.reserve(input.size());
			for (auto x : input) {
				if constexpr (std::is_same_v<M, mint>) result.push_back(mint::raw(x.val()));
				else result.push_back(x.val());
			}
			return result;
		};
		auto x = convert(a), y = &a == &b ? std::vector<M>{} : convert(b);
		const auto &other = &a == &b ? x : y;
		auto values = [&] {
			if constexpr (std::is_same_v<M, mint>) return fps_light_detail::multiply(x, other);
			else return convolution_mod(x, other, T::mod());
		}();
		std::vector<T> result;
		result.reserve(values.size());
		for (auto value : values) {
			if constexpr (std::is_same_v<M, mint>) result.push_back(T::raw(value.val()));
			else result.push_back(T::raw(value));
		}
		return result;
	} else if constexpr (std::numeric_limits<T>::is_integer && sizeof(T) <= 8) return convolution_integer(a, b);
	else if constexpr (std::is_floating_point_v<T> || detail::IsComplex<T>::value) return convolution_fft(a, b);
	else return convolution_ring(a, b);
}
}

#endif
