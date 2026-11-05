#include "../fixtures/user_template.hpp"

#undef all
#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)

#include "data_structure/dynamic_bitset.hpp"
#include <bitset>

struct AllMember {
	bool all() const { return true; }
};

void init() {}

void solve() {
	vec<int> values{3, 1, 1, 2};
	UNIQUE(values);
	assert((values == vec<int>{1, 2, 3}));
	std::reverse(all(values));
	assert((values == vec<int>{3, 2, 1}));
	const auto &constant = values;
	assert(std::count(all(constant), 2) == 1);
	int raw[]{3, 1, 2};
	std::sort(all(raw));
	assert(raw[0] == 1 && raw[1] == 2 && raw[2] == 3);
	DynamicBitset empty, bits(65, true);
	assert(empty.all() && bits.all());
	bits.reset(64);
	assert(!bits.all());
	const std::bitset<3> standard(7);
	assert(standard.all() && AllMember{}.all());
}
