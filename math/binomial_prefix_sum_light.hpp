#ifndef BINOMIAL_PREFIX_SUM_LIGHT_HPP
#define BINOMIAL_PREFIX_SUM_LIGHT_HPP
#include "../components/binomial_prefix_core_light.hpp"
#include "../components/binomial_prefix_polynomial_light.hpp"
#include "../components/binomial_prefix_online.hpp"

namespace light_combinatorics {
template<class Mint = mint> class BinomialPrefixSum : public binomial_prefix_detail::Online<Mint, light_binomial_prefix_detail::Table<Mint>> {
	using Base = binomial_prefix_detail::Online<Mint, light_binomial_prefix_detail::Table<Mint>>;

  public:
	explicit BinomialPrefixSum(int max_n = 0, int block_size = 0) : Base(max_n, block_size) {}
};

template<class Mint = mint, class K = int>
std::vector<Mint> binomial_prefix_sum(const std::vector<std::pair<int, K>>& queries) {
	static_assert(std::is_integral_v<K> && std::is_signed_v<K> && sizeof(K) <= 8);
	assert(queries.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (queries.empty()) return {};
	int max_n = 0;
	for (auto [n, k] : queries) { (void)k; assert(n >= 0); max_n = std::max(max_n, n); }
	light_binomial_prefix_detail::Table<Mint> table(max_n);
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
	if (!points.empty()) light_binomial_prefix_detail::Polynomial<Mint>(table, points, ids, result);
	return result;
}

}

#endif
