#define COMB_MAX 0
#include "math/combinatorics/comb.hpp"
#include <cassert>

const unsigned before_main = comb(0, 0).val();

int main() {
	assert(before_main == 1);
	assert(fact[0] == mint(1) && finv[0] == mint(1));
	assert(comb(0, 1) == mint(0) && comb(-1, 0) == mint(0));
	assert(comb(0, 0) == mint(1));
}
