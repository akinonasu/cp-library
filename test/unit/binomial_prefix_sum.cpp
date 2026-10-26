#include "math/binomial_prefix_sum.hpp"
#include "test/fixtures/binomial_prefix_sum_reference.hpp"
#include <random>

template<unsigned P, bool Comparable = true> struct Field {
	unsigned v = 0;
	static constexpr unsigned mod() { return P; }
	Field() = default;
	Field(long long x) { long long r = x % P; v = r < 0 ? r + P : r; }
	unsigned val() const { return v; }
	Field& operator+=(Field x) { v += x.v; if (v >= P) v -= P; return *this; }
	Field& operator-=(Field x) { v += P - x.v; if (v >= P) v -= P; return *this; }
	Field& operator*=(Field x) { v = std::uint64_t(v) * x.v % P; return *this; }
	Field operator+(Field x) const { return Field(*this) += x; }
	Field operator-(Field x) const { return Field(*this) -= x; }
	Field operator*(Field x) const { return Field(*this) *= x; }
	bool operator==(Field x) const requires Comparable { return v == x.v; }
	Field inv() {
		assert(v);
		Field a = *this, result(1);
		for (unsigned e = P - 2; e; e >>= 1, a *= a) if (e & 1) result *= a;
		return result;
	}
};

template<class Mint> void exhaustive(int limit) {
	auto pascal = binomial_prefix_sum_pascal<Mint>(limit);
	std::vector<std::pair<int, std::int64_t>> queries;
	std::vector<Mint> expected;
	for (int n = 0; n <= limit; ++n)
		for (int k = -2; k <= n + 3; ++k) queries.emplace_back(n, k), expected.push_back(pascal[n][std::clamp(k, 0, n + 1)]);
	for (int block : {0, 1, 2, 3, 7, 31, 1000000000, std::numeric_limits<int>::max()}) {
		BinomialPrefixSum<Mint> online(limit, block);
		assert(online.max_n() == limit && online.block_size() >= 1);
		for (std::size_t i = 0; i < queries.size(); ++i) assert(online.query(queries[i].first, queries[i].second) == expected[i]);
		for (int n = 0; n <= limit; ++n)
			for (int l = -1; l <= n + 1; ++l) {
				int r = (l + n + 3) / 2;
				assert(online.prod(n, l, r) == pascal[n][std::clamp(r, 0, n + 1)] - pascal[n][std::clamp(l, 0, n + 1)]);
			}
		online.build(0);
		assert(online.max_n() == 0 && online.table_size() == 0 && online.query(0, 1) == Mint(1));
	}
}

template<class Mint> void rebuild_cases(int limit) {
	auto pascal = binomial_prefix_sum_pascal<Mint>(limit);
	BinomialPrefixSum<Mint> online;
	std::mt19937_64 rng(20261026);
	auto expected = [&](int n, std::int64_t k) { return pascal[n][std::clamp(k, std::int64_t(0), std::int64_t(n) + 1)]; };
	for (int block : {0, 1, 2, 3, 7, 31, std::numeric_limits<int>::max()})
		for (int size : {0, 1, limit / 2, limit, 0, limit / 3, limit}) {
			online.build(size, block);
			const auto copied = online;
			assert(online.max_n() == size);
			for (int i = 0; i < 257; ++i) {
				int n = rng() % (size + 1);
				std::int64_t l = int(rng() % (size + 7)) - 3, r = int(rng() % (size + 7)) - 3;
				if (i % 17 == 0) l = std::numeric_limits<std::int64_t>::min();
				if (i % 17 == 1) r = std::numeric_limits<std::int64_t>::max();
				if (l > r) std::swap(l, r);
				assert(online.query(n, r).val() == expected(n, r).val());
				assert(online.prod(n, l, r).val() == (expected(n, r) - expected(n, l)).val());
				assert(copied.query(n, l).val() == expected(n, l).val());
			}
			online.build(0);
			assert(copied.max_n() == size);
			assert(copied.query(size, size / 2).val() == expected(size, size / 2).val());
			assert(online.query(0, 0).val() == Mint(0).val() && online.query(0, 1).val() == Mint(1).val());
		}
}

int main() {
	rebuild_cases<mint>(127);
	rebuild_cases<Field<2, false>>(1);
	rebuild_cases<Field<17, false>>(16);
	rebuild_cases<Field<1000000007, false>>(127);
	exhaustive<mint>(320);
	exhaustive<Field<2>>(1);
	exhaustive<Field<3>>(2);
	exhaustive<Field<17>>(16);
	exhaustive<Field<101>>(100);
	exhaustive<Field<1000000007>>(130);
	BinomialPrefixSum<> empty;
	assert(empty.query(0, 0) == mint(0) && empty.query(0, 1) == mint(1));
	constexpr int n = 50000;
	BinomialPrefixSum<> online(n);
	std::mt19937_64 rng(731);
	std::vector<std::pair<int, std::int64_t>> queries;
	for (int i = 0; i < 24000; ++i) {
		int row = rng() % (n + 1);
		std::int64_t k = int(rng() % (n + 21)) - 10;
		if (i % 17 == 0) k = std::numeric_limits<std::int64_t>::min();
		if (i % 17 == 1) k = std::numeric_limits<std::int64_t>::max();
		if (i % 17 == 2) row = n, k = 1000;
		queries.emplace_back(row, k);
	}
	auto answers = binomial_prefix_sum_baseline(queries);
	for (std::size_t i = 0; i < queries.size(); ++i) assert(answers[i] == online.query(queries[i].first, queries[i].second));
	std::vector<std::pair<int, int>> ranges;
	for (int i = 0; i < 3000; ++i) {
		int l = rng() % 10000, r = rng() % 10000;
		if (l > r) std::swap(l, r);
		ranges.emplace_back(l, r);
	}
	for (int block : {0, 1, 37, 1000000000}) {
		auto order = mo_order(ranges, block);
		std::vector<int> count(ranges.size());
		int l = 0, r = 0;
		long long sum = 0;
		for (int i : order) {
			assert(0 <= i && i < int(ranges.size()) && !count[i]++);
			while (r < ranges[i].second) sum += r++;
			while (l > ranges[i].first) sum += --l;
			while (r > ranges[i].second) sum -= --r;
			while (l < ranges[i].first) sum -= l++;
			assert(sum == 1LL * (l + r - 1) * (r - l) / 2);
		}
		assert(order.size() == ranges.size());
	}
	assert(mo_order({}).empty());
	auto boundary = mo_order({{0, 0}, {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}});
	assert(boundary.size() == 2);
}
