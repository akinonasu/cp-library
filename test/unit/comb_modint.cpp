#define COMB_MAX 200
#include "math/combinatorics/comb.hpp"
#include "test/fixtures/comb_modint_types.hpp"
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

struct Opaque {
	comb_test::Field<17, 77> value;
	static unsigned mod() { return 17; }
	Opaque(long long x = 0) : value(x) {}
	Opaque operator*(Opaque other) const { Opaque result; result.value = value * other.value; return result; }
	Opaque inv() { Opaque result; result.value = value.inv(); return result; }
};

struct Encoded : comb_test::Field<1000000007, 78> {
	using Base = comb_test::Field<1000000007, 78>;
	using Base::Base;
	Encoded() = default;
	static Encoded raw(unsigned value) { return Encoded(value ^ 0xa5a5a5a5U); }
	Encoded operator*(Encoded other) const { return Encoded(std::uint64_t(val()) * other.val() % mod()); }
	Encoded inv() { return Encoded(Base::inv().val()); }
};

struct Centered : comb_test::Field<17, 79> {
	using Base = comb_test::Field<17, 79>;
	using Base::Base;
	Centered() = default;
	long long val() const { auto x = Base::val(); return x > 8 ? static_cast<long long>(x) - 17 : x; }
	Centered operator*(Centered other) const { return Centered(Base::val() * other.Base::val()); }
	Centered inv() { return Centered(Base::inv().val()); }
};

struct Wide {
	std::uint64_t value = 0;
	static constexpr std::uint64_t mod() { return (std::uint64_t(1) << 61) - 1; }
	Wide(long long x = 0) { auto r = x % static_cast<long long>(mod()); value = r < 0 ? r + mod() : r; }
	std::uint64_t val() const { return value; }
	Wide operator*(Wide other) const { return Wide(__uint128_t(value) * other.value % mod()); }
	Wide inv() {
		Wide result(1), a = *this;
		for (auto e = mod() - 2; e; e >>= 1, a = a * a) if (e & 1) result = result * a;
		return result;
	}
};
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
	check<Encoded>();
	assert(comb<Opaque>(10, 3).value.val() == 120 % 17);
	assert(Comb<Opaque>::fact[-1].value.val() == 0 && Comb<Opaque>::finv[-1].value.val() == 0);
	for (int i = 0; i <= 16; ++i) assert((Comb<Centered>::fact[i] * Comb<Centered>::finv[i]).val() == 1);
	std::vector<std::uint64_t> row(COMB_INF + 1);
	row[0] = 1;
	std::uint64_t expected = 1;
	for (int n = 0; n <= COMB_INF; ++n) {
		if (n) expected = __uint128_t(expected) * n % Wide::mod();
		assert(Comb<Wide>::fact[n].val() == expected && (Comb<Wide>::fact[n] * Comb<Wide>::finv[n]).val() == 1);
		for (int k = 0; k <= n; ++k) assert(comb<Wide>(n, k).val() == row[k]);
		for (int k = std::min(n + 1, COMB_INF); k; --k) row[k] = (row[k] + row[k - 1]) % Wide::mod();
	}
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
