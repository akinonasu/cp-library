#define COMB_MOD 17
#define COMB_MAX 100
#include "math/combinatorics/comb_mod.hpp"
#include <cassert>

int main() {
	assert(fact[-1] == 0 && finv[-1] == 0 && comb(-1, 0) == 0);
	std::vector<unsigned> row{1};
	for (int n = 0; n < 17; ++n) {
		assert(fact[n] * finv[n] == 1);
		for (int k = -1; k <= n + 1; ++k) assert(comb(n, k).val() == (k < 0 || k > n ? 0 : row[k]));
		row.push_back(0);
		for (int k = n + 1; k; --k) row[k] = (row[k] + row[k - 1]) % 17;
	}
	assert(comb_mod_detail::table().fact.size() == 17);
	static_assert(std::is_same_v<decltype(comb(1, 1)), StaticModint<17>>);
}
