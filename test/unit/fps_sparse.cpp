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
int main() {
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
