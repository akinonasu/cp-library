#ifndef CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_LIGHT_HPP
#define CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_LIGHT_HPP

#include "binomial_prefix_core_light.hpp"
#include "binomial_prefix_polynomial_base.hpp"
#include "fps_light_core.hpp"

namespace light_binomial_prefix_detail {
template<class Mint> using PolynomialScalarArithmetic = binomial_prefix_detail::PolynomialScalarArithmetic<Mint>;
template<class Mint> struct PolynomialArithmetic : PolynomialScalarArithmetic<Mint> {};
template<> struct PolynomialArithmetic<mint> : binomial_prefix_detail::PolynomialSeriesArithmetic<mint, light_fps> {};

template<class Mint> class Polynomial : public binomial_prefix_detail::PolynomialEvaluator<Mint, PolynomialArithmetic<Mint>, Table<Mint>> {
	using Base = binomial_prefix_detail::PolynomialEvaluator<Mint, PolynomialArithmetic<Mint>, Table<Mint>>;

  public:
	Polynomial(const Table<Mint>& table, const std::vector<std::pair<int, int>>& points,
			   const std::vector<std::uint32_t>& ids, std::vector<Mint>& result) : Base(table, points, ids, result) {}
};
}

#endif
