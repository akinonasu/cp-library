#ifndef BINOMIAL_PREFIX_SUM_INCREMENTAL_LIGHT_HPP
#define BINOMIAL_PREFIX_SUM_INCREMENTAL_LIGHT_HPP

#include "../../components/binomial_prefix_core_light.hpp"
#include <algorithm>
#include <memory>

namespace light_combinatorics {
template<class Mint = mint> class IncrementalBinomialPrefixSum {
	using Table = light_binomial_prefix_detail::Table<Mint>;
	std::shared_ptr<const Table> table_;
	int n_ = 0, k_ = 0;
	Mint value_ = Mint(0);
	Mint halve(Mint value) const {
		return value * table_->half;
	}

  public:
	explicit IncrementalBinomialPrefixSum(int max_n = 0) : table_(std::make_shared<Table>(max_n)) {}
	void build(int max_n) {
		table_ = std::make_shared<Table>(max_n);
		n_ = k_ = 0;
		value_ = Mint(0);
	}
	int max_n() const { return int(table_->factorial.size()) - 1; }
	Mint binom(int n, int k) const {
		if (n < 0 || k < 0 || k > n) return Mint(0);
		assert(n <= max_n());
		return table_->binom(n, k);
	}
	[[gnu::always_inline]] Mint query(int n, std::int64_t k) {
		assert(0 <= n && n <= max_n());
		int m = int(std::clamp(k, std::int64_t(0), std::int64_t(n) + 1));
		if (m <= 1 || m == n + 1) {
			n_ = n, k_ = m;
			return value_ = m == 0 ? Mint(0) : m == 1 ? Mint(1) : table_->power2[n];
		}
		const Table& table = *table_;
		int row = n_, column = k_;
		Mint value = value_;
		if (column == m) {
			while (row < n) value = value + value - table.binom(row++, column - 1);
			while (row > n) value = halve(value + table.binom(--row, column - 1));
		} else {
			if (!column) row = n;
			while (row < n && column < m) value = value + value + table.binom(row++, column++);
			while (row > n && column > m) value = halve(value - table.binom(--row, --column));
			while (row < n) value = value + value - table.binom(row++, column - 1);
			if (column < m) value += column + 1 == m ? table.binom(row, column) : table.range(row, column, m);
			else if (column > m) value -= m + 1 == column ? table.binom(row, m) : table.range(row, m, column);
			while (row > n) value = halve(value + table.binom(--row, m - 1));
		}
		n_ = n, k_ = m;
		return value_ = value;
	}
};

}

#endif
