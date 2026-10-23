#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "../../components/matrix_arithmetic.hpp"
#include "../../components/matrix_power.hpp"
#include "../../components/matrix_elimination_modular.hpp"

namespace matrix_detail {
template <class T>
struct FastMatrixArithmetic {
	static constexpr bool modular = small_modint<T>;
	template <class Ops>
	static auto eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
		if constexpr (modular) if (h >= 32 && columns >= 32) return ModularElimination<Ops>::run(a, h, stride, columns, reduced);
		return matrix_detail::eliminate<Ops, modular>(a, h, stride, columns, reduced, eps);
	}
	template <class F>
	static auto field(std::vector<T> &a, F &&f) { return matrix_detail::field(a, std::forward<F>(f)); }
	static std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
		return matrix_detail::multiply(a, b, n, inner, m);
	}
	static std::optional<std::vector<T>> characteristic(const std::vector<T> &a, int n) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 32) return FrobeniusPower<T>(a, n).characteristic();
		}
		return std::nullopt;
	}
	static std::optional<std::vector<T>> power(const std::vector<T> &a, int n, std::uint64_t exponent) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 16 && exponent >= 256) return FrobeniusPower<T>(a, n).run(exponent);
		}
		return std::nullopt;
	}
};
}

#include "../../components/matrix_core.hpp"

#endif
