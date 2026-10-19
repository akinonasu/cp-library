#ifndef CP_BENCHMARK_BINOMIAL_PREFIX_SUM_COMPARISON_HPP
#define CP_BENCHMARK_BINOMIAL_PREFIX_SUM_COMPARISON_HPP

#include "math/combinatorics/binomial_prefix_sum.hpp"
#include "components/mo_order.hpp"

template<class Mint = mint, class K = int>
std::vector<Mint> binomial_prefix_sum_baseline(const std::vector<std::pair<int, K>>& queries) {
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
