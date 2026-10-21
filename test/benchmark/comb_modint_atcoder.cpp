#define COMB_MAX 200
#include "math/combinatorics/comb.hpp"
#include <atcoder/modint>
#include <cassert>
#include <cstdint>

template<class M> void check(int n = COMB_INF) {
	std::vector<unsigned> row(n + 1);
	row[0] = 1;
	std::uint64_t f = 1;
	for (int i = 0; i <= n; ++i) {
		if (i) f = f * i % M::mod();
		assert(Comb<M>::fact[i].val() == int(f));
		assert((Comb<M>::fact[i] * Comb<M>::finv[i]).val() == 1);
		for (int k = 0; k <= i; ++k) assert(comb<M>(i, k).val() == int(row[k]));
		for (int k = std::min(i + 1, n); k > 0; --k) row[k] = (std::uint64_t(row[k]) + row[k - 1]) % M::mod();
	}
	assert(Comb<M>::fact[-1] == M(0) && Comb<M>::finv[-1] == M(0));
}

int main() {
	check<atcoder::modint1000000007>();
	check<atcoder::modint998244353>();
	check<atcoder::static_modint<2>>(1);
	check<atcoder::static_modint<17>>(16);
	for (int p : {1000000007, 998244353, 17, 1000000007}) {
		atcoder::modint::set_mod(p);
		check<atcoder::modint>(std::min(COMB_INF, p - 1));
	}
}
