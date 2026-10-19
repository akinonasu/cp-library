#ifndef BINOMIAL_PREFIX_SUM_HPP
#define BINOMIAL_PREFIX_SUM_HPP

#include "../../components/binomial_prefix_core.hpp"
#include "../../components/mo_order.hpp"
#include <cstddef>

template<class Mint = mint> class BinomialPrefixSum {
	binomial_prefix_detail::Table<Mint> table_;
	int max_n_ = 0, block_ = 1;
	std::vector<std::size_t> offsets_;
	std::vector<Mint> data_;
	void check([[maybe_unused]] int n) const { assert(0 <= n && n <= max_n_); }

  public:
	explicit BinomialPrefixSum(int max_n = 0, int block_size = 0) { build(max_n, block_size); }
	void build(int max_n, int block_size = 0) {
		assert(max_n >= 0 && block_size >= 0);
		table_.build(max_n);
		max_n_ = max_n;
		block_ = block_size ? block_size : std::max(1, int(std::sqrt(double(max_n) + 1)));
		int blocks = ((max_n + 1) / 2) / block_;
		offsets_.resize(blocks + 1);
		std::size_t total = 0;
		for (int j = 1; j <= blocks; ++j) {
			offsets_[j] = total;
			total += max_n - 2 * (j * block_) + 2;
		}
		data_.resize(total);
		for (int j = 1; j <= blocks; ++j) {
			int m = j * block_, start = 2 * m - 1;
			Mint value = table_.factorial[m - 1] * table_.power2[start - 1];
			std::size_t offset = offsets_[j];
			data_[offset] = value;
			for (int n = start; n < max_n; ++n) {
				value = value + value - table_.factorial[n] * table_.inverse_factorial[n - m + 1];
				data_[++offset] = value;
			}
		}
	}
	int max_n() const { return max_n_; }
	int block_size() const { return block_; }
	std::size_t table_size() const { return data_.size(); }
	Mint query(int n, std::int64_t k) const {
		check(n);
		if (k <= 0) return Mint(0);
		if (k >= std::int64_t(n) + 1) return table_.power2[n];
		bool reflect = k > (n + 1) / 2;
		int m = reflect ? n + 1 - int(k) : int(k), center = (n + 1) / 2;
		Mint value(1);
		if (m != 1) {
			int j = m / block_, anchor = j * block_, distance = m - anchor;
			if (std::int64_t(anchor) + block_ <= center && block_ - distance < distance) ++j, anchor += block_, distance = anchor - m;
			if (center - m < distance) anchor = center, value = table_.center(n);
			else value = anchor ? data_[offsets_[j] + n - (2 * anchor - 1)] * table_.inverse_factorial[anchor - 1] : Mint(0);
			if (anchor < m) value += table_.range(n, anchor, m);
			else if (m < anchor) value -= table_.range(n, m, anchor);
		}
		return reflect ? table_.power2[n] - value : value;
	}
	Mint prod(int n, std::int64_t l, std::int64_t r) const {
		assert(l <= r);
		return query(n, r) - query(n, l);
	}
};

template<class Mint = mint, class K = int>
std::vector<Mint> binomial_prefix_sum(const std::vector<std::pair<int, K>>& queries) {
	static_assert(std::is_integral_v<K> && std::is_signed_v<K> && sizeof(K) <= 8);
	assert(queries.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (queries.empty()) return {};
	int max_n = 0;
	for (auto [n, k] : queries) { (void)k; assert(n >= 0); max_n = std::max(max_n, n); }
	binomial_prefix_detail::Table<Mint> table(max_n);
	std::vector<Mint> result(queries.size());
	std::vector<std::pair<int, int>> points;
	std::vector<std::uint32_t> ids;
	points.reserve(queries.size()), ids.reserve(queries.size());
	for (int i = 0; i < int(queries.size()); ++i) {
		auto [n, k] = queries[i];
		if (k <= 0) continue;
		if (std::uint64_t(k) >= std::uint64_t(n) + 1) { result[i] = table.power2[n]; continue; }
		bool reflect = k > (n + 1) / 2;
		int m = reflect ? n + 1 - int(k) : int(k);
		if (m == 1 || m == (n + 1) / 2) {
			Mint value = m == 1 ? Mint(1) : table.center(n);
			result[i] = reflect ? table.power2[n] - value : value;
		} else points.emplace_back(m, n), ids.push_back((std::uint32_t(i) << 1) | reflect);
	}
	auto order = mo_order(points);
	if (order.empty()) return result;
	int n = points[order[0]].second, m = points[order[0]].first;
	Mint value = table.range(n, 0, m);
	for (int i : order) {
		auto [mm, nn] = points[i];
		while (n < nn) value = value + value - table.binom(n++, m - 1);
		while (m < mm) value += table.binom(n, m++);
		while (m > mm) value -= table.binom(n, --m);
		while (n > nn) value = (value + table.binom(--n, m - 1)) * table.half;
		auto id = ids[i];
		result[id >> 1] = id & 1 ? table.power2[n] - value : value;
	}
	return result;
}

#endif
