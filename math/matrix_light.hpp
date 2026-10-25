#ifndef MATRIX_LIGHT_HPP
#define MATRIX_LIGHT_HPP

#include "../components/matrix_core.hpp"

namespace matrix_detail {
template <class T>
struct PlainMatrixArithmetic {
	static constexpr bool modular = false;
	template <class F>
	static auto field(std::vector<T> &a, F &&f) {
		static_assert(!integer<T>);
		return f.template operator()<Field<T>>(a);
	}
	static std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
		return basic_multiply(a, b, n, inner, m);
	}
	static std::optional<std::vector<T>> power(const std::vector<T> &, int, std::uint64_t) { return std::nullopt; }
};
}

template <class T>
using LightMatrix = Matrix<T, matrix_detail::PlainMatrixArithmetic<T>>;

#endif
