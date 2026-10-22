#ifndef CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_HPP
#define CP_COMPONENTS_BINOMIAL_PREFIX_POLYNOMIAL_HPP

#include "binomial_prefix_core.hpp"
#include "binomial_prefix_polynomial_base.hpp"
#include "../fps/fps.hpp"

namespace binomial_prefix_detail {
template<class Mint> struct PolynomialArithmetic : PolynomialScalarArithmetic<Mint> {};
template<> struct PolynomialArithmetic<mint> : binomial_prefix_detail::PolynomialSeriesArithmetic<mint, fps> {};

template<class Mint> class Polynomial : public binomial_prefix_detail::PolynomialEvaluator<Mint, PolynomialArithmetic<Mint>, Table<Mint>> {
	using Base = binomial_prefix_detail::PolynomialEvaluator<Mint, PolynomialArithmetic<Mint>, Table<Mint>>;

  public:
	Polynomial(const Table<Mint>& table, const std::vector<std::pair<int, int>>& points,
			   const std::vector<std::uint32_t>& ids, std::vector<Mint>& result) : Base(table, points, ids, result) {}
};
}

#endif
