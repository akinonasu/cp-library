#define COMB_MAX 200
#include "math/combinatorics/comb_mod.hpp"
#include "math/combinatorics/comb.hpp"
#include "math/modint.hpp"
#include <cassert>

template <class M> void check() {
	auto [comb, fact, finv] = make_comb<M>(200);
	std::vector<M> row{1};
	M factorial = 1;
	for (int n = 0; n <= 200; ++n) {
		if (n) factorial *= n;
		assert(fact[n] == factorial && fact[n] * finv[n] == M(1));
		for (int k = -1; k <= n + 1; ++k) assert(comb(n, k) == (k < 0 || k > n ? M(0) : row[k]));
		row.push_back(0);
		for (int k = n + 1; k; --k) row[k] += row[k - 1];
	}
}
const auto before_main = make_comb<StaticModint<1000000007>>(200).comb(10, 3);
int main() {
	assert(before_main == 120);
	check<StaticModint<998244353>>();
	check<StaticModint<1000000007>>();
	check<mint>();
	DynamicModint<37>::set_mod(1000000007);
	check<DynamicModint<37>>();
	assert(::comb(10, 3) == mint(120));
	assert(::fact[5] * ::finv[5] == mint(1));
	auto lazy = make_comb<StaticModint<17>>();
	assert(lazy.fact.state->max_n == 10000000);
}
