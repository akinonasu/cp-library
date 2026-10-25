#define COMB_MAX 200
#include "math/comb.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

using namespace std;

const unsigned before_main = comb(10, 3).val();
const unsigned before_main_factorial = fact[5].val();

int main() {
	assert(before_main == 120 && before_main_factorial == 120);
	mint row[201]{};
	row[0] = 1;
	for (int n = 0; n < 201; ++n) {
		for (int k = 0; k < (n + 1); ++k) assert(comb(n, k) == row[k]);
		assert(fact[n] * finv[n] == mint(1));
		assert(Comb<mint>::fact[n] == fact[n] && Comb<mint>::finv[n] == finv[n]);
		assert(comb<mint>(n, n / 2) == comb(n, n / 2));
		assert(comb(n, -1) == mint(0) && comb(n, n + 1) == mint(0));
		for (int k = (min(n + 2, 201)) - 1; k >= 1; --k) row[k] += row[k - 1];
	}
	assert(comb(-1, 0) == mint(0));
	assert(fact[-1] == mint(0) && finv[-1] == mint(0));
	assert(Comb<mint>::fact[-1] == mint(0) && Comb<mint>::finv[-1] == mint(0));
}
