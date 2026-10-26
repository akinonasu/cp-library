#define COMB_MAX 200
#include "math/comb_light.hpp"
#include "math/binomial_prefix_sum_light.hpp"
#include "math/binomial_prefix_sum_incremental_light.hpp"
#include "math/comb.hpp"
#include "math/binomial_prefix_sum.hpp"
#include "math/binomial_prefix_sum_incremental.hpp"
#include "test/fixtures/comb_modint_types.hpp"
#include <random>

template<unsigned P> struct Field {
	unsigned v = 0;
	static constexpr unsigned mod() { return P; }
	Field() = default;
	Field(long long x) { long long r = x % P; v = r < 0 ? r + P : r; }
	Field& operator+=(Field x) { v += x.v; if (v >= P) v -= P; return *this; }
	Field& operator-=(Field x) { v += P - x.v; if (v >= P) v -= P; return *this; }
	Field& operator*=(Field x) { v = std::uint64_t(v) * x.v % P; return *this; }
	Field operator+(Field x) const { return Field(*this) += x; }
	Field operator-(Field x) const { return Field(*this) -= x; }
	Field operator*(Field x) const { return Field(*this) *= x; }
	bool operator==(Field x) const { return v == x.v; }
	Field inv() {
		assert(v);
		Field a = *this, result(1);
		for (unsigned e = P - 2; e; e >>= 1, a *= a) if (e & 1) result *= a;
		return result;
	}
};

const unsigned before_main = light_combinatorics::comb(10, 3).val();
const unsigned before_main_factorial = light_combinatorics::fact[5].val();

template<class M> void prefix_cases(int limit) {
	std::vector<std::vector<M>> expected(limit + 1);
	std::vector<M> row(limit + 1);
	row[0] = M(1);
	std::vector<std::pair<int, std::int64_t>> queries;
	std::vector<M> answers;
	for (int n = 0; n <= limit; ++n) {
		expected[n].resize(n + 2);
		for (int k = 0; k <= n; ++k) expected[n][k + 1] = expected[n][k] + row[k];
		for (int k = -2; k <= n + 3; ++k) {
			queries.emplace_back(n, k);
			answers.push_back(expected[n][std::clamp(k, 0, n + 1)]);
		}
		for (int k = n + 1; k > 0 && k <= limit; --k) row[k] += row[k - 1];
	}
	for (int block : {0, 1, 7, 1000000000}) {
		light_combinatorics::BinomialPrefixSum<M> sum(limit, block);
		assert(sum.max_n() == limit && sum.block_size() > 0);
		for (std::size_t i = 0; i < queries.size(); ++i) {
			auto [n, k] = queries[i];
			assert(sum.query(n, k) == answers[i]);
			assert(sum.prod(n, 0, k + 4) == expected[n][std::clamp<std::int64_t>(k + 4, 0, n + 1)]);
		}
		sum.build(0);
		assert(sum.max_n() == 0 && sum.table_size() == 0 && sum.query(0, 1) == M(1));
	}
	light_combinatorics::IncrementalBinomialPrefixSum<M> sum(limit);
	std::mt19937_64 random(293385);
	for (int i = 0; i < 20000; ++i) {
		int n = random() % (limit + 1);
		std::int64_t k = int(random() % (limit + 21)) - 10;
		if (i % 19 == 0) k = std::numeric_limits<std::int64_t>::min();
		if (i % 19 == 1) k = std::numeric_limits<std::int64_t>::max();
		assert(sum.query(n, k) == expected[n][std::clamp<std::int64_t>(k, 0, n + 1)]);
		assert(sum.query(n, k) == expected[n][std::clamp<std::int64_t>(k, 0, n + 1)]);
	}
	assert(sum.binom(-1, 0) == M(0) && sum.binom(0, -1) == M(0) && sum.binom(0, 1) == M(0));
	for (int n = 0; n <= limit; ++n)
		for (int k = 0; k <= n; ++k) assert(sum.binom(n, k) == expected[n][k + 1] - expected[n][k]);
	auto copy = sum;
	sum.build(0);
	assert(copy.max_n() == limit && sum.max_n() == 0);
	assert(copy.query(limit, limit / 2) == expected[limit][limit / 2]);
}

void comb_cases() {
	assert(before_main == 120 && before_main_factorial == 120);
	for (int n = 0; n <= COMB_MAX; ++n) {
		assert(light_combinatorics::fact[n] == fact[n]);
		assert(light_combinatorics::finv[n] == finv[n]);
		for (int k = -1; k <= n + 1; ++k) assert(light_combinatorics::comb(n, k) == comb(n, k));
	}
	assert(light_combinatorics::fact[-1] == mint(0) && light_combinatorics::finv[-1] == mint(0));
	using M = comb_test::Field<1000000007>;
	using C = light_combinatorics::Comb<M>;
	assert(M::inversions == 0);
	assert(C::fact[-1].val() == 0 && C::finv[-1].val() == 0 && C::comb(-1, 0).val() == 0);
	assert(M::inversions == 0);
	assert(C::comb(10, 3).val() == 120 && M::inversions == 1);
	assert(C::fact[5].val() == 120 && (C::fact[10] * C::finv[10]).val() == 1 && M::inversions == 1);
	using D = comb_test::DynamicField<31>;
	D::set_mod(17);
	assert(light_combinatorics::comb<D>(16, 8).val() == 1 && D::inversions == 1);
	D::set_mod(101);
	assert(light_combinatorics::comb<D>(10, 3).val() == 19 && D::inversions == 2);
	assert(light_combinatorics::Comb<D>::fact[5].val() == 19 && D::inversions == 2);
}

int main() {
	IncrementalBinomialPrefixSum incremental(10);
	light_combinatorics::IncrementalBinomialPrefixSum light_incremental(10);
	assert(incremental.query(5, 3) == mint(16));
	assert(light_incremental.query(5, 3) == mint(16));
	mint (IncrementalBinomialPrefixSum<>::*query)(int, std::int64_t) = &IncrementalBinomialPrefixSum<>::query;
	mint (light_combinatorics::IncrementalBinomialPrefixSum<>::*light_query)(int, std::int64_t) = &light_combinatorics::IncrementalBinomialPrefixSum<>::query;
	assert((incremental.*query)(6, 3) == mint(22));
	assert((light_incremental.*light_query)(6, 3) == mint(22));
	comb_cases();
	prefix_cases<mint>(120);
	prefix_cases<Field<2>>(1);
	prefix_cases<Field<3>>(2);
	prefix_cases<Field<17>>(16);
	prefix_cases<Field<101>>(100);
	prefix_cases<Field<1000000007>>(120);
	std::mt19937_64 random(20261022);
	std::vector<std::pair<int, std::int64_t>> queries;
	for (int i = 0; i < 8192; ++i) {
		int n = random() % 20001;
		queries.emplace_back(n, int(random() % 20021) - 10);
	}
	for (int i = 0; i < 100; ++i) queries.push_back(queries[i]);
	light_combinatorics::BinomialPrefixSum<> a(20000);
	BinomialPrefixSum<> b(20000);
	light_combinatorics::IncrementalBinomialPrefixSum<> c(20000);
	IncrementalBinomialPrefixSum<> d(20000);
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [n, k] = queries[i];
		auto expected = b.query(n, k);
		assert(expected == a.query(n, k));
		assert(expected == c.query(n, k) && expected == d.query(n, k));
	}
}
