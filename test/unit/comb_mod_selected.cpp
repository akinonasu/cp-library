#include "math/combinatorics/comb_mod.hpp"
#include "../fixtures/comb_modint_types.hpp"
#include <cassert>
#include <type_traits>

struct Plain {
	unsigned value;
	Plain(long long x = 0) : value(unsigned((x % 17 + 17) % 17)) {}
	static unsigned mod() { return 17; }
	Plain operator*(Plain x) const { return Plain(value * x.value); }
	Plain inv() { return Plain(comb_test::Field<17>(value).inv().val()); }
};

int main() {
	using M = comb_test::Field<17>;
	auto [comb, fact, finv] = make_comb<M>(100);
	assert(M::inversions == 0);
	assert(fact[-1].val() == 0 && finv[-1].val() == 0 && comb(-1, 0).val() == 0);
	assert(M::inversions == 0);
	std::vector<unsigned> row{1};
	for (int n = 0; n < 17; ++n) {
		assert((fact[n] * finv[n]).val() == 1);
		for (int k = -1; k <= n + 1; ++k) assert(comb(n, k).val() == (k < 0 || k > n ? 0 : row[k]));
		row.push_back(0);
		for (int k = n + 1; k; --k) row[k] = (row[k] + row[k - 1]) % 17;
	}
	assert(M::inversions == 1);
	static_assert(std::is_same_v<decltype(comb(1, 1)), M>);
	auto copied = comb;
	assert(copied(16, 8).val() == comb(16, 8).val() && M::inversions == 1);
	using D = comb_test::DynamicField<7>;
	D::set_mod(17);
	auto f = make_comb<D>(50);
	assert(f.comb(16, 8).val() == 1 && D::inversions == 1);
	D::set_mod(23);
	assert(f.comb(10, 3).val() == 5 && D::inversions == 2);
	assert((f.fact[22] * f.finv[22]).val() == 1 && D::inversions == 2);
	D::set_mod(17);
	assert(f.comb(16, 8).val() == 1 && D::inversions == 3);
	auto only_fact = make_comb<M>(5).fact;
	assert(only_fact[5].val() == 1);
	auto zero = make_comb<M>(0);
	assert(zero.comb(0, 0).val() == 1 && zero.fact[0].val() == 1 && zero.finv[0].val() == 1);
	auto plain = make_comb<Plain>(16);
	assert(plain.comb(10, 3).value == 1);
	assert((plain.fact[16] * plain.finv[16]).value == 1);
}
