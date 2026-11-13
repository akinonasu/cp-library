#define COMB_MAX 0
#include "math/comb.hpp"
#include "math/comb_light.hpp"
#include <cassert>

const unsigned before_main = comb(0, 0).val();
const unsigned before_main_inverse = inv[0].val();
const unsigned before_main_light_inverse = light_combinatorics::inv[0].val();

int main() {
	assert(before_main == 1 && before_main_inverse == 0);
	assert(before_main_light_inverse == 0 && light_combinatorics::inv[0] == mint(0));
	assert(light_combinatorics::inv[-1] == mint(0));
	assert(fact[0] == mint(1) && finv[0] == mint(1));
	assert(fact[-1] == mint(0) && finv[-1] == mint(0));
	assert(inv[0] == mint(0) && inv[-1] == mint(0));
	assert(comb(0, 1) == mint(0) && comb(-1, 0) == mint(0));
	assert(comb(0, 0) == mint(1));
}
