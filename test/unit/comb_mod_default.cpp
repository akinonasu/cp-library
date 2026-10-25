#define COMB_MAX 200
#include "math/combinatorics/comb_mod.hpp"
#include "math/mint.hpp"
#include <cassert>

const auto before_main = comb(10, 3);
int main() {
	assert(before_main == 120 && comb_mint::mod() == 1000000007);
	std::vector<comb_mint> row{1};
	comb_mint factorial = 1;
	for (int n = 0; n <= 200; ++n) {
		if (n) factorial *= n;
		assert(fact[n] == factorial && fact[n] * finv[n] == 1);
		for (int k = -1; k <= n + 1; ++k) assert(comb(n, k) == (k < 0 || k > n ? comb_mint(0) : row[k]));
		row.push_back(0);
		for (int k = n + 1; k; --k) row[k] += row[k - 1];
	}
	mint x = 998244352;
	assert((x + 1).val() == 0);
}
