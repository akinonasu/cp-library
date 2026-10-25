#include "math/binomial_prefix_sum_incremental.hpp"
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
	Field inv() const {
		assert(v);
		Field a = *this, result(1);
		for (unsigned e = P - 2; e; e >>= 1, a *= a) if (e & 1) result *= a;
		return result;
	}
};

template<class Mint> void check(int limit) {
	std::vector<std::vector<Mint>> prefix(limit + 1);
	std::vector<Mint> row(limit + 1);
	row[0] = Mint(1);
	for (int n = 0; n <= limit; ++n) {
		prefix[n].resize(n + 2);
		for (int k = 0; k <= n; ++k) prefix[n][k + 1] = prefix[n][k] + row[k];
		for (int k = n + 1; k > 0 && k <= limit; --k) row[k] += row[k - 1];
	}
	IncrementalBinomialPrefixSum<Mint> sum(limit), other(limit);
	for (int n = 0; n <= limit; ++n)
		for (int k = 0; k <= n; ++k) assert(sum.binom(n, k) == prefix[n][k + 1] - prefix[n][k]);
	assert(sum.binom(-1, 0) == Mint(0) && sum.binom(0, -1) == Mint(0) && sum.binom(0, 1) == Mint(0));
	auto query = [&](int n, std::int64_t k) {
		Mint expected = prefix[n][std::clamp(k, std::int64_t(0), std::int64_t(n) + 1)];
		assert(sum.query(n, k) == expected);
		assert(sum.query(n, k) == expected);
	};
	for (int a = 0; a <= std::min(limit, 12); ++a)
		for (int b = -1; b <= a + 2; ++b)
			for (int n = 0; n <= std::min(limit, 12); ++n)
				for (int k = -1; k <= n + 2; ++k) query(a, b), query(n, k);
	std::mt19937_64 rng(20261020);
	for (int i = 0; i < 12000; ++i) {
		int n = rng() % (limit + 1);
		std::int64_t k = int(rng() % (limit + 21)) - 10;
		if (i % 19 == 0) k = std::numeric_limits<std::int64_t>::min();
		if (i % 19 == 1) k = std::numeric_limits<std::int64_t>::max();
		query(n, k);
		assert(other.query(limit, i % (limit + 2)) == prefix[limit][i % (limit + 2)]);
	}
	for (int n = limit; n >= 0; --n) query(n, 2), query(n, n / 2), query(n, n - 1);
	auto copied = sum;
	assert(copied.query(limit, limit / 2) == prefix[limit][limit / 2]);
	query(limit / 2, limit / 3);
	sum.build(0);
	assert(copied.max_n() == limit);
	assert(copied.query(limit, limit / 3) == prefix[limit][limit / 3]);
	other = copied;
	copied.build(0);
	assert(other.query(limit, limit / 2) == prefix[limit][limit / 2]);
	assert(sum.max_n() == 0);
	assert(sum.query(0, 0) == Mint(0));
	assert(sum.query(0, 1) == Mint(1));
	sum.build(limit);
	assert(sum.max_n() == limit);
	query(limit, limit / 2);
}

int main() {
	check<mint>(320);
	check<Field<2>>(1);
	check<Field<3>>(2);
	check<Field<17>>(16);
	check<Field<101>>(100);
	check<Field<1000000007>>(320);
	IncrementalBinomialPrefixSum<> empty;
	assert(empty.query(0, 0) == mint(0));
	assert(empty.query(0, 1) == mint(1));
	constexpr int n = 500000, k = n / 2;
	IncrementalBinomialPrefixSum<> large(n);
	mint numerator(1), denominator(1);
	for (int i = 1; i <= k; ++i) numerator *= mint(k + i), denominator *= mint(i);
	mint middle = numerator / denominator, power = mint(2).pow(n);
	assert(large.query(n, k) == (power - middle) / mint(2));
	assert(large.query(n, k + 1) == (power + middle) / mint(2));
	assert(large.query(n - 1, k) == power / mint(4));
	assert(large.query(n, n) == power - mint(1));
	assert(large.query(n, n - 1) == power - mint(n) - mint(1));
	assert(large.query(3, 2) == mint(4));
}
