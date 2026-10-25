#ifndef BINOMIAL_PREFIX_SUM_INCREMENTAL_LIGHT_HPP
#define BINOMIAL_PREFIX_SUM_INCREMENTAL_LIGHT_HPP

#include "../components/binomial_prefix_core_light.hpp"
#include "../components/binomial_prefix_incremental_core.hpp"

namespace light_combinatorics {
template<class Mint = mint> class IncrementalBinomialPrefixSum : public binomial_prefix_detail::IncrementalPrefixSum<Mint, light_binomial_prefix_detail::Table<Mint>, false> {
	using Base = binomial_prefix_detail::IncrementalPrefixSum<Mint, light_binomial_prefix_detail::Table<Mint>, false>;

  public:
	explicit IncrementalBinomialPrefixSum(int max_n = 0) : Base(max_n) {}
};
}

#endif
