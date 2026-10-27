#include "components/fps_sparse.hpp"
#include "components/fps_sparse_scalar.hpp"
#include <cassert>
#include <random>
using Sparse = fps_detail::SparseSeries;
using K = Sparse::Kind;
const mint *reciprocals(int n) {
	static std::vector<mint> v{0, 1};
	for (int i = v.size(); i <= n; ++i) v.push_back(-v[mint::mod() % i] * (mint::mod() / i));
	return v.data();
}
template <K kind>
void check(const Sparse::Terms &terms, int n, mint initial, mint exponent) {
	std::vector<mint> expected(n);
	expected[0] = initial;
	const mint *inv = reciprocals(n);
	for (int i = 1; i < n; ++i) {
		mint sum = 0;
		for (auto [j, x] : terms) if (j <= i) {
			if constexpr (kind == K::power) sum += ((exponent + mint(1)) * mint(j) - mint(i)) * x * expected[i - j];
			if constexpr (kind == K::exponential) sum += mint(j) * x * expected[i - j];
			if constexpr (kind == K::inverse || kind == K::logarithm) sum -= x * expected[i - j];
			if constexpr (kind == K::logarithm) if (j == i) sum += mint(j) * x;
		}
		if constexpr (kind == K::power || kind == K::exponential) sum *= inv[i];
		expected[i] = sum;
	}
	if constexpr (kind == K::logarithm) for (int i = 1; i < n; ++i) expected[i] *= inv[i];
	assert(Sparse::get<kind>(terms, n, initial, exponent, reciprocals) == expected);
	using Scalar = fps_detail::ScalarSparseSeries;
	assert(Scalar::get<static_cast<Scalar::Kind>(kind)>(terms, n, initial, exponent, reciprocals) == expected);
}
void check_boundaries() {
	std::mt19937 gen(684513);
	auto verify = [&](const Sparse::Terms &terms, int n, mint exponent) {
		check<K::inverse>(terms, n, mint(7), 0);
		check<K::logarithm>(terms, n, 0, 0);
		check<K::exponential>(terms, n, 1, 0);
		check<K::power>(terms, n, mint(13), exponent);
	};
	for (unsigned mask = 1; mask < 128; ++mask) {
		Sparse::Terms terms;
		for (int d = 1; d <= 7; ++d) if (mask & (1u << (d - 1)))
			terms.emplace_back(d, mint::word(mint(gen()).v % mint::mod() + mint::mod()));
		for (int n : {255, 256, 257, 263, 264, 265, 777, 1025}) verify(terms, n, mint(gen()));
		verify(terms, 521, mint(-1));
		verify(terms, 522, mint(-2));
	}
	for (int count : {11, 12, 13, 23, 24, 25, 63, 64}) {
		Sparse::Terms terms;
		for (int d = 1; d <= count; ++d) terms.emplace_back(d, mint(gen()));
		for (int n : {257, 263, 521, 1025}) verify(terms, n, mint(gen()));
	}
	for (int gap : {7, 8, 9, 255, 256, 257, 1024}) {
		Sparse::Terms terms{{gap, 3}, {gap + 1, 5}, {2 * gap + 1, 7}};
		verify(terms, 3 * gap + 17, mint(11));
	}
	for (int stride : {1, 7, 31}) {
		Sparse::Terms terms{{11 * stride, 3}, {14 * stride, 5}, {21 * stride, 7}};
		for (int n : {22 * stride, 31 * stride + 5}) check<K::exponential>(terms, n, mint(13), 0);
	}
	for (int d : {1, 7, 257}) {
		Sparse::Terms terms{{d, 3}};
		for (int n : {d + 1, 32 * d, 32 * d + 1})
			for (long long e : {5LL, 998244352LL, 998244353LL, 9223372036854775807LL})
				check<K::power>(terms, n, mint(13), mint(e));
	}
	for (int far : {256, 257, 262, 263, 264, 265, 280, 513}) {
		Sparse::Terms terms{{1, 3}, {3, 7}, {7, 9}, {far, 5}, {far + 1, 11}, {far + 8, 13}};
		verify(terms, 2 * far + 17, mint(gen()));
	}
}
int main() {
	check_boundaries();
	for (int n : {255, 256, 257, 259, 4097}) for (int stride : {1, 2, 7, 64}) {
		Sparse::Terms terms{{stride, mint::word(mint(13).v + mint::mod())}};
		if (stride >= n) terms.clear();
		check<K::inverse>(terms, n, mint(7), 0);
		check<K::logarithm>(terms, n, 0, 0);
		check<K::exponential>(terms, n, 1, 0);
		check<K::power>(terms, n, mint(17), mint(11));
	}

	std::mt19937 gen(12345);
	for (int n : {1, 2, 7, 8, 9, 15, 16, 17, 64, 100, 257, 4097}) for (int rep = 0; rep < 30; ++rep) {
		Sparse::Terms terms;
		for (int i = 1; i < n && terms.size() < (rep % 3 == 0 ? 64U : 12U); ++i) if (gen() % 5 == 0) terms.emplace_back(i, mint(gen()));
		mint initial = gen();
		check<K::inverse>(terms, n, initial, 0);
		check<K::logarithm>(terms, n, 0, 0);
		check<K::exponential>(terms, n, 1, 0);
		check<K::power>(terms, n, initial, rep < 8 ? rep : gen());
	}
}
