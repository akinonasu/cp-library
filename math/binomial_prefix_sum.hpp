#ifndef BINOMIAL_PREFIX_SUM_HPP
#define BINOMIAL_PREFIX_SUM_HPP
#include "../components/binomial_prefix_core.hpp"
#include "../components/binomial_prefix_online.hpp"

template<class Mint = mint> class BinomialPrefixSum : public binomial_prefix_detail::Online<Mint, binomial_prefix_detail::Table<Mint>> {
	using Base = binomial_prefix_detail::Online<Mint, binomial_prefix_detail::Table<Mint>>;

  public:
	explicit BinomialPrefixSum(int max_n = 0, int block_size = 0) : Base(max_n, block_size) {}
};

#endif
