#ifndef BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP
#define BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP

#include "../../components/binomial_prefix_core.hpp"
#include <algorithm>

template<class Mint = mint> class IncrementalBinomialPrefixSum {
	binomial_prefix_detail::Table<Mint> table_;
	int n_ = 0, k_ = 0;
	Mint value_ = Mint(0);

  public:
	explicit IncrementalBinomialPrefixSum(int max_n = 0) : table_(max_n) {}
	void build(int max_n) {
		table_.build(max_n);
		n_ = k_ = 0;
		value_ = Mint(0);
	}
	int max_n() const { return int(table_.factorial.size()) - 1; }
	Mint query(int n, std::int64_t k) {
		assert(0 <= n && n <= max_n());
		int m = int(std::clamp(k, std::int64_t(0), std::int64_t(n) + 1));
		if (m <= 1 || m == n + 1) {
			n_ = n, k_ = m;
			return value_ = m == 0 ? Mint(0) : m == 1 ? Mint(1) : table_.power2[n];
		}
		if (!k_) n_ = n;
		while (n_ < n) value_ = value_ + value_ - table_.binom(n_++, k_ - 1);
		if (k_ < m) value_ += table_.range(n_, k_, m);
		else if (k_ > m) value_ -= table_.range(n_, m, k_);
		k_ = m;
		while (n_ > n) value_ = (value_ + table_.binom(--n_, k_ - 1)) * table_.half;
		return value_;
	}
};

#endif
