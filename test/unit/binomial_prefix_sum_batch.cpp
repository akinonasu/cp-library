#include "math/binomial_prefix_sum.hpp"
#include "math/binomial_prefix_sum_light.hpp"
#include "test/fixtures/binomial_prefix_sum_reference.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

template<unsigned P> struct BatchField {
	unsigned v = 0;
	static constexpr unsigned mod() { return P; }
	BatchField() = default;
	BatchField(long long x) { auto r = x % P; v = r < 0 ? r + P : r; }
	unsigned val() const { return v; }
	BatchField& operator+=(BatchField x) { v += x.v; if (v >= P) v -= P; return *this; }
	BatchField& operator-=(BatchField x) { v += P - x.v; if (v >= P) v -= P; return *this; }
	BatchField& operator*=(BatchField x) { v = std::uint64_t(v) * x.v % P; return *this; }
	BatchField operator+(BatchField x) const { return BatchField(*this) += x; }
	BatchField operator-(BatchField x) const { return BatchField(*this) -= x; }
	BatchField operator*(BatchField x) const { return BatchField(*this) *= x; }
	BatchField inv() const {
		assert(v);
		BatchField a = *this, result(1);
		for (unsigned e = P - 2; e; e >>= 1, a *= a) if (e & 1) result *= a;
		return result;
	}
};

template<class Mint, class K>
void check(const std::vector<std::pair<int, K>>& queries, const std::vector<Mint>& expected, int block = 0) {
	const auto before = queries;
	const auto full = binomial_prefix_sum<Mint>(queries, block);
	const auto light = light_combinatorics::binomial_prefix_sum<Mint>(queries, block);
	assert(queries == before);
	assert(full.size() == expected.size() && light.size() == expected.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		assert(full[i].val() == expected[i].val());
		assert(light[i].val() == expected[i].val());
	}
}

template<class Mint, class K>
void pascal_check(const std::vector<std::pair<int, K>>& queries,
	const std::vector<std::vector<Mint>>& pascal, int block = 0) {
	std::vector<Mint> expected;
	for (auto [n, k] : queries)
		expected.push_back(pascal[n][std::clamp(std::int64_t(k), std::int64_t(0), std::int64_t(n) + 1)]);
	check(queries, expected, block);
}

template<class Mint> void small_cases(int limit) {
	using K = std::int64_t;
	auto pascal = binomial_prefix_sum_pascal<Mint>(limit);
	std::vector<std::pair<int, K>> all;
	for (int n = 0; n <= limit; ++n) {
		for (int k = -2; k <= n + 3; ++k) all.emplace_back(n, k);
		all.emplace_back(n, std::numeric_limits<K>::min());
		all.emplace_back(n, std::numeric_limits<K>::max());
	}
	std::mt19937_64 rng(20261028);
	std::shuffle(all.begin(), all.end(), rng);
	for (int block : {0, 1, 2, 3, 17, 1000000000, std::numeric_limits<int>::max()})
		pascal_check(all, pascal, block);
	check<Mint>(std::vector<std::pair<int, int>>{}, {});
	for (int n : {0, 1, limit / 2, limit}) {
		if (n > limit) continue;
		std::vector<std::pair<int, int>> same;
		for (int k = -2; k <= n + 3; ++k) same.emplace_back(n, k);
		const auto repeated = same;
		same.insert(same.end(), repeated.begin(), repeated.end());
		std::shuffle(same.begin(), same.end(), rng);
		for (int block : {0, 1, 2, 7, std::numeric_limits<int>::max()}) pascal_check(same, pascal, block);
	}
	for (int trial = 0; trial < 150; ++trial) {
		std::vector<std::pair<int, K>> queries;
		int same_n = rng() % (limit + 1);
		for (int i = rng() % 100; i--;) {
			int n = trial % 2 ? same_n : rng() % (limit + 1);
			K k = int(rng() % (n + 7)) - 3;
			if (i % 7 == 0) n = rng() % (limit + 1), k = i % 14 ? 1 : n + 1;
			if (i % 13 == 0) k = std::numeric_limits<K>::min();
			if (i % 17 == 0) k = std::numeric_limits<K>::max();
			queries.emplace_back(n, k);
		}
		pascal_check(queries, pascal, trial % 5 == 0 ? 1 + rng() % 30 : 0);
	}
}

int main() {
	using Queries = std::vector<std::pair<int, int>>;
	static_assert(std::is_same_v<decltype(binomial_prefix_sum(Queries{})), std::vector<mint>>);
	static_assert(std::is_same_v<decltype(light_combinatorics::binomial_prefix_sum(Queries{})), std::vector<mint>>);
	small_cases<mint>(160);
	small_cases<BatchField<2>>(1);
	small_cases<BatchField<3>>(2);
	small_cases<BatchField<17>>(16);
	small_cases<BatchField<1000000007>>(100);
	auto pascal = binomial_prefix_sum_pascal<mint>(400);
	std::vector<std::pair<int, std::int64_t>> pruned{
		{400, 0}, {257, 5}, {399, 200}, {0, 0}, {257, 250}, {400, 1},
		{400, 400}, {257, 2}, {0, 1}, {257, 250}, {400, 401}, {257, 129}
	};
	pascal_check(pruned, pascal);
	for (std::int64_t k : {1LL, 2LL, 50LL, 128LL, 200LL, 256LL})
		pascal_check(std::vector<std::pair<int, std::int64_t>>(300, {257, k}), pascal);
	pascal_check(std::vector<std::pair<int, std::int8_t>>{{0, -128}, {0, 127}, {11, 5}, {11, 7}, {17, 8}}, pascal);
	pascal_check(std::vector<std::pair<int, std::int16_t>>{{1, -32768}, {1, 32767}, {399, 200}, {400, 199}}, pascal);
	for (int trivial : {32, 2048}) {
		Queries sparse(trivial, {400, 0});
		for (int i = 0; i < 32; ++i) sparse.emplace_back(300 + i, 5 + i % 20);
		pascal_check(sparse, pascal);
	}
	std::mt19937_64 rng(983562);
	std::vector<std::pair<int, std::int64_t>> large;
	for (int i = 0; i < 5000; ++i) {
		int n = rng() % 10001;
		std::int64_t k = int(rng() % (n + 21)) - 10;
		if (i % 11 == 0) n = 10000, k = 777;
		if (i % 17 == 0) k = std::numeric_limits<std::int64_t>::min();
		if (i % 19 == 0) k = std::numeric_limits<std::int64_t>::max();
		large.emplace_back(n, k);
	}
	const auto expected = binomial_prefix_sum_baseline(large);
	for (int block : {0, 13, 79, 1000000000}) check(large, expected, block);
	std::vector<std::pair<int, int>> cluster;
	for (int i = 0; i < 1000; ++i) cluster.emplace_back(10000, 2000 + rng() % 200);
	const auto cluster_expected = binomial_prefix_sum_baseline(cluster);
	check(cluster, cluster_expected);
	check(cluster, cluster_expected, std::numeric_limits<int>::max());
}
