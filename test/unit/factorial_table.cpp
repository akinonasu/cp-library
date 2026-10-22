#include "components/factorial_table.hpp"
#include "test/fixtures/comb_modint_types.hpp"
#include <cassert>
#include <cstdint>

template<class M> void check(int max_n) {
	factorial_detail::Table<M> table(max_n);
	assert(table.modulus == M::mod());
	int n = std::min(unsigned(max_n), M::mod() - 1);
	assert(table.factorial.size() == std::size_t(n + 1));
	assert(table.inverse_factorial.size() == table.factorial.size());
	std::uint64_t expected = 1;
	for (int i = 0; i <= n; ++i) {
		if (i) expected = expected * i % M::mod();
		assert(table.factorial[i].val() == expected);
		assert((table.factorial[i] * table.inverse_factorial[i]).val() == 1);
	}
}

int main() {
	for (int n : {0, 1, 2, 15, 16, 17, 254, 255, 256, 257, 65536}) {
		check<mint>(n);
		check<comb_test::Field<1000000007>>(n);
		check<comb_test::Field<998244353>>(n);
		check<comb_test::Field<17>>(n);
	}
}
