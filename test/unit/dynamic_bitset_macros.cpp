#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)
#define rep(i, n) for (int i = 0; i < (n); ++i)
#define vec std::vector
#include "data_structure/dynamic_bitset.hpp"

#if !defined(all) || !defined(rep) || !defined(vec)
#error "Library headers must preserve user macros"
#endif

int main() {
	vec<int> order{3, 1, 2};
	std::sort(all(order));
	assert((order == vec<int>{1, 2, 3}));
	DynamicBitset empty, bits(65, true);
	assert(empty.all() && bits.all());
	bits.reset(64);
	assert(!bits.all());
	const int n = 4;
	vec<DynamicBitset> d(n, DynamicBitset(n));
	rep(i, n) d[i][i] = true;
	rep(i, n - 1) d[i][i + 1] = true;
	rep(k, n) rep(i, n) if (d[i][k]) d[i] |= d[k];
	assert(d[0].all());
	assert(d[1].to_string() == "1110");
	assert(d[2].to_string() == "1100");
	assert(d[3].to_string() == "1000");
	d[0].rotate_left(1);
	assert(d[0].all());
}
