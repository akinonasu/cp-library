#ifndef BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP
#define BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP

#include "../components/binomial_prefix_core.hpp"
#include "../components/binomial_prefix_incremental_core.hpp"

#ifndef CP_BINOMIAL_PREFIX_SUM_INCREMENTAL_API_HPP
#define CP_BINOMIAL_PREFIX_SUM_INCREMENTAL_API_HPP 1
template<class Mint = mint> class IncrementalBinomialPrefixSum : public binomial_prefix_detail::IncrementalPrefixSum<Mint, binomial_prefix_detail::Table<Mint>, std::is_same_v<Mint, mint>> {
	using Base = binomial_prefix_detail::IncrementalPrefixSum<Mint, binomial_prefix_detail::Table<Mint>, std::is_same_v<Mint, mint>>;

  public:
	explicit IncrementalBinomialPrefixSum(int max_n = 0) : Base(max_n) {}
};

#endif

#endif
