#ifndef CONVOLUTION_DISPATCH_HPP
#define CONVOLUTION_DISPATCH_HPP

#include <complex>
#include <limits>
#include <type_traits>
#include <vector>

#include "mint_convolution.hpp"
#include "../convolution/convolution_fft.hpp"
#include "../convolution/convolution_mod.hpp"
#include "../convolution/convolution_ring.hpp"

namespace convolution_detail {
template <class T>
struct IsComplex : std::false_type {};
template <class T>
struct IsComplex<std::complex<T>> : std::true_type {};
}

template <class T>
std::vector<T> convolution(const std::vector<T> &a, const std::vector<T> &b) {
	if constexpr (requires { T::mod(); T::raw(0); a[0].val(); }) {
		if constexpr (T::mod() == 998244353) {
			if constexpr (std::is_same_v<T, mint>) return mint_convolution_detail::multiply(a, b);
			else {
				std::vector<mint> x(a.size()), y(&a == &b ? 0 : b.size());
				for (std::size_t i = 0; i < a.size(); ++i) x[i] = mint::raw(a[i].val());
				if (&a != &b) for (std::size_t i = 0; i < b.size(); ++i) y[i] = mint::raw(b[i].val());
				auto values = &a == &b ? mint_convolution_detail::multiply(x, x) : mint_convolution_detail::multiply(x, y);
				std::vector<T> result;
				result.reserve(values.size());
				for (auto x : values) result.push_back(T::raw(x.val()));
				return result;
			}
		} else {
			std::vector<std::uint32_t> x(a.size()), y(b.size());
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
			for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
			auto values = convolution_mod(x, y, T::mod());
			std::vector<T> result;
			result.reserve(values.size());
			for (auto x : values) result.push_back(T::raw(x));
			return result;
		}
	} else if constexpr (std::numeric_limits<T>::is_integer && sizeof(T) <= 8) return convolution_integer(a, b);
	else if constexpr (std::is_floating_point_v<T> || convolution_detail::IsComplex<T>::value) return convolution_fft(a, b);
	else return convolution_ring(a, b);
}

#endif
