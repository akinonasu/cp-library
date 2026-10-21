#define COMB_MAX 200
#include "math/combinatorics/comb.hpp"
#include "test/benchmark/comb_modint_types.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

template<class M> void check() {
	int limit = std::min(unsigned(COMB_INF), M::mod() - 1);
	std::vector<unsigned> row(limit + 1);
	row[0] = 1;
	std::uint64_t factorial = 1;
	for (int n = 0; n <= limit; ++n) {
		if (n) factorial = factorial * n % M::mod();
		assert(Comb<M>::fact[n].val() == factorial);
		assert((Comb<M>::fact[n] * Comb<M>::finv[n]).val() == 1);
		for (int k = 0; k <= n; ++k) assert(comb<M>(n, k).val() == row[k]);
		assert(comb<M>(n, -1).val() == 0 && comb<M>(n, n + 1).val() == 0);
		for (int k = std::min(n + 1, limit); k > 0; --k) row[k] = (std::uint64_t(row[k]) + row[k - 1]) % M::mod();
	}
	assert(Comb<M>::fact[-1].val() == 0 && Comb<M>::finv[-1].val() == 0);
	assert(Comb<M>::fact[std::numeric_limits<int>::min()].val() == 0);
	assert(Comb<M>::finv[std::numeric_limits<int>::min()].val() == 0);
	assert(comb<M>(-1, 0).val() == 0);
}

using First = comb_test::Field<1000000007>;
using Second = comb_test::Field<1000000007, 1>;
using Third = comb_test::Field<998244353>;
using Dynamic = comb_test::DynamicField<>;
static_assert(std::is_same_v<decltype(Comb<First>::fact[0]), First>);
static_assert(std::is_same_v<decltype(Comb<First>::finv[0]), First>);
static_assert(std::is_same_v<decltype(comb<First>(0, 0)), First>);
static_assert(std::is_same_v<decltype(fact[0]), mint>);
static_assert(!std::is_lvalue_reference_v<decltype((fact[0]))>);
const unsigned before_main = Comb<Third>::fact[5].val();

int main() {
	assert(before_main == 120 && Third::inversions == 1);
	assert(First::inversions == 0 && Second::inversions == 0);
	const auto &accessor = Comb<First>::fact;
	(void)accessor;
	assert(First::inversions == 0);
	assert(Comb<First>::fact[-1].val() == 0 && Comb<First>::finv[-1].val() == 0);
	assert(comb<First>(-1, 0).val() == 0 && comb<First>(0, -1).val() == 0);
	assert(comb<First>(1, 2).val() == 0 && First::inversions == 0);
	assert((Comb<First>::finv[5] * First(120)).val() == 1 && First::inversions == 1);
	check<First>();
	assert(First::inversions == 1 && Second::inversions == 0);
	assert(comb<Second>(10, 3).val() == 120 && Second::inversions == 1);
	assert(Comb<Second>::comb(10, 3).val() == 120 && Second::inversions == 1);
	check<Second>();
	check<Third>();
	assert(First::inversions == 1 && Second::inversions == 1 && Third::inversions == 1);
	check<comb_test::Field<2>>();
	check<comb_test::Field<3>>();
	check<comb_test::Field<17>>();
	for (unsigned p : {1000000007U, 17U, 998244353U, 1000000007U}) {
		int previous = Dynamic::inversions;
		Dynamic::set_mod(p);
		check<Dynamic>();
		assert(Dynamic::inversions == previous + 1);
		check<Dynamic>();
		assert(Dynamic::inversions == previous + 1);
	}
	using Unused = comb_test::Field<1000000007, 99>;
	assert(Unused::inversions == 0);
}
