#include "math/binomial_prefix_sum.hpp"
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

template<class Mint> void exhaustive(int limit) {
	std::vector<std::vector<Mint>> pascal(limit + 1);
	for (int n = 0; n <= limit; ++n) {
		pascal[n].resize(n + 2);
		std::vector<Mint> row(n + 1, Mint(1));
		for (int k = 1; k < n; ++k) {
			Mint left = pascal[n - 1][k] - pascal[n - 1][k - 1];
			Mint right = pascal[n - 1][k + 1] - pascal[n - 1][k];
			row[k] = left + right;
		}
		for (int k = 0; k <= n; ++k) pascal[n][k + 1] = pascal[n][k] + row[k];
	}
	std::vector<std::pair<int, std::int64_t>> queries;
	std::vector<Mint> expected;
	for (int n = 0; n <= limit; ++n)
		for (int k = -2; k <= n + 3; ++k) queries.emplace_back(n, k), expected.push_back(pascal[n][std::clamp(k, 0, n + 1)]);
	assert(binomial_prefix_sum<Mint>(queries) == expected);
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

int main() {
	exhaustive<mint>(320);
	exhaustive<Field<2>>(1);
	exhaustive<Field<3>>(2);
	exhaustive<Field<17>>(16);
	exhaustive<Field<101>>(100);
	exhaustive<Field<1000000007>>(130);
	assert(binomial_prefix_sum(std::vector<std::pair<int, int>>{}).empty());
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
	auto answers = binomial_prefix_sum(queries);
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
